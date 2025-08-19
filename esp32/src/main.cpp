#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// WiFi credentials - UPDATE THESE WITH YOUR NETWORK
const char* ssid = "YOUR_WIFI_SSID";       // Replace with your WiFi network name
const char* password = "YOUR_WIFI_PASSWORD";   // Replace with your WiFi password

// MQTT broker settings - UPDATE THESE WITH YOUR BROKER
const char* mqtt_server = "192.168.1.100";    // Replace with your MQTT broker IP address
const int mqtt_port = 1883;                         // Default MQTT port (usually 1883)
const char* mqtt_user = "";                         // MQTT username (leave empty "" if no authentication)
const char* mqtt_pass = "";                         // MQTT password (leave empty "" if no authentication)  
const char* mqtt_topic = "sensors/mpu6500/accelerometer";  // MQTT topic to publish to

WiFiClient espClient;
PubSubClient client(espClient);

Adafruit_MPU6050 mpu6050;

// ESP32 default I2C pins
#define SDA_PIN 21
#define SCL_PIN 22

bool isMPU6050 = false;
bool isMPU6500 = false;

// MPU6500 register addresses
#define MPU6500_ADDR 0x68
#define PWR_MGMT_1   0x6B
#define ACCEL_XOUT_H 0x3B
#define GYRO_XOUT_H  0x43

// Function to read MPU6500 registers
int16_t readMPU6500Register16(uint8_t reg) {
  Wire.beginTransmission(MPU6500_ADDR);
  Wire.write(reg);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU6500_ADDR, 2);
  
  if (Wire.available() == 2) {
    int16_t value = Wire.read() << 8 | Wire.read();
    return value;
  }
  return 0;
}

// WiFi connection function
void setupWiFi() {
  // Check if WiFi credentials are configured
  if (strlen(ssid) == 0 || strcmp(ssid, "YOUR_WIFI_SSID") == 0) {
    Serial.println("ERROR: WiFi SSID not configured!");
    Serial.println("Please update the 'ssid' variable in main.cpp");
    return;
  }
  
  if (strlen(password) == 0 || strcmp(password, "YOUR_WIFI_PASSWORD") == 0) {
    Serial.println("ERROR: WiFi password not configured!");
    Serial.println("Please update the 'password' variable in main.cpp");
    return;
  }
  
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("");
    Serial.println("Failed to connect to WiFi");
    Serial.println("Check your WiFi credentials and network availability");
  }
}

// MQTT reconnection function
void reconnectMQTT() {
  // Check if MQTT server is configured
  if (strlen(mqtt_server) == 0 || strcmp(mqtt_server, "192.168.1.100") == 0) {
    Serial.println("ERROR: MQTT server not configured!");
    Serial.println("Please update the 'mqtt_server' variable in main.cpp");
    return;
  }
  
  // Only try to connect if WiFi is connected
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected, skipping MQTT connection");
    return;
  }
  
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    
    // Attempt to connect
    bool connected;
    if (strlen(mqtt_user) > 0) {
      connected = client.connect(clientId.c_str(), mqtt_user, mqtt_pass);
    } else {
      connected = client.connect(clientId.c_str());
    }
    
    if (connected) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial)
    delay(10);

  Serial.println("IMU sensor test");
  
  // Initialise I2C with explicit pins
  Wire.begin(SDA_PIN, SCL_PIN);
  Serial.printf("I2C initialized on SDA=%d, SCL=%d\n", SDA_PIN, SCL_PIN);
  
  // Scan for I2C devices
  Serial.println("Scanning for I2C devices...");
  byte count = 0;
  for (byte address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    if (Wire.endTransmission() == 0) {
      Serial.printf("I2C device found at address 0x%02X\n", address);
      count++;
    }
  }
  Serial.printf("Found %d I2C device(s)\n", count);

  // Read WHO_AM_I register to identify the sensor
  Wire.beginTransmission(0x68);
  Wire.write(0x75); // WHO_AM_I register
  Wire.endTransmission(false);
  Wire.requestFrom(0x68, 1);
  
  if (Wire.available()) {
    uint8_t whoami = Wire.read();
    Serial.printf("WHO_AM_I register: 0x%02X\n", whoami);
    
    if (whoami == 0x68) {
      Serial.println("Device identified as MPU6050!");
      isMPU6050 = true;
    } else if (whoami == 0x70) {
      Serial.println("Device identified as MPU6500!");
      isMPU6500 = true;
    } else if (whoami == 0x71) {
      Serial.println("Device identified as MPU9250!");
      isMPU6500 = true; // Use same library for MPU6500/9250
    } else {
      Serial.printf("Unknown device with WHO_AM_I: 0x%02X\n", whoami);
    }
  } else {
    Serial.println("Failed to read WHO_AM_I register");
  }

  // Initialise the appropriate sensor
  if (isMPU6050) {
    // Wake up the MPU6050 first
    Serial.println("Initializing MPU6050...");
    Wire.beginTransmission(0x68);
    Wire.write(0x6B); // PWR_MGMT_1 register
    Wire.write(0);    // Wake up the MPU6050
    Wire.endTransmission(true);
    delay(100);

    if (!mpu6050.begin(0x68, &Wire)) {
      Serial.println("Failed to initialize MPU6050");
      while (1) delay(10);
    }
    Serial.println("MPU6050 initialized successfully!");
    mpu6050.setAccelerometerRange(MPU6050_RANGE_8_G);
    mpu6050.setGyroRange(MPU6050_RANGE_500_DEG);
    mpu6050.setFilterBandwidth(MPU6050_BAND_21_HZ);
    
  } else if (isMPU6500) {
    Serial.println("Initializing MPU6500 with raw I2C...");
    
    // Wake up the MPU6500
    Wire.beginTransmission(MPU6500_ADDR);
    Wire.write(PWR_MGMT_1);
    Wire.write(0x00); // Wake up device
    Wire.endTransmission(true);
    delay(100);
    
    // Configure accelerometer (±8g)
    Wire.beginTransmission(MPU6500_ADDR);
    Wire.write(0x1C); // ACCEL_CONFIG register
    Wire.write(0x10); // ±8g range
    Wire.endTransmission(true);
    
    // Configure gyroscope (±500 deg/s)
    Wire.beginTransmission(MPU6500_ADDR);
    Wire.write(0x1B); // GYRO_CONFIG register
    Wire.write(0x08); // ±500 deg/s range
    Wire.endTransmission(true);
    
    Serial.println("MPU6500 initialised successfully!");
  } else {
    Serial.println("No supported IMU found!");
    while (1) delay(10);
  }

  // Setup WiFi and MQTT
  setupWiFi();
  client.setServer(mqtt_server, mqtt_port);
  
  Serial.println("Setup complete!");
  delay(100);
}

void loop() {
  // Check WiFi connection
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi disconnected, reconnecting...");
    setupWiFi();
  }
  
  // Check MQTT connection only if WiFi is connected
  if (WiFi.status() == WL_CONNECTED && !client.connected()) {
    reconnectMQTT();
  }
  
  if (client.connected()) {
    client.loop();
  }
  
  float ax, ay, az;
  
  if (isMPU6050) {
    // Read MPU6050
    sensors_event_t a, g, temp;
    mpu6050.getEvent(&a, &g, &temp);

    ax = a.acceleration.x;
    ay = a.acceleration.y;
    az = a.acceleration.z;
    
    Serial.print("Accel X: "); Serial.print(ax);
    Serial.print(", Y: "); Serial.print(ay);
    Serial.print(", Z: "); Serial.print(az);
    Serial.println(" m/s²");
    
  } else if (isMPU6500) {
    // Read MPU6500 raw values
    int16_t accelX = readMPU6500Register16(ACCEL_XOUT_H);
    int16_t accelY = readMPU6500Register16(ACCEL_XOUT_H + 2);
    int16_t accelZ = readMPU6500Register16(ACCEL_XOUT_H + 4);
    
    // Convert to g (±8g range: 4096 LSB/g)
    ax = accelX / 4096.0;
    ay = accelY / 4096.0;
    az = accelZ / 4096.0;
    
    Serial.print("Accel X: "); Serial.print(ax);
    Serial.print(", Y: "); Serial.print(ay);
    Serial.print(", Z: "); Serial.print(az);
    Serial.println(" g");
  } else {
    Serial.println("No supported IMU detected, continuing without sensor data...");
  }
  
  // Only publish MQTT if connected
  if (client.connected()) {
    // Create JSON payload for MQTT
    DynamicJsonDocument doc(200);
    doc["timestamp"] = millis();
    doc["device_id"] = "ESP32-MPU6500";
    doc["sensor_type"] = isMPU6050 ? "MPU6050" : (isMPU6500 ? "MPU6500" : "None");
    
    JsonObject accel = doc.createNestedObject("accelerometer");
    accel["x"] = ax;
    accel["y"] = ay;
    accel["z"] = az;
    accel["unit"] = isMPU6050 ? "m/s²" : "g";
    
    // Serialize JSON to string
    String payload;
    serializeJson(doc, payload);
    
    // Publish to MQTT
    if (client.publish(mqtt_topic, payload.c_str())) {
      Serial.println("MQTT message sent: " + payload);
    } else {
      Serial.println("Failed to send MQTT message");
    }
  } else {
    Serial.println("MQTT not connected, skipping data publish");
  }

  delay(1000);
}
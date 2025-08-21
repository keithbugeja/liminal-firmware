#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#include <memory>

#include "config/config.h"
#include "communication/wifi_manager.h"
#include "communication/mqtt_client.h"
#include "sensors/sensor_manager.h"
#include "sensors/imu_sensor.h"
#include "devices/device_manager.h"
#include "devices/led_device.h"
#include "utils/json_helper.h"

WiFiManager wifiManager;
MQTTClient mqttClient;
SensorManager sensorManager;
DeviceManager deviceManager;

unsigned long lastSensorPublish = 0;
unsigned long lastStatusReport = 0;

void onMQTTMessage(const String& topic, const String& payload);
void publishSensorData();
void publishStatusReport();
void setupSensors();
void setupDevices();

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  while (!Serial) delay(10);

  Serial.println("=== Liminal ESP32 Firmware Starting ===");
  Serial.printf("Device ID: %s\n", DEVICE_ID);
  Serial.printf("Firmware Version: %s\n", FIRMWARE_VERSION);
  
  // Test basic WiFi connection (bypass WiFiManager)
  Serial.println("Testing basic WiFi connection...");
  WiFi.mode(WIFI_STA);
  delay(1000);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  Serial.print("Connecting to WiFi");
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 120) { 
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.println("WiFi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println();
    Serial.println("WiFi connection failed!");
    Serial.printf("Status: %d\n", WiFi.status());
  }
  
  // Initialise WiFi Manager after basic test
  if (!wifiManager.begin()) {
    Serial.println("Failed to initialize WiFi manager");
  }
  
  // Initialise MQTT
  if (!mqttClient.begin()) {
    Serial.println("Failed to initialize MQTT client");
  }
  mqttClient.setCallback(onMQTTMessage);
  
  // Setup sensors and devices
  setupSensors();
  setupDevices();
  
  // Initialise sensor and device managers
  if (!sensorManager.begin()) {
    Serial.println("Warning: Some sensors failed to initialize");
  }
  
  if (!deviceManager.begin()) {
    Serial.println("Warning: Some devices failed to initialize");
  }
  
  Serial.println("=== Setup Complete ===");
  Serial.println();
}

void loop() {
  // Handle WiFi connection (avoid rapid reconnection attempts)
  static unsigned long lastWiFiAttempt = 0;
  if (!wifiManager.isConnected()) {
    if (millis() - lastWiFiAttempt > 30000) {
      Serial.println("WiFi disconnected, attempting reconnection...");
      wifiManager.connect();
      lastWiFiAttempt = millis();
    }
  }
  
  // Handle MQTT connection
  if (wifiManager.isConnected() && !mqttClient.isConnected()) {
    mqttClient.connect();
  }
  
  // Process MQTT messages
  if (mqttClient.isConnected()) {
    mqttClient.loop();
  }
  
  // Update sensors and devices
  sensorManager.update();
  deviceManager.update();
  
  // Publish sensor data periodically
  unsigned long now = millis();
  if (now - lastSensorPublish >= SENSOR_READ_INTERVAL_MS) {
    publishSensorData();
    lastSensorPublish = now;
  }
  
  // Publish status report periodically
  if (now - lastStatusReport >= STATUS_REPORT_INTERVAL_MS) {
    publishStatusReport();
    lastStatusReport = now;
  }
  
  delay(50); // Small delay to prevent excessive CPU usage
}

void onMQTTMessage(const String& topic, const String& payload) {
  Serial.printf("MQTT message received - Topic: %s, Payload: %s\n", topic.c_str(), payload.c_str());
  
  // Handle device commands
  if (topic.startsWith(String(MQTT_TOPIC_COMMANDS))) {
    if (deviceManager.handleCommand(topic, payload)) {
      Serial.println("Device command executed successfully");
    } else {
      Serial.println("Failed to execute device command");
    }
  }
  
  // Handle system commands (future expansion)
  // Could add commands like restart, status request, config updates, etc.
}

void publishSensorData() {
  if (!mqttClient.isConnected()) {
    return;
  }
  
  // Get all sensor data and publish individually
  for (auto it = sensorManager.sensors_begin(); it != sensorManager.sensors_end(); ++it) {
    auto& sensor = *it;
    if (sensor->isReady()) {
      DynamicJsonDocument data = sensor->getDataAsJson();
      if (!mqttClient.publishSensorData(sensor->getTypeString(), data)) {
        Serial.printf("Failed to publish data for sensor: %s\n", sensor->getName().c_str());
      }
    }
  }
}

void publishStatusReport() {
  if (!mqttClient.isConnected()) {
    return;
  }
  
  // Create comprehensive status report
  DynamicJsonDocument statusDoc(2048);
  statusDoc["device_id"] = DEVICE_ID;
  statusDoc["firmware_version"] = FIRMWARE_VERSION;
  statusDoc["uptime"] = millis();
  statusDoc["timestamp"] = millis();
  
  // WiFi status
  JsonObject wifi = statusDoc.createNestedObject("wifi");
  wifi["connected"] = wifiManager.isConnected();
  wifi["ip"] = wifiManager.getLocalIP();
  wifi["rssi"] = wifiManager.getSignalStrength();
  
  // MQTT status
  JsonObject mqtt = statusDoc.createNestedObject("mqtt");
  mqtt["connected"] = mqttClient.isConnected();
  mqtt["client_id"] = mqttClient.getClientId();
  
  // Memory status
  JsonObject memory = statusDoc.createNestedObject("memory");
  memory["free_heap"] = ESP.getFreeHeap();
  memory["total_heap"] = ESP.getHeapSize();
  
  // Sensor and device status
  DynamicJsonDocument sensors = sensorManager.getStatusReport();
  statusDoc["sensors"] = sensors;
  
  DynamicJsonDocument devices = deviceManager.getStatusReport();
  statusDoc["devices"] = devices;
  
  mqttClient.publishStatus(statusDoc);
}

void setupSensors() {
  Serial.println("Setting up sensors...");
  
  // Add IMU sensor
  auto imuSensor = std::make_shared<IMUSensor>("main_imu");
  if (sensorManager.addSensor(imuSensor)) {
    Serial.println("IMU sensor added to sensor manager");
  } else {
    Serial.println("Failed to add IMU sensor to sensor manager");
  }
  
  // Future sensors can be added here:
  // e.g.: auto tempSensor = std::make_shared<TemperatureSensor>("temp_sensor");
  // sensorManager.addSensor(tempSensor);
}

void setupDevices() {
  Serial.println("Setting up devices...");
  
  // Add status LED (built-in LED)
  auto statusLED = std::make_shared<LEDDevice>("status_led", STATUS_LED_PIN, false);
  if (deviceManager.addDevice(statusLED)) {
    Serial.println("Status LED added to device manager");
  } else {
    Serial.println("Failed to add status LED to device manager");
  }
  
  // Future devices can be added here:
  // e.g.: auto relay = std::make_shared<RelayDevice>("main_relay", RELAY_PIN);
  // deviceManager.addDevice(relay);
}
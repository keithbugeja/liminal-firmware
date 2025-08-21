#ifndef CONFIG_H
#define CONFIG_H

// WiFi Configuration
#define WIFI_SSID "YareYareNi"
#define WIFI_PASSWORD "B3rs3rkD4rks@ulsS3k1r@"
#define WIFI_TIMEOUT_MS 30000

// MQTT broker settings - UPDATE THESE WITH YOUR BROKER
#define MQTT_SERVER "192.168.50.87"
#define MQTT_PORT 1883
#define MQTT_USER ""
#define MQTT_PASSWORD ""
#define MQTT_CLIENT_ID_PREFIX "liminal-esp32-"
#define MQTT_TIMEOUT_MS 5000

// Device Configuration
#define DEVICE_ID "esp32-001"
#define FIRMWARE_VERSION "1.0.0"

// MQTT Topics
#define MQTT_TOPIC_BASE "liminal"
#define MQTT_TOPIC_SENSORS MQTT_TOPIC_BASE "/sensors/" DEVICE_ID
#define MQTT_TOPIC_COMMANDS MQTT_TOPIC_BASE "/commands/" DEVICE_ID
#define MQTT_TOPIC_STATUS MQTT_TOPIC_BASE "/status/" DEVICE_ID

// Pin Definitions
#define I2C_SDA_PIN 21
#define I2C_SCL_PIN 22
#define LED_BUILTIN_PIN 2
#define STATUS_LED_PIN LED_BUILTIN_PIN

// Sensor Configuration
#define SENSOR_READ_INTERVAL_MS 1000
#define STATUS_REPORT_INTERVAL_MS 30000

// I2C Addresses
#define MPU6050_ADDR 0x68
#define MPU6500_ADDR 0x68

// Serial Configuration
#define SERIAL_BAUD_RATE 115200

#endif // CONFIG_H

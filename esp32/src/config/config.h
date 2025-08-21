#ifndef CONFIG_H
#define CONFIG_H

//=============================================================================
// LIMINAL ESP32 FIRMWARE CONFIGURATION
//=============================================================================
// 
// SECURITY NOTICE: This file may contain sensitive information.
// 
// For development setup:
// 1. Copy config.h.template to config.h
// 2. Edit the placeholder values below with your actual credentials
// 3. This file is ignored by git to protect your credentials
//
// For production/CI builds, use environment variables instead of editing this file.
//
//=============================================================================

// WiFi Configuration
// Use environment variables if available, otherwise use fallback values
#ifdef WIFI_SSID_ENV
    #define WIFI_SSID WIFI_SSID_ENV
#else
    #define WIFI_SSID "YOUR_WIFI_SSID"        // Replace with your WiFi network name
#endif

#ifdef WIFI_PASSWORD_ENV
    #define WIFI_PASSWORD WIFI_PASSWORD_ENV
#else
    #define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"    // Replace with your WiFi password  
#endif

#define WIFI_TIMEOUT_MS 30000

// MQTT broker settings
// Use environment variables if available, otherwise use fallback values
#ifdef MQTT_SERVER_ENV
    #define MQTT_SERVER MQTT_SERVER_ENV
#else
    #define MQTT_SERVER "192.168.1.100"       // Replace with your MQTT broker IP address
#endif

#define MQTT_PORT 1883

#ifdef MQTT_USER_ENV
    #define MQTT_USER MQTT_USER_ENV
#else
    #define MQTT_USER ""                      // Replace with your MQTT username (leave empty if not required)
#endif

#ifdef MQTT_PASSWORD_ENV
    #define MQTT_PASSWORD MQTT_PASSWORD_ENV
#else
    #define MQTT_PASSWORD ""                  // Replace with your MQTT password (leave empty if not required)
#endif

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

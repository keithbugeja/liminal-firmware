#!/bin/bash
#
# Example build script showing how to use environment variables
# for secure configuration in CI/CD pipelines
#

# Set your configuration via environment variables
export WIFI_SSID_ENV="YourWiFiNetwork"
export WIFI_PASSWORD_ENV="YourWiFiPassword"
export MQTT_SERVER_ENV="192.168.1.100"
export MQTT_USER_ENV="mqtt_user"
export MQTT_PASSWORD_ENV="mqtt_password"

# Build with PlatformIO
cd esp32
pio run --environment esp32dev

echo "Build completed with secure environment variable configuration"
# Liminal Firmware

This repository contains firmware for various sensors and microcontrollers (MCUs) that provide data to the **Liminal** data transformation and fusion engine. Liminal is a Rust-based pipeline system designed to process, transform, and aggregate sensor data for a property monitoring system written in Erlang.

## Overview

Liminal operates on a stage-based architecture with four core unit types:

- **Input Stages**: Source data from various inputs (synthetic data generation, MQTT subscriptions, sensor readings)
- **Transform Stages**: Process and filter incoming data streams
- **Aggregate Stages**: Combine multiple data sources (e.g., depth and color camera buffers)
- **Output Stages**: Route processed data to various sinks (file logs, console output, MQTT publishers, network sockets)

This firmware repository provides the sensor-side implementation that feeds real-world data into Liminal's input stages, ultimately delivering processed information to an Erlang-based property monitoring system.

## Architecture

```
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│   MCU Firmware  │───▶│  Liminal Engine │───▶│ Property Monitor│
│   (This Repo)   │    │   (Rust-based)  │    │   (Erlang)      │
└─────────────────┘    └─────────────────┘    └─────────────────┘
       │                       │                       │
   Sensor Data            Data Pipeline           Processed Info
   Collection            Transformation            & Monitoring
```

## Supported Hardware

### ESP32 Platform

Located in the `esp32/` directory, this implementation supports:

- **MCU**: ESP32 DevKit v1 (DOIT ESP32 Development Board)
- **Connectivity**: WiFi + MQTT
- **Sensors**:
  - MPU6050 (6-axis accelerometer/gyroscope)
  - MPU6500 (6-axis accelerometer/gyroscope)
  - MPU9250 (9-axis accelerometer/gyroscope/magnetometer)

#### Pin Configuration
- **I2C SDA**: GPIO 21
- **I2C SCL**: GPIO 22
- **Default Sensor Address**: 0x68

## Getting Started

> **🔒 Security Note**: This firmware requires WiFi and MQTT credentials. The configuration system supports both direct editing for development and environment variables for production/CI to keep sensitive data secure.

### Prerequisites

- [PlatformIO](https://platformio.org/) installed
- ESP32 development board
- Supported IMU sensor (MPU6050/6500/9250)
- WiFi network access
- MQTT broker

### Hardware Setup

1. Connect your IMU sensor to the ESP32:
   ```
   IMU VCC  → ESP32 3.3V
   IMU GND  → ESP32 GND
   IMU SDA  → ESP32 GPIO 21
   IMU SCL  → ESP32 GPIO 22
   ```

2. Refer to the ESP32 pinout diagram in `esp32/docs/ESP32-DOIT-DEVKIT-V1-Board-Pinout-36-GPIOs-updated.jpg`

### Software Setup

1. **Clone the repository**:
   ```bash
   git clone <repository-url>
   cd liminal-firmware
   ```

2. **Configure WiFi and MQTT settings**:
   
   **Option A: Local Development Setup**
   ```bash
   # For team development or clean setup:
   cp src/config/config.h.template src/config/config.h
   # Edit the configuration file directly
   # Replace placeholder values in esp32/src/config/config.h:
   # - WIFI_SSID: Replace "YOUR_WIFI_SSID" with your WiFi network name
   # - WIFI_PASSWORD: Replace "YOUR_WIFI_PASSWORD" with your WiFi password  
   # - MQTT_SERVER: Replace "192.168.1.100" with your MQTT broker IP
   # - MQTT_USER: Add your MQTT username (optional)
   # - MQTT_PASSWORD: Add your MQTT password (optional)
   # Note: config.h is gitignored to protect your credentials
   ```
   
   **Option B: Environment Variables (CI/Production)**
   ```bash
   # Set environment variables before building
   export WIFI_SSID_ENV="YourWiFiNetwork"
   export WIFI_PASSWORD_ENV="YourWiFiPassword"
   export MQTT_SERVER_ENV="your.mqtt.broker.ip"
   export MQTT_USER_ENV="mqtt_username"        # Optional
   export MQTT_PASSWORD_ENV="mqtt_password"    # Optional
   ```

3. **Build and upload**:
   ```bash
   cd esp32
   pio run --target upload
   ```

4. **Monitor output**:
   ```bash
   pio device monitor
   ```

## Data Format

The firmware publishes sensor data to MQTT in JSON format:

```json
{
  "timestamp": 12345678,
  "device_id": "$DEVICE_ID",
  "sensor_name":"main_imu",
  "sensor_type":"imu",
  "imu_type":"MPU6500",
  "accelerometer": {
    "x": 0.123,
    "y": -0.456,
    "z": 9.789,
    "unit": "g"
  },
  "gyroscope": {
    "x": -0.231,
    "y": 0.156,
    "z": 0.789,
    "unit":"°/s"
  },
  "temperature": 25.4,
  "temperature_unit":"°C"  
}
```

## Integration with Liminal

This firmware acts as a data source for Liminal's input stages. The MQTT-published sensor data can be consumed by:

1. **MQTT Input Processor**: Subscribes to the sensor topics
2. **Transform Processors**: Filter, scale, or convert sensor readings
3. **Aggregate Processors**: Combine multiple sensor streams
4. **Output Processors**: Forward processed data to the Erlang monitoring system

### Example Liminal Pipeline

```TOML
# Liminal pipeline configuration with an input source (MQTT) and an output sink (console)
[inputs.mqtt_acceleration]
type = "mqtt_sub"
output = "raw_acceleration_data"
concurrency = { type = "thread" }
channel = { type = "broadcast", capacity = 256 }
parameters = { broker_url = "mqtt://localhost:1883", topics = ["liminal/sensors/esp32-001/imu"], client_id = "test_mqtt_input", qos = 0, clean_session = true, username = "", password = "" }

# Outputs: External data sinks
[outputs.log_output]
type = "console"
inputs = ["raw_acceleration_data"]
```

## Project Structure

```
liminal-firmware/
├── README.md
├── esp32/                          # ESP32 firmware sub-project
│   ├── .gitignore                  # ESP32-specific gitignore
│   ├── platformio.ini              # PlatformIO configuration
│   ├── compile_commands.json       # Language server support
│   ├── src/
│   │   ├── main.cpp                # Main firmware entry point
│   │   ├── config/
│   │   │   ├── config.h            # Configuration with credentials (gitignored)
│   │   │   └── config.h.template   # Safe configuration template
│   │   ├── communication/
│   │   │   ├── wifi_manager.h/.cpp # WiFi connection management
│   │   │   └── mqtt_client.h/.cpp  # MQTT client implementation
│   │   ├── sensors/
│   │   │   ├── sensor_base.h       # Base sensor interface
│   │   │   ├── sensor_manager.h/.cpp # Sensor discovery and management
│   │   │   └── imu_sensor.h/.cpp   # IMU sensor implementation
│   │   ├── devices/
│   │   │   ├── device_base.h       # Base device interface
│   │   │   ├── device_manager.h/.cpp # Device lifecycle management
│   │   │   └── led_device.h/.cpp   # Status LED control
│   │   └── utils/
│   │       └── json_helper.h/.cpp  # JSON serialization utilities
│   ├── include/                    # Public header files
│   ├── lib/                        # Custom libraries
│   ├── test/                       # Unit tests
│   ├── docs/                       # Hardware documentation & pinouts
│   └── .vscode/                    # VS Code configuration
└── shared/                         # Shared utilities (future)
    └── utils/                      # Common utility functions
```

## Development

### Adding New Sensors

1. Create sensor-specific initialisation code
2. Implement data reading functions
3. Update the JSON payload structure
4. Add sensor detection logic
5. Update documentation

### Adding New MCU Platforms

1. Create a new directory (e.g., `arduino_uno/`, `raspberry_pi_pico/`)
2. Implement platform-specific code
3. Maintain consistent MQTT data format
4. Update this README

## Troubleshooting

### Common Issues

- **Sensor not detected**: Check I2C wiring and addresses
- **WiFi connection fails**: Verify SSID/password and signal strength
- **MQTT connection issues**: Confirm broker IP and authentication
- **Build errors**: Ensure PlatformIO and dependencies are updated

### Debug Mode

Enable verbose logging by modifying the serial output statements in `main.cpp`.

## Security

### Configuration Security

The firmware uses a secure configuration system that supports:

- **Development**: Edit `config.h` directly with safe placeholder defaults
- **Production/CI**: Use environment variables to avoid exposing credentials
- **Template**: `config.h.template` provides documentation and examples

### Environment Variables

For secure builds, set these environment variables:

```bash
export WIFI_SSID_ENV="YourNetwork"
export WIFI_PASSWORD_ENV="YourPassword"
export MQTT_SERVER_ENV="192.168.1.100"
export MQTT_USER_ENV="username"        # Optional
export MQTT_PASSWORD_ENV="password"    # Optional
```

### Best Practices

- Never commit real credentials to version control
- Use environment variables in CI/CD pipelines
- The main `config.h` file uses safe defaults and is designed to be version-controlled
- For additional security, you can create `config-local.h` files (which are gitignored)

## Contributing

1. Fork the repository
2. Create a feature branch
3. Implement your changes
4. Test with hardware
5. Submit a pull request

## Related Projects

- **Liminal Engine**: [Rust-based data transformation pipeline](https://github.com/keithbugeja/liminal) 

---

For questions or support, please open an issue in this repository.
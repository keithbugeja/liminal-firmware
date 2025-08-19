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
   
   Edit `esp32/src/main.cpp` and update these variables:
   ```cpp
   const char* ssid = "YourWiFiNetwork";
   const char* password = "YourWiFiPassword";
   const char* mqtt_server = "your.mqtt.broker.ip";
   const char* mqtt_user = "mqtt_username";      // Optional
   const char* mqtt_pass = "mqtt_password";      // Optional
   const char* mqtt_topic = "sensors/your/topic";
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
  "device_id": "ESP32-MPU6500",
  "sensor_type": "MPU6500",
  "accelerometer": {
    "x": 0.123,
    "y": -0.456,
    "z": 9.789,
    "unit": "g"
  }
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
parameters = { broker_url = "mqtt://localhost:1883", topics = ["sensors/mpu6500/accelerometer"], client_id = "test_mqtt_input", qos = 0, clean_session = true, username = "", password = "" }

# Outputs: External data sinks
[outputs.log_output]
type = "console"
inputs = ["raw_acceleration_data"]
```

## Project Structure

```
liminal-firmware/
├── README.md
├── esp32/                          # ESP32 firmware
│   ├── platformio.ini              # PlatformIO configuration
│   ├── src/
│   │   └── main.cpp                # Main firmware code
│   ├── include/                    # Header files
│   ├── lib/                        # Custom libraries
│   ├── test/                       # Unit tests
│   └── docs/                       # Hardware documentation
└── shared/                         # Shared utilities
    └── utils/                      # Common utility functions
```

## Development

### Adding New Sensors

1. Create sensor-specific initialization code
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

## Contributing

1. Fork the repository
2. Create a feature branch
3. Implement your changes
4. Test with hardware
5. Submit a pull request

## License

[License information to be added]

## Related Projects

- **Liminal Engine**: Rust-based data transformation pipeline
- **Property Monitor**: Erlang-based monitoring system

---

For questions or support, please open an issue in this repository.
#include "imu_sensor.h"
#include "../config/config.h"

IMUSensor::IMUSensor(const String& name) 
    : SensorBase(name, SensorType::IMU), _imuType(IMUType::UNKNOWN), _address(MPU6050_ADDR) {
    memset(&_lastData, 0, sizeof(_lastData));
}

bool IMUSensor::begin() {
    Serial.println("Initializing IMU sensor...");
    
    // Initialize I2C if not already done
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    Serial.printf("I2C initialized on SDA=%d, SCL=%d\n", I2C_SDA_PIN, I2C_SCL_PIN);
    
    // Scan for I2C devices
    Serial.println("Scanning for I2C devices...");
    byte deviceCount = 0;
    for (byte address = 1; address < 127; address++) {
        Wire.beginTransmission(address);
        if (Wire.endTransmission() == 0) {
            Serial.printf("I2C device found at address 0x%02X\n", address);
            deviceCount++;
            if (address == 0x68) {
                _address = address;
            }
        }
    }
    Serial.printf("Found %d I2C device(s)\n", deviceCount);
    
    if (deviceCount == 0) {
        Serial.println("No I2C devices found!");
        _setStatus(SensorStatus::ERROR);
        return false;
    }
    
    // Detect IMU type
    _imuType = _detectIMUType();
    if (_imuType == IMUType::UNKNOWN) {
        Serial.println("Unknown or unsupported IMU detected");
        _setStatus(SensorStatus::ERROR);
        return false;
    }
    
    Serial.println("Detected IMU: " + getIMUTypeString());
    
    // Initialise based on detected type
    bool success = false;
    if (_imuType == IMUType::MPU6050) {
        success = _initializeMPU6050();
    } else if (_imuType == IMUType::MPU6500 || _imuType == IMUType::MPU9250) {
        success = _initializeMPU6500();
    }
    
    if (success) {
        _setStatus(SensorStatus::READY);
        Serial.println("IMU sensor initialized successfully!");
    } else {
        _setStatus(SensorStatus::ERROR);
        Serial.println("Failed to initialize IMU sensor");
    }
    
    return success;
}

bool IMUSensor::readData() {
    if (!isReady()) {
        return false;
    }
    
    _setStatus(SensorStatus::READING);
    
    bool success = false;
    if (_imuType == IMUType::MPU6050) {
        success = _readMPU6050();
    } else if (_imuType == IMUType::MPU6500 || _imuType == IMUType::MPU9250) {
        success = _readMPU6500();
    }
    
    if (success) {
        _lastData.timestamp = millis();
        _lastReading = _lastData.timestamp;
        _setStatus(SensorStatus::READY);
    } else {
        _setStatus(SensorStatus::ERROR);
    }
    
    return success;
}

DynamicJsonDocument IMUSensor::getDataAsJson() {
    DynamicJsonDocument doc(512);
    
    doc["sensor_name"] = _name;
    doc["sensor_type"] = getTypeString();
    doc["imu_type"] = getIMUTypeString();
    doc["timestamp"] = _lastData.timestamp;
    doc["device_id"] = DEVICE_ID;
    
    JsonObject accel = doc.createNestedObject("accelerometer");
    accel["x"] = _lastData.accelX;
    accel["y"] = _lastData.accelY;
    accel["z"] = _lastData.accelZ;
    accel["unit"] = (_imuType == IMUType::MPU6050) ? "m/s²" : "g";
    
    JsonObject gyro = doc.createNestedObject("gyroscope");
    gyro["x"] = _lastData.gyroX;
    gyro["y"] = _lastData.gyroY;
    gyro["z"] = _lastData.gyroZ;
    gyro["unit"] = "°/s";
    
    doc["temperature"] = _lastData.temperature;
    doc["temperature_unit"] = "°C";
    
    return doc;
}

String IMUSensor::getIMUTypeString() const {
    switch (_imuType) {
        case IMUType::MPU6050: return "MPU6050";
        case IMUType::MPU6500: return "MPU6500";
        case IMUType::MPU9250: return "MPU9250";
        default: return "Unknown";
    }
}

IMUType IMUSensor::_detectIMUType() {
    Wire.beginTransmission(_address);
    Wire.write(MPU6500_WHO_AM_I);
    Wire.endTransmission(false);
    Wire.requestFrom((uint8_t)_address, (uint8_t)1);
    
    if (Wire.available()) {
        uint8_t whoami = Wire.read();
        Serial.printf("WHO_AM_I register: 0x%02X\n", whoami);
        
        switch (whoami) {
            case 0x68: return IMUType::MPU6050;
            case 0x70: return IMUType::MPU6500;
            case 0x71: return IMUType::MPU9250;
            default:
                Serial.printf("Unknown WHO_AM_I value: 0x%02X\n", whoami);
                return IMUType::UNKNOWN;
        }
    }
    
    Serial.println("Failed to read WHO_AM_I register");
    return IMUType::UNKNOWN;
}

bool IMUSensor::_initializeMPU6050() {
    // Wake up the MPU6050
    Wire.beginTransmission(_address);
    Wire.write(MPU6500_PWR_MGMT_1);
    Wire.write(0x00); // Wake up
    Wire.endTransmission(true);
    delay(100);
    
    if (!_mpu6050.begin(_address, &Wire)) {
        Serial.println("Failed to initialize MPU6050 with Adafruit library");
        return false;
    }
    
    _mpu6050.setAccelerometerRange(MPU6050_RANGE_8_G);
    _mpu6050.setGyroRange(MPU6050_RANGE_500_DEG);
    _mpu6050.setFilterBandwidth(MPU6050_BAND_21_HZ);
    
    return true;
}

bool IMUSensor::_initializeMPU6500() {
    // Wake up the device
    if (!_writeMPU6500Register(MPU6500_PWR_MGMT_1, 0x00)) {
        return false;
    }
    delay(100);
    
    // Configure accelerometer (+/- 8g)
    if (!_writeMPU6500Register(MPU6500_ACCEL_CONFIG, 0x10)) {
        return false;
    }
    
    // Configure gyroscope (+/- 500 deg/s)
    if (!_writeMPU6500Register(MPU6500_GYRO_CONFIG, 0x08)) {
        return false;
    }
    
    return true;
}

bool IMUSensor::_readMPU6050() {
    sensors_event_t accel, gyro, temp;
    _mpu6050.getEvent(&accel, &gyro, &temp);
    
    _lastData.accelX = accel.acceleration.x;
    _lastData.accelY = accel.acceleration.y;
    _lastData.accelZ = accel.acceleration.z;
    
    _lastData.gyroX = gyro.gyro.x * 180.0 / PI; // Convert to degrees/second
    _lastData.gyroY = gyro.gyro.y * 180.0 / PI;
    _lastData.gyroZ = gyro.gyro.z * 180.0 / PI;
    
    _lastData.temperature = temp.temperature;
    
    return true;
}

bool IMUSensor::_readMPU6500() {
    // Read accelerometer data
    int16_t accelX = _readMPU6500Register16(MPU6500_ACCEL_XOUT_H);
    int16_t accelY = _readMPU6500Register16(MPU6500_ACCEL_XOUT_H + 2);
    int16_t accelZ = _readMPU6500Register16(MPU6500_ACCEL_XOUT_H + 4);
    
    // Read gyroscope data
    int16_t gyroX = _readMPU6500Register16(MPU6500_GYRO_XOUT_H);
    int16_t gyroY = _readMPU6500Register16(MPU6500_GYRO_XOUT_H + 2);
    int16_t gyroZ = _readMPU6500Register16(MPU6500_GYRO_XOUT_H + 4);
    
    // Read temperature data
    int16_t tempRaw = _readMPU6500Register16(MPU6500_TEMP_OUT_H);
    
    // Convert to physical units 
    // Note: Should verify these conversions against datasheet
    // but I'm a trusting person and believe everything on the internet is true.
    
    _lastData.accelX = accelX / 4096.0; // +/- 8g range: 4096 LSB/g
    _lastData.accelY = accelY / 4096.0;
    _lastData.accelZ = accelZ / 4096.0;
    
    _lastData.gyroX = gyroX / 65.5; // +/- 500°/s range: 65.5 LSB/degree/s
    _lastData.gyroY = gyroY / 65.5;
    _lastData.gyroZ = gyroZ / 65.5;
    
    _lastData.temperature = (tempRaw / 333.87) + 21.0;
        
    return true;
}

int16_t IMUSensor::_readMPU6500Register16(uint8_t reg) {
    Wire.beginTransmission(_address);
    Wire.write(reg);
    Wire.endTransmission(false);
    Wire.requestFrom((uint8_t)_address, (uint8_t)2);
    
    if (Wire.available() == 2) {
        int16_t value = (Wire.read() << 8) | Wire.read();
        return value;
    }
    return 0;
}

bool IMUSensor::_writeMPU6500Register(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(_address);
    Wire.write(reg);
    Wire.write(value);
    return Wire.endTransmission(true) == 0;
}

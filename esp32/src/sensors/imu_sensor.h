#ifndef IMU_SENSOR_H
#define IMU_SENSOR_H

#include "sensor_base.h"
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

enum class IMUType {
    UNKNOWN,
    MPU6050,
    MPU6500,
    MPU9250
};

class IMUSensor : public SensorBase {
public:
    IMUSensor(const String& name = "IMU");
    
    bool begin() override;
    bool readData() override;
    DynamicJsonDocument getDataAsJson() override;
    
    // IMU-specific methods
    IMUType getIMUType() const { return _imuType; }
    String getIMUTypeString() const;
    
    // Raw data access
    struct IMUData {
        float accelX, accelY, accelZ;
        float gyroX, gyroY, gyroZ;
        float temperature;
        unsigned long timestamp;
    };
    
    IMUData getLastReading() const { return _lastData; }
    
private:
    Adafruit_MPU6050 _mpu6050;
    IMUType _imuType;
    IMUData _lastData;
    uint8_t _address;
    
    IMUType _detectIMUType();
    bool _initializeMPU6050();
    bool _initializeMPU6500();
    bool _readMPU6050();
    bool _readMPU6500();
    
    // MPU6500 raw I2C functions
    int16_t _readMPU6500Register16(uint8_t reg);
    bool _writeMPU6500Register(uint8_t reg, uint8_t value);
    
    // Register addresses for MPU6500
    static const uint8_t MPU6500_PWR_MGMT_1 = 0x6B;
    static const uint8_t MPU6500_ACCEL_CONFIG = 0x1C;
    static const uint8_t MPU6500_GYRO_CONFIG = 0x1B;
    static const uint8_t MPU6500_ACCEL_XOUT_H = 0x3B;
    static const uint8_t MPU6500_GYRO_XOUT_H = 0x43;
    static const uint8_t MPU6500_TEMP_OUT_H = 0x41;
    static const uint8_t MPU6500_WHO_AM_I = 0x75;
};

#endif // IMU_SENSOR_H

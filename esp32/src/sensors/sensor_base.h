#ifndef SENSOR_BASE_H
#define SENSOR_BASE_H

#include <ArduinoJson.h>

enum class SensorType {
    IMU,
    TEMPERATURE,
    HUMIDITY,
    LIGHT,
    PRESSURE,
    UNKNOWN
};

enum class SensorStatus {
    UNINITIALIZED,
    READY,
    ERROR,
    READING
};

class SensorBase {
public:
    SensorBase(const String& name, SensorType type) 
        : _name(name), _type(type), _status(SensorStatus::UNINITIALIZED) {}
    
    virtual ~SensorBase() = default;
    
    // Pure virtual functions that must be implemented by derived classes
    virtual bool begin() = 0;
    virtual bool readData() = 0;
    virtual DynamicJsonDocument getDataAsJson() = 0;
    
    // Common interface
    virtual bool isReady() const { return _status == SensorStatus::READY; }
    virtual SensorStatus getStatus() const { return _status; }
    virtual String getName() const { return _name; }
    virtual SensorType getType() const { return _type; }
    virtual String getTypeString() const { return _sensorTypeToString(_type); }
    
    // Optional override for custom update intervals
    virtual unsigned long getUpdateInterval() const { return 1000; } // Default 1 second
    
protected:
    String _name;
    SensorType _type;
    SensorStatus _status;
    unsigned long _lastReading;
    
    void _setStatus(SensorStatus status) { _status = status; }
    
    friend class SensorManager; // Allow SensorManager to access _lastReading
    
private:
    String _sensorTypeToString(SensorType type) const {
        switch (type) {
            case SensorType::IMU: return "imu";
            case SensorType::TEMPERATURE: return "temperature";
            case SensorType::HUMIDITY: return "humidity";
            case SensorType::LIGHT: return "light";
            case SensorType::PRESSURE: return "pressure";
            default: return "unknown";
        }
    }
};

#endif // SENSOR_BASE_H

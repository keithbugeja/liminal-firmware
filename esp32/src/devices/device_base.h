#ifndef DEVICE_BASE_H
#define DEVICE_BASE_H

#include <ArduinoJson.h>

enum class DeviceType {
    LED,
    RELAY,
    SERVO,
    BUZZER,
    SCREEN, // Changed from DISPLAY to avoid Arduino.h conflict
    UNKNOWN
};

enum class DeviceStatus {
    UNINITIALIZED,
    READY,
    ERROR,
    BUSY
};

class DeviceBase {
public:
    DeviceBase(const String& name, DeviceType type) 
        : _name(name), _type(type), _status(DeviceStatus::UNINITIALIZED) {}
    
    virtual ~DeviceBase() = default;
    
    // Interface methods to be implemented by derived classes
    virtual bool begin() = 0;
    virtual bool handleCommand(const DynamicJsonDocument& command) = 0;
    virtual DynamicJsonDocument getStatusAsJson() = 0;
    
    // Common interface
    virtual bool isReady() const { return _status == DeviceStatus::READY; }
    virtual DeviceStatus getStatus() const { return _status; }
    virtual String getName() const { return _name; }
    virtual DeviceType getType() const { return _type; }
    virtual String getTypeString() const { return _deviceTypeToString(_type); }
    
protected:
    String _name;
    DeviceType _type;
    DeviceStatus _status;
    unsigned long _lastCommand;
    
    void _setStatus(DeviceStatus status) { _status = status; }
    
private:
    String _deviceTypeToString(DeviceType type) const {
        switch (type) {
            case DeviceType::LED: return "led";
            case DeviceType::RELAY: return "relay";
            case DeviceType::SERVO: return "servo";
            case DeviceType::BUZZER: return "buzzer";
            case DeviceType::SCREEN: return "screen";
            default: return "unknown";
        }
    }
};

#endif // DEVICE_BASE_H

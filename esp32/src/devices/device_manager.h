#ifndef DEVICE_MANAGER_H
#define DEVICE_MANAGER_H

#include <vector>
#include <memory>
#include "device_base.h"

class DeviceManager {
public:
    DeviceManager();
    ~DeviceManager();
    
    bool begin();
    void update();
    
    // Device management
    bool addDevice(std::shared_ptr<DeviceBase> device);
    bool removeDevice(const String& name);
    std::shared_ptr<DeviceBase> getDevice(const String& name);
    
    // Command handling
    bool handleCommand(const String& deviceName, const DynamicJsonDocument& command);
    bool handleCommand(const String& topic, const String& payload); // Parse from MQTT topic/payload
    
    // Status reporting
    DynamicJsonDocument getStatusReport();
    DynamicJsonDocument getDeviceStatus(const String& name);
    size_t getDeviceCount() const { return _devices.size(); }
    
    // Iteration support
    std::vector<std::shared_ptr<DeviceBase>>::iterator devices_begin() { return _devices.begin(); }
    std::vector<std::shared_ptr<DeviceBase>>::iterator devices_end() { return _devices.end(); }
    
private:
    std::vector<std::shared_ptr<DeviceBase>> _devices;
    unsigned long _lastUpdate;
    
    String _extractDeviceNameFromTopic(const String& topic);
};

#endif // DEVICE_MANAGER_H

#include "device_manager.h"
#include "led_device.h"
#include "../config/config.h"

DeviceManager::DeviceManager() : _lastUpdate(0) {
    _devices.reserve(8); // Reserve space for typical device count
}

DeviceManager::~DeviceManager() {
    _devices.clear();
}

bool DeviceManager::begin() {
    Serial.println("Initializing Device Manager...");
    
    bool allSuccess = true;
    for (auto& device : _devices) {
        if (!device->begin()) {
            Serial.printf("Failed to initialize device: %s\n", device->getName().c_str());
            allSuccess = false;
        }
    }
    
    Serial.printf("Device Manager initialized with %d devices\n", _devices.size());
    return allSuccess;
}

void DeviceManager::update() {
    unsigned long now = millis();
    
    // Update devices that need periodic updates (like blinking LEDs)
    for (auto& device : _devices) {
        if (device->getType() == DeviceType::LED) {
            auto ledDevice = std::static_pointer_cast<LEDDevice>(device);
            ledDevice->update();
        }
        
        // If we add any other devices that need periodic updates
        // we can handle them here as well:
        // e.g. if (device->getType() == DeviceType::SOME_OTHER_TYPE) {
        //     auto otherDevice = std::static_pointer_cast<OtherDeviceType>(device);
        //     otherDevice->update();
        // }
    }
    
    _lastUpdate = now;
}

bool DeviceManager::addDevice(std::shared_ptr<DeviceBase> device) {
    if (!device) {
        Serial.println("Cannot add null device");
        return false;
    }
    
    // Check for duplicate names
    for (const auto& existingDevice : _devices) {
        if (existingDevice->getName() == device->getName()) {
            Serial.printf("Device with name '%s' already exists\n", device->getName().c_str());
            return false;
        }
    }
    
    _devices.push_back(device);
    Serial.printf("Added device: %s (%s)\n", device->getName().c_str(), device->getTypeString().c_str());
    return true;
}

bool DeviceManager::removeDevice(const String& name) {
    auto it = std::find_if(_devices.begin(), _devices.end(),
        [&name](const std::shared_ptr<DeviceBase>& device) {
            return device->getName() == name;
        });
    
    if (it != _devices.end()) {
        Serial.printf("Removed device: %s\n", (*it)->getName().c_str());
        _devices.erase(it);
        return true;
    }
    
    Serial.printf("Device not found: %s\n", name.c_str());
    return false;
}

std::shared_ptr<DeviceBase> DeviceManager::getDevice(const String& name) {
    for (auto& device : _devices) {
        if (device->getName() == name) {
            return device;
        }
    }
    return nullptr;
}

bool DeviceManager::handleCommand(const String& deviceName, const DynamicJsonDocument& command) {
    auto device = getDevice(deviceName);
    if (!device) {
        Serial.printf("Device not found: %s\n", deviceName.c_str());
        return false;
    }
    
    if (!device->isReady()) {
        Serial.printf("Device not ready: %s\n", deviceName.c_str());
        return false;
    }
    
    return device->handleCommand(command);
}

bool DeviceManager::handleCommand(const String& topic, const String& payload) {
    // Extract device name from topic
    String deviceName = _extractDeviceNameFromTopic(topic);
    if (deviceName.isEmpty()) {
        Serial.printf("Could not extract device name from topic: %s\n", topic.c_str());
        return false;
    }
    
    // Parse JSON payload
    DynamicJsonDocument command(512);
    DeserializationError error = deserializeJson(command, payload);
    if (error) {
        Serial.printf("Failed to parse command JSON: %s\n", error.c_str());
        return false;
    }
    
    return handleCommand(deviceName, command);
}

DynamicJsonDocument DeviceManager::getStatusReport() {
    DynamicJsonDocument doc(1024);
    
    doc["device_count"] = _devices.size();
    doc["last_update"] = _lastUpdate;
    doc["timestamp"] = millis();
    
    JsonArray devicesArray = doc.createNestedArray("devices");
    for (auto& device : _devices) {
        JsonObject deviceInfo = devicesArray.createNestedObject();
        deviceInfo["name"] = device->getName();
        deviceInfo["type"] = device->getTypeString();
        deviceInfo["status"] = (device->getStatus() == DeviceStatus::READY) ? "ready" :
                              (device->getStatus() == DeviceStatus::ERROR) ? "error" :
                              (device->getStatus() == DeviceStatus::BUSY) ? "busy" : "uninitialized";
    }
    
    return doc;
}

DynamicJsonDocument DeviceManager::getDeviceStatus(const String& name) {
    auto device = getDevice(name);
    if (device) {
        return device->getStatusAsJson();
    }
    
    DynamicJsonDocument emptyDoc(128);
    emptyDoc["error"] = "Device not found: " + name;
    return emptyDoc;
}

String DeviceManager::_extractDeviceNameFromTopic(const String& topic) {
    // Expected format: liminal/commands/{device_id}/{device_name}
    // or: liminal/commands/{device_id}/{device_type}/{device_name}
    
    String commandsPrefix = String(MQTT_TOPIC_COMMANDS) + "/";
    
    if (!topic.startsWith(commandsPrefix)) {
        return "";
    }
    
    // Remove the prefix to get the remaining path
    String remaining = topic.substring(commandsPrefix.length());
    
    // Split by '/' and take the last part as device name
    int lastSlash = remaining.lastIndexOf('/');
    if (lastSlash >= 0) {
        return remaining.substring(lastSlash + 1);
    } else {
        // If no slash, the entire remaining string is the device name
        return remaining;
    }
}

#include "sensor_manager.h"
#include "../config/config.h"

SensorManager::SensorManager() : _lastUpdate(0) {
    _sensors.reserve(8); // Reserve space for typical sensor count
}

SensorManager::~SensorManager() {
    _sensors.clear();
}

bool SensorManager::begin() {
    Serial.println("Initializing Sensor Manager...");
    
    bool allSuccess = true;
    for (auto& sensor : _sensors) {
        if (!sensor->begin()) {
            Serial.printf("Failed to initialize sensor: %s\n", sensor->getName().c_str());
            allSuccess = false;
        }
    }
    
    Serial.printf("Sensor Manager initialized with %d sensors\n", _sensors.size());
    return allSuccess;
}

void SensorManager::update() {
    unsigned long now = millis();
    
    for (auto& sensor : _sensors) {
        if (_shouldUpdateSensor(sensor)) {
            if (!sensor->readData()) {
                Serial.printf("Failed to read data from sensor: %s\n", sensor->getName().c_str());
            }
        }
    }
    
    _lastUpdate = now;
}

bool SensorManager::addSensor(std::shared_ptr<SensorBase> sensor) {
    if (!sensor) {
        Serial.println("Cannot add null sensor");
        return false;
    }
    
    // Check for duplicate names
    for (const auto& existingSensor : _sensors) {
        if (existingSensor->getName() == sensor->getName()) {
            Serial.printf("Sensor with name '%s' already exists\n", sensor->getName().c_str());
            return false;
        }
    }
    
    _sensors.push_back(sensor);
    Serial.printf("Added sensor: %s (%s)\n", sensor->getName().c_str(), sensor->getTypeString().c_str());
    return true;
}

bool SensorManager::removeSensor(const String& name) {
    auto it = std::find_if(_sensors.begin(), _sensors.end(),
        [&name](const std::shared_ptr<SensorBase>& sensor) {
            return sensor->getName() == name;
        });
    
    if (it != _sensors.end()) {
        Serial.printf("Removed sensor: %s\n", (*it)->getName().c_str());
        _sensors.erase(it);
        return true;
    }
    
    Serial.printf("Sensor not found: %s\n", name.c_str());
    return false;
}

std::shared_ptr<SensorBase> SensorManager::getSensor(const String& name) {
    for (auto& sensor : _sensors) {
        if (sensor->getName() == name) {
            return sensor;
        }
    }
    return nullptr;
}

std::vector<DynamicJsonDocument> SensorManager::getAllSensorData() {
    std::vector<DynamicJsonDocument> allData;
    allData.reserve(_sensors.size());
    
    for (auto& sensor : _sensors) {
        if (sensor->isReady()) {
            allData.emplace_back(std::move(sensor->getDataAsJson()));
        }
    }
    
    return allData;
}

DynamicJsonDocument SensorManager::getSensorData(const String& name) {
    auto sensor = getSensor(name);
    if (sensor && sensor->isReady()) {
        return sensor->getDataAsJson();
    }
    
    DynamicJsonDocument emptyDoc(128);
    emptyDoc["error"] = "Sensor not found or not ready: " + name;
    return emptyDoc;
}

DynamicJsonDocument SensorManager::getStatusReport() {
    DynamicJsonDocument doc(1024);
    
    doc["sensor_count"] = _sensors.size();
    doc["last_update"] = _lastUpdate;
    doc["timestamp"] = millis();
    
    JsonArray sensorsArray = doc.createNestedArray("sensors");
    for (auto& sensor : _sensors) {
        JsonObject sensorInfo = sensorsArray.createNestedObject();
        sensorInfo["name"] = sensor->getName();
        sensorInfo["type"] = sensor->getTypeString();
        sensorInfo["status"] = (sensor->getStatus() == SensorStatus::READY) ? "ready" :
                              (sensor->getStatus() == SensorStatus::ERROR) ? "error" :
                              (sensor->getStatus() == SensorStatus::READING) ? "reading" : "uninitialized";
        sensorInfo["update_interval"] = sensor->getUpdateInterval();
    }
    
    return doc;
}

bool SensorManager::_shouldUpdateSensor(std::shared_ptr<SensorBase> sensor) {
    if (!sensor->isReady()) {
        return false;
    }
    
    unsigned long now = millis();
    unsigned long interval = sensor->getUpdateInterval();
    
    // Check if enough time has passed since last reading
    return (now - sensor->_lastReading) >= interval;
}

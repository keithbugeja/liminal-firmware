#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <vector>
#include <memory>
#include "sensor_base.h"

class SensorManager {
public:
    SensorManager();
    ~SensorManager();
    
    bool begin();
    void update();
    
    // Sensor management
    bool addSensor(std::shared_ptr<SensorBase> sensor);
    bool removeSensor(const String& name);
    std::shared_ptr<SensorBase> getSensor(const String& name);
    
    // Data collection
    std::vector<DynamicJsonDocument> getAllSensorData();
    DynamicJsonDocument getSensorData(const String& name);
    
    // Status reporting
    DynamicJsonDocument getStatusReport();
    size_t getSensorCount() const { return _sensors.size(); }
    
    // Iteration support
    std::vector<std::shared_ptr<SensorBase>>::iterator sensors_begin() { return _sensors.begin(); }
    std::vector<std::shared_ptr<SensorBase>>::iterator sensors_end() { return _sensors.end(); }
    
private:
    std::vector<std::shared_ptr<SensorBase>> _sensors;
    unsigned long _lastUpdate;
    
    bool _shouldUpdateSensor(std::shared_ptr<SensorBase> sensor);
};

#endif // SENSOR_MANAGER_H

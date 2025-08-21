#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

#include <PubSubClient.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>
#include "../config/config.h"

class MQTTClient;
typedef std::function<void(const String& topic, const String& payload)> MQTTCallback;

class MQTTClient {
public:
    MQTTClient();
    
    bool begin();
    bool connect();
    void disconnect();
    bool isConnected();
    
    void loop();
    
    bool publish(const String& topic, const String& payload, bool retained = false);
    bool publishSensorData(const String& sensorType, const DynamicJsonDocument& data);
    bool publishStatus(const DynamicJsonDocument& status);
    
    bool subscribe(const String& topic);
    bool subscribeToCommands();
    
    void setCallback(MQTTCallback callback);
    
    String getClientId();
    
private:
    WiFiClient _wifiClient;
    PubSubClient _mqttClient;
    String _clientId;
    MQTTCallback _userCallback;
    unsigned long _lastConnectionAttempt;
    
    static void _staticCallback(char* topic, byte* payload, unsigned int length);
    void _handleCallback(const String& topic, const String& payload);
    
    bool _isValidConfig();
    String _generateClientId();
    
    static MQTTClient* _instance;
};

#endif // MQTT_CLIENT_H

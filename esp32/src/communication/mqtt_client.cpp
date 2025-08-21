#include "mqtt_client.h"
#include <WiFi.h>

// Static member initialisation
MQTTClient* MQTTClient::_instance = nullptr;

MQTTClient::MQTTClient() : _mqttClient(_wifiClient), _lastConnectionAttempt(0) {
    _instance = this;
    _clientId = _generateClientId();
}

bool MQTTClient::begin() {
    _mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
    _mqttClient.setCallback(_staticCallback);
    _mqttClient.setKeepAlive(15);
    return true;
}

bool MQTTClient::connect() {
    if (!_isValidConfig()) {
        Serial.println("ERROR: MQTT server not configured!");
        Serial.println("Please update MQTT_SERVER in config.h");
        return false;
    }
    
    if (isConnected()) {
        return true;
    }
    
    // Avoid rapid reconnection attempts
    unsigned long now = millis();
    if (now - _lastConnectionAttempt < MQTT_TIMEOUT_MS) {
        return false;
    }
    _lastConnectionAttempt = now;
    
    Serial.print("Attempting MQTT connection...");
    
    bool connected;
    if (strlen(MQTT_USER) > 0) {
        connected = _mqttClient.connect(_clientId.c_str(), MQTT_USER, MQTT_PASSWORD);
    } else {
        connected = _mqttClient.connect(_clientId.c_str());
    }
    
    if (connected) {
        Serial.println(" connected!");
        Serial.print("Client ID: ");
        Serial.println(_clientId);
        
        // Subscribe to command topic by default
        subscribeToCommands();
        
        // Publish connection status
        DynamicJsonDocument statusDoc(256);
        statusDoc["status"] = "connected";
        statusDoc["client_id"] = _clientId;
        statusDoc["firmware_version"] = FIRMWARE_VERSION;
        statusDoc["timestamp"] = millis();
        publishStatus(statusDoc);
        
        return true;
    } else {
        Serial.print(" failed, rc=");
        Serial.print(_mqttClient.state());
        Serial.println(" will retry later");
        return false;
    }
}

void MQTTClient::disconnect() {
    if (isConnected()) {
        // Publish disconnection status
        DynamicJsonDocument statusDoc(256);
        statusDoc["status"] = "disconnecting";
        statusDoc["client_id"] = _clientId;
        statusDoc["timestamp"] = millis();
        publishStatus(statusDoc);
        
        _mqttClient.disconnect();
        Serial.println("MQTT disconnected");
    }
}

bool MQTTClient::isConnected() {
    return _mqttClient.connected();
}

void MQTTClient::loop() {
    _mqttClient.loop();
}

bool MQTTClient::publish(const String& topic, const String& payload, bool retained) {
    if (!isConnected()) {
        Serial.println("MQTT publish failed: Not connected - " + topic);
        return false;
    }
    
    // Debug payload size
    Serial.printf("MQTT publishing to %s (size: %d bytes)\n", topic.c_str(), payload.length());
    
    bool result = _mqttClient.publish(topic.c_str(), payload.c_str(), retained);
    if (result) {
        Serial.println("MQTT published: " + topic + " -> " + payload);
    } else {
        Serial.println("MQTT publish failed: " + topic);
        Serial.printf("  Payload size: %d bytes\n", payload.length());
        Serial.printf("  MQTT state: %d\n", _mqttClient.state());
    }
    return result;
}

bool MQTTClient::publishSensorData(const String& sensorType, const DynamicJsonDocument& data) {
    String topic = String(MQTT_TOPIC_SENSORS) + "/" + sensorType;
    
    String payload;
    serializeJson(data, payload);
    
    return publish(topic, payload);
}

bool MQTTClient::publishStatus(const DynamicJsonDocument& status) {
    String topic = MQTT_TOPIC_STATUS;
    
    String payload;
    serializeJson(status, payload);
    
    return publish(topic, payload, true);
}

bool MQTTClient::subscribe(const String& topic) {
    if (!isConnected()) {
        return false;
    }
    
    bool result = _mqttClient.subscribe(topic.c_str());
    if (result) {
        Serial.println("MQTT subscribed to: " + topic);
    } else {
        Serial.println("MQTT subscribe failed: " + topic);
    }
    return result;
}

bool MQTTClient::subscribeToCommands() {
    String topic = String(MQTT_TOPIC_COMMANDS) + "/#";
    return subscribe(topic);
}

void MQTTClient::setCallback(MQTTCallback callback) {
    _userCallback = callback;
}

String MQTTClient::getClientId() {
    return _clientId;
}

void MQTTClient::_staticCallback(char* topic, byte* payload, unsigned int length) {
    if (_instance) {
        String topicStr = String(topic);
        String payloadStr = "";
        for (unsigned int i = 0; i < length; i++) {
            payloadStr += (char)payload[i];
        }
        _instance->_handleCallback(topicStr, payloadStr);
    }
}

void MQTTClient::_handleCallback(const String& topic, const String& payload) {
    Serial.println("MQTT received: " + topic + " -> " + payload);
    
    if (_userCallback) {
        _userCallback(topic, payload);
    }
}

bool MQTTClient::_isValidConfig() {
    return (strlen(MQTT_SERVER) > 0 && strcmp(MQTT_SERVER, "192.168.1.100") != 0);
}

String MQTTClient::_generateClientId() {
    String clientId = MQTT_CLIENT_ID_PREFIX;
    clientId += WiFi.macAddress();
    clientId.replace(":", "");
    return clientId;
}

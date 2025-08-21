#include "json_helper.h"
#include "../config/config.h"

DynamicJsonDocument JsonHelper::createSystemStatus(const String& status, const String& message) {
    DynamicJsonDocument doc(512);
    doc["status"] = status;
    doc["device_id"] = DEVICE_ID;
    doc["firmware_version"] = FIRMWARE_VERSION;
    doc["timestamp"] = millis();
    doc["uptime"] = millis();
    
    if (message.length() > 0) {
        doc["message"] = message;
    }
    
    return doc;
}

DynamicJsonDocument JsonHelper::createErrorResponse(const String& error, const String& context) {
    DynamicJsonDocument doc(256);
    doc["error"] = error;
    doc["device_id"] = DEVICE_ID;
    doc["timestamp"] = millis();
    
    if (context.length() > 0) {
        doc["context"] = context;
    }
    
    return doc;
}

DynamicJsonDocument JsonHelper::createSuccessResponse(const String& message) {
    DynamicJsonDocument doc(256);
    doc["success"] = true;
    doc["device_id"] = DEVICE_ID;
    doc["timestamp"] = millis();
    
    if (message.length() > 0) {
        doc["message"] = message;
    }
    
    return doc;
}

bool JsonHelper::isValidJson(const String& jsonString) {
    DynamicJsonDocument doc(256);
    DeserializationError error = deserializeJson(doc, jsonString);
    return error == DeserializationError::Ok;
}

bool JsonHelper::parseJson(const String& jsonString, DynamicJsonDocument& doc) {
    DeserializationError error = deserializeJson(doc, jsonString);
    if (error) {
        Serial.printf("JSON parse error: %s\n", error.c_str());
        return false;
    }
    return true;
}

String JsonHelper::prettify(const DynamicJsonDocument& doc) {
    String output;
    serializeJsonPretty(doc, output);
    return output;
}

size_t JsonHelper::getJsonSize(const DynamicJsonDocument& doc) {
    return measureJson(doc);
}

DynamicJsonDocument JsonHelper::createLEDCommand(bool state) {
    DynamicJsonDocument doc(128);
    doc["state"] = state;
    return doc;
}

DynamicJsonDocument JsonHelper::createLEDCommand(uint8_t brightness) {
    DynamicJsonDocument doc(128);
    doc["brightness"] = brightness;
    return doc;
}

DynamicJsonDocument JsonHelper::createLEDBlinkCommand(unsigned long onTime, unsigned long offTime, int cycles) {
    DynamicJsonDocument doc(256);
    JsonObject blink = doc.createNestedObject("blink");
    blink["on_time"] = onTime;
    blink["off_time"] = offTime;
    blink["cycles"] = cycles;
    return doc;
}

#ifndef JSON_HELPER_H
#define JSON_HELPER_H

#include <ArduinoJson.h>

class JsonHelper {
public:
    // Create standardized JSON documents
    static DynamicJsonDocument createSystemStatus(const String& status, const String& message = "");
    static DynamicJsonDocument createErrorResponse(const String& error, const String& context = "");
    static DynamicJsonDocument createSuccessResponse(const String& message = "");
    
    // JSON validation and parsing
    static bool isValidJson(const String& jsonString);
    static bool parseJson(const String& jsonString, DynamicJsonDocument& doc);
    
    // JSON utilities
    static String prettify(const DynamicJsonDocument& doc);
    static size_t getJsonSize(const DynamicJsonDocument& doc);
    
    // Device command helpers
    static DynamicJsonDocument createLEDCommand(bool state);
    static DynamicJsonDocument createLEDCommand(uint8_t brightness);
    static DynamicJsonDocument createLEDBlinkCommand(unsigned long onTime, unsigned long offTime, int cycles = -1);
    
private:
    static const size_t JSON_BUFFER_SIZE = 1024;
};

#endif // JSON_HELPER_H

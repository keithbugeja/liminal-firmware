#include "wifi_manager.h"

WiFiManager::WiFiManager() : _lastConnectionAttempt(0) {
    _ssid = WIFI_SSID;
    _password = WIFI_PASSWORD;
}

bool WiFiManager::begin() {
    WiFi.mode(WIFI_STA);
    return true;
}

bool WiFiManager::connect() {
    if (!_isValidCredentials()) {
        Serial.println("ERROR: WiFi credentials not configured!");
        Serial.println("Please update WIFI_SSID and WIFI_PASSWORD in config.h");
        return false;
    }
    
    if (isConnected()) {
        return true;
    }
    
    // Avoid rapid reconnection attempts (but allow first attempt)
    unsigned long now = millis();
    if (_lastConnectionAttempt != 0 && now - _lastConnectionAttempt < 5000) {
        return false;
    }
    _lastConnectionAttempt = now;
    
    // Debugging, just in case WiFi is wonky
    Serial.println();
    Serial.println("=== WiFi Connection Attempt ===");
    Serial.print("Connecting to SSID: ");
    Serial.println(_ssid);
    Serial.print("ESP32 MAC: ");
    Serial.println(WiFi.macAddress());
    
    WiFi.mode(WIFI_STA);
    delay(100);
    WiFi.disconnect(true);
    delay(500);
    
    Serial.println("Initiating connection...");
    WiFi.begin(_ssid.c_str(), _password.c_str());
    
    unsigned long startTime = millis();
    int connectTimeoutMs = 60000; 
    
    while (WiFi.status() != WL_CONNECTED && (millis() - startTime) < connectTimeoutMs) {
        delay(250);
        Serial.print(".");
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println();
        _printConnectionInfo();
        return true;
    } else {
        Serial.println();
        Serial.printf("Connection failed after %d seconds\n", connectTimeoutMs / 1000);
        Serial.printf("Final status: %s (%d)\n", 
                     _getStatusString(WiFi.status()).c_str(), WiFi.status());
        
        // Add more debugging info
        Serial.println("Debugging info:");
        Serial.printf("  - WiFi mode: %d\n", WiFi.getMode());
        Serial.printf("  - Credentials: SSID='%s', PWD_LEN=%d\n", _ssid.c_str(), _password.length());
        return false;
    }
}

void WiFiManager::disconnect() {
    WiFi.disconnect();
    Serial.println("WiFi disconnected");
}

bool WiFiManager::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

String WiFiManager::getLocalIP() {
    return WiFi.localIP().toString();
}

String WiFiManager::getMacAddress() {
    return WiFi.macAddress();
}

int WiFiManager::getSignalStrength() {
    return WiFi.RSSI();
}

void WiFiManager::setCredentials(const char* ssid, const char* password) {
    _ssid = ssid;
    _password = password;
}

bool WiFiManager::_isValidCredentials() {
    return (_ssid.length() > 0 && _ssid != "YOUR_WIFI_SSID" &&
            _password.length() > 0 && _password != "YOUR_WIFI_PASSWORD");
}

void WiFiManager::_printConnectionInfo() {
    Serial.println("WiFi connected successfully!");
    Serial.print("IP address: ");
    Serial.println(getLocalIP());
    Serial.print("MAC address: ");
    Serial.println(getMacAddress());
    Serial.print("Signal strength: ");
    Serial.print(getSignalStrength());
    Serial.println(" dBm");
}

String WiFiManager::_getStatusString(int status) {
    switch (status) {
        case WL_IDLE_STATUS: return "Idle";
        case WL_NO_SSID_AVAIL: return "No SSID Available";
        case WL_SCAN_COMPLETED: return "Scan Completed";
        case WL_CONNECTED: return "Connected";
        case WL_CONNECT_FAILED: return "Connection Failed";
        case WL_CONNECTION_LOST: return "Connection Lost";
        case WL_DISCONNECTED: return "Disconnected";
        default: return "Unknown";
    }
}
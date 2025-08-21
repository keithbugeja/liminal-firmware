#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <WiFi.h>
#include "../config/config.h"

class WiFiManager {
public:
    WiFiManager();
    
    bool begin();
    bool connect();
    void disconnect();
    bool isConnected();
    
    String getLocalIP();
    String getMacAddress();
    int getSignalStrength();
    
    void setCredentials(const char* ssid, const char* password);
    
private:
    String _ssid;
    String _password;
    unsigned long _lastConnectionAttempt;
    
    bool _isValidCredentials();
    void _printConnectionInfo();
    String _getStatusString(int status);
};

#endif // WIFI_MANAGER_H

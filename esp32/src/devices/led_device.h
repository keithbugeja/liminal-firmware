#ifndef LED_DEVICE_H
#define LED_DEVICE_H

#include "device_base.h"

class LEDDevice : public DeviceBase {
public:
    LEDDevice(const String& name, uint8_t pin, bool activeLow = false);
    
    bool begin() override;
    bool handleCommand(const DynamicJsonDocument& command) override;
    DynamicJsonDocument getStatusAsJson() override;
    
    // LED-specific methods
    bool setState(bool state);
    bool getState() const { return _currentState; }
    bool toggle();
    
    bool setBrightness(uint8_t brightness); // 0-255, requires PWM pin
    uint8_t getBrightness() const { return _brightness; }
    
    bool blink(unsigned long onTime, unsigned long offTime, int cycles = -1);
    void stopBlink();
    bool isBlinking() const { return _isBlinking; }
    
    void update();
    
private:
    uint8_t _pin;
    bool _activeLow;
    bool _currentState;
    uint8_t _brightness;
    bool _pwmCapable;
    
    // Blinking state
    bool _isBlinking;
    unsigned long _blinkOnTime;
    unsigned long _blinkOffTime;
    unsigned long _lastBlinkChange;
    int _blinkCycles;
    int _remainingCycles;
    bool _blinkState;
    
    void _writePin(bool state);
    bool _isPWMPin(uint8_t pin);
    void _updateBlink();
};

#endif // LED_DEVICE_H

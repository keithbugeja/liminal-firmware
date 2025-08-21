#include "led_device.h"
#include "../config/config.h"

LEDDevice::LEDDevice(const String& name, uint8_t pin, bool activeLow)
    : DeviceBase(name, DeviceType::LED), _pin(pin), _activeLow(activeLow),
      _currentState(false), _brightness(255), _isBlinking(false),
      _blinkOnTime(0), _blinkOffTime(0), _lastBlinkChange(0),
      _blinkCycles(0), _remainingCycles(0), _blinkState(false) {
    
    _pwmCapable = _isPWMPin(pin);
}

bool LEDDevice::begin() {
    Serial.printf("Initializing LED '%s' on pin %d\n", _name.c_str(), _pin);
    
    pinMode(_pin, OUTPUT);
    _writePin(false); // Start with LED off
    
    _setStatus(DeviceStatus::READY);
    Serial.printf("LED '%s' initialized successfully\n", _name.c_str());
    return true;
}

bool LEDDevice::handleCommand(const DynamicJsonDocument& command) {
    if (!isReady()) {
        Serial.println("LED device not ready for commands");
        return false;
    }
    
    _setStatus(DeviceStatus::BUSY);
    _lastCommand = millis();
    
    bool success = false;
    
    // Handle different command types
    if (command.containsKey("state")) {
        bool state = command["state"].as<bool>();
        success = setState(state);
        Serial.printf("LED '%s' state command: %s\n", _name.c_str(), state ? "ON" : "OFF");
    }
    else if (command.containsKey("toggle")) {
        success = toggle();
        Serial.printf("LED '%s' toggled to: %s\n", _name.c_str(), _currentState ? "ON" : "OFF");
    }
    else if (command.containsKey("brightness") && _pwmCapable) {
        uint8_t brightness = command["brightness"].as<uint8_t>();
        success = setBrightness(brightness);
        Serial.printf("LED '%s' brightness set to: %d\n", _name.c_str(), brightness);
    }
    else if (command.containsKey("blink")) {
        JsonObjectConst blinkCmd = command["blink"];
        unsigned long onTime = blinkCmd["on_time"] | 500;
        unsigned long offTime = blinkCmd["off_time"] | 500;
        int cycles = blinkCmd["cycles"] | -1;
        success = blink(onTime, offTime, cycles);
        Serial.printf("LED '%s' blink started: on=%lu, off=%lu, cycles=%d\n", 
                     _name.c_str(), onTime, offTime, cycles);
    }
    else if (command.containsKey("stop_blink")) {
        stopBlink();
        success = true;
        Serial.printf("LED '%s' blink stopped\n", _name.c_str());
    }
    else {
        Serial.printf("Unknown command for LED '%s'\n", _name.c_str());
        success = false;
    }
    
    _setStatus(DeviceStatus::READY);
    return success;
}

DynamicJsonDocument LEDDevice::getStatusAsJson() {
    DynamicJsonDocument doc(512);
    
    doc["device_name"] = _name;
    doc["device_type"] = getTypeString();
    doc["pin"] = _pin;
    doc["state"] = _currentState;
    doc["brightness"] = _brightness;
    doc["pwm_capable"] = _pwmCapable;
    doc["is_blinking"] = _isBlinking;
    doc["active_low"] = _activeLow;
    doc["status"] = (_status == DeviceStatus::READY) ? "ready" : 
                   (_status == DeviceStatus::BUSY) ? "busy" : 
                   (_status == DeviceStatus::ERROR) ? "error" : "uninitialized";
    doc["last_command"] = _lastCommand;
    doc["timestamp"] = millis();
    
    if (_isBlinking) {
        JsonObject blinkInfo = doc.createNestedObject("blink_info");
        blinkInfo["on_time"] = _blinkOnTime;
        blinkInfo["off_time"] = _blinkOffTime;
        blinkInfo["remaining_cycles"] = _remainingCycles;
    }
    
    return doc;
}

bool LEDDevice::setState(bool state) {
    stopBlink();
    _currentState = state;
    _writePin(state);
    return true;
}

bool LEDDevice::toggle() {
    return setState(!_currentState);
}

bool LEDDevice::setBrightness(uint8_t brightness) {
    if (!_pwmCapable) {
        Serial.printf("LED '%s' on pin %d does not support PWM brightness control\n", 
                     _name.c_str(), _pin);
        return false;
    }
    
    _brightness = brightness;
    
    if (_currentState && !_isBlinking) {
        // Apply brightness immediately if LED is on and not blinking
        uint8_t pwmValue = _activeLow ? (255 - brightness) : brightness;
        analogWrite(_pin, pwmValue);
    }
    
    return true;
}

bool LEDDevice::blink(unsigned long onTime, unsigned long offTime, int cycles) {
    _isBlinking = true;
    _blinkOnTime = onTime;
    _blinkOffTime = offTime;
    _blinkCycles = cycles;
    _remainingCycles = cycles;
    _lastBlinkChange = millis();
    _blinkState = true; // Start with LED on
    
    _writePin(true);
    return true;
}

void LEDDevice::stopBlink() {
    if (_isBlinking) {
        _isBlinking = false;
        _writePin(_currentState);
    }
}

void LEDDevice::update() {
    if (_isBlinking) {
        _updateBlink();
    }
}

void LEDDevice::_writePin(bool state) {
    if (_pwmCapable && state && _brightness < 255) {
        // Use PWM for brightness control
        uint8_t pwmValue = _activeLow ? (255 - _brightness) : _brightness;
        analogWrite(_pin, pwmValue);
    } else {
        // Digital on/off
        bool pinState = _activeLow ? !state : state;
        digitalWrite(_pin, pinState ? HIGH : LOW);
    }
}

bool LEDDevice::_isPWMPin(uint8_t pin) {
    // ESP32 pins that support PWM (most GPIO pins do)
    // Exclude pins that are typically used for other purposes
    if (pin >= 0 && pin <= 33) {
        // Exclude pins that are typically not available or have special functions
        switch (pin) {
            case 6: case 7: case 8: case 9: case 10: case 11: // Flash pins
                return false;
            default:
                return true;
        }
    }
    return false;
}

void LEDDevice::_updateBlink() {
    unsigned long now = millis();
    unsigned long elapsed = now - _lastBlinkChange;
    
    bool shouldChange = false;
    if (_blinkState && elapsed >= _blinkOnTime) {
        // Currently on, switch to off
        shouldChange = true;
        _blinkState = false;
    } else if (!_blinkState && elapsed >= _blinkOffTime) {
        // Currently off, switch to on
        shouldChange = true;
        _blinkState = true;
        
        // Decrement cycle count when completing a full cycle (off->on)
        if (_remainingCycles > 0) {
            _remainingCycles--;
        }
    }
    
    if (shouldChange) {
        _lastBlinkChange = now;
        _writePin(_blinkState);
        
        // Check if we've completed all cycles
        if (_remainingCycles == 0) {
            stopBlink();
        }
    }
}

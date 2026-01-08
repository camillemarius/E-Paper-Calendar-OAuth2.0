#pragma once
#include <Arduino.h>
#include <Wire.h>

class BatteryGauge {
public:
    BatteryGauge(uint8_t address = 0x36); // MAX17048 default I2C address

    // Begin ohne Argumente — wirkt im main sauber
    bool begin();

    // Begin mit SDA/SCL Pins (ESP32 etc.)
    bool begin(int sdaPin, int sclPin);

    float getVoltage();      // Spannung in Volt
    float getPercentage();   // Ladezustand in %

    void quickStart();       // Quickstart-Befehl für die Fuel-Gauge
    void setAlertThreshold(uint8_t percent);
    bool isAlerting();

private:
    uint8_t _address;
    TwoWire* _wire = nullptr;

    uint16_t readRegister(uint8_t reg);
    void writeRegister(uint8_t reg, uint16_t value);
};

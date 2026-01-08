#include "BatteryGauge.h"

#define REG_VCELL 0x02
#define REG_SOC   0x04
#define REG_MODE  0x06
#define REG_ATRT  0x07

BatteryGauge::BatteryGauge(uint8_t address) {
    _address = address;
}

// Standard begin mit default Wire
bool BatteryGauge::begin() {
    _wire = &Wire;
    _wire->begin();
    quickStart();
    return true;
}

// Begin mit definierten SDA/SCL Pins
bool BatteryGauge::begin(int sdaPin, int sclPin) {
    _wire = &Wire;
    _wire->begin(sdaPin, sclPin);
    quickStart();
    return true;
}

float BatteryGauge::getVoltage() {
    uint16_t raw = readRegister(REG_VCELL);
    return raw * 78.125 / 1000000.0; // 78.125 μV LSB → V
}

float BatteryGauge::getPercentage() {
    uint16_t raw = readRegister(REG_SOC);
    return raw / 256.0;
}

void BatteryGauge::quickStart() {
    uint16_t mode = readRegister(REG_MODE);
    mode |= 0x4000; // QuickStart Bit
    writeRegister(REG_MODE, mode);
}

void BatteryGauge::setAlertThreshold(uint8_t percent) {
    if (percent > 32) percent = 32;
    writeRegister(REG_ATRT, percent);
}

bool BatteryGauge::isAlerting() {
    uint16_t mode = readRegister(REG_MODE);
    return mode & 0x8000;
}

// --- I2C helper functions ---
uint16_t BatteryGauge::readRegister(uint8_t reg) {
    _wire->beginTransmission(_address);
    _wire->write(reg);
    _wire->endTransmission(false);
    _wire->requestFrom(_address, (uint8_t)2);

    if (_wire->available() < 2) return 0;
    uint8_t msb = _wire->read();
    uint8_t lsb = _wire->read();
    return ((uint16_t)msb << 8) | lsb;
}

void BatteryGauge::writeRegister(uint8_t reg, uint16_t value) {
    _wire->beginTransmission(_address);
    _wire->write(reg);
    _wire->write((value >> 8) & 0xFF); // MSB
    _wire->write(value & 0xFF);        // LSB
    _wire->endTransmission();
}

#pragma once

#include <Arduino.h>
#include <SPIMemory.h>

class externalFlash {
private:
    SPIFlash flash;     // Flash driver object
    uint32_t baseAddr;  // Base address for data storage

public:
    // Constructor: pass in CS pin and optional base address
    externalFlash(uint8_t csPin, uint32_t base = 0x0000);

    // Initialize flash, return true if successful
    bool begin();

    // Write data buffer to flash at given offset
    bool writeData(uint32_t offset, const uint8_t *data, size_t len);

    // Read data buffer from flash at given offset
    bool readData(uint32_t offset, uint8_t *data, size_t len);

    // Write a large buffer to flash (splits into pages automatically)
    bool writeImage(uint32_t offset, const uint8_t* imgData, size_t len);

    // Read large buffer from flash
    bool readImage(uint32_t offset, uint8_t* buffer, size_t len);

    // Simple test: write and read back a string
    void test();
};

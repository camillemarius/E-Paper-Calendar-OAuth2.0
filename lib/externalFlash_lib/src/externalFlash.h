#pragma once

#include <Arduino.h>
#include <SPIMemory.h>

// Compile-Error if S1 is already defined
#ifdef S1
#undef S1
#endif
// Compile-Error if S1 is already defined

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

    // Erase the sectors covering [offset, offset + len) before writing (NOR flash: 1 -> 0 only)
    bool eraseArea(uint32_t offset, size_t len);

    // Deep Power-Down (0xB9): about 1 uA until the next begin(), which wakes it up again
    bool sleep();

    // Simple test: write and read back a string in the last sector (outside the image area)
    void test();
};

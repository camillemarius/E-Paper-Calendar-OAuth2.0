#include "externalFlash.h"
#include <logger.h>

externalFlash::externalFlash(uint8_t csPin, uint32_t base)
    : flash(csPin), baseAddr(base) {}

bool externalFlash::begin() {
    if (flash.begin()) {
        LOG_DEBUG("External flash initialized!");
        LOG_DEBUG("Chip ID: 0x");
        LOG_DEBUG("getJEDECID: %d, hex: 0x%X", flash.getJEDECID(), flash.getJEDECID());
        return true;
    } else {
        LOG_ERROR("Flash init failed!");
        return false;
    }
}

bool externalFlash::writeData(uint32_t offset, const uint8_t *data, size_t len) {
    return flash.writeByteArray(baseAddr + offset, (uint8_t*)data, len);
}

bool externalFlash::readData(uint32_t offset, uint8_t *data, size_t len) {
    return flash.readByteArray(baseAddr + offset, data, len);
}

bool externalFlash::writeImage(uint32_t offset, const uint8_t* imgData, size_t len) {
    const uint16_t pageSize = 256;   // W25Q16 page size
    uint32_t addr = baseAddr + offset;
    size_t remaining = len;

    while (remaining > 0) {
        size_t toWrite = remaining > pageSize ? pageSize : remaining;
        // cast away const because SPIFlash expects uint8_t*
        if (!flash.writeByteArray(addr, (uint8_t*)imgData, toWrite)) {
            return false;
        }
        addr += toWrite;
        imgData += toWrite;
        remaining -= toWrite;
    }
    return true;
}


bool externalFlash::readImage(uint32_t offset, uint8_t* buffer, size_t len) {
    return flash.readByteArray(baseAddr + offset, buffer, len);
}

void externalFlash::test() {
    const char testMsg[] = "Hello Flash!";
    size_t len = sizeof(testMsg);

    LOG_DEBUG("Writing test string...");
    if (!writeData(0, (const uint8_t*)testMsg, len)) {
        LOG_DEBUG("Write failed!");
        return;
    }

    delay(50);

    char buffer[32] = {0};
    if (!readData(0, (uint8_t*)buffer, len)) {
        LOG_DEBUG("Read failed!");
        return;
    }

    LOG_DEBUG("Read back: ");
    LOG_DEBUG(buffer);

    if (strcmp(testMsg, buffer) == 0) {
        LOG_DEBUG("Test PASSED!");
    } else {
        LOG_DEBUG("Test FAILED!");
    }
}

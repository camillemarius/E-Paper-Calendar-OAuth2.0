#include "externalFlash.h"
#include <logger.h>

externalFlash::externalFlash(uint8_t csPin, uint32_t base)
    : flash(csPin), baseAddr(base)
{
}

bool externalFlash::begin()
{
    if (flash.begin())
    {
        LOG_DEBUG("External flash initialized!");
        LOG_DEBUG("Chip ID: 0x");
        LOG_DEBUG("getJEDECID: %d, hex: 0x%X",
                  flash.getJEDECID(),
                  flash.getJEDECID());
        return true;
    }

    LOG_ERROR("Flash init failed!");
    return false;
}

bool externalFlash::writeData(uint32_t offset, const uint8_t* data, size_t len)
{
    return writeImage(offset, data, len);
}

bool externalFlash::readData(uint32_t offset, uint8_t* data, size_t len)
{
    return flash.readByteArray(baseAddr + offset, data, len);
}

bool externalFlash::writeImage(uint32_t offset, const uint8_t* imgData, size_t len)
{
    constexpr uint16_t pageSize = 256;

    uint32_t addr = baseAddr + offset;
    size_t remaining = len;

    while (remaining > 0)
    {
        // Position innerhalb der aktuellen 256-Byte-Page
        uint16_t pageOffset = addr % pageSize;

        // Noch verfügbarer Platz bis zum Ende dieser Page
        uint16_t spaceInPage = pageSize - pageOffset;

        // Nicht über die Page-Grenze schreiben
        size_t toWrite = remaining < spaceInPage
                       ? remaining
                       : spaceInPage;

        if (!flash.writeByteArray(
                addr,
                const_cast<uint8_t*>(imgData),
                toWrite))
        {
            LOG_ERROR(
                "Flash write failed at address 0x%X",
                (unsigned)addr
            );
            return false;
        }

        addr += toWrite;
        imgData += toWrite;
        remaining -= toWrite;
    }

    return true;
}

bool externalFlash::readImage(uint32_t offset, uint8_t* buffer, size_t len)
{
    return flash.readByteArray(
        baseAddr + offset,
        buffer,
        len
    );
}

void externalFlash::test()
{
    const char testMsg[] = "Hello Flash!";
    size_t len = sizeof(testMsg);

    LOG_DEBUG("Writing test string...");

    if (!writeData(0, (const uint8_t*)testMsg, len))
    {
        LOG_DEBUG("Write failed!");
        return;
    }

    delay(50);

    char buffer[32] = {0};

    if (!readData(0, (uint8_t*)buffer, len))
    {
        LOG_DEBUG("Read failed!");
        return;
    }

    LOG_DEBUG("Read back: ");
    LOG_DEBUG(buffer);

    if (strcmp(testMsg, buffer) == 0)
        LOG_DEBUG("TEST PASSED!");
    else
        LOG_DEBUG("TEST FAILED!");
}

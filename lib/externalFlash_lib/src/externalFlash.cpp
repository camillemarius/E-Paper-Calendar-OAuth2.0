#include "externalFlash.h"
#include <logger.h>

externalFlash::externalFlash(uint8_t csPin, uint32_t base)
    : flash(csPin), baseAddr(base)
{
}

bool externalFlash::begin()
{
    bool ready = flash.begin();
    if (!ready)
    {
        // Die Kalender-Firmware legt den Flash in Deep Power-Down. SPIFlash::begin()
        // weckt ihn nicht selbst auf, deshalb Release (0xAB) senden und erneut versuchen.
        flash.powerUp();
        ready = flash.begin();
    }

    if (ready)
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

bool externalFlash::eraseArea(uint32_t offset, size_t len)
{
    // Bereich auf ganze 4-KB-Sektoren erweitern, wo möglich 64-KB-Blöcke löschen.
    // (SPIFlash::eraseSection rundet die Grösse ab und lässt den Rest stehen.)
    constexpr uint32_t SECTOR = 4096;
    constexpr uint32_t BLOCK = 65536;

    uint32_t addr = (baseAddr + offset) & ~(SECTOR - 1);
    const uint32_t end = (baseAddr + offset + len + SECTOR - 1) & ~(SECTOR - 1);

    while (addr < end)
    {
        const bool wholeBlock = (addr % BLOCK == 0) && (addr + BLOCK <= end);
        const bool ok = wholeBlock ? flash.eraseBlock64K(addr) : flash.eraseSector(addr);
        if (!ok)
        {
            LOG_ERROR("Flash erase failed at address 0x%X", (unsigned)addr);
            return false;
        }
        addr += wholeBlock ? BLOCK : SECTOR;
    }

    return true;
}

bool externalFlash::sleep()
{
    return flash.powerDown();
}

void externalFlash::test()
{
    const char testMsg[] = "Hello Flash!";
    size_t len = sizeof(testMsg);

    // Letzter Sektor des Chips: ausserhalb des Bildbereichs ab Adresse 0
    const uint32_t testOffset = flash.getCapacity() - 4096 - baseAddr;

    LOG_DEBUG("Writing test string at 0x%X...", (unsigned)(baseAddr + testOffset));

    if (!eraseArea(testOffset, len) || !writeData(testOffset, (const uint8_t*)testMsg, len))
    {
        LOG_DEBUG("Write failed!");
        return;
    }

    delay(50);

    char buffer[32] = {0};

    if (!readData(testOffset, (uint8_t*)buffer, len))
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

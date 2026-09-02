#include "FlashImage.h"

bool FlashImage::readRGB565Line(uint16_t y, uint16_t* buffer, uint16_t bufferPixels)
{
    if (y >= HEIGHT || buffer == nullptr || bufferPixels < WIDTH)
        return false;

    uint8_t packedLine[PACKED_LINE_SIZE];

    if (!flash.readImage(offset + y * PACKED_LINE_SIZE, packedLine, PACKED_LINE_SIZE))
        return false;

    static constexpr uint16_t PALETTE_3C[] = {
        0x0000, // black
        0xFFFF, // white
        0xF800  // red
    };

    static constexpr uint16_t PALETTE_6C[] = {
        0x0000, // black
        0xFFFF, // white
        0xFFE0, // yellow
        0xF800, // red
        0x001F, // blue
        0x07E0  // green
    };

    const uint16_t* palette = paletteMode == ImagePalette::ThreeColor ? PALETTE_3C : PALETTE_6C;
    const uint8_t paletteSize = static_cast<uint8_t>(paletteMode);

    for (uint16_t x = 0; x < WIDTH / 2; ++x)
    {
        uint8_t packed = packedLine[x];

        uint8_t first = (packed >> 3) & 0x07;
        uint8_t second = packed & 0x07;

        buffer[x * 2] = first < paletteSize ? palette[first] : 0xFFFF;
        buffer[x * 2 + 1] = second < paletteSize ? palette[second] : 0xFFFF;
    }

    return true;
}
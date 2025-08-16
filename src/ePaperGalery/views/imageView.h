#pragma once
#include <Arduino.h>
#include <WebServer.h>
#include <JPEGDecoder.h>
#include <logger.h>

template <typename DisplayType>
class ImageView {
public:
    ImageView(DisplayType& disp, uint8_t paletteMode = 7)
        : display(disp), paletteMode(paletteMode) {}

    // Zeigt JPEG direkt auf Display, skaliert auf 800x480, Bayer + Palette
    bool showJPEG(uint8_t* jpegData, size_t jpegLen) {
        if (JpegDec.decodeArray(jpegData, jpegLen) != 0) {
            LOG_DEBUG("JPEG konnte nicht dekodiert werden!");
            return false;
        }

        uint16_t targetW = 800;
        uint16_t targetH = 480;
        float scaleX = float(targetW) / JpegDec.width;
        float scaleY = float(targetH) / JpegDec.height;

        do {
            uint16_t *pImg = JpegDec.pImage;
            uint16_t mcuX = JpegDec.MCUx;
            uint16_t mcuY = JpegDec.MCUy;
            uint16_t mcuW = JpegDec.MCUWidth;
            uint16_t mcuH = JpegDec.MCUHeight;

            for (uint16_t y = 0; y < mcuH; y++) {
                for (uint16_t x = 0; x < mcuW; x++) {
                    uint16_t color565 = pImg[y * mcuW + x];
                    uint8_t r = ((color565 >> 11) & 0x1F) << 3;
                    uint8_t g = ((color565 >> 5) & 0x3F) << 2;
                    uint8_t b = (color565 & 0x1F) << 3;

                    // Bayer-Dithering
                    uint8_t threshold = bayer4x4[(mcuY + y) % 4][(mcuX + x) % 4] * 16;
                    r = min(255, r + threshold);
                    g = min(255, g + threshold);
                    b = min(255, b + threshold);

                    // Skalierung auf 800x480
                    uint16_t dstX = mcuX + x;
                    uint16_t dstY = mcuY + y;
                    dstX = dstX * scaleX;
                    dstY = dstY * scaleY;

                    uint16_t paletteColor = findNearestColor(r, g, b);
                    display.drawPixel(dstX, dstY, paletteColor);
                }
            }
        } while (JpegDec.read());

        return true;
    }

private:
    DisplayType& display;
    uint8_t paletteMode;

    const uint16_t PALETTE7[7] = { 0x0000, 0xFFFF, 0xFFE0, 0xF800, 0x001F, 0x07E0, 0xFD20 };
    const uint8_t bayer4x4[4][4] = {
        {0, 8, 2, 10}, {12, 4, 14, 6}, {3, 11, 1, 9}, {15, 7, 13, 5}
    };

    uint16_t findNearestColor(uint8_t r, uint8_t g, uint8_t b) {
        uint16_t bestIndex = 0;
        uint32_t bestDiff = 999999;
        for (uint8_t i = 0; i < paletteMode; i++) {
            uint8_t pr = ((PALETTE7[i] >> 11) & 0x1F) << 3;
            uint8_t pg = ((PALETTE7[i] >> 5) & 0x3F) << 2;
            uint8_t pb = (PALETTE7[i] & 0x1F) << 3;
            int dr = r - pr, dg = g - pg, db = b - pb;
            uint32_t diff = dr*dr + dg*dg + db*db;
            if (diff < bestDiff) { bestDiff = diff; bestIndex = i; }
        }
        return PALETTE7[bestIndex];
    }
};
#pragma once

#include <Arduino.h>
#include <WebServer.h>
#include <JPEGDecoder.h>
#include <logger.h>
#include "../ImageConfigurator/ImageSource.h"

/*
* ImageView bleibt vollständig in der .h, da es eine Template-Klasse ist.
* Template-Implementierungen müssen beim Kompilieren des konkreten Typs sichtbar sein.
* Eine separate .cpp würde deshalb ohne explizite Template-Instanziierungen zu
* Linkerfehlern führen (z.B. "undefined reference to ImageView<GDEP073E01>...").
*
* FlashImage dagegen ist keine Template-Klasse und kann deshalb sauber in
* FlashImage.h und FlashImage.cpp getrennt werden.
*/


template <typename DisplayType>
class ImageView
{
public:
    ImageView(DisplayType& disp, uint8_t paletteMode = 7)
        : display(disp), paletteMode(paletteMode) {}

    bool showJPEG(uint8_t* jpegData, size_t jpegLen)
    {
        if (JpegDec.decodeArray(jpegData, jpegLen) != 0)
        {
            LOG_DEBUG("JPEG konnte nicht dekodiert werden!");
            return false;
        }

        uint16_t targetW = 800;
        uint16_t targetH = 480;
        float scaleX = float(targetW) / JpegDec.width;
        float scaleY = float(targetH) / JpegDec.height;

        do
        {
            uint16_t* pImg = JpegDec.pImage;
            uint16_t mcuX = JpegDec.MCUx;
            uint16_t mcuY = JpegDec.MCUy;
            uint16_t mcuW = JpegDec.MCUWidth;
            uint16_t mcuH = JpegDec.MCUHeight;

            for (uint16_t y = 0; y < mcuH; y++)
            {
                for (uint16_t x = 0; x < mcuW; x++)
                {
                    uint16_t color565 = pImg[y * mcuW + x];
                    uint8_t r = ((color565 >> 11) & 0x1F) << 3;
                    uint8_t g = ((color565 >> 5) & 0x3F) << 2;
                    uint8_t b = (color565 & 0x1F) << 3;

                    uint8_t threshold = bayer4x4[(mcuY + y) % 4][(mcuX + x) % 4] * 16;
                    r = min(255, r + threshold);
                    g = min(255, g + threshold);
                    b = min(255, b + threshold);

                    uint16_t dstX = (mcuX + x) * scaleX;
                    uint16_t dstY = (mcuY + y) * scaleY;

                    display.drawPixel(dstX, dstY, findNearestColor(r, g, b));
                }
            }
        }
        while (JpegDec.read());

        return true;
    }

    bool draw(ImageSource& image)
    {
        if (image.width() != 800 || image.height() != 480)
        {
            LOG_ERROR("Ungueltige Bildgroesse: %ux%u", image.width(), image.height());
            return false;
        }

        static uint16_t rgb565Line[800];

        display.setFullWindow();
        display.firstPage();

        do
        {
            for (uint16_t y = 0; y < image.height(); y++)
            {
                if (!image.readRGB565Line(y, rgb565Line, 800))
                {
                    LOG_ERROR("Fehler beim Lesen der Bildzeile %u", y);
                    return false;
                }

                display.drawRGBBitmap(0, y, rgb565Line, image.width(), 1);
            }
        }
        while (display.nextPage());

        return true;
    }

private:
    DisplayType& display;
    uint8_t paletteMode;

    const uint16_t PALETTE7[7] = {
        0x0000, 0xFFFF, 0xFFE0, 0xF800, 0x001F, 0x07E0, 0xFD20
    };

    const uint8_t bayer4x4[4][4] = {
        {0, 8, 2, 10},
        {12, 4, 14, 6},
        {3, 11, 1, 9},
        {15, 7, 13, 5}
    };

    uint16_t findNearestColor(uint8_t r, uint8_t g, uint8_t b)
    {
        uint16_t bestIndex = 0;
        uint32_t bestDiff = 999999;

        for (uint8_t i = 0; i < paletteMode; i++)
        {
            uint8_t pr = ((PALETTE7[i] >> 11) & 0x1F) << 3;
            uint8_t pg = ((PALETTE7[i] >> 5) & 0x3F) << 2;
            uint8_t pb = (PALETTE7[i] & 0x1F) << 3;

            int dr = r - pr;
            int dg = g - pg;
            int db = b - pb;

            uint32_t diff = dr * dr + dg * dg + db * db;

            if (diff < bestDiff)
            {
                bestDiff = diff;
                bestIndex = i;
            }
        }

        return PALETTE7[bestIndex];
    }
};
#pragma once
#include <Arduino.h>
#include <string.h> // für memcpy
#include <JPEGDecoder.h>
#include <SPIFFS.h>

template <typename DisplayType>
class ImageProcessing {
public:
    ImageProcessing(DisplayType& disp, uint8_t displayMode = 6);

    void drawBayerDitheredImage(const uint8_t* img, int w, int h);
    bool drawBayerDitheredJPEG(const char* path);
    bool drawJPEGtoDisplay(DisplayType& display, const char* path);

private:
    DisplayType& display;
    uint8_t PALETTE[7][3]; // bis zu 7 Farben

    void setPalette(uint8_t colors);
    uint16_t findNearestColor(uint8_t r, uint8_t g, uint8_t b);

    const uint8_t bayer8[8][8] = {
        { 0, 48, 12, 60,  3, 51, 15, 63},
        {32, 16, 44, 28, 35, 19, 47, 31},
        { 8, 56,  4, 52, 11, 59,  7, 55},
        {40, 24, 36, 20, 43, 27, 39, 23},
        { 2, 50, 14, 62,  1, 49, 13, 61},
        {34, 18, 46, 30, 33, 17, 45, 29},
        {10, 58,  6, 54,  9, 57,  5, 53},
        {42, 26, 38, 22, 41, 25, 37, 21}
    };
};

// ---------------- Implementierungen ----------------

template <typename DisplayType>
ImageProcessing<DisplayType>::ImageProcessing(DisplayType& disp, uint8_t displayMode)
    : display(disp)
{
    setPalette(displayMode);
}

template <typename DisplayType>
void ImageProcessing<DisplayType>::setPalette(uint8_t colors) {
    if (colors == 3) {
        static const uint8_t palette3[3][3] = {
            {0, 0, 0},       // Schwarz
            {255, 255, 255}, // Weiß
            {255, 0, 0}      // Rot
        };
        memcpy(PALETTE, palette3, sizeof(palette3));
    } 
    else if (colors == 7) {
        static const uint8_t palette7[7][3] = {
            {0,   0,   0},   // Schwarz
            {255, 255, 255}, // Weiß
            {255, 255, 0},   // Gelb
            {255, 0,   0},   // Rot
            {0,   0, 255},   // Blau
            {0, 255, 0},     // Grün
            {255, 128, 0}    // Orange
        };
        memcpy(PALETTE, palette7, sizeof(palette7));
    }
    else {
        static const uint8_t palette2[2][3] = {
            {0, 0, 0},       // Schwarz
            {255, 255, 255}  // Weiß
        };
        memcpy(PALETTE, palette2, sizeof(palette2));
    }
}

template <typename DisplayType>
uint16_t ImageProcessing<DisplayType>::findNearestColor(uint8_t r, uint8_t g, uint8_t b) {
    uint16_t bestColor = 0;
    uint32_t bestDiff = 999999;
    for (uint16_t i = 0; i < 7; i++) {
        int dr = r - PALETTE[i][0];
        int dg = g - PALETTE[i][1];
        int db = b - PALETTE[i][2];
        uint32_t diff = dr * dr + dg * dg + db * db;
        if (diff < bestDiff) {
            bestDiff = diff;
            bestColor = i;
        }
    }
    return bestColor;
}

template <typename DisplayType>
void ImageProcessing<DisplayType>::drawBayerDitheredImage(const uint8_t* img, int w, int h) {
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            int idx = (y * w + x) * 3;
            uint8_t r = img[idx];
            uint8_t g = img[idx + 1];
            uint8_t b = img[idx + 2];

            int threshold = bayer8[y % 8][x % 8] * 4; // 0–252

            r = (r > threshold) ? min(r + 32, 255) : r;
            g = (g > threshold) ? min(g + 32, 255) : g;
            b = (b > threshold) ? min(b + 32, 255) : b;

            uint16_t color = findNearestColor(r, g, b);
            display.drawPixel(x, y, color);
        }
    }
}

template <typename DisplayType>
bool ImageProcessing<DisplayType>::drawBayerDitheredJPEG(const char* path) {
    if (!SPIFFS.exists(path)) {
        Serial.println("JPEG-Datei nicht gefunden!");
        return false;
    }

    // Decode JPEG from SPIFFS
    if (JpegDec.decodeFsFile(path) != 0) {
        Serial.println("JPEG konnte nicht dekodiert werden!");
        return false;
    }

    // Image dimensions
    uint16_t w = JpegDec.width;
    uint16_t h = JpegDec.height;

    // Decode MCU by MCU
    while (JpegDec.read()) {
        uint16_t* pImg = JpegDec.pImage;

        for (uint16_t j = 0; j < JpegDec.MCUHeight; j++) {
            for (uint16_t i = 0; i < JpegDec.MCUWidth; i++) {
                uint16_t color565 = pImg[j * JpegDec.MCUWidth + i];

                // Convert 16-bit color to 8-bit RGB
                uint8_t r = ((color565 >> 11) & 0x1F) << 3;
                uint8_t g = ((color565 >> 5) & 0x3F) << 2;
                uint8_t b = (color565 & 0x1F) << 3;

                // Calculate absolute pixel coordinates
                int x = JpegDec.MCUx + i;
                int y = JpegDec.MCUy + j;

                // Apply Bayer dithering
                int threshold = bayer8[y % 8][x % 8] * 4; // 0–252
                r = (r > threshold) ? min(r + 32, 255) : r;
                g = (g > threshold) ? min(g + 32, 255) : g;
                b = (b > threshold) ? min(b + 32, 255) : b;

                // Map to nearest palette color
                uint16_t paletteColor = findNearestColor(r, g, b);
                display.drawPixel(x, y, paletteColor);
            }
        }
    }

    return true;
}

template <typename DisplayType>
bool drawJPEGtoDisplay(DisplayType& display, const char* path) {
    if (!SPIFFS.exists(path)) {
        Serial.println("JPEG-Datei nicht gefunden!");
        return false;
    }

    if (JpegDec.decodeFsFile(path) != 0) {
        Serial.println("JPEG konnte nicht dekodiert werden!");
        return false;
    }

    // MCU-blockweise zeichnen
    do {
        uint16_t *pImg = JpegDec.pImage;  // Zeiger auf den aktuellen MCU-Block
        uint16_t mcuX = JpegDec.MCUx;
        uint16_t mcuY = JpegDec.MCUy;
        uint16_t mcuW = JpegDec.MCUWidth;
        uint16_t mcuH = JpegDec.MCUHeight;

        for (uint16_t y = 0; y < mcuH; y++) {
            for (uint16_t x = 0; x < mcuW; x++) {
                uint16_t color565 = pImg[y * mcuW + x];

                // Optional: auf deine 7-Farben-Palette mappen
                uint8_t r = ((color565 >> 11) & 0x1F) << 3;
                uint8_t g = ((color565 >> 5) & 0x3F) << 2;
                uint8_t b = (color565 & 0x1F) << 3;
                uint16_t paletteColor = findNearestColor(r, g, b); // deine Funktion

                display.drawPixel(mcuX + x, mcuY + y, paletteColor);
            }
        }

    } while (JpegDec.read());  // zum nächsten MCU-Block

    return true;
}
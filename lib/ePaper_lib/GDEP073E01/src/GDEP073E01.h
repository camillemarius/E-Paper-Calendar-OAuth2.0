#pragma once

#include <GxEPD2_7C.h>
#include <SPI.h>
#include "ePaperDriver.h"

class GDEP073E01 : public EpaperDriver {
public:             
    GDEP073E01(uint8_t cs = 27, uint8_t dc = 14, uint8_t rst = 12, uint8_t busy = 13,
             uint8_t sck = 18, uint8_t miso = 19, uint8_t mosi = 23, uint8_t ss = 15);

    void init() override;
    void clear() override;
    void setRotation(int rotation) override;
    void setFullWindow() override;
    void setPartialWindow(int16_t x, int16_t y, int16_t w, int16_t h) override;
    void firstPage() override;
    bool nextPage() override;

    // Drawing
    void fillScreen(uint16_t color) override;
    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) override;
    void fillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color) override;
    void fillCircle(int16_t x, int16_t y, int16_t r, uint16_t color) override;

    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) override;
    void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) override;
    void drawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color) override;
    void drawCircle(int16_t x, int16_t y, int16_t r, uint16_t color) override;

    void drawBitmap(int16_t x, int16_t y, const uint8_t* bitmap,
                    int16_t w, int16_t h, uint16_t color) override;

    void drawRGBBitmap(int16_t x, int16_t y, uint16_t *bitmap,
                       int16_t w, int16_t h);

    void drawRGBBitmap(int16_t x, int16_t y, uint16_t *bitmap, uint8_t *mask,
                       int16_t w, int16_t h);

    void drawImage(const uint8_t bitmap[], int16_t x, int16_t y, int16_t w, int16_t h, bool invert = false, bool mirror_y = false, bool pgm = false);
    // Text
    void setTextColor(uint16_t color) override;
    void setTextColour(uint16_t color) override;
    void setFont(const GFXfont* font) override;
    void setCursor(int16_t x, int16_t y) override;
    void print(const char* text) override;
    void printAt(int16_t x, int16_t y, const char* text) override;
    virtual void getTextBounds(const String &str, int16_t x, int16_t y, int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h) override;
    void drawTextInRoundedRect(int16_t x, int16_t y, int16_t w, int16_t h, const String& text, uint16_t bgColor, uint16_t textColor, int16_t radius = 5, int16_t padding_left = 7, int16_t padding_top = 7, uint16_t borderColor = 0);
    void drawPixel(int16_t x, int16_t y, uint16_t color);

    // Display Info
    int16_t width() override;
    int16_t height() override;

private:
    uint8_t pinCS, pinDC, pinRST, pinBUSY;
    uint8_t pinSCK, pinMISO, pinMOSI, pinSS;

    GxEPD2_7C < GxEPD2_730c_GDEP073E01, GxEPD2_730c_GDEP073E01::HEIGHT / 6 > display;


};

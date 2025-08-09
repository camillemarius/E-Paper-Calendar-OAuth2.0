#pragma once

#include <GxEPD2_3C.h>
#include <SPI.h>
#include "ePaperDriver.h"

class GDEW075T7 : public EpaperDriver {
public:             
    GDEW075T7(uint8_t cs = 15, uint8_t dc = 27, uint8_t rst = 26, uint8_t busy = 25,
             uint8_t sck = 13, uint8_t miso = 12, uint8_t mosi = 14, uint8_t ss = 15);

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

    // Text
    void setTextColor(uint16_t color) override;
    void setTextColour(uint16_t color) override;
    void setFont(const GFXfont* font) override;
    void setCursor(int16_t x, int16_t y) override;
    void print(const char* text) override;
    void printAt(int16_t x, int16_t y, const char* text) override;
    virtual void getTextBounds(const String &str, int16_t x, int16_t y, int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h) override;

    // Display Info
    int16_t width() override;
    int16_t height() override;

private:
    uint8_t pinCS, pinDC, pinRST, pinBUSY;
    uint8_t pinSCK, pinMISO, pinMOSI, pinSS;

    GxEPD2_3C<GxEPD2_750c_Z08, GxEPD2_750c_Z08::HEIGHT/2> display;
};

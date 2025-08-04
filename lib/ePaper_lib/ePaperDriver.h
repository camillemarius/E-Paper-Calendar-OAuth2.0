#pragma once

#include <stdint.h>
#include <Adafruit_GFX.h>
#include <cstdint>

class EpaperDriver {
public:
    virtual void init() = 0;
    virtual void clear() = 0;
    virtual void setRotation(int rotation) = 0;
    virtual void setFullWindow() = 0;
    virtual void setPartialWindow(int16_t x, int16_t y, int16_t w, int16_t h) = 0;
    virtual void firstPage() = 0;
    virtual bool nextPage() = 0;

    // Shapes
    virtual void fillScreen(uint16_t color) = 0;
    virtual void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) = 0;
    virtual void fillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color) = 0;
    virtual void fillCircle(int16_t x, int16_t y, int16_t r, uint16_t color) = 0;

    virtual void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) = 0;
    virtual void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) = 0;
    virtual void drawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color) = 0;
    virtual void drawCircle(int16_t x, int16_t y, int16_t r, uint16_t color) = 0;

    virtual void drawBitmap(int16_t x, int16_t y, const uint8_t* bitmap,
                            int16_t w, int16_t h, uint16_t color) = 0;

    // Text
    virtual void setTextColor(uint16_t color) = 0;
    virtual void setTextColour(uint16_t color) = 0;  // Alias
    virtual void setFont(const GFXfont* font) = 0;
    virtual void setCursor(int16_t x, int16_t y) = 0;
    virtual void print(const char* text) = 0;
    virtual void printAt(int16_t x, int16_t y, const char* text) = 0;
    virtual void getTextBounds(const String &str, int16_t x, int16_t y, int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h);
    virtual void drawTextInRoundedRect(int16_t x, int16_t y, int16_t w, int16_t h, const String& text, uint16_t bgColor, uint16_t textColor, int16_t radius, int16_t padding_left, int16_t padding_top);
    

    // Display Info
    virtual int16_t width() = 0;
    virtual int16_t height() = 0;

    virtual ~EpaperDriver() = default;
};

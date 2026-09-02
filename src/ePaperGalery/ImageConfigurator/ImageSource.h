#pragma once
#include <Arduino.h>

class ImageSource
{
public:
    virtual ~ImageSource() = default;
    virtual uint16_t width() const = 0;
    virtual uint16_t height() const = 0;
    virtual bool readRGB565Line(uint16_t y, uint16_t* buffer, uint16_t bufferPixels) = 0;
};

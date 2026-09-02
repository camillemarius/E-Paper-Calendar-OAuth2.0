#pragma once

#include "ImageSource.h"
#include "ImagePalette.h"
#include <externalFlash.h>

class FlashImage : public ImageSource
{
public:
    FlashImage(externalFlash& flash, ImagePalette palette, uint32_t offset = 0)
        : flash(flash), paletteMode(palette), offset(offset) {}

    uint16_t width() const override { return 800; }
    uint16_t height() const override { return 480; }

    bool readRGB565Line(uint16_t y, uint16_t* buffer, uint16_t bufferPixels) override;

private:
    static constexpr uint16_t WIDTH = 800;
    static constexpr uint16_t HEIGHT = 480;
    static constexpr uint16_t PACKED_LINE_SIZE = WIDTH / 2;

    externalFlash& flash;
    ImagePalette paletteMode;
    uint32_t offset;
};
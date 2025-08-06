#pragma once

// Local
#include <ePaperDriver.h>

// Extenral Library
#include <Fonts/FreeSans18pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <qrcode.h>
#include "color.h"

class IDisplay {
public:
    virtual ~IDisplay() = default;
    void show(const String& qrData);

protected:
    IDisplay(EpaperDriver& display);

    // Abstrakte Inhalte, die Unterklassen bereitstellen müssen
    virtual String getTitle() const = 0;
    virtual String getDescription() const = 0;

    void drawHeader();
    void drawQRCode(const String& qrData);

    EpaperDriver& m_display;
};

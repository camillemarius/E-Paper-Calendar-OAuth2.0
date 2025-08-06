#include "WifiDisplay.h"
#include <logger.h>

// Local
#include "color.h"

// Internal Libraray


// External Library
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <qrcode.h>

#include "WifiDisplay.h"
#include <logger.h>

WifiDisplay::WifiDisplay(EpaperDriver& display)
    : IDisplay(display) {}

String WifiDisplay::getTitle() const {
    return "WLAN Einrichten";
}

String WifiDisplay::getDescription() const {
    return "Verbinde dich mit dem E-Paper Kalender\n"
           "indem du den QR-Code scannst.\n"
           "Beim Verbinden weist er dich darauf hin,\n"
           "dass du dich anmelden musst.\n"
           "Folge dem Link und verbinde dich über\n"
           "den Browser mit deinem WLAN.";
}
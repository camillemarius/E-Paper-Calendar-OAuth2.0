#include "autTimeoutDisplay.h"
#include <logger.h>

// Local
#include "color.h"

// Internal Libraray
#include <logger.h>


// External Library
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <qrcode.h>


AutTimeoutDisplay::AutTimeoutDisplay(EpaperDriver& display)
    : IDisplay(display) {}

String AutTimeoutDisplay::getTitle() const {
    return "Kein WLAN gesetzt";
}

String AutTimeoutDisplay::getDescription() const {
    return "Die Zeit fuer die Google-\n"
           "Authentifizierung ist abgelaufen.\n\n"
           "Der E-Paper-Kalender konnte nicht\n"
           "auf deinen Google Kalender\n"
           "zugreifen.\n\n"
           "Um es erneut zu versuchen,\n"
           "druecke den Knopf auf der\n"
           "Rueckseite kurz.";
}
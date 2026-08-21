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
    return "WLAN Konfiguration";
}

String WifiDisplay::getDescription() const {
    return "Oeffne die Kamera-App und scanne\n"
           "den QR-Code des E-Paper-Kalenders.\n\n"
           "Dein Handy verbindet sich\n"
           "automatisch mit dem Kalender und\n"
           "zeigt dir eine Meldung zur\n"
           "Autorisierung.\n\n"
           "Tippe auf die Meldung und waehle im\n"
           "Browser dein WLAN aus. Gib danach\n"
           "dein WLAN-Passwort ein.";
}
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
    return "Starte die Kamera-App auf deinem\n"
           "Handy und scanne den QR-Code\n"
           "des E-Paper Kalenders.\n"
           "Der QR-Code verbindet dein Handy\n"
           "automatisch mit dem Kalender.\n"
           "Dein Handy zeigt dir, dass du\n"
           "dich Autorisieren musst.\n" 
           "Tippe auf das Pop-Up und verbinde\n"
           "dich im Browser mit deinem WLAN,\n"
           "indem du den WLAN-Namen auswählst\n"
           "und das Passwort eingibst.";
}
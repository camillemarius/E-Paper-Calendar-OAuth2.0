#include "calendarSelectorDisplay.h"
#include <logger.h>

// Local
#include "color.h"

// Internal Libraray

// External Library
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <qrcode.h>


CalendarSelectorDisplay::CalendarSelectorDisplay(EpaperDriver& display)
    : IDisplay(display) {}

String CalendarSelectorDisplay::getTitle() const {
    return "Kalender Auswahl";
}

String CalendarSelectorDisplay::getDescription() const {
    return "Waehle aus, welche Google Kalender\n"
           "zukuenftig hier angezeigt werden\n"
           "sollen.\n\n"
           "Wichtig: Dein Telefon muss mit dem\n"
           "gleichen WLAN verbunden sein wie\n"
           "der E-Paper-Kalender.\n\n"
           "Scanne jetzt den QR-Code.";
}
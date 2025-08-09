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
    return "Google Kalender festlegen";
}

String CalendarSelectorDisplay::getDescription() const {
    return "Lege anhand der Liste fest, welche \n"
           "Google Kalender in der Zukunft\n"
           "hier angezeigt werden sollen.\n\n"
           "Dein Telefon, mit dem du den\n"
           "QR-Code scannst, muss dabei mit\n"
           "dem selben WLAN verbunden sein\n"
           "wie der E-Paper-Kalender.\n\n"
           "Scanne jetzt den QR-Code.";
}
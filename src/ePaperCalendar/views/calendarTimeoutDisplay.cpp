#include "calendarTimeoutDisplay.h"
#include <logger.h>

// Local
#include "color.h"

// Internal Libraray
#include <logger.h>


// External Library
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <qrcode.h>


CalendarTimeoutDisplay::CalendarTimeoutDisplay(EpaperDriver& display)
    : IDisplay(display) {}

String CalendarTimeoutDisplay::getTitle() const {
    return "Kalenderauswahl abgelaufen";
}

String CalendarTimeoutDisplay::getDescription() const {
    return "Es wurde kein Kalender innerhalb\n"
           "der vorgegebenen Zeit ausgewählt.\n\n"
           "Resete den Kalender über den\n"
           "blauen Button und warte, bis der\n"
           "QR-Code erneut angezeigt wird.\n"
           "Wähle dann deinen Kalender zur\n"
           "Synchronisierung aus.";
}

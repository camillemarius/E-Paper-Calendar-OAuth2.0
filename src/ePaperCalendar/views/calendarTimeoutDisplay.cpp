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
    return "Kalender Auswahl abgelaufen";
}

String CalendarTimeoutDisplay::getDescription() const {
    return "Es wurde kein Kalender ausgewählt.\n\n"
           "Drücke den Reset-Button und warte,\n"
           "bis der QR-Code wieder angezeigt\n"
           "wird.\n\n"
           "Wähle anschließend deinen Kalender\n"
           "zur Synchronisierung aus.";
}

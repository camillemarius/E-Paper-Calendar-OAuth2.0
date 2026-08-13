#include "credentialTimeoutDisplay.h"
#include <logger.h>

// Local
#include "color.h"

// Internal Libraray
#include <logger.h>


// External Library
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <qrcode.h>


CredentialTimeoutDisplay::CredentialTimeoutDisplay(EpaperDriver& display)
    : IDisplay(display) {}

String CredentialTimeoutDisplay::getTitle() const {
    return "Konfiguration abgelaufen";
}

String CredentialTimeoutDisplay::getDescription() const {
    return "Es wurde keine WLAN-Verbindung\n"
           "eingerichtet. Der Konfigurations-\n"
           "modus ist abgelaufen.\n\n"
           "Drücke den Reset-Button und warte,\n"
           "bis der QR-Code wieder erscheint.\n\n"
           "Scanne den QR-Code erneut, um\n"
           "deine WLAN-Daten einzugeben.";
}
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
    return "Kein WLAN gesetzt";
}

String CredentialTimeoutDisplay::getDescription() const {
    return "Es wurde keine WLAN-Verbindung\n"
           "eingestellt. Der Konfigurations-\n"
           "modus ist abgelaufen.\n\n"
           "Starte den Kalender mit dem\n"
           "blauen Button neu und warte,\n"
           "bis wieder ein QR-Code angezeigt\n"
           "wird. Scanne den QR-Code erneut,\n"
           "um deine WLAN-Daten einzugeben.";
}
#include "AuthDisplay.h"
#include <logger.h>

// Local
#include "color.h"

// Internal Libraray

// External Library
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <qrcode.h>

AuthDisplay::AuthDisplay(EpaperDriver& display)
    : IDisplay(display) {}


String AuthDisplay::getTitle() const {
    return "Google Kalender Einrichten";
}

String AuthDisplay::getDescription() const {
    return "Scanne den QR-Code,\n"
           "um dich bei Google\n"
           "zu authentifizieren.\n\n"
           "Gib dazu den Code\n"
           "auf der Webseite ein.";
}

void AuthDisplay::showWithUserCode(const String& qrData, const String& userCode) {
    m_display.firstPage();
    do {
        drawQRCode(qrData);
        drawHeader();
        drawUserCode(userCode);
    } while (m_display.nextPage());
}

void AuthDisplay::drawUserCode(const String& code) {
    LOG_INFO("Draw user code: %s", code.c_str());

    m_display.setFont(&FreeSans12pt7b);
    m_display.setTextColor(COLOR_BLACK);

    String text = "Device Code: " + code;

    // Gleiche Position wie die Beschreibung in drawHeader()
    const int scale = 7;
    QRCode qrcode;
    uint8_t qrcodeBytes[qrcode_getBufferSize(6)];

    qrcode_initText(&qrcode, qrcodeBytes, 6, ECC_LOW, " ");

    int qrPixelSize = qrcode.size * scale;

    const int leftMargin = 40;
    int textX = leftMargin + qrPixelSize + 10;

    int textY = m_display.height() - 60;

    m_display.setCursor(textX, textY);
    m_display.print(text.c_str());
}
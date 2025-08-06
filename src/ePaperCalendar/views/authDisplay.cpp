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
           "zu authentifizieren.\n"
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

    const char* label = "Device Code:";
    int16_t x1, y1;
    uint16_t labelWidth, labelHeight, codeWidth, codeHeight;

    // "Device Code:" Label ausmessen und zentrieren
    m_display.getTextBounds(label, 0, 0, &x1, &y1, &labelWidth, &labelHeight);
    int labelX = (m_display.width() - labelWidth) / 2;
    int labelY = m_display.height() - 80;  // unten etwas Abstand
    m_display.setCursor(labelX, labelY);
    m_display.print(label);

    // User Code ausmessen und darunter zentrieren
    m_display.getTextBounds(code, 0, 0, &x1, &y1, &codeWidth, &codeHeight);
    int codeX = (m_display.width() - codeWidth) / 2;
    int codeY = labelY + labelHeight + 20;
    m_display.setCursor(codeX, codeY);
    m_display.print(code.c_str());
}
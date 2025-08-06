// Local
#include "IDisplay.h"

// Internal Library

// External Library

IDisplay::IDisplay(EpaperDriver& display)
    : m_display(display) {
}

void IDisplay::show(const String& qrData) {
    m_display.firstPage();
    do {
        drawHeader();
        drawQRCode(qrData);
    } while (m_display.nextPage());
}

void IDisplay::drawHeader() {
    m_display.setTextColor(COLOR_BLACK);

    // QR-Code Größeninfos
    int scale = 7;
    QRCode qrcode;
    uint8_t qrcodeBytes[qrcode_getBufferSize(6)];
    qrcode_initText(&qrcode, qrcodeBytes, 6, ECC_LOW, " "); // Dummy-Init nur für Größe
    int size = qrcode.size;
    int qrPixelSize = size * scale;

    // QR-Code y-Offset (zentriert vertikal)
    int yOffset = (m_display.height() - qrPixelSize) / 2;

    // QR-Code x-Offset mit Abstand links
    const int leftMargin = 40;

    // Text startet rechts vom QR-Code mit kleinem Abstand
    int textX = leftMargin + qrPixelSize + 10;
    int textY = yOffset + 30;  // etwas höher, 30 px unter QR-Code oben

    // Titel mit größerer Schrift
    m_display.setFont(&FreeSans18pt7b);
    m_display.setCursor(textX, textY);
    m_display.print(getTitle().c_str());

    // Beschreibung (mehrzeilig) mit größerer Schrift und mehr Abstand
    m_display.setFont(&FreeSans12pt7b);
    int lineHeight = 24;
    textY += 35;  // größerer Abstand unter Titel

    String desc = getDescription();
    int16_t x1, y1;
    uint16_t w, h;

    int start = 0;
    while (start < desc.length()) {
        int end = desc.indexOf('\n', start);
        if (end == -1) end = desc.length();
        String line = desc.substring(start, end);
        m_display.getTextBounds(line, 0, 0, &x1, &y1, &w, &h);
        m_display.setCursor(textX, textY);
        m_display.print(line.c_str());
        textY += lineHeight;
        start = end + 1;
    }
}

void IDisplay::drawQRCode(const String& qrData) {
    QRCode qrcode;
    uint8_t qrcodeBytes[qrcode_getBufferSize(6)];
    qrcode_initText(&qrcode, qrcodeBytes, 6, ECC_LOW, qrData.c_str());

    int scale = 7;
    int size = qrcode.size;
    int qrPixelSize = size * scale;

    // QR-Code x-Offset etwas weiter links mit Abstand
    const int leftMargin = 40;
    const int yOffset = (m_display.height() - qrPixelSize) / 2;
    const int xOffset = leftMargin;

    for (int y = 0; y < size; y++) {
        for (int x = 0; x < size; x++) {
            int color = qrcode_getModule(&qrcode, x, y) ? COLOR_BLACK : COLOR_WHITE;
            m_display.fillRect(xOffset + x * scale, yOffset + y * scale, scale, scale, color);
        }
    }
}
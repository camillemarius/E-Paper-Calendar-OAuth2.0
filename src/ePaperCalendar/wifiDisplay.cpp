#include "WifiDisplay.h"
#include <logger.h>

// Local
#include "color.h"

// Internal Libraray
#include "qr_code_bitmap.h"

// External Library
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <qrcode.h>


WifiDisplay::WifiDisplay(EpaperDriver& display)
    : IDisplay(display) {}

void WifiDisplay::show(const String& url) {
    m_display.firstPage();
    do {
        drawQRCode(url);
    } while (m_display.nextPage());
}

void WifiDisplay::drawHeader() {
    const char* headerText = "Bitte QR-Code scannen";

    m_display.setFont(&FreeSans12pt7b);
    m_display.setTextColor(COLOR_BLACK);

    int16_t x1, y1;
    uint16_t textWidth, textHeight;

    m_display.getTextBounds(headerText, 0, 0, &x1, &y1, &textWidth, &textHeight);

    int x = (m_display.width() - textWidth) / 2;
    int y = textHeight + 5; // Leichter Abstand von oben

    m_display.setCursor(x, y);
    m_display.print(headerText);
}

void WifiDisplay::drawQRCode(const String& url) {
    LOG_INFO("Draw QR code for: %s", url.c_str());
    
    int x = (m_display.width() - QR_CODE_WIDTH) / 2;
    int y = ((m_display.height() - QR_CODE_HEIGHT) / 2) - 40;

    // QR-Code initialisieren
    QRCode qrcode;
    uint8_t qrcodeBytes[qrcode_getBufferSize(6)];
    qrcode_initText(&qrcode, qrcodeBytes, 6, ECC_LOW, url.c_str());

    // Anzeige
    int scale = 7;
    int size = qrcode.size;
    int qrPixelSize = size * scale;

    int xOffset = (300 - qrPixelSize) / 2;
    int yOffset = (300 - qrPixelSize) / 2;

    for (int y = 0; y < size; y++) {
        for (int x = 0; x < size; x++) {
            if (qrcode_getModule(&qrcode, x, y)) {
                m_display.fillRect(xOffset + x * scale, yOffset + y * scale, scale, scale, COLOR_BLACK);
            } else {
                m_display.fillRect(xOffset + x * scale, yOffset + y * scale, scale, scale, COLOR_WHITE);
            }
        }
    }


}

void WifiDisplay::drawUserCode(const String& code) {
    LOG_INFO("Draw user code: %s", code.c_str());

    m_display.setFont(&FreeSans12pt7b);
    m_display.setTextColor(COLOR_BLACK);

    const char* label = "Device Code:";
    int16_t x1, y1;
    uint16_t labelWidth, labelHeight, codeWidth, codeHeight;

    // "Code:" label zeichnen
    m_display.getTextBounds(label, 0, 0, &x1, &y1, &labelWidth, &labelHeight);
    int labelX = (m_display.width() - labelWidth) / 2;
    int labelY = m_display.height() - 80;
    m_display.setCursor(labelX, labelY);
    m_display.print(label);

    // Code unterhalb des Labels zeichnen
    m_display.getTextBounds(code, 0, 0, &x1, &y1, &codeWidth, &codeHeight);
    int codeX = (m_display.width() - codeWidth) / 2;
    int codeY = labelY + labelHeight + 20;
    m_display.setCursor(codeX, codeY);
    m_display.print(code.c_str());
}

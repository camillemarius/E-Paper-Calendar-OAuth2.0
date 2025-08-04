#include "calendarSelectorDisplay.h"
#include <logger.h>

// Local
#include "color.h"

// Internal Libraray
#include "qr_code_bitmap.h"

// External Library
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <qrcode.h>


CalendarSelectorDisplay::CalendarSelectorDisplay(EpaperDriver& display)
    : IDisplay(display) {}

void CalendarSelectorDisplay::show(const String& url) {
    m_display.firstPage();
    do {
        drawQRCode(url);
    } while (m_display.nextPage());
}

void CalendarSelectorDisplay::drawQRCode(const String& url) {
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

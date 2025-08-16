#include "GDEP073E01.h"

//Internal lib
#include <logger.h>

#include <vector>


GDEP073E01::GDEP073E01(uint8_t cs, uint8_t dc, uint8_t rst, uint8_t busy,
                      uint8_t sck, uint8_t miso, uint8_t mosi, uint8_t ss)
    : pinCS(cs), pinDC(dc), pinRST(rst), pinBUSY(busy),
      pinSCK(sck), pinMISO(miso), pinMOSI(mosi), pinSS(ss),
      display(GxEPD2_730c_GDEP073E01(/*CS=D8*/ cs, /*DC=D3*/ dc, /*RST=D4*/ rst, /*BUSY=D2*/ busy))
{
}

void GDEP073E01::init() {
    LOG_DEBUG("Init FPC8612 Display and SPI");
    display.init(115200);
    SPI.end();
    SPI.begin(pinSCK, pinMISO, pinMOSI, pinSS);
}

void GDEP073E01::clear() {
    LOG_DEBUG("Clear the screen");
    display.setFullWindow();
    display.firstPage();
    do {
        display.fillScreen(GxEPD_WHITE);
    } while (display.nextPage());
}

void GDEP073E01::setRotation(int rotation) {
    LOG_DEBUG("Rotate the screen to %d", rotation);
    display.setRotation(rotation);
}

void GDEP073E01::setFullWindow() {
    LOG_DEBUG("set Full Window");
    display.setFullWindow();
}

void GDEP073E01::setPartialWindow(int16_t x, int16_t y, int16_t w, int16_t h) {
    display.setPartialWindow(x, y, w, h);
}

void GDEP073E01::firstPage() {
    display.firstPage();
}

bool GDEP073E01::nextPage() {
    return display.nextPage();
}

void GDEP073E01::fillScreen(uint16_t color) {
    display.fillScreen(color);
}

void GDEP073E01::fillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color) {
    display.fillRoundRect(x, y, w, h, r, color);
}

void GDEP073E01::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    display.fillRect(x, y, w, h, color);
}

void GDEP073E01::fillCircle(int16_t x, int16_t y, int16_t r, uint16_t color) {
    display.fillCircle(x, y, r, color);
}

void GDEP073E01::drawCircle(int16_t x, int16_t y, int16_t r, uint16_t color) {
    display.drawCircle(x, y, r, color);
}

void GDEP073E01::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    display.drawRect(x, y, w, h, color);
}

void GDEP073E01::drawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color) {
    display.drawRoundRect(x, y, w, h, r, color);
}

void GDEP073E01::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
    display.drawLine(x0, y0, x1, y1, color);
}

void GDEP073E01::drawBitmap(int16_t x, int16_t y, const uint8_t* bitmap,
                          int16_t w, int16_t h, uint16_t color) {
    display.drawBitmap(x, y, bitmap, w, h, color);
}

void GDEP073E01::drawRGBBitmap(int16_t x, int16_t y, uint16_t *bitmap,
                               int16_t w, int16_t h) {
    display.drawRGBBitmap(x,y,bitmap,w,h);
}

void GDEP073E01::drawRGBBitmap(int16_t x, int16_t y, uint16_t *bitmap, uint8_t *mask,
                               int16_t w, int16_t h) {
    display.drawRGBBitmap(x,y,bitmap,mask,w,h);
}

void GDEP073E01::drawImage(const uint8_t bitmap[], int16_t x, int16_t y, int16_t w, int16_t h, bool invert, bool mirror_y, bool pgm) {
    display.drawImage(bitmap,x,y,w,h,invert,mirror_y,pgm);
}

void GDEP073E01::setTextColor(uint16_t color) {
    display.setTextColor(color);
}

void GDEP073E01::setTextColour(uint16_t color) {
    setTextColor(color);  // Alias
}

void GDEP073E01::setFont(const GFXfont* font) {
    display.setFont(font);
}

void GDEP073E01::setCursor(int16_t x, int16_t y) {
    display.setCursor(x, y);
}

void GDEP073E01::print(const char* text) {
    display.print(text);
}

void GDEP073E01::printAt(int16_t x, int16_t y, const char* text) {
    display.setCursor(x, y);
    display.print(text);
}

int16_t GDEP073E01::width() {
    return display.width();
}

int16_t GDEP073E01::height() {
    return display.height();
}

void GDEP073E01::drawPixel(int16_t x, int16_t y, uint16_t color) {
    display.drawPixel(x, y, color);
}

void GDEP073E01::getTextBounds(const String &str, int16_t x, int16_t y, int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h) {
    display.getTextBounds(str,x,y,x1,y1,w,h);
}

void GDEP073E01::drawTextInRoundedRect(int16_t x, int16_t y, int16_t w, int16_t h,  
                                     const String& text, uint16_t bgColor, uint16_t textColor,
                                     int16_t radius, int16_t padding_left, int16_t padding_top,
                                     uint16_t borderColor) {
    // Hintergrund-Rounded-Rect zeichnen
    fillRoundRect(x, y, w, h, radius, bgColor);

    // Umrandung zeichnen, falls borderColor gesetzt
    drawRoundRect(x, y, w, h, radius, borderColor);

    setTextColor(textColor);

    int16_t x1, y1;
    uint16_t textW, textH;

    // Zeilenhöhe anhand "Ay" bestimmen (für Ober- und Unterlänge)
    getTextBounds("Ay", 0, 0, &x1, &y1, &textW, &textH);

    int16_t maxTextWidth = w - 2 * padding_left;
    int16_t maxTextHeight = h - 2 * padding_top;

    int maxLines = maxTextHeight / textH; // maximale Anzahl Zeilen
    if (maxLines < 1) maxLines = 1;

    int16_t cursorX = x + padding_left;
    int16_t cursorY = y + padding_top + textH;

    String remainingText = text;
    std::vector<String> lines; // Zeilen puffern

    // --- 1) Zeilen vorbereiten ---
    for (int lineNum = 0; lineNum < maxLines && remainingText.length() > 0; lineNum++) {
        String line = remainingText;
        getTextBounds(line, 0, 0, &x1, &y1, &textW, &textH);
        // Kürze line so, dass sie in maxTextWidth passt
        while (textW > maxTextWidth && line.length() > 0) {
            line.remove(line.length() - 1);
            getTextBounds(line, 0, 0, &x1, &y1, &textW, &textH);
        }

        if (line.length() == 0) break;

        // Prüfe, ob nächstes ganzes Wort noch reinpasst (erweitern)
        int nextWordEnd = remainingText.indexOf(' ', line.length());
        if (nextWordEnd == -1) nextWordEnd = remainingText.length();

        String testLine = remainingText.substring(0, nextWordEnd);
        getTextBounds(testLine, 0, 0, &x1, &y1, &textW, &textH);

        if (textW <= maxTextWidth) {
            line = testLine; // nächstes Wort passt noch komplett
        } else {
            // Versuche Umbruch an letztem Leerzeichen oder Bindestrich
            int lastSpace = line.lastIndexOf(' ');
            int lastDash  = line.lastIndexOf('-');
            int splitPos  = (lastSpace > lastDash) ? lastSpace : lastDash;
            if (splitPos > 0) {
                line = line.substring(0, splitPos);
                // Trim Ende-Leerzeichen
                while (line.length() > 0 && line.charAt(line.length() - 1) == ' ')
                    line.remove(line.length() - 1);
            }
        }

        // Letzte Zeile und noch mehr Text: mit "..." kürzen
        if (lineNum == maxLines - 1 && remainingText.length() > line.length()) {
            const String ell = "...";

            // Starte mit maximal möglichem Text (line) ohne Kürzung
            String base = line;

            // Versuche base + "..." passen zu lassen
            getTextBounds(base + ell, 0, 0, &x1, &y1, &textW, &textH);

            while (textW > maxTextWidth && base.length() > 0) {
                base.remove(base.length() - 1);  // letzte char entfernen
                // Entferne ggf. Leerzeichen am Ende
                while (base.length() > 0 && base.charAt(base.length() - 1) == ' ')
                    base.remove(base.length() - 1);

                getTextBounds(base + ell, 0, 0, &x1, &y1, &textW, &textH);
            }

            // Wenn gar nichts passt, zeige nur "..."
            if (base.length() == 0) {
                getTextBounds(ell, 0, 0, &x1, &y1, &textW, &textH);
                if (textW <= maxTextWidth) {
                    line = ell;
                } else {
                    line = "";  // gar nichts oder alternativ "..."
                }
            } else {
                line = base + ell;
            }
        }

        lines.push_back(line);

        // Schneide schon abgezeichneten Text ab
        remainingText = remainingText.substring(line.length());
    }

    // --- 2) Zeichnen ---
    for (const auto& l : lines) {
        setCursor(cursorX, cursorY);
        print(l.c_str());
        cursorY += textH;
    }
}


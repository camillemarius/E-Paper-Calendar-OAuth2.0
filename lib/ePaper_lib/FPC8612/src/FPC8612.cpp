#include "FPC8612.h"

//Internal lib
#include <logger.h>


FPC_8612::FPC_8612(uint8_t cs, uint8_t dc, uint8_t rst, uint8_t busy,
                   uint8_t sck, uint8_t miso, uint8_t mosi, uint8_t ss)
    : pinCS(cs), pinDC(dc), pinRST(rst), pinBUSY(busy),
      pinSCK(sck), pinMISO(miso), pinMOSI(mosi), pinSS(ss),
      display(GxEPD2_750c_Z08(cs, dc, rst, busy))
{
}

void FPC_8612::init() {
    LOG_DEBUG("Init FPC8612 Display and SPI");
    display.init(115200);
    SPI.end();
    SPI.begin(pinSCK, pinMISO, pinMOSI, pinSS);
}

void FPC_8612::clear() {
    LOG_DEBUG("Clear the screen");
    display.setFullWindow();
    display.firstPage();
    do {
        display.fillScreen(GxEPD_WHITE);
    } while (display.nextPage());
}

void FPC_8612::setRotation(int rotation) {
    LOG_DEBUG("Rotate the screen to %d", rotation);
    display.setRotation(rotation);
}

void FPC_8612::setFullWindow() {
    LOG_DEBUG("set Full Window");
    display.setFullWindow();
}

void FPC_8612::setPartialWindow(int16_t x, int16_t y, int16_t w, int16_t h) {
    display.setPartialWindow(x, y, w, h);
}

void FPC_8612::firstPage() {
    display.firstPage();
}

bool FPC_8612::nextPage() {
    return display.nextPage();
}

void FPC_8612::fillScreen(uint16_t color) {
    display.fillScreen(color);
}

void FPC_8612::fillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color) {
    display.fillRoundRect(x, y, w, h, r, color);
}

void FPC_8612::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    display.fillRect(x, y, w, h, color);
}

void FPC_8612::fillCircle(int16_t x, int16_t y, int16_t r, uint16_t color) {
    display.fillCircle(x, y, r, color);
}

void FPC_8612::drawCircle(int16_t x, int16_t y, int16_t r, uint16_t color) {
    display.drawCircle(x, y, r, color);
}

void FPC_8612::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    display.drawRect(x, y, w, h, color);
}

void FPC_8612::drawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color) {
    display.drawRoundRect(x, y, w, h, r, color);
}

void FPC_8612::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
    display.drawLine(x0, y0, x1, y1, color);
}

void FPC_8612::drawBitmap(int16_t x, int16_t y, const uint8_t* bitmap,
                          int16_t w, int16_t h, uint16_t color) {
    display.drawBitmap(x, y, bitmap, w, h, color);
}

void FPC_8612::setTextColor(uint16_t color) {
    display.setTextColor(color);
}

void FPC_8612::setTextColour(uint16_t color) {
    setTextColor(color);  // Alias
}

void FPC_8612::setFont(const GFXfont* font) {
    display.setFont(font);
}

void FPC_8612::setCursor(int16_t x, int16_t y) {
    display.setCursor(x, y);
}

void FPC_8612::print(const char* text) {
    display.print(text);
}

void FPC_8612::printAt(int16_t x, int16_t y, const char* text) {
    display.setCursor(x, y);
    display.print(text);
}

int16_t FPC_8612::width() {
    return display.width();
}

int16_t FPC_8612::height() {
    return display.height();
}

void FPC_8612::getTextBounds(const String &str, int16_t x, int16_t y, int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h) {
    display.getTextBounds(str,x,y,x1,y1,w,h);
}

/*void FPC_8612::drawTextInRoundedRect(int16_t x, int16_t y, int16_t w, int16_t h,
                                     const String& text, uint16_t bgColor, uint16_t textColor,
                                     int16_t radius, int16_t padding_left, int16_t padding_top,
                                     uint16_t borderColor) {
    // Hintergrund-Rounded-Rect zeichnen
    fillRoundRect(x, y, w, h, radius, bgColor);

     // Falls borderColor gesetzt (ungleich 0), Umrandung zeichnen
     // Umrandung nur zeichnen, wenn borderColor NICHT 0xFFFF ist
    //if (borderColor != 0xFFFF) {
        drawRoundRect(x, y, w, h, radius, borderColor);
    //}

    setTextColor(textColor);

    // Textgröße bestimmen
    int16_t x1, y1;
    uint16_t textW, textH;
    String toDraw = text;
    getTextBounds(toDraw, x + padding_left, y + padding_top, &x1, &y1, &textW, &textH);

    // Text kürzen, falls zu breit für den Bereich (w - padding_left*2)
    int16_t maxTextWidth = w - 2 * padding_left;
    while (textW > maxTextWidth && toDraw.length() > 0) {
        toDraw.remove(toDraw.length() - 1);
        getTextBounds(toDraw, x + padding_left, y + padding_top, &x1, &y1, &textW, &textH);
    }

    // "..." anhängen, wenn gekürzt
    if (toDraw.length() < text.length()) {
        if (toDraw.length() > 3) {
            toDraw.remove(toDraw.length() - 3);
            toDraw += "...";
        }
    }

    // Cursor auf linke obere Position mit Padding
    setCursor(x + padding_left, y + padding_top + textH);
    print(toDraw.c_str());
}*/

void FPC_8612::drawTextInRoundedRect(int16_t x, int16_t y, int16_t w, int16_t h, 
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

    // Zeilenhöhe bestimmen anhand "Ay" (für Ober- und Unterlänge)
    getTextBounds("Ay", 0, 0, &x1, &y1, &textW, &textH);

    int16_t maxTextWidth = w - 2 * padding_left;
    int16_t maxTextHeight = h - 2 * padding_top;

    int maxLines = maxTextHeight / textH; // maximale Anzahl Zeilen die reinpassen
    if (maxLines < 1) maxLines = 1;       // mindestens 1 Zeile erlauben

    int16_t cursorX = x + padding_left;
    int16_t cursorY = y + padding_top + textH;

    String remainingText = text;

    for (int lineNum = 0; lineNum < maxLines && remainingText.length() > 0; lineNum++) {
        // Versuche den größtmöglichen Text zu finden, der in maxTextWidth passt
        String line = remainingText;
        getTextBounds(line, 0, 0, &x1, &y1, &textW, &textH);

        while (textW > maxTextWidth && line.length() > 0) {
            line.remove(line.length() - 1);
            getTextBounds(line, 0, 0, &x1, &y1, &textW, &textH);
        }

        if (line.length() == 0) break;  // Nichts passt, abbrechen

        // Versuche, den Umbruch möglichst an Leerzeichen oder Bindestrichen vorzunehmen
        int lastSpace = line.lastIndexOf(' ');
        int lastDash = line.lastIndexOf('-');
        int splitPos = (lastSpace > lastDash) ? lastSpace : lastDash;

        if (splitPos > 0 && splitPos < line.length() - 1) {
            // Prüfe, ob nächstes Wort hinter splitPos noch komplett in die Zeile passt
            int nextWordStart = splitPos + 1;
            // Suche Ende des nächsten Wortes (bis Leerzeichen oder Ende des Textes)
            int nextWordEnd = remainingText.indexOf(' ', nextWordStart);
            if (nextWordEnd == -1) nextWordEnd = remainingText.length();

            String nextWord = remainingText.substring(nextWordStart, nextWordEnd);
            String testLine = line.substring(0, splitPos) + remainingText.substring(splitPos, nextWordEnd);

            getTextBounds(testLine, 0, 0, &x1, &y1, &textW, &textH);

            if (textW <= maxTextWidth) {
                // Das nächste Wort passt noch, also erweitere die Zeile bis zum Ende dieses Wortes
                line = remainingText.substring(0, nextWordEnd);
            } else {
                // Das nächste Wort passt nicht mehr, also umbrechen am splitPos
                line = remainingText.substring(0, splitPos);
            }
        }

        // Wenn dies die letzte erlaubte Zeile ist, prüfen ob noch Text übrig ist
        if (lineNum == maxLines - 1 && remainingText.length() > line.length()) {
            if (line.length() > 3) {
                line.remove(line.length() - 3);
                line += "...";
            }
        }

        // Zeile zeichnen
        setCursor(cursorX, cursorY);
        print(line.c_str());

        // Resttext ermitteln (Zeile vom Anfang abschneiden)
        remainingText = remainingText.substring(line.length());

        // Cursor für nächste Zeile verschieben
        cursorY += textH;
    }
}



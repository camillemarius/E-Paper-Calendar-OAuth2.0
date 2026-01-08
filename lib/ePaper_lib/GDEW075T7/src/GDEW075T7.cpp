#include "GDEW075T7.h"

// Internal Library
#include <logger.h>

// External Library
#include <vector>


GDEW075T7::GDEW075T7(uint8_t cs, uint8_t dc, uint8_t rst, uint8_t busy,
                   uint8_t sck, uint8_t miso, uint8_t mosi, uint8_t ss)
    : pinCS(cs), pinDC(dc), pinRST(rst), pinBUSY(busy),
      pinSCK(sck), pinMISO(miso), pinMOSI(mosi), pinSS(ss),
      display(GxEPD2_750c(cs, dc, rst, busy))
{
}

void GDEW075T7::init() {
    LOG_DEBUG("Init GDEW075T7 Display and SPI");
    display.init(115200);
    SPI.end();
    SPI.begin(pinSCK, pinMISO, pinMOSI, pinSS);
}

void GDEW075T7::clear() {
    LOG_DEBUG("Clear the screen");
    display.setFullWindow();
    display.firstPage();
    do {
        display.fillScreen(GxEPD_WHITE);
    } while (display.nextPage());
}

void GDEW075T7::setRotation(int rotation) {
    LOG_DEBUG("Rotate the screen to %d", rotation);
    display.setRotation(rotation);
}

void GDEW075T7::setFullWindow() {
    LOG_DEBUG("set Full Window");
    display.setFullWindow();
}

void GDEW075T7::setPartialWindow(int16_t x, int16_t y, int16_t w, int16_t h) {
    display.setPartialWindow(x, y, w, h);
}

void GDEW075T7::firstPage() {
    display.firstPage();
}

bool GDEW075T7::nextPage() {
    return display.nextPage();
}

void GDEW075T7::fillScreen(uint16_t color) {
    display.fillScreen(color);
}

void GDEW075T7::fillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color) {
    display.fillRoundRect(x, y, w, h, r, color);
}

void GDEW075T7::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    display.fillRect(x, y, w, h, color);
}

void GDEW075T7::fillCircle(int16_t x, int16_t y, int16_t r, uint16_t color) {
    display.fillCircle(x, y, r, color);
}

void GDEW075T7::drawCircle(int16_t x, int16_t y, int16_t r, uint16_t color) {
    display.drawCircle(x, y, r, color);
}

void GDEW075T7::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    display.drawRect(x, y, w, h, color);
}

void GDEW075T7::drawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color) {
    display.drawRoundRect(x, y, w, h, r, color);
}

void GDEW075T7::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
    display.drawLine(x0, y0, x1, y1, color);
}

void GDEW075T7::drawBitmap(int16_t x, int16_t y, const uint8_t* bitmap,
                          int16_t w, int16_t h, uint16_t color) {
    display.drawBitmap(x, y, bitmap, w, h, color);
}

void GDEW075T7::setTextColor(uint16_t color) {
    display.setTextColor(color);
}

void GDEW075T7::setTextColour(uint16_t color) {
    setTextColor(color);  // Alias
}

void GDEW075T7::setFont(const GFXfont* font) {
    display.setFont(font);
}

void GDEW075T7::setCursor(int16_t x, int16_t y) {
    display.setCursor(x, y);
}

void GDEW075T7::print(const char* text) {
    display.print(text);
}

void GDEW075T7::printAt(int16_t x, int16_t y, const char* text) {
    display.setCursor(x, y);
    display.print(text);
}

int16_t GDEW075T7::width() {
    return display.width();
}

int16_t GDEW075T7::height() {
    return display.height();
}

void GDEW075T7::getTextBounds(const String &str, int16_t x, int16_t y, int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h) {
    display.getTextBounds(str,x,y,x1,y1,w,h);
}

void GDEW075T7::drawRGBBitmap(int16_t x, int16_t y, uint16_t *bitmap,
                               int16_t w, int16_t h) {
    display.drawRGBBitmap(x,y,bitmap,w,h);
}

String replaceUmlauts(String s) {
    s.replace("ä","ae");
    s.replace("ö","oe");
    s.replace("ü","ue");
    s.replace("Ä","Ae");
    s.replace("Ö","Oe");
    s.replace("Ü","Ue");
    s.replace("ß","ss");
    return s;
}

void GDEW075T7::drawTextInRoundedRect(int16_t x, int16_t y, int16_t w, int16_t h,   
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

    int16_t maxTextWidth  = w - 2 * padding_left;
    int16_t maxTextHeight = h - 2 * padding_top;

    int maxLines = maxTextHeight / textH; 
    if (maxLines < 1) maxLines = 1;

    int16_t cursorX = x + padding_left;
    int16_t cursorY = y + padding_top + textH;

    String remainingText = replaceUmlauts(text);
    std::vector<String> lines; 

    // --- 1) Zeilen vorbereiten (wortweise) ---
    for (int lineNum = 0; lineNum < maxLines && remainingText.length() > 0; lineNum++) {
        String line;
        String testLine;
        uint16_t lineW, lineH;  

        while (remainingText.length() > 0) {
            // Nächstes Wort finden
            int nextSpace = remainingText.indexOf(' ');
            String word;
            if (nextSpace == -1) {
                word = remainingText; // letztes Wort
            } else {
                word = remainingText.substring(0, nextSpace + 1); // inkl. Leerzeichen
            }

            // Teste, ob das Wort allein überhaupt passt
            getTextBounds(word, 0, 0, &x1, &y1, &lineW, &lineH);
            if (line.length() == 0 && lineW > maxTextWidth) {
                // --- Wort passt alleine nicht -> kürzen + "..." ---
                const String ell = "...";
                String base = word;
                getTextBounds(base + ell, 0, 0, &x1, &y1, &lineW, &lineH);
                while (lineW > maxTextWidth && base.length() > 0) {
                    base.remove(base.length() - 1);
                    getTextBounds(base + ell, 0, 0, &x1, &y1, &lineW, &lineH);
                }
                line = (base.length() > 0) ? base + ell : ell;
                remainingText.remove(0, word.length()); // Rest vom Wort abschneiden
                break; // Zeile abschließen
            }

            // Teste, ob dieses Wort noch in die Zeile passt
            testLine = line + word;
            getTextBounds(testLine, 0, 0, &x1, &y1, &lineW, &lineH);

            if (lineW <= maxTextWidth) {
                // passt noch, übernehmen
                line = testLine;
                remainingText.remove(0, word.length());
            } else {
                // passt nicht -> Zeile abschließen
                break;
            }
        }

        // Falls letzte Zeile: ggf. "..." anhängen
        if (lineNum == maxLines - 1 && remainingText.length() > 0) {
            const String ell = "...";
            String base = line;
            uint16_t ellW, ellH;

            getTextBounds(base + ell, 0, 0, &x1, &y1, &ellW, &ellH);
            while (ellW > maxTextWidth && base.length() > 0) {
                base.remove(base.length() - 1);
                getTextBounds(base + ell, 0, 0, &x1, &y1, &ellW, &ellH);
            }
            line = (base.length() > 0) ? base + ell : ell;
            remainingText = "";
        }

        lines.push_back(line);
    }

    // --- 2) Zeichnen ---
    for (const auto& l : lines) {
        setCursor(cursorX, cursorY);
        print(l.c_str());
        cursorY += textH;
    }
}


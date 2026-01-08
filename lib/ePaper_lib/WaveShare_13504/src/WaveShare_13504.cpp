#include "WaveShare_13504.h"

//Internal lib
#include <logger.h>

#include <vector>


WaveShare_13504::WaveShare_13504(uint8_t cs, uint8_t dc, uint8_t rst, uint8_t busy,
                   uint8_t sck, uint8_t miso, uint8_t mosi, uint8_t ss)
    : pinCS(cs), pinDC(dc), pinRST(rst), pinBUSY(busy),
      pinSCK(sck), pinMISO(miso), pinMOSI(mosi), pinSS(ss),
      //display(GxEPD2_750_T7(cs, dc, rst, busy))
      display(GxEPD2_750_GDEY075T7(cs, dc, rst, busy))
{
}

void WaveShare_13504::init() {
    LOG_DEBUG("Init FPC8612 Display and SPI");
    display.init(115200);
    SPI.end();
    SPI.begin(pinSCK, pinMISO, pinMOSI, pinSS);
}

void WaveShare_13504::clear() {
    LOG_DEBUG("Clear the screen");
    display.setFullWindow();
    display.firstPage();
    do {
        display.fillScreen(GxEPD_WHITE);
    } while (display.nextPage());
}

void WaveShare_13504::setRotation(int rotation) {
    LOG_DEBUG("Rotate the screen to %d", rotation);
    display.setRotation(rotation);
}

void WaveShare_13504::setFullWindow() {
    LOG_DEBUG("set Full Window");
    display.setFullWindow();
}

void WaveShare_13504::setPartialWindow(int16_t x, int16_t y, int16_t w, int16_t h) {
    display.setPartialWindow(x, y, w, h);
}

void WaveShare_13504::firstPage() {
    display.firstPage();
}

bool WaveShare_13504::nextPage() {
    return display.nextPage();
}

void WaveShare_13504::fillScreen(uint16_t color) {
    display.fillScreen(color);
}

void WaveShare_13504::fillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color) {
    display.fillRoundRect(x, y, w, h, r, color);
}

void WaveShare_13504::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    display.fillRect(x, y, w, h, color);
}

void WaveShare_13504::fillCircle(int16_t x, int16_t y, int16_t r, uint16_t color) {
    display.fillCircle(x, y, r, color);
}

void WaveShare_13504::drawCircle(int16_t x, int16_t y, int16_t r, uint16_t color) {
    display.drawCircle(x, y, r, color);
}

void WaveShare_13504::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    display.drawRect(x, y, w, h, color);
}

void WaveShare_13504::drawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color) {
    display.drawRoundRect(x, y, w, h, r, color);
}

void WaveShare_13504::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
    display.drawLine(x0, y0, x1, y1, color);
}

void WaveShare_13504::drawBitmap(int16_t x, int16_t y, const uint8_t* bitmap,
                          int16_t w, int16_t h, uint16_t color) {
    display.drawBitmap(x, y, bitmap, w, h, color);
}

void WaveShare_13504::drawRGBBitmap(int16_t x, int16_t y, uint16_t *bitmap,
                               int16_t w, int16_t h) {
    display.drawRGBBitmap(x,y,bitmap,w,h);
}

void WaveShare_13504::drawRGBBitmap(int16_t x, int16_t y, uint16_t *bitmap, uint8_t *mask,
                               int16_t w, int16_t h) {
    display.drawRGBBitmap(x,y,bitmap,mask,w,h);
}

void WaveShare_13504::drawImage(const uint8_t bitmap[], int16_t x, int16_t y, int16_t w, int16_t h, bool invert, bool mirror_y, bool pgm) {
    display.drawImage(bitmap,x,y,w,h,invert,mirror_y,pgm);
}

void WaveShare_13504::setTextColor(uint16_t color) {
    display.setTextColor(color);
}

void WaveShare_13504::setTextColour(uint16_t color) {
    setTextColor(color);  // Alias
}

void WaveShare_13504::setFont(const GFXfont* font) {
    display.setFont(font);
}

void WaveShare_13504::setCursor(int16_t x, int16_t y) {
    display.setCursor(x, y);
}

void WaveShare_13504::print(const char* text) {
    display.print(text);
}

void WaveShare_13504::printAt(int16_t x, int16_t y, const char* text) {
    display.setCursor(x, y);
    display.print(text);
}

int16_t WaveShare_13504::width() {
    return display.width();
}

int16_t WaveShare_13504::height() {
    return display.height();
}

void WaveShare_13504::getTextBounds(const String &str, int16_t x, int16_t y, int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h) {
    display.getTextBounds(str,x,y,x1,y1,w,h);
}

/*void WaveShare_13504::drawTextInRoundedRect(int16_t x, int16_t y, int16_t w, int16_t h,
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


/*void WaveShare_13504::drawTextInRoundedRect(int16_t x, int16_t y, int16_t w, int16_t h,  
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
}*/

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

void WaveShare_13504::drawTextInRoundedRect(int16_t x, int16_t y, int16_t w, int16_t h,   
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




#include "GDEW075T7.h"

//Internal lib
#include <logger.h>


GDEW075T7::GDEW075T7(uint8_t cs, uint8_t dc, uint8_t rst, uint8_t busy,
                   uint8_t sck, uint8_t miso, uint8_t mosi, uint8_t ss)
    : pinCS(cs), pinDC(dc), pinRST(rst), pinBUSY(busy),
      pinSCK(sck), pinMISO(miso), pinMOSI(mosi), pinSS(ss),
      display(GxEPD2_750c_Z08(cs, dc, rst, busy))
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

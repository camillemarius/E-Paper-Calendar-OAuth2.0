// Local
#include "ImageConfigurator/WebUploader.h"
#include "ImageConfigurator/FlashImage.h"
#include "views/imageView.h"
#include "views/IMG_20250621_093707_POP_OUT.h"
#include "views/IMG_20250621_093656.h"
#include "views/IMG_20250712_154110.h"

// Internal Lib
#include <ePaperDriver.h>
#include <logger.h>

// External Libraries
#include <SPIFFS.h>
#include <externalFlash.h>
#include <WebServer.h>
#include <qrcode.h>
#include <esp_sleep.h>
#include <driver/gpio.h>
#include <Fonts/FreeSansBold18pt7b.h>
#include <Fonts/FreeSans12pt7b.h>

WebServer server(80);

// Universal-Platine: SPI auf IO18/19/23 für Displays und Flash U6 (W25Q16JV, CS_Flash an IO4).
// Stecker _24 (7,5"): CS 27, D/C 25, RES 26, BUSY 32. Stecker _50 (7,3"): CS 16, D/C 5, RES 17, BUSY 33.
// DISPLAY_ID muss zu den Display-Typen der Webseite passen (WebUploaderHtml.cpp, DISPLAYS)
#ifdef GALLERY_UNIVERSALDRIVER_CACH_GDEP073E01
    #define FLASH_CS 4
    #define DISPLAY_ID "GDEP073E01"
    #include <GDEP073E01.h>
    GDEP073E01 display(16, 5, 17, 33, 18, 19, 23, 16);
    ImageView<GDEP073E01> viewer(display, 7);
    constexpr ImagePalette DISPLAY_PALETTE = ImagePalette::SixColor;

#elif defined(GALLERY_UNIVERSALDRIVER_CACH_FPC8612)
    #define FLASH_CS 4
    #define DISPLAY_ID "FPC8612"
    #include <FPC8612.h>
    FPC_8612 display(27, 25, 26, 32, 18, 19, 23, 27);
    ImageView<FPC_8612> viewer(display, 3);
    constexpr ImagePalette DISPLAY_PALETTE = ImagePalette::ThreeColor;

#elif defined(GALLERY_UNIVERSALDRIVER_CACH_DEPG0750RWF86BF)
    #define FLASH_CS 4
    #define DISPLAY_ID "DEPG0750RWF86BF"
    #include <DEPG0750RWF86BF.h>
    DEPG0750RWF86BF display(27, 25, 26, 32, 18, 19, 23, 27);
    ImageView<DEPG0750RWF86BF> viewer(display, 3);
    constexpr ImagePalette DISPLAY_PALETTE = ImagePalette::ThreeColor;

#elif defined(GALLERY_UNIVERSALDRIVER_CACH_WAVESHARE_13504)
    #define FLASH_CS 4
    #define DISPLAY_ID "WAVESHARE_13504"
    #include <WaveShare_13504.h>
    WaveShare_13504 display(27, 25, 26, 32, 18, 19, 23, 27);
    ImageView<WaveShare_13504> viewer(display, 2);
    constexpr ImagePalette DISPLAY_PALETTE = ImagePalette::TwoColor;

#elif defined(GALLERY_V1DRIVER_FPC8612)
    #define FLASH_CS 2
    #define DISPLAY_ID "FPC8612"
    #include <FPC8612.h>
    FPC_8612 display(15, 27, 26, 25, 13, 12, 14, 15);
    ImageView<FPC_8612> viewer(display, 3);
    constexpr ImagePalette DISPLAY_PALETTE = ImagePalette::ThreeColor;

#elif defined(GALLERY_V1DRIVER_GDEW075T7)
    // 640 x 384: die Galerie arbeitet mit 800 x 480, die Webseite sperrt den Upload
    #define FLASH_CS 2
    #define DISPLAY_ID "GDEW075T7"
    #include <GDEW075T7.h>
    GDEW075T7 display(15, 27, 26, 25, 13, 12, 14, 15);
    ImageView<GDEW075T7> viewer(display, 3);
    constexpr ImagePalette DISPLAY_PALETTE = ImagePalette::ThreeColor;

#elif defined(GALLERY_V1DRIVER_GDEP073E01)
    #define FLASH_CS 2
    #define DISPLAY_ID "GDEP073E01"
    #include <GDEP073E01.h>
    GDEP073E01 display(27, 14, 12, 13, 18, 19, 23, 15);
    ImageView<GDEP073E01> viewer(display, 3);
    constexpr ImagePalette DISPLAY_PALETTE = ImagePalette::SixColor;

#elif defined(GALLERY_V1DRIVER_WAVESHARE_13504)
    #define FLASH_CS 2
    #define DISPLAY_ID "WAVESHARE_13504"
    #include <WaveShare_13504.h>
    WaveShare_13504 display(15, 27, 26, 25, 13, 12, 14, 15);
    ImageView<WaveShare_13504> viewer(display, 2);
    constexpr ImagePalette DISPLAY_PALETTE = ImagePalette::TwoColor;
#endif

// External Flash
externalFlash myFlash(FLASH_CS, 0x000000);

WebUploader uploader(server, myFlash, DISPLAY_PALETTE, DISPLAY_ID);

// Bedienung über den Taster S4 (IO2), einzige Weckquelle (kein Timer):
//   Reset oder kurz drücken -> gespeichertes Bild neu zeichnen, danach Deep Sleep
//   lang drücken (5 s)       -> Upload-Modus: WLAN "EPaper", http://192.168.4.1
// Ohne gespeichertes Bild startet direkt der Upload-Modus.
#define BUTTON_PIN 2
#define LONG_PRESS_MS 5000
#ifndef UPLOAD_TIMEOUT_MS
#define UPLOAD_TIMEOUT_MS (5UL * 60UL * 1000UL)   // ohne Upload zurück in den Deep Sleep
#endif
#define AP_SSID "EPaper"
#define AP_PASSWORD "12345678"

enum class Wakeup { Reset, ShortPress, LongPress };

static bool uploadDone = false;

void onUpload(externalFlash& flash, size_t len, ImagePalette palette) {
    LOG_DEBUG("EPD Daten im Flash: %u Bytes", (unsigned)len);
    if (len != 192000) {
        LOG_ERROR("FEHLER: Erwartet 192000 Bytes!");
        return;
    }
    FlashImage image(flash, palette);
    LOG_DEBUG("Zeige Bild aus externem Flash...");
    viewer.draw(image);
    LOG_DEBUG("Bildanzeige abgeschlossen!");
    uploadDone = true;
}

Wakeup detectWakeup()
{
    pinMode(BUTTON_PIN, INPUT_PULLUP);
#ifdef GALLERY_TEST_LONG_PRESS
    return Wakeup::LongPress;   // Test: Upload-Modus ohne Tastendruck
#endif
    if (esp_sleep_get_wakeup_cause() != ESP_SLEEP_WAKEUP_EXT0)
        return Wakeup::Reset;

    const unsigned long start = millis();
    while (digitalRead(BUTTON_PIN) == LOW)
    {
        if (millis() - start >= LONG_PRESS_MS)
            return Wakeup::LongPress;
        delay(20);
    }
    return Wakeup::ShortPress;
}

// Ein gültiges Bild enthält nur Bytes mit zwei 3-Bit-Farbindizes (<= 0x3F).
// Gelöschter Flash (0xFF) fällt damit heraus.
bool hasStoredImage()
{
    uint8_t line[400];
    if (!myFlash.readImage(0, line, sizeof(line)))
        return false;
    for (uint8_t b : line)
    {
        if (b > 0x3F)
            return false;
    }
    return true;
}

void drawStoredImage()
{
    FlashImage image(myFlash, DISPLAY_PALETTE);
    LOG_DEBUG("Zeige gespeichertes Bild...");
    viewer.draw(image);
}

void drawUploadHint()
{
    QRCode qr;
    uint8_t qrData[qrcode_getBufferSize(4)];
    qrcode_initText(&qr, qrData, 4, ECC_LOW, "WIFI:T:WPA;S:" AP_SSID ";P:" AP_PASSWORD ";;");

    const int scale = 8;
    const int qrX = 40;
    const int qrY = (display.height() - qr.size * scale) / 2;
    const int textX = qrX + qr.size * scale + 40;

    display.setFullWindow();
    display.firstPage();
    do
    {
        display.fillScreen(0xFFFF);
        for (uint8_t y = 0; y < qr.size; y++)
            for (uint8_t x = 0; x < qr.size; x++)
                if (qrcode_getModule(&qr, x, y))
                    display.fillRect(qrX + x * scale, qrY + y * scale, scale, scale, 0x0000);

        display.setTextColor(0x0000);
        display.setFont(&FreeSansBold18pt7b);
        display.printAt(textX, qrY + 30, "Bild hochladen");
        display.setFont(&FreeSans12pt7b);
        display.printAt(textX, qrY + 90, "1. QR-Code scannen oder WLAN");
        display.printAt(textX, qrY + 120, "   " AP_SSID " (Passwort " AP_PASSWORD ")");
        display.printAt(textX, qrY + 160, "2. Im Browser 192.168.4.1");
        display.printAt(textX, qrY + 200, "3. Bild auswaehlen, hochladen");
        display.printAt(textX, qrY + 250, "Ohne Upload schlaeft das Geraet");
        display.printAt(textX, qrY + 280, "nach 5 Minuten wieder.");
    }
    while (display.nextPage());
}

void runUploadMode()
{
    LOG_DEBUG("Upload-Modus");
    myFlash.test();
    drawUploadHint();

    WiFi.mode(WIFI_AP);
    WiFi.softAP(AP_SSID, AP_PASSWORD);
    LOG_DEBUG("Access Point %s, IP %s", AP_SSID, WiFi.softAPIP().toString().c_str());

    uploader.setUploadCallback(onUpload);
    uploader.begin();
    server.begin();

    const unsigned long start = millis();
    while (!uploadDone && millis() - start < UPLOAD_TIMEOUT_MS)
    {
        server.handleClient();
        delay(2);
    }

    // Antwort an den Browser noch ausliefern, bevor das WLAN aus geht
    const unsigned long finish = millis();
    while (uploadDone && millis() - finish < 1000)
    {
        server.handleClient();
        delay(2);
    }

    server.stop();
    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_OFF);

    if (!uploadDone)
    {
        LOG_DEBUG("Kein Upload, Upload-Modus beendet");
        if (hasStoredImage())
            drawStoredImage();
    }
}

void goToSleep()
{
    // Vor hibernate() loggen: der Pin-Hold friert die Pads ein, danach kommt nichts mehr an
    LOG_DEBUG("Deep Sleep bis zum naechsten Tastendruck");
    Serial.flush();

    // Flash U6 in Deep Power-Down; CS_Flash hat keinen Pull-up und wird im Deep Sleep High gehalten
    myFlash.sleep();
    if (FLASH_CS != BUTTON_PIN)
    {
        digitalWrite(FLASH_CS, HIGH);
        pinMode(FLASH_CS, OUTPUT);
        gpio_hold_en((gpio_num_t)FLASH_CS);
    }

    display.hibernate();                            // Controller in Deep Sleep, Leitungen gehalten
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_2, 0);    // nur der Taster weckt
    esp_deep_sleep_start();
}

void setup()
{
    Serial.begin(115200);

    // CS_Flash sofort High, bevor SPI benutzt wird (Pegel vor dem Lösen des Holds setzen)
    if (FLASH_CS != BUTTON_PIN)
    {
        digitalWrite(FLASH_CS, HIGH);
        pinMode(FLASH_CS, OUTPUT);
        gpio_hold_dis((gpio_num_t)FLASH_CS);
    }

    const Wakeup wakeup = detectWakeup();

    if (!myFlash.begin()) {
        LOG_ERROR("External Flash konnte nicht initialisiert werden!");
    }

    display.init();

    if (wakeup == Wakeup::LongPress || !hasStoredImage())
    {
        runUploadMode();
    }
    else
    {
        LOG_DEBUG(wakeup == Wakeup::ShortPress ? "Taster kurz: Bild neu zeichnen" : "Reset: Bild neu zeichnen");
        drawStoredImage();
    }

    goToSleep();

    /* //////////////////////////////////////////////////////// WORKING
    display.firstPage();
    do {
        IMG_20250621_093707_POP_OUT
        display.drawRGBBitmap(0, 0, const_cast<uint16_t*>(IMG_20260130_191741), 800, 480);
        display.drawRGBBitmap(0, 0, const_cast<uint16_t*>(E6_Vespa_3c), 640, 384);
    } while (display.nextPage());
    //////////////////////////////////////////////////////// WORKING */
}

void loop()
{
    // nicht erreicht: setup() endet im Deep Sleep
}

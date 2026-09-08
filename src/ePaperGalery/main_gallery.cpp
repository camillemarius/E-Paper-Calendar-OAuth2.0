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

WebServer server(80);

// External Flash
#define FLASH_CS 2
externalFlash myFlash(FLASH_CS, 0x000000);

#ifdef GALLERY_UNIVERSALDRIVER_CACH_GDEP073E01
    #include <GDEP073E01.h>
    GDEP073E01 display(16, 5, 17, 33, 18, 19, 23, 16);
    ImageView<GDEP073E01> viewer(display, 7);
    constexpr ImagePalette DISPLAY_PALETTE = ImagePalette::SixColor;

#elif defined(GALLERY_UNIVERSALDRIVER_CACH_FPC8612)
    #include <FPC8612.h>
    FPC_8612 display(15, 27, 26, 25, 13, 12, 14, 15);
    ImageView<FPC_8612> viewer(display, 3);
    constexpr ImagePalette DISPLAY_PALETTE = ImagePalette::ThreeColor;

#elif defined(GALLERY_V1DRIVER_FPC8612)
    #include <FPC8612.h>
    FPC_8612 display(15, 27, 26, 25, 13, 12, 14, 15);
    ImageView<FPC_8612> viewer(display, 3);

#elif defined(GALLERY_V1DRIVER_GDEW075T7)
    #include <GDEW075T7.h>
    GDEW075T7 display(15, 27, 26, 25, 13, 12, 14, 15);
    ImageView<GDEW075T7> viewer(display, 3);

#elif defined(GALLERY_V1DRIVER_GDEP073E01)
    #include <GDEP073E01.h>
    GDEP073E01 display(27, 14, 12, 13, 18, 19, 23, 15);
    ImageView<GDEP073E01> viewer(display, 3);

#elif defined(GALLERY_V1DRIVER_WAVESHARE_13504)
    #include <WaveShare_13504.h>
    WaveShare_13504 display(15, 27, 26, 25, 13, 12, 14, 15);
    ImageView<WaveShare_13504> viewer(display, 2);
#endif

WebUploader uploader(server, myFlash, DISPLAY_PALETTE);

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
}

void setup()
{
    Serial.begin(115200);

    if (!myFlash.begin()) {
        LOG_ERROR("External Flash konnte nicht initialisiert werden!");
    } else {
        LOG_DEBUG("External Flash bereit.");
        myFlash.test();
    }

    display.init();

    WiFi.mode(WIFI_AP);
    WiFi.softAP("EPaper", "12345678");

    Serial.print("ESP32 IP: ");
    Serial.println(WiFi.softAPIP());

    uploader.setUploadCallback(onUpload);
    uploader.begin();

    server.begin();
    LOG_DEBUG("Webserver gestartet!");
    
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
    server.handleClient();
}

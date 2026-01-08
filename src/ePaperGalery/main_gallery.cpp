

// Local
#include "ImageConfigurator/WebUploader.h"

#include "views/imageView.h"
#include "views/IMG_20250621_093707_POP_OUT.h"
#include "views/IMG_20250621_093656.h"
#include "views/IMG_20250712_154110.h"
#include "views/IMG_20250101_133528.h"
#include "views/IMG_20250423_185506.h"
#include "views/Snapchat_913704275.h"
#include "views/E6_Vespa.h"
#include "views/E6_Vespa_bw.h"
#include "views/E6_Vespa_bw_v2.h"
#include "views/E6_Vespa_bw_640_384.h"
#include "views/E6_Vespa_3C_640_384.h"
#include "views/IMG_2223.h"
#include "views/IMG_2223_BW.h"
#include "views/IMG_20171230_000229_224.h"
#include "views/E6_Vespa_3c.h"
//#include "views/DSCN1745_bw.h"




//Internal Lib
#include <ePaperDriver.h>
#include <logger.h>

// External Libraries
#include <SPIFFS.h>
#include <WebServer.h>


#ifdef GALLERY_UNIVERSALDRIVER_CACH_GDEP073E01
    #include <GDEP073E01.h>
    //display(GxEPD2_730c_GDEP073E01(/*CS=D8*/ cs, /*DC=D3*/ dc, /*RST=D4*/ rst, /*BUSY=D2*/ busy))
    //SPI.begin(pinSCK, pinMISO, pinMOSI, pinSS);
    GDEP073E01 display(16, 5, 17, 33, 18, 19, 23, 16);

    ImageView<GDEP073E01> viewer(display, 7);
    //#include <externalFlash.h>
    //#define FLASH_CS 2   // Your W25Q16 chip-select pin
    //externalFlash myFlash(FLASH_CS, 0x0000);  // base address 0
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


// --- WLAN Daten ---
//const char* ssid = "";
//const char* password = "";

//WebServer server(80);
//WebUploader uploader(server);

/*void onUpload(uint8_t* jpegData, size_t len) {
    Serial.printf("JPEG empfangen: %d Bytes\n", len);
    // Zeige direkt auf ePaper
    if (viewer.showJPEG(jpegData, len)) {
        LOG_DEBUG("Bild erfolgreich angezeigt!");
        //display.refresh(); // ePaper aktualisieren
    } else {
        LOG_DEBUG("Fehler beim Anzeigen!");
    }
}*/

/*void sleepUntilOneAM() {
    time_t now = time(nullptr);
    struct tm timeinfo;
    localtime_r(&now, &timeinfo);

    // Aktuelle Zeit debuggen
    LOG_DEBUG("Current local time before sleep: %04d-%02d-%02d %02d:%02d:%02d",
        timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
        timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);

    // Berechne Zeitpunkt der nächsten 1 Uhr nachts
    timeinfo.tm_hour = 1;
    timeinfo.tm_min = 0;
    timeinfo.tm_sec = 0;

    time_t wakeupTime = mktime(&timeinfo);
    if (wakeupTime <= now) {
        // Wenn 1 Uhr heute schon vorbei ist, auf morgen 1 Uhr setzen
        wakeupTime += 24 * 3600; 
    }

    time_t sleepSeconds = wakeupTime - now;

    LOG_DEBUG("Going to sleep for %ld seconds until 1 AM", sleepSeconds);

    // Tiefschlaf starten (in Mikrosekunden)
    esp_sleep_enable_timer_wakeup((uint64_t)sleepSeconds * 1000000ULL);
    esp_deep_sleep_start();
}*/


void setup() {

   Serial.begin(115200);

   //if (myFlash.begin()) {
        //myFlash.test();
        //delay(20);  // optional small pause
    //}


    // --- WLAN verbinden ---
    // WiFi.begin(ssid, password);
    // while (WiFi.status() != WL_CONNECTED) {
    //     delay(500);
    //     Serial.print(".");
    // }
    // Serial.println("\nWLAN verbunden! IP: " + WiFi.localIP().toString());

    // --- Display initialisieren ---
    display.init();

    // --- WebUploader konfigurieren ---
    // uploader.setUploadCallback(onUpload);
    // uploader.begin();

    // server.begin();
    // Serial.println("Webserver gestartet!");

    /*const uint32_t IMG_OFFSET = 0x0000;
    const size_t IMG_SIZE = sizeof(IMG_20250101_133528);  // exact size of packed 6-color data
    static uint8_t buffer[IMG_SIZE];  // buffer to read back


    if (!myFlash.writeImage(IMG_OFFSET, reinterpret_cast<const uint8_t*>(IMG_20250101_133528), IMG_SIZE)) {
        Serial.println("Write to flash failed!");
    }

    // Read image back
    if (!myFlash.readImage(IMG_OFFSET, buffer, IMG_SIZE)) {
        Serial.println("Read from flash failed!");
    }

    display.firstPage();
    do {
        display.drawRGBBitmap(0, 0, reinterpret_cast<uint16_t*>(buffer), 800, 480);
    } while (display.nextPage());*/

  
   ////////////////////////////////////////////////////////WORKING
    display.firstPage();
    do {
        //IMG_20250621_093707_POP_OUT
        display.drawRGBBitmap(0, 0, const_cast<uint16_t*>(IMG_20250101_133528), 800, 480);
        //display.drawRGBBitmap(0, 0, const_cast<uint16_t*>(E6_Vespa_3c), 640, 384);
    } while (display.nextPage());
   ////////////////////////////////////////////////////////WORKING


}

void loop() {
    // Example: write and read a number
    //uint8_t value = 42;
    //myFlash.writeData(100, &value, 1);

    //uint8_t readVal = 0;
    //myFlash.readData(100, &readVal, 1);

    //LOG_DEBUG("Stored: %d | Read: %d\n", value, readVal);
    //delay(2000);
    //server.handleClient();
}

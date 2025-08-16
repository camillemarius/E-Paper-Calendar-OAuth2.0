

// Local
#include "ImageConfigurator/WebUploader.h"

#include "views/imageView.h"
#include "views/IMG_20250621_093707_POP_OUT.h"
#include "views/IMG_20250621_093656.h"
#include "views/IMG_20250712_154110.h"

//Internal Lib
#include <GDEP073E01.h>
#include <ePaperDriver.h>
#include <logger.h>

// External Libraries
#include <SPIFFS.h>
#include <WebServer.h>


#ifdef E_PAPER_GALLERY_GDEP073E01
    GDEP073E01 display(27, 14, 12, 13, 18, 19, 23, 15);
    ImageView<GDEP073E01> viewer(display, 7);
//#elif defined()

#endif

// --- WLAN Daten ---
const char* ssid = "";
const char* password = "";

WebServer server(80);
WebUploader uploader(server);

void onUpload(uint8_t* jpegData, size_t len) {
    Serial.printf("JPEG empfangen: %d Bytes\n", len);
    // Zeige direkt auf ePaper
    if (viewer.showJPEG(jpegData, len)) {
        LOG_DEBUG("Bild erfolgreich angezeigt!");
        //display.refresh(); // ePaper aktualisieren
    } else {
        LOG_DEBUG("Fehler beim Anzeigen!");
    }
}

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


void sleepUntilOneAM() {
  time_t now = time(nullptr);
  struct tm timeinfo;
  localtime_r(&now, &timeinfo);

  LOG_DEBUG("Current local time before sleep: %04d-%02d-%02d %02d:%02d:%02d\n",
      timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
      timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);

  // Zeitpunkt der nächsten 1 Uhr
  timeinfo.tm_hour = 1;
  timeinfo.tm_min  = 0;
  timeinfo.tm_sec  = 0;

  time_t wakeupTime = mktime(&timeinfo);
  if (wakeupTime <= now) {
    wakeupTime += 24 * 3600; // morgen 1 Uhr
  }

  time_t sleepSeconds = wakeupTime - now;
  
  LOG_DEBUG("Going to sleep for %ld seconds until 1 AM\n", sleepSeconds);

  ESP.deepSleep((uint64_t)sleepSeconds * 1000000ULL);
}

void setup() {

   Serial.begin(115200);

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
  
   ////////////////////////////////////////////////////////WORKING
    display.firstPage();
    do {
        //display.drawImage(IMG_20250621_093707_POP_OUT, 0, 0, 800, 480, false, false, true);
        display.drawRGBBitmap(0, 0, const_cast<uint16_t*>(IMG_20250712_154110), 800, 480);
    } while (display.nextPage());
   ////////////////////////////////////////////////////////WORKING

   
    sleepUntilOneAM();
}

void loop() {
    //server.handleClient();
}

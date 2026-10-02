
// System
#include <WiFi.h>
#include <SPI.h>
#include <esp_sleep.h>
#include <driver/gpio.h>

// External Libraries
#include <qrcode.h>
#include <GoogleCalendar.h>
#include <GoogleAuth.h>
#include <logger.h>
//#include <BatteryGauge.h>

// Internal Libraries
#include "credentials.h"
#include "wifiHandler.h"

// Local UI
#include "CalendarConfigurator/CalendarConfigurator.h"
#include "views/weeklyCalendarDisplay.h"
#include "views/authDisplay.h"
#include "views/authTimeoutDisplay.h"
#include "views/credentialTimeoutDisplay.h"
#include "views/wifiDisplay.h"
#include "views/calendarTimeoutDisplay.h"
#include "views/calendarSelectorDisplay.h"

WiFiHandler wifiHandler(180);

#ifdef GOOGLECALENDAR_UNIVERSALDRIVERCACH_DEPG0750RWF86BF
    #include <DEPG0750RWF86BF.h>
    DEPG0750RWF86BF epaperDisplay(27, 25, 26, 32, 18, 19, 23, 27);
#elif defined(GOOGLECALENDAR_UNIVERSALDRIVERCACH_FPC8612)
    #include <FPC8612.h>
    FPC_8612 epaperDisplay(27, 25, 26, 32, 18, 19, 23, 27);
#elif defined(GOOGLECALENDAR_UNIVERSALDRIVERCACH_GDEW075T7)
    #include <GDEW075T7.h>
    GDEW075T7 epaperDisplay(27, 25, 26, 32, 18, 19, 23, 27);
#elif defined(GOOGLECALENDAR_UNIVERSALDRIVERCACH_GDEP073E01)
    #include <GDEP073E01.h>
    GDEP073E01 epaperDisplay(27, 25, 26, 32, 18, 19, 23, 27);
    
#elif defined(GOOGLECALENDAR_V1DRIVER_FPC8612)
    #include <FPC8612.h>
    FPC_8612 epaperDisplay(15, 27, 26, 25, 13, 12, 14, 15);
#elif defined(GOOGLECALENDAR_V1DRIVER_GDEW075T7)
    #include <GDEW075T7.h>
    GDEW075T7 epaperDisplay(15, 27, 26, 25, 13, 12, 14, 15);
#elif defined(GOOGLECALENDAR_V1DRIVER_GDEP073E01)
    #include <GDEP073E01.h>
    GDEP073E01 epaperDisplay(27, 14, 12, 13, 18, 19, 23, 27);
#endif

#define TIMEZONE "CET-1CEST,M3.5.0/2,M10.5.0/3"   // Schweizer Zeitzone
#define LED_PIN 32   // GPIO32
#define BUTTON_PIN 2
#define EXT_FLASH_CS BUTTON_PIN   // Universal-Platine: CS des Galerie-Flash, geteilt mit dem Knopf
#define LONG_BUTTON_PRESS_TIME 5000   // 2 Sekunden

enum class WakeupReason {
    NormalReset,
    ShortButton2Press,
    LongButton2Press,
    Timer,
    Other
};
WakeupReason wakeupReason = WakeupReason::NormalReset;

// Google & Calendar
GoogleAuth auth(_clientId, _clientSecret, _scope);
GoogleCalendar calendar(auth);
CalendarConfigurator calendarConfigurator(calendar);

// UI
WeeklyCalendar weeklyCalendar(epaperDisplay);
AuthDisplay authDisplay(epaperDisplay);
AuthTimeoutDisplay authTimeoutDisplay(epaperDisplay);
CredentialTimeoutDisplay credentialTimeoutDisplay(epaperDisplay);
WifiDisplay wifiDisplay(epaperDisplay);
CalendarTimeoutDisplay calendarTimeoutDisplay(epaperDisplay);
CalendarSelectorDisplay calendarSelectorDisplay(epaperDisplay);

// Funktion: Light Sleep, solange der Display-Controller BUSY meldet
// Ersetzt das Polling von GxEPD2 (delay(1)) während des 18–24 s langen Refreshs.
// Nur ohne WLAN; geweckt wird beim Pegelwechsel an BUSY, spätestens nach 1 s,
// damit der Busy-Timeout von GxEPD2 wirksam bleibt.
void lightSleepWhileDisplayBusy(const void* busyPinParam) {
    if (WiFi.getMode() != WIFI_OFF) {
        delay(1);
        return;
    }

    const gpio_num_t busyPin = (gpio_num_t)*static_cast<const uint8_t*>(busyPinParam);
    gpio_wakeup_enable(busyPin, digitalRead(busyPin) == HIGH ? GPIO_INTR_LOW_LEVEL : GPIO_INTR_HIGH_LEVEL);
    esp_sleep_enable_gpio_wakeup();
    esp_sleep_enable_timer_wakeup(1000000ULL);

    Serial.flush();
    esp_light_sleep_start();

    esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_GPIO);
    esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_TIMER);
    gpio_wakeup_disable(busyPin);
}

// Funktion: Ein Befehl an den externen SPI-Flash
// CS wird nur aktiv auf Low gezogen und über den Pull-up wieder High: wird dabei der Knopf
// gedrückt (zieht GPIO2 auf GND), entsteht kein Kurzschluss.
void extFlashCommand(uint8_t command, uint8_t* response = nullptr, size_t responseLen = 0) {
    SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
    digitalWrite(EXT_FLASH_CS, LOW);
    pinMode(EXT_FLASH_CS, OUTPUT);
    SPI.transfer(command);
    for (size_t i = 0; i < responseLen; i++) {
        response[i] = SPI.transfer(0x00);
    }
    pinMode(EXT_FLASH_CS, INPUT_PULLUP);
    SPI.endTransaction();
    delayMicroseconds(50);   // Anstieg über den Pull-up und Release-/Power-Down-Zeit des Flash
}

// Funktion: Externen SPI-Flash in Deep Power-Down
// Auf der Universal-Platine hängt der Bild-Flash der Galerie am SPI-Bus des Displays. Der
// Kalender nutzt ihn nicht, im Standby zieht er aber rund um die Uhr Strom. Im Deep
// Power-Down (0xB9) bleibt er, bis er wieder 0xAB erhält oder die Platine stromlos wird.
void powerDownExternalFlash() {
    if (digitalRead(EXT_FLASH_CS) == LOW) {
        return;   // Knopf gedrückt: CS ist belegt, beim nächsten Start erneut
    }

    extFlashCommand(0xAB);                      // Release, falls er schon schläft (sonst keine ID)
    uint8_t jedec[3] = {0};
    extFlashCommand(0x9F, jedec, sizeof(jedec));
    if (jedec[0] == 0x00 || jedec[0] == 0xFF) {
        LOG_DEBUG("Kein externer SPI-Flash gefunden");
        return;
    }

    extFlashCommand(0xB9);
    LOG_DEBUG("Externer SPI-Flash (JEDEC %02X %02X %02X) in Deep Power-Down", jedec[0], jedec[1], jedec[2]);
}

// Funktion: Vorbereitung Deep Sleep
// GxEPD2 schaltet nach dem Refresh nur die Spannungen ab (powerOff). Erst hibernate()
// schickt den Display-Controller in Deep Sleep, init() weckt ihn beim nächsten Start per Reset.
void prepareDeepSleep() {
    epaperDisplay.hibernate();
    Serial.flush();
}

// Tag (lokal), für den zuletzt gezeichnet wurde. Überlebt den Deep Sleep.
RTC_DATA_ATTR int lastRenderedDay = -1;

int localDayKey(const struct tm& t) {
    return (t.tm_year + 1900) * 1000 + t.tm_yday;
}

// Funktion: Sleep
void sleepUntilOneAM() {
    time_t now = time(nullptr);
    struct tm timeinfo;
    localtime_r(&now, &timeinfo);

    // Aktuelle Zeit debuggen
    LOG_DEBUG("Current local time before sleep: %04d-%02d-%02d %02d:%02d:%02d",
        timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
        timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);

    // Der Deep-Sleep-Timer läuft auf dem internen RC-Oszillator und driftet. Wacht der ESP
    // kurz vor 1 Uhr auf, wurde der heutige Tag bereits gezeichnet: dann erst morgen wieder.
    const bool todayRendered = (localDayKey(timeinfo) == lastRenderedDay);

    // Berechne Zeitpunkt der nächsten 1 Uhr nachts
    timeinfo.tm_hour = 1;
    timeinfo.tm_min = 0;
    timeinfo.tm_sec = 0;
    timeinfo.tm_isdst = -1;

    time_t wakeupTime = mktime(&timeinfo);
    if (wakeupTime <= now || todayRendered) {
        // Morgen 1 Uhr (über tm_mday, damit die Umstellung Sommer-/Winterzeit stimmt)
        timeinfo.tm_mday += 1;
        timeinfo.tm_isdst = -1;
        wakeupTime = mktime(&timeinfo);
    }

    time_t sleepSeconds = wakeupTime - now;
    uint64_t sleepMicros = (uint64_t)sleepSeconds * 1000000ULL;

    LOG_DEBUG("Going to sleep for %ld seconds until 1 AM", sleepSeconds);
    LOG_DEBUG("Going to sleep for %llu micro seconds until 1 AM", sleepMicros);

    prepareDeepSleep();
    esp_sleep_enable_timer_wakeup(sleepMicros);
    //esp_sleep_enable_timer_wakeup(60ULL * 1000000ULL); // 60 Sekunden Test
    esp_deep_sleep_start();
}

// Funktion: Sleep nach vorübergehendem Fehler (WLAN, Zeit, Google)
// Die Anzeige bleibt unverändert, nach 3 Versuchen geht es normal um 1 Uhr weiter.
RTC_DATA_ATTR uint8_t retryCount = 0;
void sleepForRetry() {
    static const uint32_t backoffSeconds[] = {15 * 60, 60 * 60, 3 * 60 * 60};
    const uint8_t maxRetries = sizeof(backoffSeconds) / sizeof(backoffSeconds[0]);

    if (retryCount >= maxRetries) {
        retryCount = 0;
        sleepUntilOneAM();
    }

    uint32_t seconds = backoffSeconds[retryCount++];
    LOG_DEBUG("Retry %u/%u in %u seconds", retryCount, maxRetries, seconds);

    prepareDeepSleep();
    esp_sleep_enable_timer_wakeup((uint64_t)seconds * 1000000ULL);
    esp_deep_sleep_start();
}

// Funktion: Sleep ohne Timer, nur der Knopf weckt (Benutzeraktion nötig)
void sleepUntilButtonPress() {
    LOG_DEBUG("Going to sleep until button press");
    prepareDeepSleep();
    esp_deep_sleep_start();
}

// Interaktive Einrichtung (WLAN-Portal, Google-Anmeldung, Kalenderauswahl) nur nach
// Power-on oder Tastendruck. Beim nächtlichen Timer-Wakeup ist niemand am Gerät.
bool isInteractiveWakeup() {
    return wakeupReason != WakeupReason::Timer;
}

bool isButtonWakeup() {
    return wakeupReason == WakeupReason::ShortButton2Press || wakeupReason == WakeupReason::LongButton2Press;
}

// Funktion: Button Wakeup
void handleButtonWakeup() {
  // Button Wakeup Analyse
  esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();
  if (cause == ESP_SLEEP_WAKEUP_EXT0) {
    LOG_DEBUG("Wakeup durch BUTTON");
    unsigned long start = millis();

    // Long press
    while (digitalRead(BUTTON_PIN) == LOW) {
      if (millis() - start > LONG_BUTTON_PRESS_TIME) {
        wakeupReason = WakeupReason::LongButton2Press;
        LOG_DEBUG("WakeupReason::LongButton2Press");
        break;
      }
        delay(20);
    }
    // Short press
    if (wakeupReason != WakeupReason::LongButton2Press) {
        wakeupReason = WakeupReason::ShortButton2Press;
          LOG_DEBUG("WakeupReason::ShortButton2Press");
    }
  }
  else if (cause == ESP_SLEEP_WAKEUP_TIMER) {
      wakeupReason = WakeupReason::Timer;
      LOG_DEBUG("WakeupReason::Timer");
  }
}

// Funktion: Setup WiFi
void setupWiFi() {
  WiFi.disconnect(true);  // alte Verbindung löschen
  WiFi.mode(WIFI_STA);    // Station Mode erzwingen
  delay(100);
}

// Funktion: Setup Time (NTP)
bool setupTime() {
    configTzTime(TIMEZONE, "pool.ntp.org", "time.nist.gov");

    LOG_DEBUG("Wait for NTP Sync");
    int attempts = 0;
    while (time(nullptr) < 100000 && attempts++ < 50) delay(100);

    time_t now = time(nullptr);
    struct tm local;
    localtime_r(&now, &local);
    LOG_DEBUG("Local time: %04d-%02d-%02d %02d:%02d:%02d",
        local.tm_year + 1900, local.tm_mon + 1, local.tm_mday,
        local.tm_hour, local.tm_min, local.tm_sec);

    return (now >= 100000);
}

enum class SetupResult {
    Ok,
    RetryLater,     // vorübergehender Fehler, später automatisch erneut versuchen
    NeedsUser       // Anmeldung oder Kalenderauswahl durch den Benutzer nötig
};

// Funktion: Setup Google Auth
SetupResult setupGoogleAuth() {
    if (!auth.initialize()) {
        LOG_ERROR("Token Storage nicht initialisiert");
        return SetupResult::RetryLater;
    }

    if (wakeupReason == WakeupReason::LongButton2Press) {
        auth.deleteRefreshToken();
    }

    if (!auth.authorize(60, isInteractiveWakeup())) {
        LOG_ERROR("Keinen gültigen Access Token erhalten");
        if (!auth.needsUserAuthorization()) {
            return SetupResult::RetryLater;
        }
        // Beim Timer-Wakeup keinen Device Code Flow starten, nur den Hinweis zeigen
        if (!isInteractiveWakeup()) {
            authTimeoutDisplay.show("");
        }
        return SetupResult::NeedsUser;
    }

    calendarConfigurator.begin(isInteractiveWakeup() && !isButtonWakeup());
    if (isButtonWakeup()) {
        calendarConfigurator.forceSelection();
    }

    if (!calendarConfigurator.hasSelectedCalendars()) {
        // Beim Timer-Wakeup keinen Webserver starten, nur den Hinweis zeigen
        if (!isInteractiveWakeup()) {
            calendarTimeoutDisplay.show("");
        }
        return SetupResult::NeedsUser;
    }

    return SetupResult::Ok;
}

// Funktion: Load & Draw Calendar
// Gibt false zurück, wenn kein einziger Kalender geladen werden konnte (Anzeige bleibt unverändert)
bool loadAndDrawCalendar() {
    // Aktualisiere den Akkuanzeige-Modus
    weeklyCalendar.setBatteryDisplayMode(calendarConfigurator.getBatteryDisplayMode());

    std::vector<CalendarEvent> allEvents;
    size_t loadedCalendars = 0;
    for (const auto& calendarId : calendarConfigurator.getSelectedCalendarIds()) {
        std::vector<CalendarEvent> events;
        if (calendar.getEvents(calendarId, events)) {
            loadedCalendars++;
            for (const auto& c : events)
                LOG_DEBUG("Kalender %s: Event: %s: Date: %s", calendarId.c_str(), c.title.c_str(), c.startISO.c_str());
            allEvents.insert(allEvents.end(), events.begin(), events.end());
        } else {
            LOG_ERROR("Fehler beim Laden der Events für Kalender %s", calendarId.c_str());
        }
    }

    if (loadedCalendars == 0) {
        LOG_ERROR("Keine Kalender geladen - Anzeige bleibt unverändert");
        return false;
    }

    // Alle Daten sind im RAM: WLAN aus und CPU drosseln, bevor der lange Display-Refresh startet
    calendar.closeConnection();
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    setCpuFrequencyMhz(80);

    weeklyCalendar.drawCalendar(allEvents);

    time_t now = time(nullptr);
    struct tm local;
    localtime_r(&now, &local);
    lastRenderedDay = localDayKey(local);
    return true;
}

void setup() {

  // Initialize Serial for Debugging
  Serial.begin(115200);

  // Die RTC-Uhr läuft im Deep Sleep weiter, die Zeitzone muss nach jedem Start neu gesetzt werden
  setenv("TZ", TIMEZONE, 1);
  tzset();

  // Logger Setup
#if LOG_FS_ENABLED
  Logger::getInstance().begin();
#endif
  LOG_FS_DEBUG("========== ESP START ==========");
  LOG_FS_DEBUG("Free heap: %u", ESP.getFreeHeap());

  // Initialize Button
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_2, 0);  
  
  // Akku lesen, solange das WLAN noch aus ist (Spannung ohne Funklast)
  weeklyCalendar.readBattery();

  // Initialize WiFi
  setupWiFi();

  // Initialize e-Paper
  epaperDisplay.init();
  epaperDisplay.setBusyCallback(lightSleepWhileDisplayBusy);

  // Initialize LED
  //pinMode(LED_PIN, OUTPUT);
  //digitalWrite(LED_PIN, HIGH);

  // Button Wakeup Analyse
  handleButtonWakeup();

  // Externen Flash schlafen legen (SPI ist seit epaperDisplay.init() konfiguriert)
  powerDownExternalFlash();

  // Register Callbacks
  wifiHandler.onAccessPointStart([&](const String& url) {
      wifiDisplay.show(url);
  });
  wifiHandler.onTimeout([]() {
    LOG_DEBUG("Timeout Setting Wifi Credentials");
    const String qr_code = "";
    credentialTimeoutDisplay.show("");
  });

  calendarConfigurator.onServerStarted([&](const String& url) {
    calendarSelectorDisplay.show(url);
  });
  calendarConfigurator.onTimeout([]() {
    LOG_DEBUG("Timeout Setting Calendar");
    calendarTimeoutDisplay.show("");
  });

  auth.onAuthPrompt([](const String& url, const String& code) {
      authDisplay.showWithUserCode(url, code);
  });
  auth.onTimeout([]() {
        authTimeoutDisplay.show("");
  });

  // Connect to Wifi (Konfigurationsportal nur bei Power-on oder Tastendruck)
  if(!wifiHandler.begin(isInteractiveWakeup())) {
    LOG_ERROR("Kein Wifi Verfügbar");
    //digitalWrite(LED_PIN, LOW);
    sleepForRetry();
  }

  // Schweizer Zeitzone
  if(!setupTime()) {
    sleepForRetry();
  }

  // Initialize Google Calendar
  switch (setupGoogleAuth()) {
    case SetupResult::Ok:
      break;
    case SetupResult::RetryLater:
      sleepForRetry();
      break;
    case SetupResult::NeedsUser:
      sleepUntilButtonPress();
      break;
  }

  if (!loadAndDrawCalendar()) {
    sleepForRetry();
  }

  retryCount = 0;
  sleepUntilOneAM();
}

void loop() {

}

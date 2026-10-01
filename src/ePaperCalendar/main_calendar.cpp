
// System
#include <WiFi.h>

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

#define LED_PIN 32   // GPIO32
#define BUTTON_PIN 2
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

// Funktion: Sleep
void sleepUntilOneAM() {
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
    uint64_t sleepMicros = (uint64_t)sleepSeconds * 1000000ULL;

    LOG_DEBUG("Going to sleep for %ld seconds until 1 AM", sleepSeconds);
    LOG_DEBUG("Going to sleep for %llu micro seconds until 1 AM", sleepMicros);

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

    esp_sleep_enable_timer_wakeup((uint64_t)seconds * 1000000ULL);
    esp_deep_sleep_start();
}

// Funktion: Sleep ohne Timer, nur der Knopf weckt (Benutzeraktion nötig)
void sleepUntilButtonPress() {
    LOG_DEBUG("Going to sleep until button press");
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
    configTzTime("CET-1CEST,M3.5.0/2,M10.5.0/3", "pool.ntp.org", "time.nist.gov");

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

    weeklyCalendar.drawCalendar(allEvents);
    return true;
}

void setup() {

  // Initialize Serial for Debugging
  Serial.begin(115200);
  delay(1000);

  // Logger Setup
  Logger::getInstance().begin();
  LOG_FS_DEBUG("========== ESP START ==========");
  LOG_FS_DEBUG("Free heap: %u", ESP.getFreeHeap());

  // Initialize Button
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_2, 0);  
  
  // Initialize WiFi
  setupWiFi();

  // Initialize e-Paper
  epaperDisplay.init();

  // Initialize LED
  //pinMode(LED_PIN, OUTPUT);
  //digitalWrite(LED_PIN, HIGH);

  // Button Wakeup Analyse
  handleButtonWakeup();

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

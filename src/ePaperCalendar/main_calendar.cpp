
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
#include "views/autTimeoutDisplay.h"
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
AutTimeoutDisplay autTimeoutDisplay(epaperDisplay);
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

// Funktion: Setup Google Auth
bool setupGoogleAuth() {
    if (!auth.initialize()) {
        LOG_ERROR("Token Storage nicht initialisiert");
        return false;
    }

    if (wakeupReason == WakeupReason::LongButton2Press) {
        auth.deleteRefreshToken();
    }

    if (!auth.authorize(300)) {
        LOG_ERROR("Keinen gültigen Access Token erhalten");
        return false;
    }

    calendarConfigurator.begin();
    if (wakeupReason == WakeupReason::ShortButton2Press || wakeupReason == WakeupReason::LongButton2Press) {
        calendarConfigurator.forceSelection();
    }

    return true;
}

// Funktion: Load & Draw Calendar
void loadAndDrawCalendar() {
    if (!calendarConfigurator.hasSelectedCalendars()) return;

    std::vector<CalendarEvent> allEvents;
    for (const auto& calendarId : calendarConfigurator.getSelectedCalendarIds()) {
        std::vector<CalendarEvent> events;
        if (calendar.getEvents(calendarId, events)) {
            for (const auto& c : events)
                LOG_DEBUG("Kalender %s: Event: %s: Date: %s", calendarId.c_str(), c.title.c_str(), c.startISO.c_str());
            allEvents.insert(allEvents.end(), events.begin(), events.end());
        } else {
            LOG_ERROR("Fehler beim Laden der Events für Kalender %s", calendarId.c_str());
        }
    }
    weeklyCalendar.drawCalendar(allEvents);
}

void setup() {

  // Initialize Serial for Debugging
  Serial.begin(115200);
  delay(1000);

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
        autTimeoutDisplay.show("");
  });

  // Connect to Wifi
  if(!wifiHandler.begin()) {
    LOG_ERROR("Kein Wifi Verfügbar");
    //digitalWrite(LED_PIN, LOW);
    sleepUntilOneAM();
    return;
  }

  // Schweizer Zeitzone
  if(!setupTime()) {
    sleepUntilOneAM();
  }

  // Initialize Google Calendar
  if(!setupGoogleAuth()) {
    sleepUntilOneAM();
  }

  loadAndDrawCalendar();
  
  sleepUntilOneAM();
}

void loop() {

}

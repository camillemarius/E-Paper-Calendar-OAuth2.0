
// System
#include <WiFi.h>

// External Libraries
#include <qrcode.h>
#include <FPC8612.h>
#include <GoogleCalendar.h>
#include <GoogleAuth.h>
#include <logger.h>

// Internal Libraries
#include "credentials.h"
#include "wifiHandler.h"

// Local UI
#include "CalendarConfigurator/CalendarConfigurator.h"
#include "views/weeklyCalendar.h"
#include "views/authDisplay.h"
#include "views/wifiDisplay.h"
#include "views/calendarSelectorDisplay.h"

WiFiHandler wifiHandler(15);

#ifdef E_PAPER_CALENDAR_FPC8612
    FPC_8612 epaperDisplay(15, 27, 26, 25, 13, 12, 14, 15);
#elif defined(E_PAPER_CALENDAR_GDWE075T7)
    GDWE075T7 epaperDisplay(15, 27, 26, 25, 13, 12, 14, 15);
#endif

GoogleAuth auth(_clientId, _clientSecret, _scope);
GoogleCalendar calendar(auth);
CalendarConfigurator calendarConfigurator(calendar);

WeeklyCalendar weeklyCalendar(epaperDisplay);
AuthDisplay authDisplay(epaperDisplay);
WifiDisplay wifiDisplay(epaperDisplay);
CalendarSelectorDisplay calendarSelectorDisplay(epaperDisplay);

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

    LOG_DEBUG("Going to sleep for %ld seconds until 1 AM", sleepSeconds);

    esp_sleep_enable_timer_wakeup((uint64_t)sleepSeconds * 1000000ULL);
    esp_deep_sleep_start();
}

void setup() {

  // Initialize Serial for Debugging
  Serial.begin(115200);
  delay(1000);

  // Initialize e-Paper
  epaperDisplay.init();

  // Register Callbacks
  auth.onAuthPrompt([](const String& url, const String& code) {
      authDisplay.showWithUserCode(url, code);
  });
  wifiHandler.onAccessPointStart([&](const String& url) {
      wifiDisplay.show(url);
  });
  calendarConfigurator.onServerStarted([&](const String& url) {
    calendarSelectorDisplay.show(url);
  });

  // Connect to Wifi
  if(!wifiHandler.begin()) {
    LOG_ERROR("Kein Wifi Verfügbar");
    return;
  }

  // Schweizer Zeitzone
  configTzTime("CET-1CEST,M3.5.0/2,M10.5.0/3", 
              "pool.ntp.org", "time.nist.gov");

  //WORKING
  //configTime(0, 0, "pool.ntp.org", "time.nist.gov"); // Nur NTP
  //setenv("TZ", "CET-1CEST,M3.5.0/2,M10.5.0/3", 1);   // Schweizer Sommerzeit
  //tzset(); // WICHTIG
  //WORKING

  // Warten auf NTP Sync
  while (time(nullptr) < 100000) {
    delay(100);
  }

  time_t now = time(nullptr);

  struct tm local;
  localtime_r(&now, &local);
  LOG_DEBUG("Local time: %04d-%02d-%02d %02d:%02d:%02d\n",
                local.tm_year + 1900, local.tm_mon + 1, local.tm_mday,
                local.tm_hour, local.tm_min, local.tm_sec);

  // Initialize Google Calendar
  if(!auth.initialize()) {
    LOG_ERROR("Token Storage nicht initialisiert");
    return;
  }

  if (!auth.authorize(300)) {
    LOG_ERROR("Keinen gültigen Access Token erhalten");
    return;
  }

  calendarConfigurator.begin();

  if (calendarConfigurator.hasSelectedCalendars()) {
    std::vector<CalendarEvent> allEvents;

    // Für jeden ausgewählten Kalender alle Events abrufen und sammeln
    for (const auto& calendarId : calendarConfigurator.getSelectedCalendarIds()) {
        std::vector<CalendarEvent> events;
        if (calendar.getEvents(calendarId, events)) {
            for (const auto& c : events) {
                //LOG_DEBUG("Kalender %s: Event: %s: Date: %s", calendarId.c_str(), c.title.c_str(), c.startISO.c_str());
            }
            allEvents.insert(allEvents.end(), events.begin(), events.end());
        } else {
            LOG_ERROR("Fehler beim Laden der Events für Kalender %s", calendarId.c_str());
        }
    } 
    weeklyCalendar.drawCalendar(allEvents);
  }
  
  //uint64_t sleepTime = 8ULL * 60ULL * 60ULL * 1000000ULL;
  //ESP.deepSleep(sleepTime);
  sleepUntilOneAM();
}

void loop() {

}

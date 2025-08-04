
// System
#include <WiFi.h>
#include <time.h>

// External Libraries
#include <qrcode.h>
#include <FPC8612.h>
#include <GoogleCalendar.h>
#include <GoogleAuth.h>
#include <logger.h>

// Internal Libraries
#include "credentials.h"
#include "wifiHandler.h"
#include "calendarSelector.h"

// Local UI
#include "ePaperCalendar/weeklyCalendar.h"
#include "ePaperCalendar/authDisplay.h"
#include "ePaperCalendar/wifiDisplay.h"
#include "ePaperCalendar/calendarSelectorDisplay.h"

WiFiHandler wifiHandler(15);

FPC_8612 epaperDisplay(15, 27, 26, 25, 13, 12, 14, 15);

GoogleAuth auth(_clientId, _clientSecret, _scope);
GoogleCalendar calendar(auth);
CalendarSelector calendarSelector(calendar);

WeeklyCalendar weeklyCalendar(epaperDisplay);
AuthDisplay authDisplay(epaperDisplay);
WifiDisplay wifiDisplay(epaperDisplay);
CalendarSelectorDisplay calendarSelectorDisplay(epaperDisplay);

void setup() {

  // Initialize Serial for Debugging
  Serial.begin(115200);
  delay(1000);

  // Initialize e-Paper
  epaperDisplay.init();

  // Register Callbacks
  auth.onAuthPrompt([](const String& url, const String& code) {
      authDisplay.show(url, code);
  });
  wifiHandler.onAccessPointStart([&](const String& url) {
      wifiDisplay.show(url);
  });
  calendarSelector.onServerStarted([&](const String& url) {
    calendarSelectorDisplay.show(url);
  });

  // Connect to Wifi
  wifiHandler.begin();

  // Schweizer Zeitzone
  configTzTime("CET-1CEST,M3.5.0,M10.5.0/3", "pool.ntp.org", "time.nist.gov");
  while (time(nullptr) < 100000) {
    delay(100);
  }
  LOG_DEBUG("Time after sync: %ld", time(nullptr));
  time_t now = time(nullptr);
  struct tm *local = localtime(&now);
  struct tm *utc = gmtime(&now);
  //LOG_DEBUG("Now timestamp: %ld", now);
  //LOG_DEBUG("Local time: %d-%d-%d %d:%d:%d", local->tm_year + 1900, local->tm_mon + 1, local->tm_mday, local->tm_hour, local->tm_min, local->tm_sec);
  //LOG_DEBUG("UTC time: %d-%d-%d %d:%d:%d", utc->tm_year + 1900, utc->tm_mon + 1, utc->tm_mday, utc->tm_hour, utc->tm_min, utc->tm_sec);




  // Initialize Google Calendar
  if(!auth.initialize()) {
    LOG_ERROR("Token Storage nicht initialisiert");
    return;
  }

  if (!auth.authorize(300)) {
    LOG_ERROR("Keinen gültigen Access Token erhalten");
    return;
  }


  calendarSelector.begin();

  if (calendarSelector.hasSelectedCalendars()) {
    std::vector<CalendarEvent> allEvents;

    // Für jeden ausgewählten Kalender alle Events abrufen und sammeln
    for (const auto& calendarId : calendarSelector.getSelectedCalendarIds()) {
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
  
  uint64_t sleepTime = 8ULL * 60ULL * 60ULL * 1000000ULL;
  ESP.deepSleep(sleepTime);
}

void loop() {
}



// Local
#include "ePaperCalendar/weeklyCalendar.h"
#include "ePaperCalendar/authDisplay.h"

// Internal Library
#include <FPC8612.h>
#include <GoogleCalendar.h>
#include <GoogleAuth.h>
#include <calendarSelector.h>
#include <logger.h>
#include <credentials.h>

// External Library
#include <WiFi.h>
#include <qrcode.h>

FPC_8612 epaperDisplay(15, 27, 26, 25, 13, 12, 14, 15);
WeeklyCalendar weeklyCalendar(epaperDisplay);

AuthDisplay authDisplay(epaperDisplay);
GoogleAuth auth(_clientId, _clientSecret, _scope);
GoogleCalendar calendar(auth);
CalendarSelector calendarSelector(calendar);


void setup() {

  // Initialize Serial for Debugging
  Serial.begin(115200);
  delay(1000);

  // Connect to Wifi
  WiFi.setAutoReconnect(true);
  LOG_DEBUG("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    LOG_DEBUG(".");
  }
  LOG_DEBUG("WiFi RSSI: %d dBm", WiFi.RSSI());
  LOG_DEBUG(WiFi.localIP().toString().c_str());

  // Schweizer Zeitzone
  //configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  configTzTime("CET-1CEST,M3.5.0,M10.5.0/3", "pool.ntp.org", "time.nist.gov");
  while (time(nullptr) < 100000) {
    delay(100);
  }
  LOG_DEBUG("Time after sync: %ld", time(nullptr));
  time_t now = time(nullptr);
  struct tm *local = localtime(&now);
  struct tm *utc = gmtime(&now);
  LOG_DEBUG("Now timestamp: %ld", now);
  LOG_DEBUG("Local time: %d-%d-%d %d:%d:%d", local->tm_year + 1900, local->tm_mon + 1, local->tm_mday, local->tm_hour, local->tm_min, local->tm_sec);
  LOG_DEBUG("UTC time: %d-%d-%d %d:%d:%d", utc->tm_year + 1900, utc->tm_mon + 1, utc->tm_mday, utc->tm_hour, utc->tm_min, utc->tm_sec);

  //setenv("TZ", "CET-1CEST,M3.5.0,M10.5.0/3", 1);
  //tzset();

  epaperDisplay.init();

  

  // The structure to manage the QR code
  //QRCode qrcode;

  // Allocate a chunk of memory to store the QR code
  //uint8_t qrcodeBytes[qrcode_getBufferSize(40)];

  //qrcode_initText(&qrcode, qrcodeBytes, 40, ECC_LOW, "www.google.com/device");

  // TEST
  authDisplay.show("www.google.com/device", "563-234");
  return;

  // Register Authorisation Display callback
  auth.onAuthPrompt([](const String& url, const String& code) {
      authDisplay.show(url, code);  // Use the global directly
  });


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
                LOG_DEBUG("Kalender %s: Event: %s: Date: %s", calendarId.c_str(), c.title.c_str(), c.startISO.c_str());
            }
            // Events an allEvents anhängen
            allEvents.insert(allEvents.end(), events.begin(), events.end());
        } else {
            LOG_ERROR("Fehler beim Laden der Events für Kalender %s", calendarId.c_str());
        }
    } 
    // Kalenderanzeige aktualisieren mit allen gesammelten Events
    weeklyCalendar.drawCalendar(allEvents);
  }
  
  uint64_t sleepTime = 8ULL * 60ULL * 60ULL * 1000000ULL;
  ESP.deepSleep(sleepTime);
}

void loop() {
}

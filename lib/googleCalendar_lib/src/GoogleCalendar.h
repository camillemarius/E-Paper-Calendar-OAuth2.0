#pragma once

// Local

// Internal Library
#include "GoogleAuth.h"
#include "CalendarTypes.h"

// External Library
#include <vector>
#include <string>
#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

class GoogleCalendar {
public:
  GoogleCalendar(GoogleAuth& auth);
  bool getAvailableCalendars(std::vector<CalendarInfo>& outCalendars);
  bool getEvents(const String& calendarId, std::vector<CalendarEvent>& events);
  String getUserEmail();
  // Gemeinsame TLS-Verbindung schliessen und ihren Speicher freigeben
  void closeConnection();

private:
    bool isAllDayEvent(const String& isoStart, const String& isoEnd);
    int isoStringToHour(const String& iso);
    int isoStringToWeekday(const String& iso);
    String getISO8601TimeTodayStart();
    int httpGet(const String& url, const String& token, String& payload);

    GoogleAuth& _auth;
    String _calendarListJson;

    // Gemeinsame Verbindung zu www.googleapis.com (Keep-Alive über mehrere Anfragen)
    WiFiClientSecure _client;
    HTTPClient _http;
};


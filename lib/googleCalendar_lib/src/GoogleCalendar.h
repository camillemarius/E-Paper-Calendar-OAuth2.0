#pragma once

// Local

// Internal Library
#include "GoogleAuth.h"
#include "CalendarTypes.h"

// External Library
#include <vector>
#include <string>
#include <Arduino.h>

class GoogleCalendar {
public:
  GoogleCalendar(GoogleAuth& auth);
  bool getAvailableCalendars(std::vector<CalendarInfo>& outCalendars);
  bool getEvents(const String& calendarId, std::vector<CalendarEvent>& events);

private:
    bool isAllDayEvent(const String& isoStart, const String& isoEnd);
    int isoStringToHour(const String& iso);
    int isoStringToWeekday(const String& iso);
    String getISO8601TimeTodayStart();

    GoogleAuth& _auth;
    String _calendarListJson;
};


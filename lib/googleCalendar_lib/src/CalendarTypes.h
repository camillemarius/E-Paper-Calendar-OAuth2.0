#pragma once

// Local

// Internal Linbrary

// External Library
#include <string>

// Unified calendar event structure
struct CalendarEvent {
    String title;
    String startISO;   
    String endISO;

    int day = -1;           // 0 = Monday, 6 = Sunday
    int startHour = 0;
    int endHour = 0;
    bool isAllDay = false;

    // optionally add calendarId, color, etc.

    CalendarEvent() = default;
    
    CalendarEvent(String t, String s, String e,
                  int d, int sh, int eh, bool allDay)
        : title(std::move(t)), startISO(std::move(s)), endISO(std::move(e)),
          day(d), startHour(sh), endHour(eh), isAllDay(allDay) {}
};

struct CalendarInfo {
    String id;
    String summary;
};
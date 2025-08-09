#pragma once

// Local

// Internal Linbrary

// External Library
#include <string>

// Unified calendar event structure
struct CalendarEvent {
    String calendarId;
    String title;
    String startISO;   
    String endISO;

    int day = -1;           // 0 = Monday, 6 = Sunday
    int startHour = 0;
    int endHour = 0;
    int startMinute = 0;
    int endMinute = 0;
    bool isAllDay = false;

    // optionally add calendarId, color, etc.

    CalendarEvent() = default;
    
    CalendarEvent(String _calendarId, String _titel, String _startISO, String _endISO,
                  int _day, int _startHour, int _endHour, int _startMinute, int _endMinute, bool _isAllDay)
        : calendarId(_calendarId), title(std::move(_titel)), startISO(std::move(_startISO)), endISO(std::move(_endISO)),
          day(_day), startHour(_startHour), endHour(_endHour), startMinute(_startMinute), endMinute(_endMinute), isAllDay(_isAllDay) {}
};

struct CalendarInfo {
    String id;
    String summary;
};
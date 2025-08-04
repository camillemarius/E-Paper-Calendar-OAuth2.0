#pragma once

#include <vector>
#include <Arduino.h>
#include <Preferences.h>
#include <WebServer.h>
#include "GoogleCalendar.h"

class CalendarSelector {
public:
    explicit CalendarSelector(GoogleCalendar& calendar);

    void begin();

    bool hasSelectedCalendars() const;
    const std::vector<String>& getSelectedCalendarIds() const;

private:
    GoogleCalendar& _calendar;
    WebServer _server;
    Preferences _prefs;

    std::vector<CalendarInfo> _availableCalendars;
    std::vector<String> _selectedCalendarIds;

    void setupRoutes();

    void handleRoot();
    void handleSelect();

    void saveSelectedCalendars();
    void loadSelectedCalendars();
};


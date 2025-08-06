#pragma once

#include <vector>
#include <Arduino.h>
#include <Preferences.h>
#include <WebServer.h>
#include "GoogleCalendar.h"

class CalendarSelector {
public:
    using ServerStartedCallback = std::function<void(const String&)>;

    explicit CalendarSelector(GoogleCalendar& calendar);
    void begin();
    void onServerStarted(ServerStartedCallback cb);
    void forceSelection();

    bool hasSelectedCalendars() const;
    const std::vector<String>& getSelectedCalendarIds() const;

private:
    GoogleCalendar& _calendar;
    WebServer _server;
    Preferences _prefs;

    std::vector<CalendarInfo> _availableCalendars;
    std::vector<String> _selectedCalendarIds;

    ServerStartedCallback _serverStartedCallback = nullptr;

    void setupRoutes();

    void handleRoot();
    void handleSelect();
    void handleReset();

    void saveSelectedCalendars();
    void loadSelectedCalendars();
};


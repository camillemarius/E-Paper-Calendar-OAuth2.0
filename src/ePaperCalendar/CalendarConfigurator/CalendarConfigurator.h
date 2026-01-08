#pragma once

#include <vector>
#include <Arduino.h>
#include <Preferences.h>
#include <WebServer.h>
#include "GoogleCalendar.h"

class CalendarConfigurator {
public:
    using ServerStartedCallback = std::function<void(const String&)>;
    typedef std::function<void()> TimeoutCallback;

    explicit CalendarConfigurator(GoogleCalendar& calendar);

    void begin();

    void onServerStarted(ServerStartedCallback cb);
    void onTimeout(TimeoutCallback cb);

    void setTimeoutSeconds(int seconds);


    bool hasSelectedCalendars() const;
    const std::vector<String>& getSelectedCalendarIds() const;
    void forceSelection();

private:
    void setupRoutes();
    void handleRoot();
    void handleSelect();
    void handleReset();

    void saveSelectedCalendars();
    void loadSelectedCalendars();

    GoogleCalendar& _calendar;
    WebServer _server;
    Preferences _prefs;

    std::vector<String> _selectedCalendarIds;
    std::vector<CalendarInfo> _availableCalendars;

    ServerStartedCallback _serverStartedCallback = nullptr;
    TimeoutCallback _timeoutCallback = nullptr;

    int _timeoutSeconds = 120;
    
    String _googleAccountEmail;

};


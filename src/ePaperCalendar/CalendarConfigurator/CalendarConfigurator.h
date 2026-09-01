#pragma once

#include <vector>
#include <Arduino.h>
#include <Preferences.h>
#include <WebServer.h>
#include "GoogleCalendar.h"

enum class BatteryDisplayMode {
    PERCENT = 0,    // Prozentanzeige (85%)
    BAR = 1,        // Batteriebalken (voll->leer)
    VOLTAGE = 2     // Spannungsanzeige (4.2V)
};

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
    
    BatteryDisplayMode getBatteryDisplayMode() const;
    void setBatteryDisplayMode(BatteryDisplayMode mode);

private:
    void setupRoutes();
    void handleRoot();
    void handleSelect();
    void handleReset();

    void saveSelectedCalendars();
    void loadSelectedCalendars();
    void saveBatteryDisplayMode();
    void loadBatteryDisplayMode();

    GoogleCalendar& _calendar;
    WebServer _server;
    Preferences _prefs;

    std::vector<String> _selectedCalendarIds;
    std::vector<CalendarInfo> _availableCalendars;

    ServerStartedCallback _serverStartedCallback = nullptr;
    TimeoutCallback _timeoutCallback = nullptr;

    int _timeoutSeconds = 120;
    
    String _googleAccountEmail;
    BatteryDisplayMode _batteryDisplayMode = BatteryDisplayMode::PERCENT;

};


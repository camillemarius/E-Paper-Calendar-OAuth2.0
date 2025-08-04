#include "CalendarSelector.h"
#include <logger.h>

CalendarSelector::CalendarSelector(GoogleCalendar& calendar)
    : _calendar(calendar), _server(80) {}

void CalendarSelector::begin() {
    _prefs.begin("calendar", false);
    loadSelectedCalendars();

    if (_selectedCalendarIds.empty()) {
        if (!_calendar.getAvailableCalendars(_availableCalendars)) {
            LOG_ERROR("Fehler beim Laden der Kalender");
            return;
        } 
        for (const auto& c : _availableCalendars) {
                LOG_DEBUG("%s", c.summary.c_str());
        }


        setupRoutes();
        _server.begin();
        String url = "http://" + WiFi.localIP().toString() + "/";
        LOG_DEBUG("Webserver gestartet. Öffne %s zur Kalenderauswahl.", url.c_str());
        
        if (_serverStartedCallback) {
            _serverStartedCallback(url);
        }

        while (_selectedCalendarIds.empty()) {
            _server.handleClient();
            delay(10);
        }
    }
}

void CalendarSelector::onServerStarted(ServerStartedCallback cb) {
    _serverStartedCallback = cb;
}

bool CalendarSelector::hasSelectedCalendars() const {
    return !_selectedCalendarIds.empty();
}

const std::vector<String>& CalendarSelector::getSelectedCalendarIds() const {
    return _selectedCalendarIds;
}

void CalendarSelector::setupRoutes() {
    _server.on("/", HTTP_GET, [this]() { handleRoot(); });
    _server.on("/select", HTTP_POST, [this]() { handleSelect(); });
}

void CalendarSelector::handleRoot() {
    String html = "<html><body><h2>Wähle Kalender (Mehrfachauswahl möglich)</h2>";
    html += "<form method='POST' action='/select'>";
    for (const auto& c : _availableCalendars) {
        html += "<input type='checkbox' name='calendarId' value='";
        html += c.id;
        html += "'>";
        html += c.summary;
        html += "<br>";
    }
    html += "<input type='submit' value='Speichern'>";
    html += "</form></body></html>";
    _server.send(200, "text/html", html);
}

void CalendarSelector::handleSelect() {
    if (_server.hasArg("calendarId")) {
        _selectedCalendarIds.clear();
        // Sammle alle ausgewählten Kalender-IDs
        int argsCount = _server.args();
        for (int i = 0; i < argsCount; ++i) {
            if (_server.argName(i) == "calendarId") {
                _selectedCalendarIds.push_back(_server.arg(i));
            }
        }
        saveSelectedCalendars();

        _server.send(200, "text/html", "<h3>Kalender gespeichert.</h3><p>Bitte Gerät neu starten.</p>");
        delay(2000);
    } else {
        _server.send(400, "text/plain", "Fehlender Parameter: calendarId");
    }
}

void CalendarSelector::saveSelectedCalendars() {
    String csv;
    for (const auto& id : _selectedCalendarIds) {
        if (!csv.isEmpty()) csv += ",";
        csv += id;
    }
    _prefs.putString("calendarIds", csv);
}

void CalendarSelector::loadSelectedCalendars() {
    _selectedCalendarIds.clear();
    String csv = _prefs.getString("calendarIds", "");
    int start = 0;
    int commaIndex;
    while ((commaIndex = csv.indexOf(',', start)) != -1) {
        _selectedCalendarIds.push_back(csv.substring(start, commaIndex));
        start = commaIndex + 1;
    }
    if (start < csv.length()) {
        _selectedCalendarIds.push_back(csv.substring(start));
    }
}

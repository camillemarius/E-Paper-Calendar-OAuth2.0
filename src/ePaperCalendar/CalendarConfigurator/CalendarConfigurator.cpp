#include "CalendarConfigurator.h"
#include <logger.h>

CalendarConfigurator::CalendarConfigurator(GoogleCalendar& calendar)
    : _calendar(calendar), _server(80) {}

void CalendarConfigurator::begin() {
    _prefs.begin("calendar", false);
    loadSelectedCalendars();

    if (_selectedCalendarIds.empty()) {
        LOG_DEBUG("selectedCalendarIds is empty");
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
    } else{
        LOG_DEBUG("selectedCalendarIds is not empty");
    }
}

void CalendarConfigurator::onServerStarted(ServerStartedCallback cb) {
    _serverStartedCallback = cb;
}

bool CalendarConfigurator::hasSelectedCalendars() const {
    return !_selectedCalendarIds.empty();
}

const std::vector<String>& CalendarConfigurator::getSelectedCalendarIds() const {
    return _selectedCalendarIds;
}

void CalendarConfigurator::forceSelection() {
    _prefs.begin("calendar", false);
    _prefs.remove("calendarIds");
    _prefs.end();

    _selectedCalendarIds.clear();
    begin();  // startet Auswahl neu
}
void CalendarConfigurator::setupRoutes() {
    _server.on("/", HTTP_GET, [this]() { handleRoot(); });
    _server.on("/select", HTTP_POST, [this]() { handleSelect(); });
    _server.on("/reset", HTTP_GET, [this]() { handleReset(); });
}

void CalendarConfigurator::handleRoot() {
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

void CalendarConfigurator::handleSelect() {
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

        _server.send(200, "text/html", "<h3>Kalender gespeichert.</h3><p>Diese Seite kann nun geschlossen werden.</p>");
        delay(2000);
    } else {
        _server.send(400, "text/plain", "Fehlender Parameter: calendarId");
    }
}

void CalendarConfigurator::handleReset() {
    _prefs.begin("calendar", false);
    _prefs.remove("calendarIds");
    _prefs.end();
    
    _selectedCalendarIds.clear();
    _server.send(200, "text/html", "<p>Diese Seite kann nun geschlossen werden.</p>");
}


void CalendarConfigurator::saveSelectedCalendars() {
    _prefs.begin("calendar", false);
    String csv;
    for (const auto& id : _selectedCalendarIds) {
        if (!csv.isEmpty()) csv += ",";
        csv += id;
    }
    _prefs.putString("calendarIds", csv);
    _prefs.end();
}

void CalendarConfigurator::loadSelectedCalendars() {
    _prefs.begin("calendar", false);
    _selectedCalendarIds.clear();
    String csv = _prefs.getString("calendarIds", "");
    LOG_DEBUG("Loaded calendarIds: '%s'", csv.c_str());

    int start = 0;
    int commaIndex;
    while ((commaIndex = csv.indexOf(',', start)) != -1) {
        _selectedCalendarIds.push_back(csv.substring(start, commaIndex));
        start = commaIndex + 1;
    }
    if (start < csv.length()) {
        _selectedCalendarIds.push_back(csv.substring(start));
    }
    _prefs.end();
}


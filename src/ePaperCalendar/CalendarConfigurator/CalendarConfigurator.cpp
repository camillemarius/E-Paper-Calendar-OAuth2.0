#include "CalendarConfigurator.h"
#include <logger.h>

CalendarConfigurator::CalendarConfigurator(GoogleCalendar& calendar)
    : _calendar(calendar), _server(80), _timeoutCallback(nullptr), _timeoutSeconds(120)  {
    }

void CalendarConfigurator::begin() {
    _prefs.begin("calendar", false);

    // ---------------------------------------------------
    //handleReset(); //TO BE DELETED
    // ---------------------------------------------------

    loadSelectedCalendars();
    loadBatteryDisplayMode();

    if (_selectedCalendarIds.empty()) {
        LOG_DEBUG("selectedCalendarIds is empty");
        if (!_calendar.getAvailableCalendars(_availableCalendars)) {
            LOG_ERROR("Fehler beim Laden der Kalender");
            return;
        } 
        for (const auto& c : _availableCalendars) {
                LOG_DEBUG("%s", c.summary.c_str());
        }
        _googleAccountEmail = _calendar.getUserEmail();


        setupRoutes();
        _server.begin();
        String url = "http://" + WiFi.localIP().toString() + "/";
        LOG_DEBUG("Webserver gestartet. Öffne %s zur Kalenderauswahl.", url.c_str());
        
        if (_serverStartedCallback) {
            _serverStartedCallback(url);
        }

        unsigned long startMillis = millis();
        while (_selectedCalendarIds.empty()) {
            _server.handleClient();
            delay(10);
            if ((millis() - startMillis) / 1000 > _timeoutSeconds) {
                LOG_ERROR("Kalenderauswahl Timeout erreicht.");
                if (_timeoutCallback) {
                    _timeoutCallback();
                }
                break;
            }
        }
    } else{
        LOG_DEBUG("selectedCalendarIds is not empty");
    }
}

void CalendarConfigurator::onServerStarted(ServerStartedCallback cb) {
    _serverStartedCallback = cb;
}

void CalendarConfigurator::onTimeout(TimeoutCallback cb) {
    _timeoutCallback = cb;
}

void CalendarConfigurator::setTimeoutSeconds(int seconds) {
    _timeoutSeconds = seconds;
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
    String html = "<html><body><h2>Waehle Kalender (Mehrfachauswahl moeglich)</h2>";

    // ---> Google-Account anzeigen
    html += "<p><b>Angemeldeter Google-Account:</b><br>";
    html += _googleAccountEmail;
    html += "</p><hr>";

    html += "<form method='POST' action='/select'>";
    for (const auto& c : _availableCalendars) {
        html += "<input type='checkbox' name='calendarId' value='";
        html += c.id;
        html += "'>";
        html += c.summary;
        html += "<br>";
    }
    
    // Akkustandsanzeige-Option
    html += "<hr><h3>Akkustandsanzeige</h3>";
    html += "<label><input type='radio' name='batteryMode' value='0'";
    if (_batteryDisplayMode == BatteryDisplayMode::PERCENT) html += " checked";
    html += "> Prozent (z.B. 85%)</label><br>";
    
    html += "<label><input type='radio' name='batteryMode' value='1'";
    if (_batteryDisplayMode == BatteryDisplayMode::BAR) html += " checked";
    html += "> Batteriebalken (voll->leer)</label><br>";
    
    html += "<label><input type='radio' name='batteryMode' value='2'";
    if (_batteryDisplayMode == BatteryDisplayMode::VOLTAGE) html += " checked";
    html += "> Spannungsanzeige (z.B. 4.2V)</label><br>";
    
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
            // Akkustandsanzeige-Modus speichern
            if (_server.argName(i) == "batteryMode") {
                int mode = _server.arg(i).toInt();
                _batteryDisplayMode = static_cast<BatteryDisplayMode>(mode);
            }
        }
        saveSelectedCalendars();
        saveBatteryDisplayMode();

        _server.send(200, "text/html", "<h3>Kalender und Einstellungen gespeichert.</h3><p>Diese Seite kann nun geschlossen werden.</p>");
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

void CalendarConfigurator::saveBatteryDisplayMode() {
    _prefs.begin("calendar", false);
    _prefs.putUChar("batteryMode", static_cast<unsigned char>(_batteryDisplayMode));
    _prefs.end();
}

void CalendarConfigurator::loadBatteryDisplayMode() {
    _prefs.begin("calendar", false);
    unsigned char mode = _prefs.getUChar("batteryMode", static_cast<unsigned char>(BatteryDisplayMode::PERCENT));
    _batteryDisplayMode = static_cast<BatteryDisplayMode>(mode);
    LOG_DEBUG("Loaded battery display mode: %d", mode);
    _prefs.end();
}

BatteryDisplayMode CalendarConfigurator::getBatteryDisplayMode() const {
    return _batteryDisplayMode;
}

void CalendarConfigurator::setBatteryDisplayMode(BatteryDisplayMode mode) {
    _batteryDisplayMode = mode;
    saveBatteryDisplayMode();
}


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
    String html = "<html><head>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<style>";
    html += "body { font-family: Arial, sans-serif; font-size: 18px; margin: 20px; padding: 10px; background: #f5f5f5; }";
    html += "h2 { font-size: 28px; margin-bottom: 20px; color: #333; }";
    html += "h3 { font-size: 22px; margin-top: 20px; margin-bottom: 15px; color: #333; }";
    html += "p { font-size: 18px; line-height: 1.6; margin: 15px 0; }";
    html += "b { font-weight: bold; }";
    html += "hr { margin: 20px 0; border: none; border-top: 2px solid #ccc; }";
    html += "input[type='checkbox'], input[type='radio'] { width: 20px; height: 20px; margin-right: 10px; cursor: pointer; }";
    html += "label { display: block; font-size: 18px; margin: 15px 0; padding: 10px; background: white; border-radius: 5px; cursor: pointer; }";
    html += "input[type='submit'] { font-size: 18px; padding: 15px 30px; margin-top: 20px; background: #4CAF50; color: white; border: none; border-radius: 5px; cursor: pointer; width: 100%; box-sizing: border-box; }";
    html += "input[type='submit']:active { background: #45a049; }";
    html += "</style>";
    html += "</head><body>";
    
    html += "<h2>Waehle Kalender (Mehrfachauswahl moeglich)</h2>";

    // ---> Google-Account anzeigen
    html += "<p><b>Angemeldeter Google-Account:</b><br>";
    html += _googleAccountEmail;
    html += "</p><hr>";

    html += "<form method='POST' action='/select'>";
    for (const auto& c : _availableCalendars) {
        html += "<label><input type='checkbox' name='calendarId' value='";
        html += c.id;
        html += "'>";
        html += c.summary;
        html += "</label>";
    }
    
    // Akkustandsanzeige-Option
    html += "<hr><h3>Akkustandsanzeige</h3>";
    html += "<label><input type='radio' name='batteryMode' value='0'";
    if (_batteryDisplayMode == BatteryDisplayMode::PERCENT) html += " checked";
    html += "> Prozent (0-100%)</label>";
    
    html += "<label><input type='radio' name='batteryMode' value='1'";
    if (_batteryDisplayMode == BatteryDisplayMode::BAR) html += " checked";
    html += "> Batteriebalken (voll->leer)</label>";
    
    html += "<label><input type='radio' name='batteryMode' value='2'";
    if (_batteryDisplayMode == BatteryDisplayMode::VOLTAGE) html += " checked";
    html += "> Spannungsanzeige (3.0-4.2V)</label>";
    
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

        sendStatusPage( "Einstellungen gespeichert", "Kalender und Einstellungen wurden erfolgreich gespeichert.", true ); 
        delay(2000); 
    } 
    else 
    { 
        sendStatusPage( "Fehler", "Fehlender Parameter: calendarId", false ); 
    }
}

void CalendarConfigurator::handleReset() {
    _prefs.begin("calendar", false);
    _prefs.remove("calendarIds");
    _prefs.end();
    
    _selectedCalendarIds.clear();
    sendStatusPage( "Zurückgesetzt", "Die Kalenderauswahl wurde erfolgreich zurückgesetzt.", true );
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

void CalendarConfigurator::sendStatusPage(const String& title, const String& message, bool success) { 
    String html = "<html><head>"; 
    html += "<meta name='viewport' content='width=device-width, initial-scale=1'>"; 
    html += "<style>"; 
    html += "body { " "font-family: Arial, sans-serif; " "font-size: 18px; " "margin: 20px; " "padding: 10px; " "background: #f5f5f5; " "}"; 
    html += ".container { " "max-width: 600px; " "margin: 40px auto; " "padding: 25px; " "background: white; " "border-radius: 8px; " "box-shadow: 0 2px 8px rgba(0,0,0,0.1); " "text-align: center; " "}"; 
    html += "h2 { " "font-size: 28px; " "margin-bottom: 20px; " "color: #333; " "}"; 
    html += "p { " "font-size: 18px; " "line-height: 1.6; " "margin: 15px 0; " "color: #333; " "}"; 
    html += ".status { " "font-size: 22px; " "font-weight: bold; " "margin-bottom: 20px; " "}"; 
    html += ".success { color: #4CAF50; }"; 
    html += ".error { color: #d32f2f; }"; 
    html += "</style>"; 
    html += "</head><body>"; 
    html += "<div class='container'>"; 
    html += "<h2>"; 
    html += title; 
    html += "</h2>"; 
    html += "<p class='status "; 
    html += success ? "success" : "error"; 
    html += "'>"; 
    html += message; 
    html += "</p>"; 
    html += "<p>Diese Seite kann nun geschlossen werden.</p>"; 
    html += "</div>"; 
    html += "</body></html>"; _server.send(success ? 200 : 400, "text/html", html); 
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


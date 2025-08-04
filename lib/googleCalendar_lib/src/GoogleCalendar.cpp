// Local
#include "GoogleCalendar.h"

// Internal Library
#include <logger.h>

// External Libraray
#include <ctime>
#include <string>
#include <ArduinoJson.h>
#include <HTTPClient.h>
GoogleCalendar::GoogleCalendar(GoogleAuth& auth)
  : _auth(auth) {

}

String GoogleCalendar::getISO8601TimeTodayStart() {
    time_t now = time(nullptr);
    struct tm t = *gmtime(&now);

    t.tm_hour = 0;
    t.tm_min = 0;
    t.tm_sec = 0;

    char buf[30];
    strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", &t);
    return String(buf);
}


bool GoogleCalendar::getEvents(const String& calendarId, std::vector<CalendarEvent>& events) {
    String token = _auth.getAccessToken();

    // Aktuelles Datum in ISO 8601 (UTC)
    //time_t now = time(nullptr);
    //struct tm* timeinfo = gmtime(&now);
    //char timeMin[32];
    //strftime(timeMin, sizeof(timeMin), "%Y-%m-%dT%H:%M:%SZ", timeinfo);
    String startOfToday = getISO8601TimeTodayStart();
    LOG_DEBUG("Abfrage ab: %s", startOfToday.c_str());

    String url = "https://www.googleapis.com/calendar/v3/calendars/" + String(calendarId) +
                 "/events?maxResults=10&orderBy=startTime&singleEvents=true&timeMin=" + String(startOfToday);

    HTTPClient http;
    http.begin(url);
    http.addHeader("Authorization", "Bearer " + token);

    int httpCode = http.GET();
    if (httpCode != 200) {
        LOG_ERROR("HTTP Fehler beim Abrufen der Events: %d", httpCode);
        http.end();
        return false;
    }

    String payload = http.getString();
    http.end();

    DynamicJsonDocument doc(16 * 1024);
    DeserializationError error = deserializeJson(doc, payload);
    if (error) {
        LOG_ERROR("JSON Parsing Fehler beim Abrufen der Events");
        return false;
    }

    JsonArray items = doc["items"].as<JsonArray>();
    events.clear();

    for (JsonObject item : items) {
        String title = item["summary"] | "Ohne Titel";

        String startISO, endISO;
        if (item["start"].containsKey("dateTime")) {
            startISO = item["start"]["dateTime"].as<const char*>();
        } else {
            startISO = item["start"]["date"].as<const char*>();
        }

        if (item["end"].containsKey("dateTime")) {
            endISO = item["end"]["dateTime"].as<const char*>();
        } else {
            endISO = item["end"]["date"].as<const char*>();
        }

        bool allDay = isAllDayEvent(startISO, endISO);

        int day = -1, startHour = 0, endHour = 0;
        if (!allDay) {
            day = isoStringToWeekday(startISO);
            startHour = isoStringToHour(startISO);
            endHour = isoStringToHour(endISO);
        }

        events.emplace_back(title, startISO, endISO, day, startHour, endHour, allDay);
    }

    LOG_INFO("Kalendereinträge geladen: %d", events.size());
    return true;
}

bool GoogleCalendar::getAvailableCalendars(std::vector<CalendarInfo>& outCalendars) {
    outCalendars.clear();
    String token = _auth.getAccessToken();  // garantiert gültiger Token

    // Anfrage an Google API
    HTTPClient http;
    http.begin("https://www.googleapis.com/calendar/v3/users/me/calendarList");
    http.addHeader("Authorization", "Bearer " + token);

    int httpCode = http.GET();
    if (httpCode != 200) {
        LOG_ERROR("HTTP Fehler beim Laden der Kalenderliste: %d", httpCode);
        http.end();
        return false;
    }

    String payload = http.getString();
    http.end();

    // JSON parsen
    DynamicJsonDocument doc(16 * 1024);
    DeserializationError error = deserializeJson(doc, payload);
    if (error) {
        LOG_ERROR("Fehler beim Parsen der Kalenderliste.");
        return false;
    }

    // Kalender extrahieren
    JsonArray items = doc["items"].as<JsonArray>();
    for (JsonObject item : items) {
        CalendarInfo info;
        info.id = item["id"] | "";
        info.summary = item["summary"] | "";
        outCalendars.push_back(info);
    }

    LOG_INFO("Kalender geladen: %d", outCalendars.size());
    return true;
}


int GoogleCalendar::isoStringToWeekday(const String& iso) {
    std::tm tm = {};
    strptime(iso.c_str(), "%Y-%m-%dT%H:%M:%S", &tm); 
    // Wochentag: 0 = Sonntag ... 6 = Samstag (tm_wday)
    // Wir wollen 0=Montag ... 6=Sonntag:
    int day = tm.tm_wday == 0 ? 6 : tm.tm_wday - 1;
    return day;
}

int GoogleCalendar::isoStringToHour(const String& iso) {
    std::tm tm = {};
    strptime(iso.c_str(), "%Y-%m-%dT%H:%M:%S", &tm);
    return tm.tm_hour;
}

bool GoogleCalendar::isAllDayEvent(const String& isoStart, const String& isoEnd) {
    return isoStart.indexOf('T') == -1 && isoEnd.indexOf('T') == -1;
}



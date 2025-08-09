// Local
#include "GoogleCalendar.h"

// Internal Library
#include <logger.h>

// External Libraray
#include <time.h>
#include <ctime>
#include <string>
#include <stdio.h>
#include <stdint.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <stdint.h>

static int64_t days_from_civil(int64_t y, int64_t m, int64_t d) {
    y -= m <= 2;
    int64_t era = (y >= 0 ? y : y - 399) / 400;
    int64_t yoe = y - era * 400;             // [0,399]
    int64_t doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1; // [0,365]
    int64_t doe = yoe * 365 + yoe/4 - yoe/100 + doy;
    return era * 146097 + doe - 719468;
}

// Parse ISO8601 (YYYY-MM-DDTHH:MM:SS[Z|(+|-)HH:MM]) -> time_t (seconds since epoch, UTC)
time_t parseIso8601ToEpochUTC(const char *s) {
    int year, mon, day, hour, min, sec;
    const char *tpos = strchr(s, 'T');
    if (!tpos) return (time_t)-1;

    // parse date/time part first (ignore trailing timezone for now)
    if (sscanf(s, "%d-%d-%dT%d:%d:%d", &year, &mon, &day, &hour, &min, &sec) != 6) {
        return (time_t)-1;
    }

    // find offset (after the time)
    const char *off = strchr(tpos, 'Z');
    int offset_seconds = 0;
    if (!off) {
        // check for + or - after the 'T' (search after the seconds position)
        const char *plus = strchr(tpos, '+');
        const char *minus = strchr(tpos, '-'); // careful: date has '-' before T, but we search after tpos
        // pick the first occurrence after the time (plus or minus) if present
        const char *candidate = NULL;
        if (plus) candidate = plus;
        if (minus && (!candidate || minus > plus)) candidate = minus; // ensure we pick the actual sign after time
        // but to be safe, ensure candidate is after the seconds digits:
        if (candidate && candidate > strchr(tpos, ':') ) {
            int off_h = 0, off_m = 0;
            char sign = *candidate;
            if (sscanf(candidate + 1, "%d:%d", &off_h, &off_m) == 2) {
                offset_seconds = off_h * 3600 + off_m * 60;
                if (sign == '+') offset_seconds = +offset_seconds;  // time string is local = UTC + offset -> UTC = given - offset
                else offset_seconds = -offset_seconds;              // sign '-' -> UTC = given + offset_seconds_abs
            }
        }
    } else {
        offset_seconds = 0; // 'Z' means UTC
    }

    int64_t days = days_from_civil(year, mon, day);
    int64_t secs = days * 86400 + (int64_t)hour * 3600 + (int64_t)min * 60 + sec;

    // adjust to UTC: if offset_seconds is positive (+02:00), the string time = UTC+2,
    // so UTC = given_time - offset_seconds.
    secs -= offset_seconds;

    return (time_t)secs;
}

// Hauptroutine: gibt time_t (Epoch UTC) zurück und loggt lokale Darstellung
time_t utcStringtoLocal(const char* utcString) {
    time_t t = parseIso8601ToEpochUTC(utcString);
    if (t == (time_t)-1) {
        LOG_ERROR("Invalid time string: %s", utcString);
        return (time_t)0;
    }

    struct tm tm_local;
    localtime_r(&t, &tm_local); // reentrant variant

    char buf[64];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S %Z", &tm_local);
    //LOG_DEBUG("Local Time: %s", buf);

    return t; // t ist die Epoch-Sekunden des Moments (UTC). Das ist normal und korrekt.
}



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

    // Startzeitpunkt heute (lokale Zeit) als ISO für Google
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
        LOG_DEBUG("calendarId: %s", calendarId.c_str());

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

        int day = -1, startHour = 0, endHour = 0, startMinute = 0, endMinute = 0;
        if (!allDay) {
            // hier wandeln wir in lokale Zeit um
            time_t t_start = utcStringtoLocal(startISO.c_str());
            time_t t_end   = utcStringtoLocal(endISO.c_str());

            struct tm tm_start, tm_end;
            localtime_r(&t_start, &tm_start);
            localtime_r(&t_end,   &tm_end);

            day = tm_start.tm_wday; // Sonntag=0, Montag=1, etc.
            startHour = tm_start.tm_hour;
            endHour   = tm_end.tm_hour;
            startMinute = tm_start.tm_min;
            endMinute   = tm_end.tm_min;

            /*LOG_DEBUG("event: %s, startLocal: %04d-%02d-%02d %02d:%02d:%02d, endLocal: %04d-%02d-%02d %02d:%02d:%02d",
                      title.c_str(),
                      tm_start.tm_year + 1900, tm_start.tm_mon + 1, tm_start.tm_mday,
                      tm_start.tm_hour, tm_start.tm_min, tm_start.tm_sec,
                      tm_end.tm_year + 1900, tm_end.tm_mon + 1, tm_end.tm_mday,
                      tm_end.tm_hour, tm_end.tm_min, tm_end.tm_sec);*/
        }

        events.emplace_back(calendarId, title, startISO, endISO, day, startHour, endHour, startMinute, endMinute, allDay);
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


/*static inline int64_t days_from_civil(int64_t y, unsigned m, unsigned d) {
    y -= m <= 2;
    const int64_t era = (y >= 0 ? y : y - 399) / 400;
    const unsigned yoe = static_cast<unsigned>(y - era * 400);        // [0,399]
    const unsigned doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1; // [0,365]
    const unsigned doe = yoe * 365 + yoe/4 - yoe/100 + doy;           // [0, 146096]
    return era * 146097 + static_cast<int64_t>(doe) - 719468;
}*/

static inline time_t utc_from_fields(int year, int month, int day, int hour, int minute, int second) {
    int64_t days = days_from_civil(year, month, day);
    int64_t epoch = days * 86400LL + (int64_t)hour * 3600 + (int64_t)minute * 60 + second;
    return (time_t)epoch;
}

// Gibt den lokalen Offset (in Sekunden) für genau diesen UTC-Epoch zurück.
// offset = utcEpoch - mktime(gmtime(utcEpoch))  -> berücksichtigt DST für das Datum.
static int getLocalOffsetSecondsForUtc(time_t utcEpoch) {
    struct tm gm = *gmtime(&utcEpoch);          // UTC broken-down time
    time_t asLocal = mktime(&gm);               // interpretiere diese broken-down time als lokale Zeit
    return (int)difftime(utcEpoch, asLocal);    // positiv wenn lokal > UTC (z.B. +7200)
}


// Hilfsfunktion, die Zeitzone (z.B. +02:00) ausliest und in Sekunden umrechnet
int parseTimezoneOffsetSeconds(const String& iso) {
    // Suche nach '+' oder '-' nach den Sekunden
    int pos = iso.indexOf('+', 19);
    if (pos == -1) pos = iso.indexOf('-', 19);
    if (pos == -1) return 0;  // keine Zeitzone gefunden (z.B. Z für UTC)

    String tz = iso.substring(pos);  // z.B. "+02:00" oder "-03:30"
    int sign = (tz[0] == '-') ? -1 : 1;
    int hours = tz.substring(1, 3).toInt();
    int minutes = tz.substring(4, 6).toInt();
    return sign * (hours * 3600 + minutes * 60);
}

time_t timegm(struct tm *tm) {
    // mktime interpretiert tm als lokale Zeit, wir wollen UTC
    // Also mktime und dann Zeitzonen-Offset abziehen

    time_t t = mktime(tm);
    if (t == -1) return -1;

    // Ermittel Zeitzonen-Offset in Sekunden (lokal vs UTC)
    struct tm local_tm = *localtime(&t);
    struct tm gm_tm = *gmtime(&t);

    time_t local_sec = mktime(&local_tm);
    time_t gm_sec = mktime(&gm_tm);

    time_t offset = difftime(local_sec, gm_sec);
    return t - offset;
}

/*time_t isoStringToTime(const String& iso) {
    // Kopie, da substring() intern keine Nullterminierung garantiert
    char buf[32];
    strncpy(buf, iso.c_str(), sizeof(buf));
    buf[sizeof(buf) - 1] = '\0';

    struct tm tm = {};
    int year, month, day, hour, minute, second;
    char tzSign = 0;
    int tzHour = 0, tzMin = 0;

    if (strchr(buf, 'T') && (strchr(buf, '+') || strchr(buf, '-') || strchr(buf, 'Z'))) {
        if (strchr(buf, 'Z')) {
            sscanf(buf, "%d-%d-%dT%d:%d:%dZ",
                   &year, &month, &day, &hour, &minute, &second);
            tzHour = 0;
            tzMin = 0;
        } else {
            sscanf(buf, "%d-%d-%dT%d:%d:%d%c%d:%d",
                   &year, &month, &day, &hour, &minute, &second,
                   &tzSign, &tzHour, &tzMin);
        }

        tm.tm_year = year - 1900;
        tm.tm_mon = month - 1;
        tm.tm_mday = day;
        tm.tm_hour = hour;
        tm.tm_min = minute;
        tm.tm_sec = second;

        time_t t = timegm(&tm); // interpretiert als UTC
        int offsetSec = ((tzHour * 60) + tzMin) * 60;
        if (tzSign == '-') offsetSec = -offsetSec;

        return t - offsetSec;
    }

    return 0;
}*/

time_t isoStringToTime(const String& iso) {
    // parse Y-M-DTh:m:s and optional ±HH:MM or Z
    int year=0, month=0, day=0, hour=0, minute=0, second=0;
    char tzSign = 0;
    int tzHour = 0, tzMin = 0;

    if (iso.endsWith("Z")) {
        if (sscanf(iso.c_str(), "%d-%d-%dT%d:%d:%dZ",
                   &year, &month, &day, &hour, &minute, &second) < 6) {
            return 0;
        }
    } else {
        // catches both +HH:MM and -HH:MM
        int matched = sscanf(iso.c_str(), "%d-%d-%dT%d:%d:%d%c%d:%d",
                             &year, &month, &day, &hour, &minute, &second,
                             &tzSign, &tzHour, &tzMin);
        if (matched < 6) return 0;
        if (matched < 9) { tzSign = 0; tzHour = 0; tzMin = 0; } // fallback
    }

    // UTC epoch of the given Y/M/D H:M:S (treat fields as UTC)
    time_t utc = utc_from_fields(year, month, day, hour, minute, second);

    // If timezone offset was provided (±HH:MM), convert to UTC:
    // ISO "YYYY...+02:00" means local = utc+2 -> to get UTC subtract +2h
    if (tzSign == '+') {
        int off = tzHour * 3600 + tzMin * 60;
        utc -= off;
    } else if (tzSign == '-') {
        int off = tzHour * 3600 + tzMin * 60;
        utc += off;
    }
    // if tzSign == 0 and not 'Z' then we assumed fields are already local — but your Google events always include Z or +HH:MM.
    return utc;
}



/*time_t isoStringToTime(const String& iso) {
    std::tm tm = {};
    strptime(iso.c_str(), "%Y-%m-%dT%H:%M:%S", &tm);

    // Die Zeit ist in lokaler Zeit, also mktime verwenden
    time_t localTime = mktime(&tm);

    // Offset parsen und abziehen → ergibt UTC-Zeit
    int tzOffset = parseTimezoneOffsetSeconds(iso);
    return localTime - tzOffset;
}*/



/*int GoogleCalendar::isoStringToWeekday(const String& iso) {
    std::tm tm = {};
    strptime(iso.c_str(), "%Y-%m-%dT%H:%M:%S", &tm); 
    // Wochentag: 0 = Sonntag ... 6 = Samstag (tm_wday)
    // Wir wollen 0=Montag ... 6=Sonntag:
    int day = tm.tm_wday == 0 ? 6 : tm.tm_wday - 1;
    return day;
}*/
int GoogleCalendar::isoStringToHour(const String& iso) {
    time_t utcTime = isoStringToTime(iso);
    if (utcTime == 0) return 0;

    int offsetSec = getLocalOffsetSecondsForUtc(utcTime);
    time_t localEpoch = utcTime + offsetSec;

    struct tm tm = *localtime(&localEpoch); // <-- hier statt gmtime
    /*LOG_DEBUG("iso: %s -> utc:%ld offset:%d local:%ld -> %02d:%02d",
              iso.c_str(), (long)utcTime, offsetSec, (long)localEpoch, tm.tm_hour, tm.tm_min);*/
    return tm.tm_hour;
}


int GoogleCalendar::isoStringToWeekday(const String& iso) {
    time_t utcTime = isoStringToTime(iso);
    if (utcTime == 0) return -1;

    int offsetSec = getLocalOffsetSecondsForUtc(utcTime);
    time_t localEpoch = utcTime + offsetSec;
    struct tm tm = *localtime(&localEpoch); // <-- hier statt gmtime
    int wday = tm.tm_wday;
    return wday == 0 ? 6 : wday - 1;
}

/*int GoogleCalendar::isoStringToHour(const String& iso) {
    std::tm tm = {};
    strptime(iso.c_str(), "%Y-%m-%dT%H:%M:%S", &tm);
    return tm.tm_hour;
}*/

bool GoogleCalendar::isAllDayEvent(const String& isoStart, const String& isoEnd) {
    return isoStart.indexOf('T') == -1 && isoEnd.indexOf('T') == -1;
}



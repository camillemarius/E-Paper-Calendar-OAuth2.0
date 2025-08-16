#pragma once
// Local
#include "DateTimeUtils.h"

// Internal Library

// External Library

namespace DateTimeUtils {
    bool parseISODate(const char* iso, struct tm& tmOut) {
        memset(&tmOut, 0, sizeof(tmOut));

        if (strptime(iso, "%Y-%m-%dT%H:%M:%SZ", &tmOut)) return true;
        if (strptime(iso, "%Y-%m-%d", &tmOut)) return true;
        if (strptime(iso, "%Y-%m-%dT%H:%M:%S", &tmOut)) return true;

        return false;
    }

    time_t timegm_portable(struct tm *tm) {
        // Speicher die aktuelle Zeitzone
        char *tz = getenv("TZ");
        char tzbuf[128] = {0};
        if (tz) {
            strncpy(tzbuf, tz, sizeof(tzbuf) - 1);
        }

        // TZ auf UTC setzen
        setenv("TZ", "UTC0", 1);
        tzset();

        // mktime jetzt als UTC interpretieren
        time_t ret = mktime(tm);

        // Alte TZ wiederherstellen
        if (tz) {
            setenv("TZ", tzbuf, 1);
        } else {
            unsetenv("TZ");
        }
        tzset();

        return ret;
    }

    int getDayOfWeek(const String& iso) {
        struct tm tm{};
        strptime(iso.c_str(), "%Y-%m-%dT%H:%M:%S", &tm);
        int wday = tm.tm_wday;
        return (wday == 0) ? 6 : wday - 1; // Sunday=0 → 6
    }

    int getHour(const String& iso) {
        struct tm tm{};
        char* ret = strptime(iso.c_str(), "%Y-%m-%dT%H:%M:%S", &tm);
        if (ret == nullptr) {
            // Parsing failed; handle error, e.g.:
            return -1;  // or some invalid hour indicator
        }
        return tm.tm_hour;
    }

    int getMinute(const String& iso) {
        struct tm tm{};
        strptime(iso.c_str(), "%Y-%m-%dT%H:%M:%S", &tm);
        return tm.tm_min;
    }

    int getDayOffsetFromWeekStart(const String& iso, const struct tm& weekStart) {
        struct tm utcTime{};
        strptime(iso.c_str(), "%Y-%m-%dT%H:%M:%SZ", &utcTime);
        time_t utcTimestamp = timegm_portable(&utcTime);  // bleibt in UTC

        time_t weekStartTimestamp = mktime(const_cast<struct tm*>(&weekStart));  // local time
        struct tm* weekStartTm = localtime(&weekStartTimestamp);
        time_t correctedWeekStart = timegm_portable(weekStartTm);  // konvertiere weekStart in UTC

        int dayDiff = (utcTimestamp - correctedWeekStart) / (60 * 60 * 24);
        return dayDiff;
    }

    struct tm getWeekStart(const struct tm& someDate) {
        struct tm weekStart = someDate;

        // Auf Mitternacht setzen
        weekStart.tm_hour = 0;
        weekStart.tm_min = 0;
        weekStart.tm_sec = 0;

        // Wochentag: Sonntag=0, Montag=1 ... Samstag=6
        int wday = weekStart.tm_wday == 0 ? 7 : weekStart.tm_wday;  // Sonntag=7

        // Verschiebe den Tag zurück auf Montag
        weekStart.tm_mday -= (wday - 1);

        // Normalisieren
        mktime(&weekStart);

        return weekStart;
    }

    struct tm getTodayAsWeekStart() {
        time_t now = time(nullptr);
        struct tm today = *localtime(&now);
        today.tm_hour = 0;
        today.tm_min = 0;
        today.tm_sec = 0;
        mktime(&today); // normalisieren
        return today;
    }
};
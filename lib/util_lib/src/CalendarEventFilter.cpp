// Local
#include "CalendarEventFilter.h"

// Internal Library
#include "DateTimeUtils.h"
#include <logger.h>

// External Library
#include <algorithm>


using namespace DateTimeUtils;

namespace CalendarEventFilter {
    std::vector<CalendarEvent> filterByDateRange(
    const std::vector<CalendarEvent>& events, 
    const time_t& rangeStart, 
    const time_t& rangeEnd
    ) {
        std::vector<CalendarEvent> result;
        for (const auto& event : events) {
            struct tm ts, te;
            DateTimeUtils::parseISODate(event.startISO.c_str(), ts);
            DateTimeUtils::parseISODate(event.endISO.c_str(), te);

            time_t eventStart = DateTimeUtils::timegm_portable(&ts);
            time_t eventEnd   = DateTimeUtils::timegm_portable(&te);

            // Prüfen, ob sich die Zeiträume überschneiden
            if (eventStart < rangeEnd && eventEnd > rangeStart) {
                result.push_back(event);
                //LOG_DEBUG("Event: %s, startDay: %d, endDay: %d", event.title.c_str(), event.startDay, event.endDay);
            }
        }
        return result;
    }

    /*int calculateAllDayEventLines(const std::vector<CalendarEvent>& events, const struct tm& weekStart) {
        std::vector<std::pair<int, int>> points;
        points.reserve(events.size() * 2);

        for (const auto& ev : events) {
            if (!ev.isAllDay) continue;

            // Clamp innerhalb der Woche (Sicherheitscheck)
            //int startDay = ev.startDay < 0 ? 0 : (ev.startDay > 6 ? 6 : ev.startDay);
            //int endDay   = ev.endDay   < 0 ? 0 : (ev.endDay   > 6 ? 6 : ev.endDay);
            
            struct tm localTime = weekStart;
            int startDay = std::max(0, getDayOffsetFromWeekStart(ev.startISO, localTime));
            int endDay = getDayOffsetFromWeekStart(ev.endISO, localTime);
            
            LOG_DEBUG("event: %s; ev.startDay: %d; ev.endDay: %d", ev.title.c_str(), ev.startDay, ev.endDay);
            LOG_DEBUG("event %s; startDay: %d; endDay: %d", ev.title.c_str(), startDay, endDay);

            if (endDay < startDay) continue;

            points.push_back(std::pair<int, int>(startDay, +1));
            points.push_back(std::pair<int, int>(endDay + 1, -1));
        }

        if (points.empty()) return 0;

        std::sort(points.begin(), points.end(),
            [](const std::pair<int, int>& a, const std::pair<int, int>& b) -> bool {
                if (a.first != b.first) return a.first < b.first;
                return a.second < b.second; // Endpunkte zuerst
            }
        );

        int current = 0;
        int maxOverlap = 0;
        for (const auto& p : points) {
            current += p.second;
            if (current > maxOverlap) {
                maxOverlap = current;
            }
        }

        return maxOverlap;
    }*/

int calculateAllDayEventLines( const std::vector<CalendarEvent>& events, const struct tm& weekStart) {
    int dayCounts[7] = {0};

    for (const auto& ev : events) {
        if (!ev.isAllDay) continue;

        struct tm localTime = weekStart;
        int startDay = getDayOffsetFromWeekStart(ev.startISO, localTime);
        int endDay   = getDayOffsetFromWeekStart(ev.endISO, localTime);

        //LOG_DEBUG("Startday: %d", startDay);
        //LOG_DEBUG("Endday: %d", endDay);

        int clampedStart = std::max(0, std::min(7, startDay));
        int clampedEnd   = std::max(0, std::min(7, endDay));

        if (clampedEnd <= clampedStart) {
            LOG_DEBUG("Event außerhalb der Woche, skip: %s", ev.title.c_str());
            continue;
        }

        for (int d = clampedStart; d < clampedEnd; ++d) {
            dayCounts[d]++;
        }
    }

    int maxEvents = 0;
    for (int d = 0; d < 7; ++d) {
        if (dayCounts[d] > maxEvents) maxEvents = dayCounts[d];
    }

    return maxEvents;
}






#include <algorithm>
#include <climits>

void calculateTimeRange( 
    int height,
    const std::vector<CalendarEvent>& events,
    int& outStartHour,
    int& outEndHour,
    int& outHourHeight
) {
    /*LOG_DEBUG("calculateTimeRange() called: height=%d, events=%zu",
              height, events.size());*/

    if (events.empty()) {
        // Fallback: Standardwerte
        outStartHour = 6;
        outEndHour = 20;
        outHourHeight = height / (outEndHour - outStartHour);
        LOG_DEBUG("Fallback result: start=%d end=%d hourHeight=%d",
                  outStartHour, outEndHour, outHourHeight);
        return;
    }

    // Annahme: CalendarEvent hat start und end als std::tm oder std::chrono::time_point
    int minMinutes = INT_MAX;
    int maxMinutes = INT_MIN;
    int skippedAllDay = 0;

    for (const auto& e : events) {
        // Beispiel: wenn CalendarEvent startHour, startMinute, endHour, endMinute hat
        if(e.isAllDay) {
            skippedAllDay++;
            continue;
        }
        int startTotal = e.startHour * 60 + e.startMinute;
        int endTotal   = e.endHour   * 60 + e.endMinute;

        /*LOG_DEBUG("Event: %s, %02d:%02d -> %02d:%02d (%d -> %d)",
                  e.title.c_str(), e.startHour, e.startMinute,
                  e.endHour, e.endMinute,
                  startTotal, endTotal);*/

        minMinutes = std::min(minMinutes, startTotal);
        maxMinutes = std::max(maxMinutes, endTotal);
    }

    /*LOG_DEBUG("Events processed. skippedAllDay=%d", skippedAllDay);
    LOG_DEBUG("Min/Max before offset: min=%d max=%d",
              minMinutes, maxMinutes);*/

    // Offset von 30 Minuten
    minMinutes -= 30;
    maxMinutes += 30;

    /*LOG_DEBUG("After 30min offset: min=%d max=%d",
              minMinutes, maxMinutes);*/

    // Nicht negativ und nicht über 24h
    minMinutes = std::max(0, minMinutes);
    maxMinutes = std::min(24 * 60, maxMinutes);

    /*LOG_DEBUG("Clamped to day range: min=%d max=%d",
              minMinutes, maxMinutes);*/

    // Auf volle Stunden abrunden / aufrunden
    outStartHour = minMinutes / 60;
    if (minMinutes % 60 != 0) {
        outStartHour = std::max(0, outStartHour); // Sicherheit
    }

    outStartHour = minMinutes / 60;
    outEndHour = (maxMinutes + 59) / 60;

    outStartHour = std::max(0, outStartHour);
    outEndHour = std::min(24, outEndHour);

    /*LOG_DEBUG("Computed hours: startHour=%d endHour=%d",
              outStartHour, outEndHour);*/

    // Höhe pro Stunde berechnen
    int hourSpan = outEndHour - outStartHour;
    if (hourSpan <= 0) {
        /*LOG_DEBUG("ERROR: invalid hour span (%d) -> forcing span=1", hourSpan);*/
        hourSpan = 1;
    }

    outHourHeight = height / hourSpan;

    /*LOG_DEBUG("Final result: hourSpan=%d hourHeight=%d",
              hourSpan, outHourHeight);*/
}

};

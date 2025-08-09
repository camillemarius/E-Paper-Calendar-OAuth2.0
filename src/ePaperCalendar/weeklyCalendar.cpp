// Local
#include "weeklyCalendar.h"

// Internal Library
#include <logger.h>

// External Library
#include <ctime>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>
#include <time.h>

WeeklyCalendar::WeeklyCalendar(EpaperDriver& disp)
    : display(disp) {}

bool WeeklyCalendar::parseISODate(const char* iso, struct tm& tmOut) {
    memset(&tmOut, 0, sizeof(tmOut));

    if (strptime(iso, "%Y-%m-%dT%H:%M:%SZ", &tmOut)) return true;
    if (strptime(iso, "%Y-%m-%d", &tmOut)) return true;
    if (strptime(iso, "%Y-%m-%dT%H:%M:%S", &tmOut)) return true;

    return false;
}

void WeeklyCalendar::drawCalendar(const std::vector<CalendarEvent>& events) {
    // Schritt 1: Berechne heutiges Datum um 00:00 Uhr (lokale Zeit)
    time_t now = time(nullptr);
    struct tm todayTm = *localtime(&now);
    todayTm.tm_hour = 0;
    todayTm.tm_min = 0;
    todayTm.tm_sec = 0;
    time_t today = mktime(&todayTm);
    constexpr int SECONDS_PER_DAY = 86400;
    time_t cutoff = today + 4 * SECONDS_PER_DAY; // heute + 3 Tage

    // Schritt 2: Filtere Events, die zumindest teilweise im Zeitraum [heute, heute+3] liegen
    std::vector<CalendarEvent> filteredEvents;
    for (const auto& event : events) {
        struct tm startTm{};
        struct tm endTm{};

        // Parse Startzeit
        if (!parseISODate(event.startISO.c_str(), startTm)) continue;
        time_t eventStartUtc = timegm_portable(&startTm);
        struct tm localStartTm = *localtime(&eventStartUtc);
        localStartTm.tm_hour = 0;
        localStartTm.tm_min = 0;
        localStartTm.tm_sec = 0;
        time_t eventStartDay = mktime(&localStartTm);

        // Parse Endzeit (wenn leer, dann wie Startzeit behandeln)
        if (!event.endISO.isEmpty()) {
            if (!parseISODate(event.endISO.c_str(), endTm)) continue;
        } else {
            endTm = startTm;
        }
        time_t eventEndUtc = timegm_portable(&endTm);
        struct tm localEndTm = *localtime(&eventEndUtc);
        localEndTm.tm_hour = 0;
        localEndTm.tm_min = 0;
        localEndTm.tm_sec = 0;
        time_t eventEndDay = mktime(&localEndTm);

        // Überprüfe, ob Event mit Anzeigezeitraum überschneidet
        if (eventEndDay >= today && eventStartDay <= cutoff) {
            filteredEvents.push_back(event);
        }
    }

    // Schritt 5: Berechne Woche ab heute (heute = Wochenstart)
    struct tm weekStart = getTodayAsWeekStart();

    // Schritt 3: Berechne Layout-Parameter basierend auf den gefilterten Events
    int allDayLines = calculateAllDayEventLines(filteredEvents, weekStart);
    int dynamicHeaderHeight = allDayLines * allDayEventLineHeight;

    // Grid Layout Definition
    int headerY = marginTop;
    int headerHeight = baseHeaderHeight;
    
    int allDayY = headerY + headerHeight;;
    int allDayHeight = dynamicHeaderHeight;
    
    int gridY = allDayY + dynamicHeaderHeight;
    int gridHeight = display.height() - gridY - marginBottom;

    int timedEventsY = allDayY + dynamicHeaderHeight;
    int timedEventsHeight = display.height() - timedEventsY - marginBottom;


    int startHour, endHour, hourH;
    calculateTimeRange(gridHeight, filteredEvents, startHour, endHour, hourH);

    // Schritt 6: Zeichne Seiten
    display.firstPage();
    do {
        drawDayLabels(headerY, headerHeight, filteredEvents, weekStart);
        drawGrid(gridY, startHour, endHour, hourH);
        drawAllDayEvents(allDayY, allDayHeight, filteredEvents, weekStart);
        drawTimedEvents(timedEventsY, timedEventsHeight, filteredEvents, startHour, endHour, hourH, weekStart);
    } while (display.nextPage());
}

void WeeklyCalendar::drawDayLabelsAndGrid(int y, int height, const std::vector<CalendarEvent>& events, const struct tm& weekStart, int startHour, int endHour, int hourHeight) {

    // ---------- DAY LABELS ----------
    display.setFont(&FreeSansBold9pt7b);
    const char* days[] = {"Mo", "Di", "Mi", "Do", "Fr", "Sa", "So"};

    const int verticalOffset = 15;

    if (!events.empty()) {
        for (int i = 0; i < numberOfDays; ++i) {
            struct tm labelDate = weekStart;
            labelDate.tm_mday += i;
            mktime(&labelDate);  // normalisiert das Datum

            int wday = labelDate.tm_wday == 0 ? 6 : labelDate.tm_wday - 1;

            char label[16];
            snprintf(label, sizeof(label), "%s %02d", days[wday], labelDate.tm_mday);

            int x = originX + i * dayColumnWidth;
            int columnCenterX = x + dayColumnWidth / 2;
            int labelWidth = strlen(label) * 12;  // grobe Schätzung
            int labelX = columnCenterX - labelWidth / 2;
            int labelY = y + verticalOffset;

            display.setTextColor(i == 0 ? COLOR_RED : COLOR_BLACK);
            display.setCursor(labelX, labelY);
            display.print(label);
        }
    }

    // ---------- GRID ----------
    int gridTop = y + height;
    int screenBottom = display.height();

    for (int i = 1; i < numberOfDays; ++i) {
        int x = originX + i * dayColumnWidth;
        display.drawLine(x, gridTop, x, screenBottom, COLOR_BLACK);
    }

    for (int h = startHour; h <= endHour; ++h) {
        int lineY = gridTop + (h - startHour) * hourHeight;
        display.drawLine(originX, lineY, originX + calendarWidth, lineY, COLOR_BLACK);

        display.setCursor(originX - 28, lineY + 5);
        display.setFont(&FreeSans9pt7b);
        display.setTextColor(COLOR_BLACK);
        display.print(String(h).c_str());
    }
}

void WeeklyCalendar::drawGrid(int y, int startHour, int endHour, int hourHeight) {
    int screenBottom = display.height();

    for (int i = 1; i < numberOfDays; ++i) {
        int x = originX + i * dayColumnWidth;
        display.drawLine(x, y, x, screenBottom, COLOR_BLACK);
    }

    for (int h = startHour; h <= endHour; ++h) {
        
        
        
        int lineY = y + (h - startHour) * hourHeight;
        if ((h - startHour) % 2 != 1)  {
            display.drawLine(originX, lineY, originX + calendarWidth, lineY, COLOR_BLACK);
        }

        display.setCursor(originX - 28, lineY + 5);
        display.setFont(&FreeSans9pt7b);
        display.setTextColor(COLOR_BLACK);
        display.print(String(h).c_str());
    }
}

time_t WeeklyCalendar::timegm_portable(struct tm *tm) {
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

void WeeklyCalendar::drawDayLabels(int y, int height, const std::vector<CalendarEvent>& events, const struct tm& weekStart) {
    display.setFont(&FreeSansBold9pt7b);
    const char* days[] = {"Mo", "Di", "Mi", "Do", "Fr", "Sa", "So"};
    const int verticalOffset = 15;

    if (!events.empty()) {
        for (int i = 0; i < numberOfDays; ++i) {
            struct tm labelDate = weekStart;
            labelDate.tm_mday += i;
            mktime(&labelDate);  // normalisiert das Datum

            int wday = labelDate.tm_wday == 0 ? 6 : labelDate.tm_wday - 1;

            char label[16];
            snprintf(label, sizeof(label), "%s %02d", days[wday], labelDate.tm_mday);

            int x = originX + i * dayColumnWidth;
            int columnCenterX = x + dayColumnWidth / 2;
            int labelWidth = strlen(label) * 12;  // grobe Schätzung
            int labelX = columnCenterX - labelWidth / 2;
            int labelY = y + verticalOffset;

            display.setTextColor(i == 0 ? COLOR_RED : COLOR_BLACK);
            display.setCursor(labelX, labelY);
            display.print(label);
        }
    }
}

int WeeklyCalendar::getDayOfWeek(const String& iso) {
    struct tm tm{};
    strptime(iso.c_str(), "%Y-%m-%dT%H:%M:%S", &tm);
    int wday = tm.tm_wday;
    return (wday == 0) ? 6 : wday - 1; // Sunday=0 → 6
}

int WeeklyCalendar::getHour(const String& iso) {
    struct tm tm{};
    char* ret = strptime(iso.c_str(), "%Y-%m-%dT%H:%M:%S", &tm);
    if (ret == nullptr) {
        // Parsing failed; handle error, e.g.:
        return -1;  // or some invalid hour indicator
    }
    return tm.tm_hour;
}

int WeeklyCalendar::getMinute(const String& iso) {
    struct tm tm{};
    strptime(iso.c_str(), "%Y-%m-%dT%H:%M:%S", &tm);
    return tm.tm_min;
}

/*int WeeklyCalendar::getDayOffsetFromWeekStart(const String& iso, const struct tm& weekStart) {
    struct tm utcTime{};
    strptime(iso.c_str(), "%Y-%m-%dT%H:%M:%SZ", &utcTime);
    time_t utcTimestamp = timegm_portable(&utcTime);                // UTC korrekt interpretieren
    struct tm localTime = *localtime(&utcTimestamp);       // konvertiere in lokale Zeit
    time_t localTimestamp = mktime(&localTime);            // lokale Zeit als time_t

    time_t weekStartTimestamp = mktime(const_cast<struct tm*>(&weekStart));
    int dayDiff = (localTimestamp - weekStartTimestamp) / (60 * 60 * 24);
    return dayDiff;
}*/

int WeeklyCalendar::getDayOffsetFromWeekStart(const String& iso, const struct tm& weekStart) {
    struct tm utcTime{};
    strptime(iso.c_str(), "%Y-%m-%dT%H:%M:%SZ", &utcTime);
    time_t utcTimestamp = timegm_portable(&utcTime);  // bleibt in UTC

    time_t weekStartTimestamp = mktime(const_cast<struct tm*>(&weekStart));  // local time
    struct tm* weekStartTm = localtime(&weekStartTimestamp);
    time_t correctedWeekStart = timegm_portable(weekStartTm);  // konvertiere weekStart in UTC

    int dayDiff = (utcTimestamp - correctedWeekStart) / (60 * 60 * 24);
    return dayDiff;
}
struct tm WeeklyCalendar::getWeekStart(const struct tm& someDate) {
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

struct tm WeeklyCalendar::getWeekStartFromFirstEvent(const std::vector<CalendarEvent>& events) {
    time_t now = time(nullptr);
    struct tm today = *localtime(&now);
    today.tm_hour = 0;
    today.tm_min = 0;
    today.tm_sec = 0;
    mktime(&today); // normalisieren
    return today;
}

struct tm WeeklyCalendar::getTodayAsWeekStart() {
    time_t now = time(nullptr);
    struct tm today = *localtime(&now);
    today.tm_hour = 0;
    today.tm_min = 0;
    today.tm_sec = 0;
    mktime(&today); // normalisieren
    return today;
}

int WeeklyCalendar::calculateAllDayEventLines(const std::vector<CalendarEvent>& events, const struct tm& weekStart)  {
    std::vector<int> dayEventCount(numberOfDays, 0);

    if (events.empty()) return 0;

    bool hasValidAllDay = false;

    for (const auto& event : events) {
        if (!event.isAllDay) continue;


        // Überprüfe, ob endISO existiert
        if (event.endISO.isEmpty()) continue;

        int startOffset = getDayOffsetFromWeekStart(event.startISO, weekStart);
        int endOffsetRaw = getDayOffsetFromWeekStart(event.endISO, weekStart);
        int endOffset = event.isAllDay ? endOffsetRaw - 1 : endOffsetRaw;
        LOG_DEBUG("event: %s -> %s | startOffset: %d, endOffset: %d",event.startISO.c_str(), event.endISO.c_str(), startOffset, endOffset);

        // Ignoriere Events, die vollständig außerhalb der aktuellen Woche liegen
        //if (endOffset < 0 || startOffset >= numberOfDays) continue;
        //if (startOffset < 0 || endOffset >= numberOfDays) continue;

        int start = std::max(0, startOffset);
        int end   = std::min(numberOfDays - 1, endOffset);

        hasValidAllDay = true; // Mindestens ein sichtbares AllDay-Event gefunden

        for (int d = start; d <= end; ++d) {
            dayEventCount[d]++;
        }
    }

    if (!hasValidAllDay) return 0;

    return *std::max_element(dayEventCount.begin(), dayEventCount.end());
}

void WeeklyCalendar::calculateTimeRange(int height, const std::vector<CalendarEvent>& events, int& outStartHour, int& outEndHour, int& outHourHeight) {
    int minHour = 24;
    int maxHour = 0;

    for (const auto& event : events) {
        if (event.isAllDay) continue;

        int start = event.startHour;//getHour(event.startISO);
        int end = event.endHour; //getHour(event.endISO);
        if (start < minHour) minHour = start;
        if (end > maxHour) maxHour = end;
    }

    if (minHour > maxHour) {
        // Keine Timed-Events, Default-Bereich verwenden
        minHour = visibleStartHour;
        maxHour = visibleEndHour;
    }

    // Reserve für Lesbarkeit
    if (minHour > 0) minHour--;
    if (maxHour < 23) maxHour++;

    int hoursVisible = maxHour - minHour;
    int hourH = height / hoursVisible;

    // Optional: Mindesthöhe pro Stunde setzen
    if (hourH < minEventHeight) {
        // Fallback: Zeitfenster einschränken, wenn zu wenig Platz
        hourH = minEventHeight;
        int maxHoursFit = height / hourH;
        maxHour = minHour + maxHoursFit;
    }

    outStartHour = minHour;
    outEndHour = maxHour;
    outHourHeight = hourH;
}


void WeeklyCalendar::drawAllDayEvents(int y, int height, const std::vector<CalendarEvent>& events, const struct tm& weekStart) {
    display.setFont(&FreeSans9pt7b);

    if (events.empty()) return;

    struct tm localTime = weekStart;

    // Anzahl der AllDay-Lines aus der headerHeight ableiten
    int allDayLines = calculateAllDayEventLines(events, weekStart);
    if (allDayLines == 0) return;

    int lineHeight = height / allDayLines;
    

    // Linienverwaltung: Jede Linie hält eine Liste von Intervallen (Start- und Endtag) für Events, die dort gezeichnet werden
    std::vector<std::vector<std::pair<int,int>>> lines;

    for (const auto& event : events) {
        if (!event.isAllDay) continue;

        int startDay = std::max(0, getDayOffsetFromWeekStart(event.startISO, localTime));
        int endDay = std::min(numberOfDays - 1, getDayOffsetFromWeekStart(event.endISO, localTime) - 1);
        if (endDay < startDay) continue;

        int lineIndex = -1;
        for (int i = 0; i < (int)lines.size(); ++i) {
            bool overlap = false;
            for (const auto& interval : lines[i]) {
                if (!(endDay < interval.first || startDay > interval.second)) {
                    overlap = true;
                    break;
                }
            }
            if (!overlap) {
                lineIndex = i;
                break;
            }
        }

        if (lineIndex == -1) {
            lines.emplace_back();
            lineIndex = (int)lines.size() - 1;
        }

        lines[lineIndex].push_back({startDay, endDay});

        int x = originX + startDay * dayColumnWidth + eventBoxMargin / 2;
        int w = (endDay - startDay + 1) * dayColumnWidth - eventBoxMargin;
        int yPos = y + lineIndex * lineHeight + eventBoxMargin;
        int h = lineHeight - 2 * eventBoxMargin;

        // Farben je nach Kalender anpassen
        uint16_t bgColor = COLOR_BLACK;   // Standard Hintergrundfarbe
        uint16_t ftColor = COLOR_WHITE;   // Standard Schriftfarbe
        uint16_t brColor = COLOR_BLACK;   // Standart Randfarbe

        if (event.calendarId == "8132566a2c345c1b5f411b936db874e78af0907f3dd20fccf7b59f7198459b4a@group.calendar.google.com") {
            bgColor = COLOR_WHITE;
            ftColor = COLOR_BLACK;
            brColor = COLOR_BLACK;
        }

        display.drawTextInRoundedRect(x, yPos, w, h, event.title,
                                      bgColor, ftColor,
                                      eventRadius, eventTextMarginX, eventTextMarginY, 
                                      brColor);
    }
}

void WeeklyCalendar::drawTimedEvents(int y, int height, const std::vector<CalendarEvent>& events, int startHour, int endHour, int hourHeight, const struct tm& weekStart) {
    display.setFont(&FreeSans9pt7b);

    for (const auto& event : events) {
        if (event.isAllDay) continue;

        struct tm localTime = weekStart;
        int day = getDayOffsetFromWeekStart(event.startISO, localTime);
        if (day < 0 || day >= numberOfDays) continue;

        int startH = event.startHour;// getHour(event.startISO);
        int endH   = event.endHour;//getHour(event.endISO);
        int startMin = event.startMinute; //getMinute(event.startISO);
        int endMin   = event.endMinute; //getMinute(event.endISO);

        if (endH <= startHour || startH >= endHour) continue;

        float startTime = std::max((float)startHour, startH + startMin / 60.0f);
        float endTime   = std::min((float)endHour,   endH + endMin   / 60.0f);

        int yStart = y + static_cast<int>((startTime - startHour) * hourHeight + eventBoxMargin);
        int yEnd   = y + static_cast<int>((endTime   - startHour) * hourHeight - eventBoxMargin);

        int x = originX + day * dayColumnWidth + eventBoxMargin;
        int w = dayColumnWidth - 2 * eventBoxMargin;
        int h = yEnd - yStart;

        if (h < minEventHeight) h = minEventHeight;

        // Farben je nach Kalender anpassen
        uint16_t bgColor = COLOR_BLACK;   // Standard Hintergrundfarbe
        uint16_t ftColor = COLOR_WHITE;   // Standard Schriftfarbe
        uint16_t brColor = COLOR_BLACK;   // Standart Randfarbe

        if (event.calendarId == "8132566a2c345c1b5f411b936db874e78af0907f3dd20fccf7b59f7198459b4a@group.calendar.google.com") {
            bgColor = COLOR_WHITE;
            ftColor = COLOR_BLACK;
            brColor = COLOR_BLACK;
        }

        display.drawTextInRoundedRect(x, yStart, w, h, event.title,
                                      bgColor, ftColor,
                                      eventRadius, eventTextMarginX, eventTextMarginY, 
                                      brColor);
    }
}

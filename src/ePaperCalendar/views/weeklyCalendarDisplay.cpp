// Local
#include "weeklyCalendarDisplay.h"

// Internal Library
#include <logger.h>
#include <DateTimeUtils.h>
#include <CalendarEventFilter.h>
#include <CalendarLayout.h>

// External Library
#include <ctime>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>
#include <time.h>


using namespace DateTimeUtils;
using namespace CalendarEventFilter;

WeeklyCalendar::WeeklyCalendar(EpaperDriver& disp)
    : display(disp) {  
    }

void WeeklyCalendar::drawCalendar(const std::vector<CalendarEvent>& events) {
    struct tm today = getTodayAsWeekStart();
    time_t today_ts = mktime(&today);
    time_t cutoff_ts = today_ts + 4 * 24 * 3600; // 4 Tage später

    auto filteredEvents = CalendarEventFilter::filterByDateRange(events, today_ts, cutoff_ts);
    auto weekStart = today; // da today schon der Wochenstart ist

    int allDayLines = CalendarEventFilter::calculateAllDayEventLines(filteredEvents, weekStart);
    auto layout = CalendarLayout::compute(display, allDayLines);

    int startHour, endHour, hourHeight;
    CalendarEventFilter::calculateTimeRange(layout.gridHeight, filteredEvents, startHour, endHour, hourHeight);

    
    // Fuel Gauge
    // Setup i2c
    if(!battery.begin(21, 22))  { // SDA, SCL
        LOG_DEBUG("MAX17048 nicht gefunden!");
    } 
    float voltage = battery.getVoltage();
    int batteryPercent = battery.getPercentage();
    LOG_DEBUG("Voltage: %.3f V\tSOC: %d %%", voltage, batteryPercent);

    display.firstPage();
    do {
        drawBatteryLevel(batteryPercent);
        drawDayLabels(layout.headerY, layout.headerHeight, filteredEvents, weekStart);
        drawGrid(layout.gridY, startHour, endHour, hourHeight);
        drawAllDayEvents(layout.allDayY, layout.allDayHeight, filteredEvents, weekStart);
        drawTimedEvents(layout.timedEventsY, layout.timedEventsHeight, filteredEvents, startHour, endHour, hourHeight, weekStart);
    } while (display.nextPage());
}



void WeeklyCalendar::drawAllDayEvents(int y, int height, const std::vector<CalendarEvent>& events, const struct tm& weekStart) {
    
    //LOG_DEBUG("Parameter: y: %d, height: %d", y, height);
    display.setFont(&FreeSans9pt7b);

    if (events.empty()) return;

    struct tm localTime = weekStart;

    // Linienverwaltung: Jede Linie hält eine Liste von Intervallen (Start- und Endtag) für Events, die dort gezeichnet werden
    std::vector<std::vector<std::pair<int,int>>> lines;

    // Zuweisung der Events zu Linien (ähnlich wie schon in deinem Loop)
    std::vector<int> eventLineIndices;  // speichert Linie für jedes Event

    for (const auto& event : events) {
        if (!event.isAllDay) {
            eventLineIndices.push_back(-1);
            continue;
        }
        //LOG_DEBUG("All day event title: %s",event.title.c_str());

        int startDay = std::max(0, getDayOffsetFromWeekStart(event.startISO, localTime));
        int endDay = std::min(numberOfDays - 1, getDayOffsetFromWeekStart(event.endISO, localTime) - 1);
        if (endDay < startDay) {
            eventLineIndices.push_back(-1);
            continue;
        }
        //LOG_DEBUG("startday: %d; endday: %d", startDay, endDay);

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
        eventLineIndices.push_back(lineIndex);
    }

    int allDayLines = (int)lines.size();
    if (allDayLines == 0) return;
    
    const int minLineHeight = 15;
    int lineHeight = height / allDayLines;
    if (lineHeight < minLineHeight) lineHeight = minLineHeight;
    //LOG_DEBUG("allDayLines: %d; lineHeight: %d",allDayLines, lineHeight);

    // Zeichne Events auf den jeweiligen Linien
    for (size_t i = 0; i < events.size(); ++i) {
        if (eventLineIndices[i] == -1) continue;  // kein AllDay Event

        const auto& event = events[i];

        int startDay = std::max(0, getDayOffsetFromWeekStart(event.startISO, localTime));
        int endDay = std::min(numberOfDays - 1, getDayOffsetFromWeekStart(event.endISO, localTime) - 1);

        int lineIndex = eventLineIndices[i];

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

/*void WeeklyCalendar::drawTimedEvents(int y, int height, const std::vector<CalendarEvent>& events, int startHour, int endHour, int hourHeight, const struct tm& weekStart) {
    display.setFont(&FreeSans9pt7b);

    for (const auto& event : events) {
        if (event.isAllDay) continue;

        struct tm localTime = weekStart;
        int day = getDayOffsetFromWeekStart(event.startISO, localTime);
        if (day < 0 || day >= numberOfDays) continue;

        int startH = event.startHour;
        int endH   = event.endHour;
        int startMin = event.startMinute;
        int endMin   = event.endMinute;

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
}*/
void WeeklyCalendar::drawTimedEvents(int y, int height, const std::vector<CalendarEvent>& events, int startHour, int endHour, int hourHeight, const struct tm& weekStart) {
    display.setFont(&FreeSans9pt7b);

    for (int day = 0; day < numberOfDays; ++day) {
        // Alle Events für diesen Tag sammeln
        std::vector<const CalendarEvent*> dayEvents;
        for (const auto& event : events) {
            if (event.isAllDay) continue;

            struct tm localTime = weekStart;
            int eventDay = getDayOffsetFromWeekStart(event.startISO, localTime);
            if (eventDay == day) dayEvents.push_back(&event);
        }

        // Events nach Startzeit sortieren
        std::sort(dayEvents.begin(), dayEvents.end(), [](const CalendarEvent* a, const CalendarEvent* b){
            return (a->startHour + a->startMinute/60.0f) < (b->startHour + b->startMinute/60.0f);
        });

        // Jedes Event zeichnen
        for (size_t i = 0; i < dayEvents.size(); ++i) {
            const auto& event = *dayEvents[i];

            int startH = event.startHour;
            int endH   = event.endHour;
            int startMin = event.startMinute;
            int endMin   = event.endMinute;

            if (endH <= startHour || startH >= endHour) continue;

            float startTime = std::max((float)startHour, startH + startMin / 60.0f);
            float endTime   = std::min((float)endHour,   endH + endMin   / 60.0f);

            int yStart = y + static_cast<int>((startTime - startHour) * hourHeight + eventBoxMargin);
            int yEnd   = y + static_cast<int>((endTime   - startHour) * hourHeight - eventBoxMargin);
            int h = yEnd - yStart;
            if (h < minEventHeight) h = minEventHeight;

            // Überlappende Events zählen
            int overlapCount = 1;
            int positionIndex = 0;
            for (size_t j = 0; j < dayEvents.size(); ++j) {
                if (i == j) continue;
                const auto& other = *dayEvents[j];
                float otherStart = other.startHour + other.startMinute / 60.0f;
                float otherEnd   = other.endHour   + other.endMinute   / 60.0f;
                if (otherEnd > startTime && otherStart < endTime) {
                    overlapCount++;
                    if (j < i) positionIndex++;  // Reihenfolge im Slot
                }
            }

            int eventWidth = (dayColumnWidth - 2 * eventBoxMargin) / overlapCount;
            int x = originX + day * dayColumnWidth + eventBoxMargin + positionIndex * eventWidth;

            // Farben je nach Kalender anpassen
            uint16_t bgColor = COLOR_BLACK;   // Standard Hintergrundfarbe
            uint16_t ftColor = COLOR_WHITE;   // Standard Schriftfarbe
            uint16_t brColor = COLOR_BLACK;   // Standard Randfarbe

            if (event.calendarId == "8132566a2c345c1b5f411b936db874e78af0907f3dd20fccf7b59f7198459b4a@group.calendar.google.com") {
                bgColor = COLOR_WHITE;
                ftColor = COLOR_BLACK;
                brColor = COLOR_BLACK;
            }

            display.drawTextInRoundedRect(x, yStart, eventWidth, h, event.title,
                                          bgColor, ftColor,
                                          eventRadius, eventTextMarginX, eventTextMarginY, 
                                          brColor);
        }
    }
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
        if (h % 2 == 0) {   // ungerade Stunden auslassen
        //if ((h - startHour) % 2 != 1)  { // gerade Stunden auslassen
            display.drawLine(originX, lineY, originX + calendarWidth, lineY, COLOR_BLACK);
        }

        if (h != startHour && h != endHour) {
            display.setCursor(originX - 28, lineY + 5);
            display.setFont(&FreeSans9pt7b);
            display.setTextColor(COLOR_BLACK);
            display.print(String(h).c_str());
        }
    }
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

void WeeklyCalendar::drawBatteryLevel(int batteryPercent) {
    display.setFont(&FreeSans9pt7b);
    display.setTextColor(COLOR_BLACK);

    String text = String(batteryPercent) + "%";

    // Position rechts oben (10px vom Rand)
    int x = display.width() - 20 - text.length() * 6; // 6 px pro Zeichen grob
    int y = 15; // 15 px von oben

    display.setCursor(x, y);
    display.print(text.c_str());
}



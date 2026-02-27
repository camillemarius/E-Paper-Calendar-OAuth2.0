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
        drawAllDayEvents(layout.allDayY, layout.allDayHeight, filteredEvents, weekStart);


        // Filtere nur Timed-Events (Noch tewas unschön gelöst.....) Besser timedEvents direkt drawTimedEvent übergeben?
        //std::vector<CalendarEvent> timedEvents;
        //for (const auto& e : filteredEvents) {
        //    if (!e.isAllDay) timedEvents.push_back(e);
        //}
        //if (!timedEvents.empty()) {
        //} else {
        //    drawGrid(layout.gridY, startHour, endHour, hourHeight);
            drawTimedEvents(layout.timedEventsY, layout.timedEventsHeight, filteredEvents, startHour, endHour, hourHeight, weekStart);
        //}
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

    // Prüfen, ob überhaupt Timed-Events in der ganzen Woche existieren
    bool hasTimedEvents = false;
    for (const auto& e : events) {
        if (!e.isAllDay) {
            hasTimedEvents = true;
            break;
        }
    }

    if (hasTimedEvents) {
        drawGrid(y, startHour, endHour, hourHeight);
    }



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
}*/

void WeeklyCalendar::drawTimedEvents(int y, int height, const std::vector<CalendarEvent>& events,
                                     int startHour, int endHour, int hourHeight, const struct tm& weekStart) {
    display.setFont(&FreeSans9pt7b);

    // --- Schritt 1: Timed-Events pro Tag sammeln ---
    std::vector<std::vector<const CalendarEvent*>> eventsPerDay(numberOfDays);
    bool hasTimedEvents = false;

    for (const auto& event : events) {
        if (event.isAllDay) continue;

        struct tm localTime = weekStart;
        int day = getDayOffsetFromWeekStart(event.startISO, localTime);
        if (day < 0 || day >= numberOfDays) continue;

        eventsPerDay[day].push_back(&event);
        hasTimedEvents = true;  // Sobald ein Timed-Event existiert
    }

    // --- Schritt 2: nur zeichnen, wenn Timed-Events existieren ---
    if (!hasTimedEvents) {
        display.setTextColor(COLOR_BLACK);

        // --- Trennlinie am oberen Rand des Timed-Event-Bereichs ---
        display.drawLine(originX, y, originX+calendarWidth, y, COLOR_BLACK);

        const char* text1 = "\"Zit zum d Bei ufehebe\"";
        const char* text2 = "Keine Termine gefunden";

        // Text1-Breite ermitteln
        int16_t x1, y1;
        uint16_t w1, h1;
        display.getTextBounds(text1, 0, 0, &x1, &y1, &w1, &h1);
        int xText1 = (display.width() - w1) / 2;
        int yText1 = display.height() / 2 + 40;  // obere Zeile leicht nach unten

        display.setCursor(xText1, yText1);
        display.print(text1);

        // Text2-Breite ermitteln
        int16_t x2, y2;
        uint16_t w2, h2;
        display.getTextBounds(text2, 0, 0, &x2, &y2, &w2, &h2);
        int xText2 = (display.width() - w2) / 2;
        int yText2 = yText1 + h1 + 8;  // Abstand 4px nach unten
        display.setCursor(xText2, yText2);
        display.print(text2);

        return;
    }


    drawGrid(y, startHour, endHour, hourHeight);

    // --- Schritt 3: Events pro Tag zeichnen ---
    for (int day = 0; day < numberOfDays; ++day) {
        auto& dayEvents = eventsPerDay[day];
        if (dayEvents.empty()) continue;

        // Events nach Startzeit sortieren
        std::sort(dayEvents.begin(), dayEvents.end(), [](const CalendarEvent* a, const CalendarEvent* b) {
            return (a->startHour + a->startMinute / 60.0f) < (b->startHour + b->startMinute / 60.0f);
        });

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
                    if (j < i) positionIndex++;  
                }
            }

            int eventWidth = (dayColumnWidth - 2 * eventBoxMargin) / overlapCount;
            int x = originX + day * dayColumnWidth + eventBoxMargin + positionIndex * eventWidth;

            // Farben
            uint16_t bgColor = COLOR_BLACK;
            uint16_t ftColor = COLOR_WHITE;
            uint16_t brColor = COLOR_BLACK;
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

    batteryPercent = constrain(batteryPercent, 0, 100);
    String text = String(batteryPercent) + "%";


    // Position rechts oben (10px vom Rand)
    int x = display.width() - 30 - text.length() * 6; // 6 px pro Zeichen grob
    int y = 25; // 15 px von oben

    display.setCursor(x, y);
    display.print(text.c_str());
}



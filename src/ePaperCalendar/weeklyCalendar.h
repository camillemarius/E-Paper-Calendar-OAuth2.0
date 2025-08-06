#pragma once

// Local
#include "views/IDisplay.h"
#include "color.h"

// Internal Library
#include <ePaperDriver.h>
#include <CalendarTypes.h>

// External Library
#include <vector>
#include <string>

class EpaperDriver;

class WeeklyCalendar {
public:
    explicit WeeklyCalendar(EpaperDriver& disp);
    void drawCalendar(const std::vector<CalendarEvent>& events);

private:
    EpaperDriver& display;


    // Außenabstände
    static constexpr int marginLeft   = 40;
    static constexpr int marginRight  = 5;
    static constexpr int marginTop    = 10;
    static constexpr int marginBottom = 20;

    // Layout
    static constexpr int originX = marginLeft;

    static constexpr int numberOfDays = 4;
    static constexpr int calendarWidth = 800 - marginLeft - marginRight;
    static constexpr int dayColumnWidth = calendarWidth / numberOfDays;

    // Kopfbereich
    static constexpr int baseHeaderHeight = 30;

    // Default-Zeitbereich (Fallback)
    static constexpr int visibleStartHour = 7;
    static constexpr int visibleEndHour   = 18;

    // Ereignisdarstellung
    static constexpr int minEventHeight = 25;
    static constexpr int eventTextMarginX = 5;
    static constexpr int eventTextMarginY = 5;
    static constexpr int eventBoxMargin = 2;
    static constexpr int eventRadius = 4;

    // All-Day Events
    static constexpr int allDayEventLineHeight = 30;




    void drawGrid(int y, int startHour, int endHour, int hourHeight);
    void drawDayLabelsAndGrid(int y, int height,const std::vector<CalendarEvent>& events,const struct tm& weekStart,int startHour,int endHour,int hourHeight);
    void drawDayLabels(int y, int height, const std::vector<CalendarEvent>& events, const struct tm& weekStart);
    void drawAllDayEvents(int y, int height, const std::vector<CalendarEvent>& events, const struct tm& weekStart);
    void drawTimedEvents( int y, int height, const std::vector<CalendarEvent>& events, int startHour, int endHour, int hourHeight, const struct tm& weekStart);



    time_t timegm_portable(struct tm *tm);
    int getDayOfWeek(const String& isoString);
    int getHour(const String& isoString);
    int getMinute(const String& iso);
    int calculateAllDayEventLines(const std::vector<CalendarEvent>& events, const struct tm& weekStart);
    int getDayOffsetFromWeekStart(const String& iso, const struct tm& weekStart);
    struct tm getWeekStart(const struct tm& someDate);
    struct tm getWeekStartFromFirstEvent(const struct tm& eventDate);
    struct tm getWeekStartFromFirstEvent(const std::vector<CalendarEvent>& events);
    struct tm getTodayAsWeekStart();
    bool parseISODate(const char* iso, struct tm& tmOut);
    void calculateTimeRange(int height, const std::vector<CalendarEvent>& events, int& outStartHour, int& outEndHour, int& outHourHeight);
};



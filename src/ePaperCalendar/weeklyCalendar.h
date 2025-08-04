#pragma once

// Local
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

    static constexpr int displayHeight = 480;


    // Margins
    static constexpr int marginLeft   = 40;
    static constexpr int marginRight  = 5;
    static constexpr int marginTop    = 30;
    static constexpr int marginBottom = 20;

    // Calendar layout
    static constexpr int originX = marginLeft;
    static constexpr int originY = marginTop;

    static constexpr int visibleStartHour = 7;
    static constexpr int visibleEndHour = 18;
    static constexpr int visibleHours = visibleEndHour - visibleStartHour;

    static constexpr int numberOfDays = 4;
    static constexpr int calendarWidth = 800 - marginLeft - marginRight; 
    static constexpr int dayColumnWidth = calendarWidth / numberOfDays;

    static constexpr int baseHeaderHeight= 30;

    static constexpr int calendarHeight = displayHeight - marginTop- marginBottom - baseHeaderHeight;
    static constexpr int hourHeight = calendarHeight/visibleHours;
    static constexpr int minEventHeight = (hourHeight*  3) / 4;

    static constexpr int eventTextMarginX = 5;
    static constexpr int eventTextMarginY = 5;

    static constexpr int eventBoxMargin = 2;
    static constexpr int eventRadius = 4;


    static constexpr int baseAllDayEventsHeight = 15;
    static constexpr int allDayEventLineHeight = 15;




    void drawGrid(int headerHeight, int startHour, int endHour, int hourHeight);
    void drawDayLabels(const std::vector<CalendarEvent>& events, const struct tm& weekStart);
    void drawAllDayEvents(const std::vector<CalendarEvent>& events, int headerHeight, const struct tm& weekStart);
    void drawTimedEvents(const std::vector<CalendarEvent>& events, int headerHeight, int startHour, int endHour, int hourHeight, const struct tm& weekStart);



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
    void calculateTimeRange(const std::vector<CalendarEvent>& events, int dynamicHeaderHeight, int& outStartHour, int& outEndHour, int& outHourHeight);
};



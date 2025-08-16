#pragma once
#include <vector>
#include <ctime>
#include <CalendarTypes.h>

namespace CalendarEventFilter {

    std::vector<CalendarEvent> filterByDateRange(
        const std::vector<CalendarEvent>& events,
        const time_t& rangeStart,
        const time_t& rangeEnd
    );

    int calculateAllDayEventLines(
        const std::vector<CalendarEvent>& events,
        const struct tm& weekStart
    );

    void calculateTimeRange(
        int height,
        const std::vector<CalendarEvent>& events,
        int& outStartHour,
        int& outEndHour,
        int& outHourHeight
    );

} // namespace CalendarEventFilter

// CalendarLayout.cpp
#include "CalendarLayout.h"

CalendarLayout CalendarLayout::compute(EpaperDriver& display, int allDayLines) {
    CalendarLayout l;
    l.headerY = 10;
    l.headerHeight = 20;

    l.allDayY = l.headerY + l.headerHeight;
    l.allDayHeight = allDayLines * 30;

    l.gridY = l.allDayY + l.allDayHeight;
    l.gridHeight = display.height() - l.gridY;

    l.timedEventsY = l.gridY;
    l.timedEventsHeight = l.gridHeight;
    return l;
}

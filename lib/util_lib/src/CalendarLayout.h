// CalendarLayout.h
#pragma once

// Local

// Internal Library
#include <ePaperDriver.h>
#include <CalendarTypes.h>

// External Library
#include <vector>
#include <string>

struct CalendarLayout {
    int headerY;
    int headerHeight;
    int allDayY;
    int allDayHeight;
    int gridY;
    int gridHeight;
    int timedEventsY;
    int timedEventsHeight;

    static CalendarLayout compute(EpaperDriver& display, int allDayLines);
};

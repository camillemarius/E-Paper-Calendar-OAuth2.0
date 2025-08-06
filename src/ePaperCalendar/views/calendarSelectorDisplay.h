#pragma once
// Local
#include "IDisplay.h"

// Internal Library
#include <ePaperDriver.h>

// External Library
#include <string>

class CalendarSelectorDisplay : public IDisplay{
public:
    CalendarSelectorDisplay(EpaperDriver& display);

protected:
    String getTitle() const override;
    String getDescription() const override;
};

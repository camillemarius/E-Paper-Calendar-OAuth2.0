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
    void show(const String& url);

private:
    void drawQRCode(const String& url);
    void drawUserCode(const String& code);
    void drawHeader();
};

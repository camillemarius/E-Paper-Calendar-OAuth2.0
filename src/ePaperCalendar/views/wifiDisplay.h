#pragma once
// Local
#include "IDisplay.h"

// Internal Library
#include <ePaperDriver.h>

// External Library
#include <string>

class WifiDisplay : public IDisplay{
public:
    WifiDisplay(EpaperDriver& display);

protected:
    String getTitle() const override;
    String getDescription() const override;
};

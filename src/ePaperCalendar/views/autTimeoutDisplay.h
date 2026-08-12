#pragma once
// Local
#include "IDisplay.h"

// Internal Library
#include <ePaperDriver.h>

// External Library
#include <string>

class AutTimeoutDisplay : public IDisplay{
public:
    AutTimeoutDisplay(EpaperDriver& display);

protected:
    String getTitle() const override;
    String getDescription() const override;
};

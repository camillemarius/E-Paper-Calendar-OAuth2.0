#pragma once
// Local
#include "IDisplay.h"

// Internal Library
#include <ePaperDriver.h>

// External Library
#include <string>

class AuthTimeoutDisplay : public IDisplay{
public:
    AuthTimeoutDisplay(EpaperDriver& display);

protected:
    String getTitle() const override;
    String getDescription() const override;
};

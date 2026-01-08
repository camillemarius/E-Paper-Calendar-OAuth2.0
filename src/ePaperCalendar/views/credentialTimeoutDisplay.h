#pragma once
// Local
#include "IDisplay.h"

// Internal Library
#include <ePaperDriver.h>

// External Library
#include <string>

class CredentialTimeoutDisplay : public IDisplay{
public:
    CredentialTimeoutDisplay(EpaperDriver& display);

protected:
    String getTitle() const override;
    String getDescription() const override;
};

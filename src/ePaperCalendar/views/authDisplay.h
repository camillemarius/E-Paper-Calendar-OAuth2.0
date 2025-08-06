#pragma once
// Local
#include "IDisplay.h"

// Internal Library
#include <ePaperDriver.h>

// External Library
#include <string>

class AuthDisplay : public IDisplay{

public:
    AuthDisplay(EpaperDriver& display);
    
    // Spezialmethode NUR für Auth
    void showWithUserCode(const String& qrData, const String& userCode);

protected:
    // Spezialmethode NUR für Auth
    void drawUserCode(const String& code);

    String getTitle() const override;
    String getDescription() const override;
};

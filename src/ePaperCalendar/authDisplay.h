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
    void show(const String& verificationUrl, const String& userCode);

private:
    void drawQRCode(const String& url);
    void drawUserCode(const String& code);
    void drawHeader();
};

#ifndef WIFIHANDLER_H
#define WIFIHANDLER_H

#include <WiFiManager.h>
#include <functional>

class WiFiHandler {
public:
    // Callback type for when AP mode starts
    typedef std::function<void(const String& portalUrl)> APCallback;

    WiFiHandler(int timeout);

    // Starts WiFi connection or AP mode if needed
    bool begin();

    // Register a callback function to be called when AP mode starts
    void onAccessPointStart(APCallback cb);

private:
    APCallback userCallback;

    const String m_encryption;
    const String m_ssid;
    const String m_password;
};

#endif // WIFIHANDLER_H

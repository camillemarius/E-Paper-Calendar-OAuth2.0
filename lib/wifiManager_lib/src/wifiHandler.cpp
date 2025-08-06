#include "WiFiHandler.h"
#include <WiFi.h>  // Use <ESP8266WiFi.h> for ESP8266
#include <logger.h>

WiFiHandler::WiFiHandler(int timeout) : userCallback(nullptr),
                m_encryption("WPA"),m_ssid("E-Paper Kalender"), m_password("123456789") {}

void WiFiHandler::onAccessPointStart(APCallback cb) {
    userCallback = cb;
}


bool WiFiHandler::begin() {
    WiFiManager wifiManager;

    wifiManager.setConnectTimeout(30); // Timeout für Verbindungsversuch

    // Optional: Nur "WiFi"-Eintrag im Menü anzeigen
    std::vector<const char*> menu = {"wifi"};
    wifiManager.setMenu(menu);

    // Callback beim Start des Access Points
    wifiManager.setAPCallback([this](WiFiManager* wm) {
        if (userCallback) {
            String wifiQR = "WIFI:T:" + m_encryption + ";S:" + m_ssid + ";P:" + m_password + ";;";
            userCallback(wifiQR);
        }
    });

    // Öffnet direkt das Konfigurationsportal (WLAN-Auswahlseite)
    wifiManager.startConfigPortal("E-Paper Kalender", "123456789");

    if (WiFi.status() == WL_CONNECTED) {
        LOG_INFO("WiFi connected.");
        LOG_INFO("IP Address: %s", WiFi.localIP().toString().c_str());
        return true;
    } else {
        LOG_ERROR("No WiFi connection established.");
        return false;
    }
}


/*void WiFiHandler::begin() {
    WiFiManager wifiManager;

    wifiManager.setConnectTimeout(30); // timeout in seconds

    // Setzt das Konfigurationsmenü – nur der "WiFi"-Eintrag ist sichtbar
    std::vector<const char*> menu = {"wifi"};
    wifiManager.setMenu(menu);

    wifiManager.setAPCallback([this](WiFiManager* wm) {
        if (userCallback) {
            //String portalURL = "http://" + WiFi.softAPIP().toString();
            //String wifiQR = "WIFI:T:WPA;S:E-Paper Kalender;P:123456789;;";
            String wifiQR = "WIFI:T:" + m_encryption + ";S:" + m_ssid + ";P:" + m_password + ";;";
            userCallback(wifiQR);
        }
    });

    if (!wifiManager.autoConnect("E-Paper Kalender", "123456789")) {
        LOG_ERROR("Failed to connect to WiFi. Restarting...");
        delay(3000);
        ESP.restart();
    }

    LOG_INFO("WiFi connected.");
    LOG_INFO("IP Address: %s", WiFi.localIP().toString().c_str());
}*/

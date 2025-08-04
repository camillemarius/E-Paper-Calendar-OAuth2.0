#include "WiFiHandler.h"
#include <WiFi.h>  // Use <ESP8266WiFi.h> for ESP8266
#include <logger.h>

WiFiHandler::WiFiHandler(int timeout) : userCallback(nullptr),
                m_encryption("WPA"),m_ssid("E-Paper Kalender"), m_password("123456789") {}

void WiFiHandler::onAccessPointStart(APCallback cb) {
    userCallback = cb;
}

void WiFiHandler::begin() {
    WiFiManager wifiManager;

    wifiManager.setConnectTimeout(30); // timeout in seconds
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
}

#include "WiFiHandler.h"
#include <WiFi.h>  // Use <ESP8266WiFi.h> for ESP8266
#include <esp_wifi.h>
#include <logger.h>

// Kanal und BSSID des letzten Access Points überleben den Deep Sleep im RTC-Speicher.
// Damit entfällt beim nächsten Wakeup der Kanal-Scan.
RTC_DATA_ATTR static uint8_t rtcChannel = 0;
RTC_DATA_ATTR static uint8_t rtcBssid[6];
static const unsigned long FAST_CONNECT_TIMEOUT_MS = 5000;

WiFiHandler::WiFiHandler(int timeout) : userCallback(nullptr), timeoutCallback(nullptr),
                m_encryption("WPA"),m_ssid("E-Paper Kalender"), m_password("123456789"),
                m_timeout(timeout) {}

void WiFiHandler::onAccessPointStart(APCallback cb) {
    userCallback = cb;
}

void WiFiHandler::onTimeout(std::function<void()> cb) {
    timeoutCallback = cb;
}


// Verbindet direkt mit Kanal und BSSID vom letzten Mal (ohne Scan).
bool WiFiHandler::fastConnect(WiFiManager& wifiManager) {
    if (rtcChannel == 0) return false;

    String ssid = wifiManager.getWiFiSSID(true);
    String pass = wifiManager.getWiFiPass(true);
    if (ssid.isEmpty()) return false;

    // Kanal/BSSID nur im RAM setzen: die gespeicherte WLAN-Konfiguration bleibt unverändert
    esp_wifi_set_storage(WIFI_STORAGE_RAM);
    WiFi.begin(ssid.c_str(), pass.c_str(), rtcChannel, rtcBssid);
    bool connected = (WiFi.waitForConnectResult(FAST_CONNECT_TIMEOUT_MS) == WL_CONNECTED);
    if (!connected) {
        LOG_WARNING("Schnellverbindung fehlgeschlagen - normaler Verbindungsaufbau");
        rtcChannel = 0;
        WiFi.disconnect();
        // Laufende Konfiguration wieder ohne Kanal/BSSID, damit WiFiManager normal sucht
        WiFi.begin(ssid.c_str(), pass.c_str(), 0, nullptr, false);
    }
    esp_wifi_set_storage(WIFI_STORAGE_FLASH);
    return connected;
}

void WiFiHandler::rememberAccessPoint() {
    const uint8_t* bssid = WiFi.BSSID();
    if (bssid == nullptr) return;
    memcpy(rtcBssid, bssid, sizeof(rtcBssid));
    rtcChannel = WiFi.channel();
}

bool WiFiHandler::begin(bool allowPortal) {
    WiFiManager wifiManager;

    if (fastConnect(wifiManager)) {
        LOG_INFO("WiFi connected (Kanal %u, ohne Scan).", rtcChannel);
        LOG_INFO("IP Address: %s", WiFi.localIP().toString().c_str());
        return true;
    }

    wifiManager.setConnectTimeout(allowPortal ? 30 : 10); // Timeout für Verbindungsversuch
    wifiManager.setConfigPortalTimeout(m_timeout);
    // Ohne Portal kehrt autoConnect bei einem Fehler sofort zurück (kein Access Point)
    wifiManager.setEnableConfigPortal(allowPortal);
    
    // Optional: Nur "WiFi"-Eintrag im Menü anzeigen
    //std::vector<const char*> menu = {"wifi"};
    //wifiManager.setMenu(menu);

    // Callback beim Start des Access Points
    wifiManager.setAPCallback([this](WiFiManager* wm) {
        if (userCallback) {
            String wifiQR = "WIFI:T:" + m_encryption + ";S:" + m_ssid + ";P:" + m_password + ";;";
            userCallback(wifiQR);
        }
    });

    // Öffnet direkt das Konfigurationsportal (WLAN-Auswahlseite)
    //wifiManager.startConfigPortal("E-Paper Kalender", "123456789");
    if (!wifiManager.autoConnect(m_ssid.c_str(), m_password.c_str())) {
        LOG_ERROR("WiFi setup timeout or failed.");
        // Timeout-Anzeige nur, wenn das Portal tatsächlich lief
        if (allowPortal && timeoutCallback) {
            timeoutCallback(); 
        }
        return false;
    }

    if (WiFi.status() == WL_CONNECTED) {
        rememberAccessPoint();
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

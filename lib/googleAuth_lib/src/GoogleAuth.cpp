// Local
#include "GoogleAuth.h"

// Internal Library
#include <logger.h>

// External Library
#include <HTTPClient.h>

GoogleAuth::GoogleAuth(const String& clientId, const String& clientSecret, const String& scope)
  : _clientId(clientId), _clientSecret(clientSecret), _scope(scope) {}

bool GoogleAuth::initialize() {
  return _tokenStorage.mount();
}

bool GoogleAuth::authorize(unsigned long maxWaitSeconds) {
  if (hasValidAccessToken()) return true;
  if (hasRefreshToken() && refreshAccessToken()) return true;

  if (!startDeviceCodeFlow()) return false;

  unsigned long start = millis();
  while ((millis() - start) < maxWaitSeconds * 1000UL) {
    if (pollForToken()) return true;
    delay(_interval * 1000);
  }

  LOG_WARNING("Timeout bei der Autorisierung.");
  return false;
}

String GoogleAuth::getAccessToken() {
  authorize();
  return _accessToken;
}

void GoogleAuth::onAuthPrompt(AuthPromptCallback cb) { 
  _authPromptCallback = cb; 
}


String GoogleAuth::urlEncode(const String &str) {
  String encoded = "";
  char c;
  char code0;
  char code1;
  for (int i = 0; i < str.length(); i++) {
    c = str.charAt(i);
    if ( ('a' <= c && c <= 'z') ||
         ('A' <= c && c <= 'Z') ||
         ('0' <= c && c <= '9') ||
          c == '-' || c == '_' || c == '.' || c == '~' ) {
      encoded += c;
    } else {
      code1 = (c & 0xf) + '0';
      if ((c & 0xf) > 9) code1 = (c & 0xf) - 10 + 'A';
      c = (c >> 4) & 0xf;
      code0 = c + '0';
      if (c > 9) code0 = c - 10 + 'A';
      encoded += '%';
      encoded += code0;
      encoded += code1;
    }
  }
  return encoded;
}

bool GoogleAuth::hasValidAccessToken() const {
  return !_accessToken.isEmpty() && millis() < _accessTokenExpiresAt;
}

bool GoogleAuth::hasRefreshToken() {
  if (!_refreshToken.isEmpty()) {
    return true;
  }

  _refreshToken = _tokenStorage.loadRefreshToken();
  return !_refreshToken.isEmpty();
}

String GoogleAuth::getRefreshToken() const {
  return _refreshToken;
}

bool GoogleAuth::postFormUrlencoded(const String& url, const String& postData, String& responsePayload) {
  HTTPClient http;
  http.begin(url);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  int httpCode = http.POST(postData);
  responsePayload = http.getString();
  http.end();

  if (httpCode < 200 || httpCode >= 300) {
    LOG_ERROR("HTTP Fehler %d bei POST zu %s", httpCode, url);
    return false;
  }

  return true;
}

bool GoogleAuth::parseJson(const String& payload, DynamicJsonDocument& doc) {
  auto err = deserializeJson(doc, payload);
  if (err) {
    LOG_ERROR("JSON Fehler: %s", err);
    return false;
  }
  return true;
}

bool GoogleAuth::startDeviceCodeFlow() {
  String payload;
  String postData = "client_id=" + _clientId + "&scope=" + urlEncode(_scope);

  if (!postFormUrlencoded("https://oauth2.googleapis.com/device/code", postData, payload)) {
    return false;
  }

  DynamicJsonDocument doc(1024);
  if (!parseJson(payload, doc)) return false;

  _deviceCode = doc["device_code"].as<String>();
  _interval = doc["interval"] | 5;

  LOG_INFO("Besuche: %s", doc["verification_url"].as<const char*>());
  LOG_INFO("Gib den Code ein: %s", doc["user_code"].as<const char*>());

  if (_authPromptCallback) {
    _authPromptCallback(doc["verification_url"].as<String>(), doc["user_code"].as<String>());
  }

  return true;
}

bool GoogleAuth::pollForToken() {
  LOG_INFO("Warte auf Benutzerautorisierung...");

  while (true) {
    String payload;
    String postData = "client_id=" + _clientId +
                      "&client_secret=" + _clientSecret +
                      "&device_code=" + _deviceCode +
                      "&grant_type=urn:ietf:params:oauth:grant-type:device_code";

    if (!postFormUrlencoded("https://oauth2.googleapis.com/token", postData, payload)) {
      return false;
    }

    DynamicJsonDocument doc(2048);
    if (!parseJson(payload, doc)) return false;

    if (doc.containsKey("access_token")) {
      _accessToken = doc["access_token"].as<String>();
      _accessTokenExpiresAt = millis() + ((doc["expires_in"] | 3600) * 1000UL);

      if (doc.containsKey("refresh_token")) {
        _refreshToken = doc["refresh_token"].as<String>();
        _tokenStorage.saveRefreshToken(_refreshToken);
      }

      LOG_INFO("Token erhalten");
      return true;
    }

    String err = doc["error"] | "";
    if (err == "authorization_pending") {
      LOG_INFO("Noch nicht autorisiert...");
    } else if (err == "slow_down") {
      _interval += 5;
      LOG_INFO("Polling verlangsamt...");
    } else {
      LOG_ERROR("Fehler bei Polling: %s", err);
      return false;
    }

    delay(_interval * 1000);
  }
}

bool GoogleAuth::refreshAccessToken() {
  if (_refreshToken.isEmpty()) {
    _refreshToken = _tokenStorage.loadRefreshToken();
    if (_refreshToken.isEmpty()) {
      LOG_ERROR("Kein Refresh Token verfügbar");
      return false;
    }
  }

  String payload;
  String postData = "client_id=" + _clientId +
                    "&client_secret=" + _clientSecret +
                    "&refresh_token=" + _refreshToken +
                    "&grant_type=refresh_token";

  if (!postFormUrlencoded("https://oauth2.googleapis.com/token", postData, payload)) {
    return false;
  }

  DynamicJsonDocument doc(1024);
  if (!parseJson(payload, doc)) return false;

  _accessToken = doc["access_token"].as<String>();
  _accessTokenExpiresAt = millis() + ((doc["expires_in"] | 3600) * 1000UL);

  if (doc.containsKey("refresh_token")) {
    _refreshToken = doc["refresh_token"].as<String>();
    _tokenStorage.saveRefreshToken(_refreshToken);
    LOG_INFO("Refresh Token erneuert und gespeichert.");
  }

  LOG_INFO("Access Token erfolgreich erneuert.");
  return true;
}




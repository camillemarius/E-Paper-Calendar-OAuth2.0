#pragma once

// Internal

// Internal Library
#include "TokenStorage.h"

// External Library
#include <Arduino.h>
#include <ArduinoJson.h>
#include <functional>

using AuthPromptCallback = std::function<void(const String& verificationUrl, const String& userCode)>;
using AuthTimeoutCallback = std::function<void()>;

class GoogleAuth {
public:
    GoogleAuth(const String& clientId, const String& clientSecret, const String& scope);
    bool initialize();
    // allowDeviceFlow = false: nur Refresh versuchen, keine Benutzerinteraktion starten
    bool authorize(unsigned long maxWaitSeconds = 300, bool allowDeviceFlow = true);
    String getAccessToken();
    void onAuthPrompt(AuthPromptCallback cb);
    void onTimeout(AuthTimeoutCallback cb);
    void deleteRefreshToken();
    // true, wenn kein Refresh Token mehr vorhanden ist (Anmeldung durch den Benutzer nötig)
    bool needsUserAuthorization();

private: 
    bool hasValidAccessToken() const;
    bool hasRefreshToken();
    String getRefreshToken() const;
    bool refreshAccessToken();
    bool startDeviceCodeFlow();
    bool pollForToken();
    bool postFormUrlencoded(const String& url, const String& postData, String& responsePayload, int& httpCode);
    bool parseJson(const String& payload, DynamicJsonDocument& doc);
    String urlEncode(const String &str);


    String _clientId;
    String _clientSecret;
    String _accessToken;
    String _refreshToken;
    String _scope;

    String _deviceCode;

    int _interval = 5;
    unsigned long _accessTokenExpiresAt = 0;

    TokenStorage _tokenStorage;

    AuthPromptCallback _authPromptCallback;
    AuthTimeoutCallback _authTimeoutCallback;

};


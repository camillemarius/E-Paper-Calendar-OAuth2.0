#pragma once
// Local

// Internal Library

// External Library
#include <Arduino.h>

class TokenStorage {
public:
  static bool mount();

  static bool saveRefreshToken(const String& token);
  static String loadRefreshToken();
  static bool clearRefreshToken();

private:
  static constexpr const char* FILE_PATH = "/refresh_token.txt";
};


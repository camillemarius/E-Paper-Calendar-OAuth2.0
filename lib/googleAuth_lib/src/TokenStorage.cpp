// Local
#include "TokenStorage.h"

// Internal Library
#include <logger.h>

// External Library
#include <FS.h>
#include <SPIFFS.h>

bool TokenStorage::mount() {
  if (!SPIFFS.begin(true)) {
    LOG_DEBUG("Fehler beim Mounten von SPIFFS");
    return false;
  }
  return true;
}

bool TokenStorage::saveRefreshToken(const String& token) {
  File file = SPIFFS.open(FILE_PATH, FILE_WRITE);
  if (!file) {
    LOG_DEBUG("Fehler beim Öffnen der Datei zum Schreiben");
    return false;
  }

  file.print(token);
  file.close();
  LOG_DEBUG("Refresh Token gespeichert");
  return true;
}

String TokenStorage::loadRefreshToken() {
  if (!SPIFFS.exists(FILE_PATH)) {
    LOG_DEBUG("Kein gespeicherter Refresh Token gefunden");
    return "";
  }

  File file = SPIFFS.open(FILE_PATH, FILE_READ);
  if (!file) {
    LOG_DEBUG("Fehler beim Öffnen der Datei zum Lesen");
    return "";
  }

  String token = file.readString();
  file.close();
  LOG_DEBUG("Refresh Token geladen");
  return token;
}

bool TokenStorage::clearRefreshToken() {
  return SPIFFS.remove(FILE_PATH);
}

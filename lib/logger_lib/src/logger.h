#pragma once

#include "log.h"

#define LOG_DEBUG(fmt, ...)   Logger::getInstance().log(Logger::Level::DEBUG,   __FILENAME__, __FUNCTION__, fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...)    Logger::getInstance().log(Logger::Level::INFO,    __FILENAME__, __FUNCTION__, fmt, ##__VA_ARGS__)
#define LOG_WARNING(fmt, ...) Logger::getInstance().log(Logger::Level::WARNING, __FILENAME__, __FUNCTION__, fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...)   Logger::getInstance().log(Logger::Level::ERROR,   __FILENAME__, __FUNCTION__, fmt, ##__VA_ARGS__)

#define LOG_PLAIN(fmt, ...)   Logger::getInstance().log(Logger::Level::PLAIN, ##__VA_ARGS__)

// Logging in die Datei (SPIFFS) kostet bei jeder Zeile mehrere Flash-Zugriffe bei laufendem WLAN.
// Für die Fehlersuche einschalten mit build_flags = -D LOG_FS_ENABLED=1
#ifndef LOG_FS_ENABLED
#define LOG_FS_ENABLED 0
#endif

#if LOG_FS_ENABLED
#define LOG_FS_DEBUG(fmt, ...) Logger::getInstance().logFS(Logger::Level::DEBUG, __FILENAME__, __FUNCTION__, fmt, ##__VA_ARGS__)
#else
#define LOG_FS_DEBUG(fmt, ...) do {} while (0)
#endif

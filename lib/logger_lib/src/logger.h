#pragma once

#include "log.h"

#define LOG_DEBUG(fmt, ...)   Logger::getInstance().log(Logger::Level::DEBUG,   __FILENAME__, __FUNCTION__, fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...)    Logger::getInstance().log(Logger::Level::INFO,    __FILENAME__, __FUNCTION__, fmt, ##__VA_ARGS__)
#define LOG_WARNING(fmt, ...) Logger::getInstance().log(Logger::Level::WARNING, __FILENAME__, __FUNCTION__, fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...)   Logger::getInstance().log(Logger::Level::ERROR,   __FILENAME__, __FUNCTION__, fmt, ##__VA_ARGS__)

#define LOG_PLAIN(fmt, ...)   Logger::getInstance().log(Logger::Level::PLAIN, ##__VA_ARGS__)

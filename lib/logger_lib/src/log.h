#pragma once
#include <Arduino.h>
#include <stdarg.h>

class Logger {
public:
    enum class Level { DEBUG, INFO, WARNING, ERROR, PLAIN, NONE };

    static Logger& getInstance();

    void setLevel(Level level);
    void log(Level level, const char* file, const char* func, const char* format, ...);
    void logPlain(Level level, const char* format, ...);

private:
    Logger(Level level = Level::DEBUG);
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    Level currentLevel;
    String levelToString(Level level);
};

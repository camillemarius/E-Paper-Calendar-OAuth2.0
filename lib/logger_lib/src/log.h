#pragma once
#include <Arduino.h>
#include <stdarg.h>
#include <FS.h>

class Logger {
public:
    enum class Level { DEBUG, INFO, WARNING, ERROR, PLAIN, NONE };

    static Logger& getInstance();

    void setLevel(Level level);
    void log(Level level, const char* file, const char* func, const char* format, ...);
    void logPlain(Level level, const char* format, ...);// NEU
    void logFS(Level level, const char* file, const char* func, const char* format, ...);

    // File logging
    bool begin(const char* filename = "/log.txt",size_t maxFileSize = 32 * 1024);
    void setFileLogging(bool enabled);
    bool isFileLoggingEnabled() const;
    void clearLog();
    String readLog();

private:
    Logger(Level level = Level::DEBUG);
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    Level currentLevel;
    String levelToString(Level level);

    // File logging
    fs::FS* fs = nullptr;
    String logFile;
    size_t maxFileSize = 32 * 1024;
    bool fileLoggingEnabled = false;

    void writeToFile(const String& message);
    void trimLogFile();
};

#include "log.h"

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

Logger::Logger(Level level) : currentLevel(level) {}

void Logger::setLevel(Level level) {
    currentLevel = level;
}

void Logger::log(Level level, const char* file, const char* func, const char* format, ...) {
    if (level < currentLevel || currentLevel == Level::NONE) {
        return;
    }

    char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

     // Extract filename from path
    const char* filename = strrchr(file, '/');
    #ifdef _WIN32
        if (!filename) filename = strrchr(file, '\\');  // Handle Windows paths
    #endif
    filename = filename ? filename + 1 : file;

    // Remove extension (.cpp, .h, etc.)
    char className[64];
    strncpy(className, filename, sizeof(className));
    className[sizeof(className) - 1] = '\0';
    char* dot = strrchr(className, '.');
    if (dot) *dot = '\0';


    Serial.printf("[%s] [%s] [%s] %s", levelToString(level), className, func, buffer);
    Serial.print("\r\n");
}

void Logger::logPlain(Level level, const char* format, ...) {
    if (level < currentLevel || currentLevel == Level::NONE) {
        return;
    }

    char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    Serial.print("[");
    Serial.print(levelToString(level));
    Serial.print("]");
    Serial.println(buffer);
}

String Logger::levelToString(Level level) {
    switch (level) {
        case Level::DEBUG:   return "DEBUG";
        case Level::INFO:    return "INFO";
        case Level::WARNING: return "WARNING";
        case Level::ERROR:   return "ERROR";
        case Level::PLAIN:   return "PLAIN";
        default:             return "UNKNOWN";
    }
}

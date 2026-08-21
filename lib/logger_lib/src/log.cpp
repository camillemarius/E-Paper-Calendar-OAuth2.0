#include "log.h"
#include <SPIFFS.h>
#include <stdarg.h>

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

Logger::Logger(Level level)
    : currentLevel(level),
      logFile("/log.txt"),
      maxFileSize(32 * 1024),
      fileLoggingEnabled(false) {
}

void Logger::setLevel(Level level) {
    currentLevel = level;
}

void Logger::setFileLogging(bool enabled) {
    fileLoggingEnabled = enabled;
}

bool Logger::isFileLoggingEnabled() const {
    return fileLoggingEnabled;
}

bool Logger::begin(const char* filename, size_t maxSize) {

    logFile = filename;
    maxFileSize = maxSize;

    if (!SPIFFS.begin(true)) {

        Serial.println(
            "[LOGGER] ERROR: SPIFFS initialization failed"
        );

        return false;
    }

    // SPIFFS als Dateisystem für den Logger verwenden
    fs = &SPIFFS;

    // Logdatei erstellen, falls sie noch nicht existiert
    if (!fs->exists(logFile)) {

        File file = fs->open(
            logFile,
            FILE_WRITE
        );

        if (!file) {

            Serial.println(
                "[LOGGER] ERROR: Could not create log file"
            );

            fs = nullptr;
            return false;
        }

        file.close();
    }

    fileLoggingEnabled = true;

    Serial.printf(
        "[LOGGER] File logging enabled: %s\n",
        logFile.c_str()
    );

    return true;
}

void Logger::log(
    Level level,
    const char* file,
    const char* func,
    const char* format,
    ...
) {
    if (level < currentLevel ||
        currentLevel == Level::NONE) {
        return;
    }

    char buffer[256];

    va_list args;
    va_start(args, format);
    vsnprintf(
        buffer,
        sizeof(buffer),
        format,
        args
    );
    va_end(args);

    // Dateiname aus Pfad extrahieren
    const char* filename = strrchr(
        file,
        '/'
    );

#ifdef _WIN32
    if (!filename) {
        filename = strrchr(
            file,
            '\\'
        );
    }
#endif

    filename = filename
        ? filename + 1
        : file;

    // Dateiendung entfernen
    char className[64];

    strncpy(
        className,
        filename,
        sizeof(className)
    );

    className[sizeof(className) - 1] = '\0';

    char* dot = strrchr(
        className,
        '.'
    );

    if (dot) {
        *dot = '\0';
    }

    Serial.printf(
        "[%s] [%s] [%s] %s",
        levelToString(level).c_str(),
        className,
        func,
        buffer
    );

    Serial.print("\r\n");
}

void Logger::logPlain(
    Level level,
    const char* format,
    ...
) {
    if (level < currentLevel ||
        currentLevel == Level::NONE) {
        return;
    }

    char buffer[256];

    va_list args;
    va_start(args, format);

    vsnprintf(
        buffer,
        sizeof(buffer),
        format,
        args
    );

    va_end(args);

    Serial.print("[");
    Serial.print(
        levelToString(level)
    );
    Serial.print("]");

    Serial.println(buffer);
}

void Logger::logFS(
    Level level,
    const char* file,
    const char* func,
    const char* format,
    ...
) {
    if (level < currentLevel ||
        currentLevel == Level::NONE) {
        return;
    }

    char buffer[256];

    va_list args;
    va_start(args, format);

    vsnprintf(
        buffer,
        sizeof(buffer),
        format,
        args
    );

    va_end(args);

    // Dateiname aus Pfad extrahieren
    const char* filename = strrchr(
        file,
        '/'
    );

#ifdef _WIN32
    if (!filename) {
        filename = strrchr(
            file,
            '\\'
        );
    }
#endif

    filename = filename
        ? filename + 1
        : file;

    // Dateiendung entfernen
    char className[64];

    strncpy(
        className,
        filename,
        sizeof(className)
    );

    className[sizeof(className) - 1] = '\0';

    char* dot = strrchr(
        className,
        '.'
    );

    if (dot) {
        *dot = '\0';
    }

    // Gesamte Logmeldung erzeugen
    char logBuffer[320];

    snprintf(
        logBuffer,
        sizeof(logBuffer),
        "[%s] [%s] [%s] %s\r\n",
        levelToString(level).c_str(),
        className,
        func,
        buffer
    );

    String message = logBuffer;

    // Immer auf Serial ausgeben
    Serial.print(message);

    // Nur LOG_FS_* zusätzlich in SPIFFS speichern
    if (fileLoggingEnabled &&
        fs != nullptr) {

        writeToFile(message);
    }
}

void Logger::writeToFile(
    const String& message
) {
    if (!fileLoggingEnabled ||
        fs == nullptr) {
        return;
    }

    // Dateigröße prüfen
    if (fs->exists(logFile)) {

        File file = fs->open(
            logFile,
            FILE_READ
        );

        if (file) {

            size_t size = file.size();

            file.close();

            if (size + message.length() >
                maxFileSize) {

                trimLogFile();
            }
        }
    }

    // Datei öffnen
    File file = fs->open(
        logFile,
        FILE_APPEND
    );

    if (!file) {

        Serial.println(
            "[LOGGER] ERROR: Could not open log file"
        );

        return;
    }

    file.print(message);

    file.close();
}

void Logger::trimLogFile() {

    if (fs == nullptr ||
        !fs->exists(logFile)) {
        return;
    }

    File file = fs->open(
        logFile,
        FILE_READ
    );

    if (!file) {
        return;
    }

    size_t size = file.size();

    // Ungefähr die Hälfte behalten
    size_t keepFrom = size / 2;

    file.seek(keepFrom);

    String remaining =
        file.readString();

    file.close();

    // Angefangene Zeile entfernen
    int newline =
        remaining.indexOf('\n');

    if (newline >= 0) {

        remaining =
            remaining.substring(
                newline + 1
            );
    }

    // Datei neu schreiben
    file = fs->open(
        logFile,
        FILE_WRITE
    );

    if (!file) {
        return;
    }

    file.print(remaining);

    file.close();
}

void Logger::clearLog() {

    if (fs == nullptr) {
        return;
    }

    if (fs->exists(logFile)) {

        fs->remove(logFile);
    }
}

String Logger::readLog() {

    if (fs == nullptr ||
        !fs->exists(logFile)) {

        return "";
    }

    File file = fs->open(
        logFile,
        FILE_READ
    );

    if (!file) {
        return "";
    }

    String content =
        file.readString();

    file.close();

    return content;
}

String Logger::levelToString(
    Level level
) {
    switch (level) {

        case Level::DEBUG:
            return "DEBUG";

        case Level::INFO:
            return "INFO";

        case Level::WARNING:
            return "WARNING";

        case Level::ERROR:
            return "ERROR";

        case Level::PLAIN:
            return "PLAIN";

        default:
            return "UNKNOWN";
    }
}
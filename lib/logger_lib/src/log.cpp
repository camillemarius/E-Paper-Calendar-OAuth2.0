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

    fs = nullptr;
    fileLoggingEnabled = false;

    if (!SPIFFS.begin(false)) {

        Serial.println("[LOGGER] ERROR: SPIFFS initialization failed");
        return false;
    }

    // SPIFFS als Dateisystem für den Logger verwenden
    fs = &SPIFFS;

    Serial.printf("[LOGGER] SPIFFS mounted. Total=%u Used=%u Free=%u\n",(unsigned)SPIFFS.totalBytes(),(unsigned)SPIFFS.usedBytes(),(unsigned)(SPIFFS.totalBytes() - SPIFFS.usedBytes()));

    // Logdatei erstellen, falls sie noch nicht existiert
    if (!fs->exists(logFile)) {
        
        Serial.printf("[LOGGER] Creating log file: %s\n",logFile.c_str());

        File file = fs->open(
            logFile,
            FILE_WRITE
        );

        if (!file) {

            Serial.println("[LOGGER] ERROR: Could not create log file");

            fs = nullptr;
            fileLoggingEnabled = false;
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

String Logger::getDateTime()
{
    time_t now = time(nullptr);

    struct tm timeinfo;

    if (!localtime_r(&now, &timeinfo)) {
        return "[0000-00-00 00:00:00]";
    }

    char buffer[24];

    strftime(
        buffer,
        sizeof(buffer),
        "%Y-%m-%d %H:%M:%S",
        &timeinfo
    );

    return String("[") + buffer + "]";
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
        "%s [%s] [%s] [%s] %s\r\n",
        getDateTime().c_str(),
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

void Logger::writeToFile(const String& message)
{
    if (!fileLoggingEnabled || fs == nullptr) {
        Serial.println(
            "[LOGGER] WRITE ABORTED: logging disabled or fs=null"
        );
        return;
    }

    if (!fs->exists(logFile)) {
        Serial.println(
            "[LOGGER] Logfile does not exist!"
        );
        return;
    }

    // Aktuelle Dateigröße bestimmen
    File file = fs->open(logFile, FILE_READ);

    if (!file) {
        Serial.println(
            "[LOGGER] ERROR: Could not open log file for READ"
        );
        return;
    }

    size_t size = file.size();
    file.close();

    // Prüfen, ob vor dem Schreiben getrimmt werden muss
    if (size + message.length() > maxFileSize) {

        Serial.printf(
            "[LOGGER] MAX SIZE: current=%u message=%u max=%u\n",
            (unsigned)size,
            (unsigned)message.length(),
            (unsigned)maxFileSize
        );

        trimLogFile();
    }

    // Datei zum Anhängen öffnen
    file = fs->open(logFile, FILE_APPEND);

    if (!file) {
        Serial.println(
            "[LOGGER] ERROR: Could not open log file for APPEND"
        );
        return;
    }

    size_t before = file.size();

    // Nachricht schreiben
    size_t written = file.print(message);

    // Daten sicher ins Dateisystem schreiben
    file.flush();

    // Datei schließen
    file.close();

    // Datei erneut öffnen und tatsächliche Größe prüfen
    size_t after = 0;

    File checkFile = fs->open(logFile, FILE_READ);

    if (checkFile) {
        after = checkFile.size();
        checkFile.close();
    }
    else {
        Serial.println(
            "[LOGGER] ERROR: Could not reopen log file for SIZE CHECK"
        );
    }

    Serial.printf(
        "[LOGGER] WRITE: before=%u requested=%u written=%u after=%u\n",
        (unsigned)before,
        (unsigned)message.length(),
        (unsigned)written,
        (unsigned)after
    );
}

void Logger::trimLogFile()
{
    if (fs == nullptr || !fs->exists(logFile)) {
        Serial.println("[LOGGER] TRIM ABORTED: file missing");
        return;
    }

    File file = fs->open(logFile, FILE_READ);

    if (!file) {
        Serial.println("[LOGGER] TRIM ERROR: READ open failed");
        return;
    }

    size_t oldSize = file.size();

    size_t keepFrom = oldSize / 2;

    file.seek(keepFrom);

    String remaining = file.readString();

    file.close();

    int newline = remaining.indexOf('\n');

    if (newline >= 0) {
        remaining = remaining.substring(newline + 1);
    }

    file = fs->open(logFile, FILE_WRITE);

    if (!file) {
        Serial.println("[LOGGER] TRIM ERROR: WRITE open failed");
        return;
    }

    size_t written = file.print(remaining);

    file.flush();
    file.close();

    // tatsächliche Größe nach dem Schreiben prüfen
    size_t newSize = 0;

    File checkFile = fs->open(logFile, FILE_READ);

    if (checkFile) {
        newSize = checkFile.size();
        checkFile.close();
    }

    Serial.printf(
        "[LOGGER] TRIM: old=%u kept=%u written=%u new=%u\n",
        (unsigned)oldSize,
        (unsigned)remaining.length(),
        (unsigned)written,
        (unsigned)newSize
    );
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
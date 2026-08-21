#include <Arduino.h>
#include <SPIFFS.h>

#include <logger.h>

void setup() {

    Serial.begin(115200);
    delay(1000);

    Serial.println();
    Serial.println("========================================");
    Serial.println("        LOGGER / SPIFFS READ TEST");
    Serial.println("========================================");

    // SPIFFS mounten
    if (!SPIFFS.begin(true)) {

        Serial.println(
            "[ERROR] SPIFFS konnte nicht gemountet werden."
        );

        return;
    }

    Serial.println(
        "[INFO] SPIFFS erfolgreich gemountet."
    );

    // SPIFFS Informationen
    Serial.printf(
        "[INFO] SPIFFS Total: %u Bytes\n",
        SPIFFS.totalBytes()
    );

    Serial.printf(
        "[INFO] SPIFFS Used:  %u Bytes\n",
        SPIFFS.usedBytes()
    );

    Serial.printf(
        "[INFO] SPIFFS Free:  %u Bytes\n",
        SPIFFS.totalBytes() - SPIFFS.usedBytes()
    );

    Serial.println();

    // Prüfen, ob Logdatei existiert
    if (!SPIFFS.exists("/log.txt")) {

        Serial.println(
            "[INFO] /log.txt existiert nicht."
        );

        return;
    }

    File file = SPIFFS.open(
        "/log.txt",
        FILE_READ
    );

    if (!file) {

        Serial.println(
            "[ERROR] /log.txt konnte nicht geöffnet werden."
        );

        return;
    }

    Serial.printf(
        "[INFO] Logdatei Größe: %u Bytes\n",
        file.size()
    );

    Serial.println();
    Serial.println("========================================");
    Serial.println("                 LOG");
    Serial.println("========================================");

    while (file.available()) {
        Serial.write(file.read());
    }

    file.close();

    Serial.println();
    Serial.println("========================================");
    Serial.println("             END OF LOG");
    Serial.println("========================================");
}

void loop() {
}
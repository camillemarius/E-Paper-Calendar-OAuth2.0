# Energieoptimierung der Kalender-Firmware

Stand: 02.10.2026, Branch `energy-optimization`, gemessen auf der Universal-Platine mit
Display DEPG0750RWF86BF (Env `GoogleCalendar_UniversalDriverCACH_DEPG0750RWF86BF`).

## Ausgangslage

Der Kalender wacht einmal pro Nacht um 01:00 auf, verbindet sich mit dem WLAN, erneuert den
Google-Token, lädt die Termine der ausgewählten Kalender, zeichnet die Ansicht und schläft bis
zur nächsten Nacht. Der Verbrauch setzt sich aus zwei Teilen zusammen:

- **Wachphase (einmal pro Tag):** WLAN, TLS, Google-Abfragen und ein Vollrefresh des
  3-Farben-Displays (24 s beim DEPG-Panel).
- **Deep Sleep (rund um die Uhr):** jedes zusätzliche µA kostet 0,024 mAh pro Tag.

Vor der Optimierung wartete der ESP32 den ganzen Refresh mit eingeschaltetem WLAN bei 240 MHz
ab, der Display-Controller blieb im Standby statt im Deep Sleep, und mehrere Fehlerpfade
starteten jede Nacht minutenlange Einrichtungsabläufe.

## Übersicht der Massnahmen

| # | Massnahme | Wirkt auf | Geprüft |
|---|---|---|---|
| 1 | Refresh-Token nur bei `invalid_grant` löschen | Fehlerpfad | Build, Gerät (Anmeldung) |
| 2 | Kalenderauswahl bei Timeout behalten | Fehlerpfad | Build |
| 3 | Keine Einrichtung beim Timer-Wakeup, Retry mit Backoff | Fehlerpfad | Build, Gerät (Timer-Wakeup) |
| 4 | Display-Controller vor dem Deep Sleep in Hibernate | Deep Sleep | Gerät |
| 5 | WLAN aus und CPU 80 MHz vor dem Zeichnen | Wachphase | Gerät |
| 6 | Light Sleep während BUSY | Wachphase | Gerät (Refresh läuft normal) |
| 7 | Kein zweiter Durchlauf nach zu frühem Aufwachen | Wachphase | Build, Gerät (Schlafdauer) |
| 8 | Weniger Daten und gemeinsame TLS-Verbindung zu Google | Wachphase | Gerät |
| 9 | WLAN-Schnellverbindung ohne Scan | Wachphase | Gerät |
| 10 | Datei-Logging standardmässig aus, keine Secrets im Log | Wachphase, Sicherheit | Gerät |
| 11 | Akku vor dem WLAN lesen, kein QuickStart | Messgenauigkeit | Gerät |
| 12 | TLS-Verbindung vor dem Zeichnen schliessen | Wachphase (Speicher) | Gerät |
| 13 | Display-Leitungen im Deep Sleep festhalten | Deep Sleep | Gerät (Timer-Wakeup) |
| 14 | Externer SPI-Flash in Deep Power-Down | Deep Sleep | Gerät (kein Chip erkannt) |

## Die Massnahmen im Detail

### 1. Refresh-Token nur bei `invalid_grant` löschen

`lib/googleAuth_lib/src/GoogleAuth.cpp`

Vorher löschte `refreshAccessToken()` den Refresh-Token bei jedem Fehler, auch bei DNS-Fehlern,
Timeouts oder HTTP 5xx. Danach startete jede Nacht der Device-Code-Flow (QR-Code, 60 s Polling,
zwei Display-Refreshes), bis jemand das Gerät neu koppelte.

Jetzt wird der Token nur gelöscht, wenn Google mit HTTP 400/401 und `invalid_grant` antwortet.
Bei allen anderen Fehlern bleibt er erhalten, `authorize()` gibt `false` zurück und das Gerät
versucht es später erneut. `postFormUrlencoded()` liefert dafür zusätzlich den HTTP-Code.
Fehlt in einer 200-Antwort der Access Token, gilt das ebenfalls als vorübergehender Fehler.

### 2. Kalenderauswahl bei Timeout behalten

`src/ePaperCalendar/CalendarConfigurator/CalendarConfigurator.cpp`

`forceSelection()` (Knopfdruck) löschte die gespeicherte Auswahl, bevor eine neue gespeichert
war. Lief der 120-s-Webserver ins Timeout, war die Auswahl weg, und ab dann startete jede Nacht
der Webserver erneut.

Jetzt bleibt die gespeicherte Auswahl bestehen, bis in `handleSelect()` eine neue gespeichert
wird. Läuft die Auswahl ab, werden die bisherigen Kalender weiter angezeigt. Der Webserver-Teil
steckt in `runSelectionPortal()`, die Routen werden nur einmal registriert und der Server wird
nach der Auswahl gestoppt.

### 3. Keine Einrichtung beim Timer-Wakeup, Retry mit Backoff

`src/ePaperCalendar/main_calendar.cpp`, `lib/wifiManager_lib/src/wifiHandler.cpp`

Beim nächtlichen Timer-Wakeup ist niemand am Gerät. WLAN-Portal, Device-Code-Flow und
Kalenderauswahl starten deshalb nur noch nach Power-on oder Tastendruck
(`isInteractiveWakeup()`).

- **Vorübergehende Fehler** (WLAN, NTP, Google-Netzwerkfehler, kein Kalender geladen):
  neuer Versuch nach 15 min, 1 h und 3 h, danach wieder um 01:00 (`sleepForRetry()`,
  Zähler im RTC-Speicher). Die Anzeige bleibt dabei unverändert.
- **Benutzer nötig** (Token widerrufen, keine Kalenderauswahl): der Hinweis wird einmal
  angezeigt, danach schläft das Gerät ohne Timer, bis der Knopf gedrückt wird
  (`sleepUntilButtonPress()`).
- Kann kein einziger Kalender geladen werden, wird nicht neu gezeichnet.

Vorher kostete ein ausgeschalteter Router um 01:00 rund 30 s Verbindungsversuch plus 180 s
Access-Point-Portal plus zwei Refreshes, und der Kalender wurde durch den Timeout-Hinweis
ersetzt. Ohne Portal bricht `autoConnect` nach 10 s ab (`setEnableConfigPortal(false)`).

### 4. Display-Controller vor dem Deep Sleep in Hibernate

`lib/ePaper_lib/ePaperDriver.h` und alle Treiber, `prepareDeepSleep()` in `main_calendar.cpp`

GxEPD2 schaltet nach dem Vollrefresh nur die Panel-Spannungen ab (`powerOff()`), der Controller
bleibt im Standby. `hibernate()` schickt ihn zusätzlich in Deep Sleep (Kommando `0x07/0xA5`).
Alle drei Sleep-Funktionen rufen vorher `prepareDeepSleep()` auf. `init()` setzt den Controller
beim nächsten Start per Reset zurück.

### 5. WLAN aus und CPU 80 MHz vor dem Zeichnen

`loadAndDrawCalendar()` in `main_calendar.cpp`

Sobald alle Termine im RAM sind, werden WLAN (`WIFI_OFF`) und die TLS-Verbindung beendet und
die CPU auf 80 MHz gedrosselt. Vorher lief der ganze Refresh mit eingeschaltetem WLAN bei 240 MHz.

### 6. Light Sleep während BUSY

`lightSleepWhileDisplayBusy()` in `main_calendar.cpp`, `setBusyCallback()` in allen Treibern

GxEPD2 pollte während des Refreshs den BUSY-Pin mit `delay(1)`. Über den Busy-Callback von
GxEPD2 geht der ESP32 jetzt in Light Sleep, bis BUSY den Pegel wechselt, spätestens nach 1 s,
damit der Busy-Timeout von GxEPD2 weiter greift. Light Sleep wird nur bei ausgeschaltetem WLAN
verwendet, die Einrichtungs-Screens verhalten sich wie bisher.

### 7. Kein zweiter Durchlauf nach zu frühem Aufwachen

`sleepUntilOneAM()` in `main_calendar.cpp`

Der Deep-Sleep-Timer läuft auf dem internen RC-Oszillator und driftet. Wachte der ESP32 kurz vor
01:00 auf, korrigierte SNTP die Uhr, und `sleepUntilOneAM()` schlief nur noch bis 01:00 heute:
ein zweiter kompletter Durchlauf. Jetzt merkt sich das Gerät im RTC-Speicher den Tag der letzten
Anzeige (`lastRenderedDay`) und schläft bis zum nächsten 01:00, an dessen Tag noch nicht
gezeichnet wurde. „Morgen“ wird über `tm_mday` berechnet, damit die Zeitumstellung stimmt. Die
Zeitzone wird direkt nach dem Start gesetzt, damit auch Fehlerpfade vor dem NTP-Sync lokal
rechnen.

### 8. Weniger Daten und gemeinsame TLS-Verbindung zu Google

`lib/googleCalendar_lib/src/GoogleCalendar.cpp`

- Termine werden mit `maxAttendees=1` und
  `fields=items(id,status,summary,start,end,attendees(self,responseStatus))` abgefragt, die
  Kalenderliste mit `fields=items(id,summary)`.
- Alle Anfragen an `www.googleapis.com` teilen sich einen `WiFiClientSecure` mit Keep-Alive.
  Das spart pro weiterem Kalender einen TLS-Handshake. Die Zertifikatsprüfung ist unverändert
  (ohne Prüfung, wie vorher mit `HTTPClient::begin(url)`).

### 9. WLAN-Schnellverbindung ohne Scan

`fastConnect()` in `lib/wifiManager_lib/src/wifiHandler.cpp`

Kanal und BSSID des letzten Access Points liegen im RTC-Speicher. Beim nächsten Wakeup wird
direkt damit verbunden, ohne Kanal-Scan. Diese Einstellung wird nur im RAM gesetzt
(`esp_wifi_set_storage(WIFI_STORAGE_RAM)`), die gespeicherte WLAN-Konfiguration bleibt
unverändert. Schlägt die Schnellverbindung fehl, folgt der normale WiFiManager-Ablauf.

### 10. Datei-Logging standardmässig aus, keine Secrets im Log

`lib/logger_lib/src/logger.h`, `GoogleAuth.cpp`, `main_calendar.cpp`

- `LOG_FS_DEBUG` ist ohne `-D LOG_FS_ENABLED=1` ein leeres Makro. Jede Zeile kostete mehrere
  SPIFFS-Zugriffe bei laufendem WLAN, beim Erreichen von 32 KB wurde die Datei neu geschrieben.
- Die 1-s-Wartezeit nach `Serial.begin()` ist entfernt.
- Die Token-Anfragen loggen weder die POST-Daten (enthielten `client_secret` und Refresh-Token)
  noch erfolgreiche Antworten (enthielten Access- und Refresh-Token), nur noch Fehlerantworten.

### 11. Akku vor dem WLAN lesen, kein QuickStart

`lib/BatteryGauge_lib/src/BatteryGauge.cpp`, `weeklyCalendarDisplay.cpp`, `main_calendar.cpp`

`BatteryGauge::begin()` löste bei jedem Wakeup einen QuickStart des MAX17048 aus. Damit verwarf
der Gauge seine Schätzung und rechnete aus der Momentanspannung neu. Gelesen wurde zudem erst
nach den HTTP-Anfragen, unter WLAN-Last. Jetzt liest `WeeklyCalendar::readBattery()` den Gauge
am Anfang von `setup()`, vor dem WLAN, und `begin()` macht keinen QuickStart mehr.

### 12. TLS-Verbindung vor dem Zeichnen schliessen

`GoogleCalendar::closeConnection()`

Die per Keep-Alive offene TLS-Verbindung belegt rund 44 KB Heap. Sie wird geschlossen, sobald
alle Kalender geladen sind.

### 13. Display-Leitungen im Deep Sleep festhalten

`lib/ePaper_lib/ePaperPinHold.h`, `hibernate()` und `init()` in allen Treibern

Ohne Hold sind die Pins im Deep Sleep hochohmig. Offene CMOS-Eingänge am Display können
Querstrom ziehen, und ein schwebender RST kann den Controller aus dem Hibernate wecken.
`hibernate()` hält CS, DC, RST, SCK und MOSI auf ihrem letzten Pegel (`gpio_hold_en`,
`gpio_deep_sleep_hold_en`). `init()` löst die Holds, bevor GxEPD2 den Controller zurücksetzt.
Die Strapping-Pins 0, 2 und 12 werden nie gehalten.

### 14. Externer SPI-Flash in Deep Power-Down

`powerDownExternalFlash()` in `main_calendar.cpp`, `lib/externalFlash_lib/src/externalFlash.cpp`

Der Bild-Flash der Galerie hängt am SPI-Bus des Displays, sein CS liegt auf GPIO2 zusammen mit
dem Knopf. Der Kalender liest beim Start die JEDEC-ID und schickt den Chip mit `0xB9` in Deep
Power-Down, wenn einer antwortet. CS wird nur aktiv auf Low gezogen und über den Pull-up wieder
High, damit ein gedrückter Knopf nie gegen einen High-Pegel kurzschliesst.
`externalFlash::begin()` in der Galerie sendet bei einem Fehlschlag `0xAB` (Release) und
versucht es erneut.

Auf der getesteten Platine antwortet an diesem CS kein Chip, die Massnahme greift dort also nicht.

## Messwerte vom Gerät

Serielle Logs vom 02.10.2026, Universal-Platine mit DEPG0750RWF86BF, WLAN-Empfang −80 dBm.
Strom wurde nicht gemessen, alle Angaben sind Zeiten.

| Abschnitt | Gemessen |
|---|---|
| Start bis WLAN verbunden (Power-on, WiFiManager) | 1,6–3,2 s |
| WLAN-Schnellverbindung (Timer-Wakeup, 1 Messung) | 2,9 s |
| NTP-Wartezeit nach Power-on / nach Timer-Wakeup | 0,8–3,7 s / 0 s |
| Token-Erneuerung | 1,4–1,8 s |
| Erste Kalenderabfrage (mit TLS-Handshake) | 1,3–1,9 s |
| Weitere Kalenderabfragen (Verbindung wiederverwendet) | 0,2–0,3 s |
| Antwortgrösse mit Feld-Filter (5 Termine) | 1 474 Byte |
| Vollrefresh DEPG-Panel | 24,0 s |
| Start bis Refresh-Beginn ohne / mit Datei-Logging | 10,1 s / 13,6 s |
| Timer-Wakeup bis Refresh-Beginn | 7,6 s |

## Schalter für Entwicklung und Test

| Build-Flag | Wirkung |
|---|---|
| `-D LOG_FS_ENABLED=1` | Datei-Logging nach `/log.txt` (SPIFFS) und mehr Details auf Serial |
| `-D SLEEP_TEST_SECONDS=120` | schläft 120 s statt bis 01:00, um den nächtlichen Ablauf sofort zu prüfen |

Beide lassen sich ohne Dateiänderung setzen, zum Beispiel:

```bash
PLATFORMIO_BUILD_FLAGS="-D SLEEP_TEST_SECONDS=120" pio run -e GoogleCalendar_UniversalDriverCACH_DEPG0750RWF86BF -t upload
```

## Geändertes Verhalten für Benutzer

- WLAN-Portal, Google-Anmeldung und Kalenderauswahl starten nur nach Power-on oder Tastendruck.
- Fehlt die Anmeldung oder die Kalenderauswahl, zeigt das Gerät den Hinweis und wartet auf den
  Knopf, statt es jede Nacht erneut zu versuchen.
- Bei Netzwerkfehlern bleibt die alte Anzeige stehen, das Gerät versucht es nach 15 min, 1 h und
  3 h erneut.
- Die Akkuanzeige zeigt die laufende Schätzung des MAX17048. Nach dem Entfernen des QuickStarts
  kann sie einige Stunden brauchen, bis sie zur Spannung passt.

## Offene Punkte

- **Ruhestrom messen:** Der grösste verbleibende Posten ist der Strom im Deep Sleep. Er hängt
  stark von der Platine ab und wurde nicht gemessen. Massnahmen 4, 13 und 14 zielen darauf.
- **WLAN-Schnellverbindung:** greift, ein Zeitvorteil war in der einen Messung bei −80 dBm nicht
  belegbar.
- **Externer Flash:** auf der getesteten Platine an CS GPIO2 nicht ansprechbar, auch nicht von
  der Galerie-Firmware. Bestückung und Verdrahtung prüfen.
- **Gmail-Kalender mit HTTP 404:** einer der gespeicherten Kalender ist für das angemeldete
  Konto nicht erreichbar. Kalenderauswahl per Knopf neu treffen.
- **Galerie-Pins:** Das Env `Gallery_UniversalDriverCACH_fpc8612` verwendet die V1-Pins,
  `Gallery_UniversalDriverCACH_WAVESHARE_13504` hat keinen Zweig in `main_gallery.cpp`.
- **`timeMin` in UTC:** Die Terminabfrage beginnt um 00:00 UTC, also 01:00/02:00 Lokalzeit.
  Termine zwischen Mitternacht und diesem Zeitpunkt fehlen.

# Galerie-Firmware

Stand: 02.10.2026. Die Galerie zeigt ein Foto, das im externen Flash U6 gespeichert ist, und
schläft dazwischen im Deep Sleep. Sie wacht nur über die Taster auf, nie per Timer.

## Bedienung

| Auslöser | Verhalten |
|---|---|
| Reset (Taster S2) | gespeichertes Foto neu zeichnen, danach Deep Sleep |
| Taster S4 kurz drücken | gespeichertes Foto neu zeichnen, danach Deep Sleep |
| Taster S4 mindestens 5 s drücken | Upload-Modus |
| Kein Foto im Flash | Upload-Modus |

**Upload-Modus:**

1. Das Display zeigt „Bild hochladen“ mit einem QR-Code für das WLAN.
2. Die Galerie öffnet das WLAN **EPaper** (Passwort `12345678`), die Seite liegt auf
   **http://192.168.4.1**.
3. Nach dem Upload zeichnet sie das neue Foto und schläft.
4. Kommt innerhalb von 5 Minuten kein Upload, zeichnet sie das bisherige Foto wieder und schläft.

## Upload-Seite

- Die Seite fragt die Firmware über `GET /info` nach dem angeschlossenen Display und wählt es
  aus. Liste: DEPG0750RWF86BF und FPC8612 (3 Farben), GDEP073E01 (6 Farben), Waveshare 13504
  (Schwarz/Weiss), alle 800 × 480.
- Ein anderes Display kann man für die Vorschau wählen. Der Upload ist dann gesperrt, weil die
  Farben nicht zur Firmware passen würden.
- Der Browser rechnet das Bild per Floyd-Steinberg-Dithering auf die Palette um und schickt
  192 000 Byte (2 Pixel pro Byte). Die Firmware löscht den Bildbereich und schreibt die Daten
  in U6 ab Adresse 0.

## Strom

- **Ohne Tastendruck** läuft nichts: kein Timer, kein WLAN.
- **Vor dem Deep Sleep:**
  - Der Display-Controller geht in Hibernate, seine Leitungen werden gehalten.
  - U6 geht in Deep Power-Down, `CS_Flash` (IO4) wird High gehalten.
- **Der Schreibtest des Flash** läuft nur noch im Upload-Modus. Er nutzt den letzten Sektor
  (`0x1FF000`).

Vorher hielt die Galerie Access Point und Webserver dauerhaft an und schlief nie.

## Varianten

| Env | Display | Flash-CS |
|---|---|---|
| `Gallery_UniversalDriverCACH_DEPG0750RWF86BF` | DEPG0750RWF86BF, Stecker _24 | IO4 |
| `Gallery_UniversalDriverCACH_fpc8612` | FPC8612, Stecker _24 | IO4 |
| `Gallery_UniversalDriverCACH_GDEP073E01` | GDEP073E01, Stecker _50 | IO4 |
| `Gallery_UniversalDriverCACH_WAVESHARE_13504` | Waveshare 13504, Stecker _24 | IO4 |
| `Gallery_v1Driver_*` | V1-Platine | IO2 |

Bei 7,5″-Displays muss der RESE-Schalter S6 auf 0,47 Ω stehen, sonst werden Fotos fast schwarz
(siehe [hardware-notes.md](hardware-notes.md)).

## Schalter für Tests

| Build-Flag | Wirkung |
|---|---|
| `-D GALLERY_TEST_LONG_PRESS` | startet immer im Upload-Modus, als wäre S4 lang gedrückt |
| `-D UPLOAD_TIMEOUT_MS=60000UL` | kürzerer Timeout im Upload-Modus |

## Geprüft auf dem Gerät (Universal-Platine, DEPG0750RWF86BF)

- **Reset:** Das Foto wird neu gezeichnet, danach meldet das Log den Deep Sleep.
- **Upload-Modus:** Hinweis-Screen mit QR-Code und Access Point. Nach dem Timeout kommt das
  gespeicherte Foto zurück, dann Deep Sleep.
- **Upload eines Fotos über das Handy:** 192 000 Byte in etwa 7,4 s. Die Darstellung ist mit
  S6 auf 0,47 Ω korrekt.
- **Kurz- und Langdruck an S4:** noch nicht auf dem Gerät geprüft. Die Logik entspricht der
  Erkennung im Kalender.

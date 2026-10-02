# Hardware-Hinweise zur Universal-Platine

Stand: 02.10.2026, gemessen auf zwei Universal-Platinen mit DEPG0750RWF86BF. Grundlage ist das
Schema „Universal E-Paper Driver“ (Revision 00).

## Belegung

| Funktion | ESP32-Pin | Bemerkung |
|---|---|---|
| SPI SCLK / SDI (MOSI) / SDO (MISO) | IO18 / IO23 / IO19 | gemeinsam für beide Displays und den Flash |
| Display-Stecker _24 (7,5″): CS, D/C, RES, BUSY | IO27, IO25, IO26, IO32 | Kalender und Galerie |
| Display-Stecker _50 (7,3″): CS, D/C, RES, BUSY | IO16, IO5, IO17, IO33 | Galerie GDEP073E01 |
| Flash U6 (W25Q16JV, 2 MB): CS_Flash | IO4 | **ohne Pull-up**, WP# und HOLD# fest auf 3,3 V |
| Taster S2 | EN | Reset, löst `POWERON_RESET` aus |
| Taster S4 | IO2 | Benutzer-Taster (Weckquelle ext0, kurz/lang drücken) |
| Akku-Gauge MAX17048: SDA / SCL | IO21 / IO22 | |

## RESE-Schalter S6 (Stecker _24)

S6 wählt den Messwiderstand RESE des Boost-Wandlers, der die Panelspannungen erzeugt:
Stellung 1 = R24 **0,47 Ω**, Stellung 3 = R25 **3 Ω**. Laut Good Display (Handbuch DESPI-C02)
brauchen die 7,5″-Panels GDEW075T7, GDEW075Z08, GDEW075Z09, GDEW075C21 und GDEW075C64 **0,47 Ω**.
3 Ω ist bei 7,5″ nur für GDEW075T8 vorgesehen. Ein falscher Wert führt dazu, dass das Panel nicht
richtig refresht.

**Befund mit 3 Ω am DEPG0750RWF86BF:** Grosse Flächen in Schwarz, Weiss und Rot werden richtig
dargestellt, Kalender und Text wirken deshalb normal. Dichte Muster schaltet der Boost-Wandler
nicht mehr vollständig um, geditherte Fotos werden fast schwarz. Kamerafotos der Testmuster:

| Testmuster | 3 Ω | 0,47 Ω |
|---|---|---|
| Schachbrett 8–32 px Schwarz/Weiss: schwarze Felder (Helligkeit, Schwarz-Referenz 33) | 117 | 22 |
| Schachbrett 8–32 px Rot/Weiss: rote Felder | dunkelgrau, ohne Rotanteil | rot |
| Kontrast Schwarz/Weiss bzw. Rot/Weiss | 29 / 81 | 128 / 106 |
| Schachbrett 1–4 px, mittlere Helligkeit | 73 (dunkel) | 119 (mittelgrau) |

Mit dem Standard-Treiber für UC8179-Panels (GxEPD2_750c_Z08) zeigte sich mit 3 Ω dasselbe Bild.
Es liegt also nicht an der Initialisierung des DEPG-Treibers.

**Empfehlung:** S6 bei allen 7,5″-Displays auf **0,47 Ω** stellen. In einer nächsten Revision die
Werkseinstellung auf 0,47 Ω setzen oder den Schalter beschriften.

## Flash U6 ohne Pull-up auf CS

`CS_Flash` (IO4) hat keinen Pull-up. Nach einem Reset und im Deep Sleep zieht der interne
Pull-down von IO4 den CS auf Low, U6 ist dann dauernd ausgewählt. Er hört dabei den Busverkehr
der Displays mit und kommt nicht in den Standby. Die Kalender-Firmware treibt CS deshalb sofort
nach dem Start High, schickt U6 in Deep Power-Down und hält CS im Deep Sleep High.

**Empfehlung:** In einer nächsten Revision einen 10-kΩ-Pull-up von `CS_Flash` auf +3V3_FLASH
vorsehen.

## Stecker _50 (7,3″, GDEP073E01): offene Punkte

Stand: 02.10.2026. Das 7,3″-Panel lief an der Universal-Platine bisher nicht.

**1. Positive und negative Panelspannungen sind verbunden (Schema und Layout)**
- An X4 Pin 32 und 31 stehen die Netznamen VSH_LV und VSH_LV2. Richtig sind laut Datenblatt
  GDEP073E01 (S. 6) und Referenz DESPI-C73 (P1 Pin 19/20) **VSL_LV** und **VSL_LV2**.
- Gleiche Netznamen verbindet Altium. Im Layout sind dadurch verbunden:
  - Pin 35 ↔ 32 über zwei VSH_LV-Vias und eine Bahn auf der Unterseite;
  - Pin 34 ↔ 31 über eine Bahn auf der Oberseite am inneren Ende der Pads, unter dem
    Steckergehäuse.
- **Korrektur in der nächsten Revision:** Netznamen an Pin 32/31 in VSL_LV/VSL_LV2 ändern.
- **Vorhandene Platinen:** Die Bahn auf der Unterseite lässt sich mit dem Skalpell trennen. Die
  Bahn unter dem Stecker ist erst nach Auslöten von X4 erreichbar. Nicht von unten bohren: Dort
  läuft auf der Unterseite die VPH-Leitung.

**2. Firmware: Kalender-Env spricht den falschen Stecker an**
- `GoogleCalendar_UniversalDriverCACH_gdep073e01` nutzt die Pins des Steckers _24
  (CS 27, D/C 25, RES 26, BUSY 32).
- Das Panel hängt am Stecker _50: **CS 16, D/C 5, RES 17, BUSY 33**. Die Galerie-Variante
  `Gallery_UniversalDriverCACH_GDEP073E01` nutzt diese Pins bereits.
- Noch nicht korrigiert, Hinweis an der Stelle in `src/ePaperCalendar/main_calendar.cpp`.

## Prüfungen auf dem Gerät

- **Flash U6:** JEDEC-ID `EF 40 15`, Löschen, Schreiben und Rücklesen eines Testsektors bestanden.
- **Upload über den Webserver:** 192 000 Byte in etwa 7,4 s inklusive Löschen.
  - Die Farbindizes im Flash stimmen mit dem überein, was beim Zeichnen gelesen wird.
  - Das Foto wird mit 0,47 Ω sauber dargestellt.
- **Deep Power-Down von U6:** Nach dem Befehl antwortet U6 nicht mehr. Nach einem Neustart ist er
  noch im Deep Power-Down. Die Galerie findet ihn danach wieder.

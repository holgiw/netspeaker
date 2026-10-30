# NetSpeaker

**Deutsch** | [English](#english)

> **Fertige Firmware (`.bin`):** Das ZIP unter *Code → Download ZIP* enthält die Firmware des jeweiligen Stands in `build_netspeaker/esp32s3/` und das Flash-Werkzeug. Doppelklick auf `build_netspeaker/flashESP.bat` genügt; fehlt `esptool`, lädt das Skript es selbst. Die komplette `merged.bin` und die Werkzeug-ZIP mit `esptool.exe` liegen im [neuesten Release](https://github.com/holgiw/ESP32-netspeaker/releases/latest) unter *Assets*. Siehe *Fertige Firmware flashen*.

Sprachausgabe und MP3-Player für den **ESP32-S3** (4 MB Flash, 2 MB PSRAM) mit einem **MAX98357A** I2S-Verstärker. Texte werden über [VoiceRSS](https://www.voicerss.org/) in Sprache umgewandelt, MP3-Dateien und Webradio-Streams werden direkt abgespielt. Bedienung über eine Weboberfläche, eine HTTP-API und MQTT.

## Hardware

| MAX98357A | ESP32-S3 (Standard) |
|---|---|
| VIN | 5 V |
| GND | GND |
| DIN | GPIO 2 |
| BCLK | GPIO 3 |
| LRC | GPIO 4 |
| SD | offen (Mono-Mix L+R) |
| GAIN | offen (9 dB) |

Lautsprecher 4–8 Ω an die beiden Ausgänge, keiner davon an GND. Ein Elko (100–470 µF) nahe VIN verhindert Knacken. Die Pins sind im Tab *Audio* änderbar (wirkt nach Neustart); erlaubt sind 1–18, 21, 38–42, 47, 48.

### Bauteile (Beispiele)

| Bauteil | Beispiel |
|---|---|
| Mikrocontroller | [ESP32-S3 Mini Development Board](https://www.amazon.de/dp/B0F1CR5RHV) (4 MB Flash, 2 MB PSRAM) |
| Verstärker | [MAX98357A I2S-DAC-Verstärkermodul für ESP32](https://www.amazon.de/dp/B0H5VLLTHY) |
| SD-Kartenleser (optional) | [Create idea Micro-SD-Kartenleser-Modul, 3,3 V, SPI, SD-Kartensteckplatz](https://www.amazon.de/dp/B0DRXBF5RW) |
| Lautsprecher | 4–8 Ω, 3 W oder mehr |

Die Links sind nur Beispiele; andere Module mit denselben Anschlüssen funktionieren ebenso.

### SD-Kartenleser (optional)

> **Hinweis:** Die SD-Karten-Unterstützung ist erst kurz getestet: Das Modul wird mit einer FAT32-Karte erkannt und eingebunden. Dauerbetrieb, Tempo, Dateimanager und Zufallswiedergabe sind noch nicht ausführlich geprüft.

Das Modul arbeitet mit 3,3 V über SPI und braucht keinen Pegelwandler. Es nimmt Micro-SD-Karten bis 2 GB und Micro-SDHC-Karten bis 32 GB, formatiert als FAT32.

| SD-Modul | ESP32-S3 (Standard) |
|---|---|
| 3V3 | 3V3 |
| GND | GND |
| MISO | GPIO 10 |
| CLK (SCK) | GPIO 11 |
| MOSI | GPIO 12 |
| CS | GPIO 13 |

Die Pins sind im Tab *Audio* änderbar (wirkt sofort). Das Verkabelungsbild ([verkabelung.svg](verkabelung.svg)) steht auch im Tab *Doku* der Weboberfläche. Leitungen kurz halten (unter 10 cm). Ohne Karte läuft alles andere unverändert.

## Fertige Firmware flashen

Ohne Arduino IDE und ohne das Repository klonen zu müssen. Es gibt zwei Wege:

**A. Werkzeug-ZIP aus dem Release (klein):**

1. Aus dem [neuesten Release](https://github.com/holgiw/ESP32-netspeaker/releases/latest) `netspeaker_flash.zip` laden und **komplett** in einen Ordner auspacken (`esptool.exe` muss neben `flashESP.bat` liegen).
2. ESP32-S3 per USB anschließen und `flashESP.bat` doppelklicken.

**B. ZIP des Repositorys (*Code → Download ZIP*):** auspacken und `build_netspeaker/flashESP.bat` doppelklicken. Fehlt `esptool.exe` (keine Arduino IDE), lädt das Skript es aus `netspeaker_flash.zip` des Releases (rund 14 MB, Prüfsumme geprüft) und legt es neben die Datei.

In beiden Fällen sucht das Skript den ESP, lädt `netspeaker-esp32s3.zip` vom neuesten Release (Prüfsumme SHA-256, Internet nötig), flasht ihn und fragt danach das WLAN ab (siehe unten). Die Einstellungen im Gerät bleiben erhalten, auch die Sprachdatei. Meldet der Virenscanner `esptool.exe` (Fehlalarm), richte eine Ausnahme für den Ordner ein.

Der Ordner `build_netspeaker/` enthält die Flash-Werkzeuge (`flashESP.bat`, `voicesetup.ps1`, `publish_builds.ps1`, `cleanup_history.ps1`, `readme.txt`) und in `esp32s3/` die Firmware des jeweiligen Stands (`cleanup_history.ps1` entfernt sie nach einem neuen Build aus allen älteren Commits, damit das Repository nicht wächst); ohne `esptool.exe` und ohne die vollständige `merged.bin` – beide kommen als Anhang der Releases (`netspeaker-esp32s3.zip`, `netspeaker-esp32s3.sha256`, `netspeaker_flash.zip` und die vollständige `netspeaker-<Version>-esp32s3-merged.bin`). Mit der `merged.bin` flasht man auch von Hand: `esptool --chip esp32s3 --port COM3 write_flash 0x0 netspeaker-<Version>-esp32s3-merged.bin` (das löscht das Dateisystem; danach die Sprachdatei neu hochladen).

## Bauen

Mit der Arduino IDE (oder `buildAll.bat`):

- Board: **ESP32S3 Dev Module** mit 4 MB Flash, Partitionsschema **No OTA (2MB APP/2MB SPIFFS)**, PSRAM (QSPI) und USB CDC On Boot aktivieren. Die Firmware ist etwa 1,9 MB groß und passt nicht in zwei OTA-Slots; es gibt daher kein OTA, Updates nur per USB. Das Dateisystem hat etwa 1,9 MB für MP3-Dateien.
- Bibliotheken: **ESP32-audioI2S** von schreibfaul1 (Version 4.0.0) und **PubSubClient**.

`buildAll.bat` baut mit `arduino-cli` und legt die Dateien in `build_netspeaker\esp32s3` ab. `build_netspeaker\flashESP.bat` flasht ein Gerät per USB, mit dem eigenen Build aus `esp32s3` oder, wenn es ihn nicht gibt, mit dem Paket vom neuesten Release. `flashAll.bat` prüft die Geräte im Netz und flasht angeschlossene Geräte per USB (OTA-Updates gibt es nicht). `build_netspeaker\publish_builds.ps1` erzeugt die Pakete für ein Release (`-Upload` lädt sie hoch).

**WLAN beim Flashen einrichten:** Beide Skripte fragen nach dem Flashen das WLAN ab (bei `flashAll.bat -Yes` entfällt die Frage). Sie zeigen die sichtbaren 2,4-GHz-Netze nach Signalstärke; ist der PC mit einem davon verbunden, genügt ein Tastendruck, um Namen und das im PC gespeicherte Passwort zu übernehmen. Sonst gibst du die Nummer oder den Namen ein und das Passwort zweimal verdeckt (8 bis 63 Zeichen, bei einem offenen Netz leer). Mit der Eingabetaste bleibt das WLAN unverändert. Die Daten gehen über das USB-Kabel an das Gerät; Zugangspunkt und Browser sind nicht nötig, und die übrigen Einstellungen bleiben erhalten. Kommt keine Bestätigung vom Gerät, richtest du das WLAN wie unter *Erster Start* im Browser ein.

## Erster Start

Ohne WLAN-Daten öffnet das Gerät den offenen Zugangspunkt `netspeaker-setup-…`, Weboberfläche unter `http://192.168.4.1/`. Im Tab *WLAN* bis zu drei Netze eintragen. Danach: `http://<hostname>.local/`.

## Weboberfläche

Tabs: **MP3 (Flash)** (Dateien im Gerät hochladen, abspielen, löschen, REST-Link kopieren), **Sprechen** (Text ansagen, Stopp, Lautstärke), **SD-Karte** (Dateimanager: abspielen, hochladen, neuer Ordner, umbenennen, löschen, herunterladen), **Webradio** (URL abspielen, 30 Webradio-/URL-Plätze mit Name und eigener Lautstärke speichern (angezeigt werden die belegten plus ein freier), per Knopf abspielen, verschieben, löschen, REST-Link kopieren), **Audio** (Anschlüsse mit einheitlicher Pin-Auswahl für Verstärker und SD-Karte, Lautstärke, überblenden und Mischen, Titelregel, Start, Sprachausgabe VoiceRSS mit API-Schlüssel, Sprache, Stimme, Tempo, Format), **MQTT**, **WLAN** (Hostname, drei Netze), **Zeit** (NTP, Zeitzone), **Status**, **Log**, **Doku** (alle REST- und MQTT-Aufrufe zum Kopieren). Die Statusleiste oben zeigt MQTT, VoiceRSS, Audio und Zeit; Export/Import der Einstellungen (ohne WLAN-Passwörter, mit API-Schlüssel) und Neustart sind dort als Symbole.

Den kostenlosen API-Schlüssel gibt es bei voicerss.org. VoiceRSS meldet Fehler (falscher Schlüssel, Tageslimit) im Log und im Tab *Sprechen*. Der Schlüssel steht nicht im Export der Einstellungen und muss auf einem neuen Gerät neu eingetragen werden (ein Import übernimmt ihn nur aus alten Exportdateien).

## Screenshots

| | |
|---|---|
| ![Sprechen mit laufendem Webradio](docs/screenshots/01-sprechen.jpg) | ![Webradio mit Stream-Info](docs/screenshots/02-webradio.jpg) |
| ![SD-Karte](docs/screenshots/03-sd-karte.jpg) | ![MP3 im Flash](docs/screenshots/04-mp3-flash.jpg) |
| ![Eingänge](docs/screenshots/05-eingaenge.jpg) | ![Zeit mit NTP-Servern](docs/screenshots/06-zeit.jpg) |

Weitere Bilder: [Doku mit Verkabelungsplan](docs/screenshots/07-doku.jpg), [Webradio mit englischer Oberfläche](docs/screenshots/08-webradio-english.jpg). Die Bilder zeigen den Hostnamen und die IP-Adresse eines Testgeräts.

## HTTP-API

```
curl -d "text=Hallo Welt" http://<ip>/api/say              # optional lang=, voice=, rate=, vol=
curl -H "Content-Type: application/json" -d "{\"text\":\"Hallo\",\"vol\":12}" http://<ip>/api/say
curl -d "url=https://example.com/a.mp3" http://<ip>/api/play    # Webradio-URL oder MP3-URL
curl -d "file=gong.mp3" http://<ip>/api/play                    # Datei aus dem Tab MP3 (Flash)
curl -d "url=http://stream.example.com/radio" http://<ip>/api/url
curl -d "file=gong.mp3" http://<ip>/api/file                    # ohne sd: zuerst Flash, dann SD-Karte; sd:/Pfad.mp3 nur Karte
curl -d "n=3" http://<ip>/api/station                          # gespeicherter Platz 1-30, oder name=Radio%20Eins
curl http://<ip>/api/stations
curl http://<ip>/stations.txt -o stations.txt              # Senderliste als Textdatei sichern
curl --data-binary @stations.txt -H "Content-Type: text/plain" http://<ip>/importstations   # Senderliste laden (ersetzt die ganze Liste)
curl -d "n=3" http://<ip>/api/stationdelete                    # Platz löschen (Name und URL); die folgenden Plätze rücken auf
curl -X POST http://<ip>/api/stop
curl -d "v=50" http://<ip>/api/volume                      # 0-100
curl http://<ip>/api/volume                                # meldet volume (allgemein) und playVolume (gerade wirkend)
curl http://<ip>/api/status
curl -d "broker=192.168.0.70&port=1883" http://<ip>/api/mqtt
curl -d "group1=netspeaker/all&group2=netspeaker/haus" http://<ip>/api/mqtt   # Gruppen-Topics 1 bis 5 (leer = entfernt); GET liefert sie als groups
curl -d "file=sd:/Musik/&shuffle=1&loop=1" http://<ip>/api/play   # Ordner der SD-Karte, zufällig und endlos
curl http://<ip>/api/pins                                  # freie Pins mit Zustand und Aktion
curl -d "pin=5&type=say&text=Die Tür ist offen" http://<ip>/api/pins   # type: say, file oder station; leer = keine Aktion
curl http://<ip>/api/lang                                  # vorhandene Sprachdateien
curl -F "file=@lang/en.json" "http://<ip>/upload?target=lang"   # Sprachdatei hochladen
curl -d "enable=1" http://<ip>/api/log                     # Log ein (Standard: aus); clear=1 leert ihn
```

Alle Parameter können als Formular, Query oder flaches JSON im Body kommen. Webradio-Streams über https brauchen viel RAM; http-URLs sind unkritisch.

## MQTT

Basis-Topic (Standard `netspeaker/<hostname>`): `…/say` (Text oder JSON `{"text":"…","lang":"de-de","voice":"Hanna","rate":0,"vol":60}`), `…/play` (URL oder Dateiname, auch JSON `{"url":"…"}` oder `{"file":"…"}`), `…/url` (nur URL), `…/file` (nur Dateiname aus dem Tab MP3 (Flash)), `…/station` (gespeicherter Platz 1–30 oder Name), `…/stop`, `…/volume` (0–21), `…/restart` (ohne Nutzlast, startet das Gerät neu; nicht retained senden, in den ersten 3 s nach dem Verbinden wird es ignoriert). Status in `…/status` (`idle`, `loading`, `playing`, `error: …`), die Lautstärke in `…/volume/state` (gerade wirkend) und `…/volume/general` (allgemein, beide retained), die Senderliste als JSON in `…/stations` (retained, nach jeder Änderung neu; Anfrage mit beliebiger Nutzlast an `…/list`), was gerade läuft in `…/now` (Sendername – Titel, Dateiname oder „Ansage“), Erreichbarkeit in `…/availability`.

## Hinweise

- Läuft ein Webradio und kommt eine Ansage oder MP3-Datei, wird das Radio sanft ausgeblendet (Tab *Audio*, Standard 700 ms, 0 = sofort) und dann gestoppt; nach der Ansage wird es mit derselben Dauer wieder eingeblendet, die Ansage gespielt und das Radio danach mit der alten Lautstärke neu verbunden (Live-Stream: geht live weiter). Die Audio-Bibliothek spielt nur eine Quelle gleichzeitig, ein Abblenden mit Mischen ist nicht möglich. `/api/stop` beendet auch das Radio.
- Der Text wird als MP3 in `/tts.mp3` im Flash-Dateisystem zwischengespeichert und dann abgespielt (höchstens 500 Zeichen je Ansage).
- Die TLS-Verbindung zu VoiceRSS prüft das Zertifikat nicht; der API-Schlüssel geht verschlüsselt, aber ohne Serverprüfung über das Netz.
- Webradio Start, Stopp und Wechsel: Ein Webradio wird beim Start aus dem Leerlauf langsam eingeblendet und beim Stopp (`/api/stop`, MQTT `…/stop`) langsam ausgeblendet. Kommt bei laufendem Radio ein anderer Sender (neue URL, `/api/station`, MQTT `…/station`), wird der alte langsam ausgeblendet, der neue leise gestartet und langsam eingeblendet (Tab *Audio*, „Radio Start/Stopp/Wechsel (ms)“, Standard 1500, 0 = sofort). Die Lautstärke des Senders ist dabei das Ziel. Wird der Sender gewählt, der schon läuft, spielt er einfach weiter (kein Ausblenden, kein Neustart). Ansagen und Dateien stoppen sofort.
- Lautstärke: ausdrücklicher Wert (`vol`) vor der Lautstärke des gespeicherten Senders vor der allgemeinen Lautstärke. Jede Wiedergabe setzt sie neu; die allgemeine Lautstärke ändert sich dadurch nicht.
- Das Log in die Datei ist standardmäßig aus (Tab *Log*, oder `/api/log`); die serielle Ausgabe läuft immer.
- Das Gerät hat keine Anmeldung und ist für das Heimnetz gedacht. Es gibt kein OTA (keine OTA-Partition, kein offener Update-Port); Updates nur per USB.

**Vorbelegter Sender:** Ein neues Gerät trägt auf Platz 1 den Sender *RSA* ein (`DEFAULT_STATION_NAME` und `DEFAULT_STATION_URL` in `config.h`). Das gilt, solange Platz 1 leer ist (neues Gerät oder gelöschter Platz); ein belegter Platz bleibt unverändert.

## Zuverlässigkeit

- **WLAN:** Der Treiber verbindet nach einem Abriss selbst neu. Bleibt die Verbindung länger als 6 s weg, probiert das Gerät der Reihe nach alle gespeicherten Netze (je 10 s), ohne die Hauptschleife zu blockieren, mit Abständen von 2 bis 15 s. Nach 10 Minuten ohne WLAN startet es neu; hat es den Zugangspunkt `netspeaker-setup-…` geöffnet, bleibt dieser stehen. Nach dem Wiederkommen werden mDNS und MQTT sofort neu aufgebaut.
- **MQTT:** Die Adresse des Brokers wird nur einmal aufgelöst, der Verbindungsaufbau hat ein TCP-Zeitlimit von 0,8 s und das Keepalive beträgt 15 s. Abstände der Versuche 1, 2, 4, 8, dann 10 s; beim Abspielen mindestens 8 s, damit der Ton selten gestört wird.
- **Webradio:** Reißt ein Live-Stream ab (WLAN oder Server weg), wird er automatisch neu verbunden (Abstände 3 bis 30 s), bis jemand stoppt. Endliche MP3-URLs werden nicht wiederholt.
- **Stream ohne Ton:** Kommt nach dem Start oder nach einem Verbindungsverlust 20 s lang weder Ton noch irgendein neues Datenstück an (der Server schweigt, die Audio-Bibliothek wartet dabei ohne Zeitlimit), verbindet das Gerät den Stream selbst neu, auch wenn es dieselbe URL ist. Ein langsamer Server, der noch Daten liefert, wird nicht unterbrochen; nach jedem erfolglosen Versuch verdoppelt sich die Wartezeit (20, 40, 80, 90 s).
- **Kennung:** Die MQTT-Client-ID wird immer aus der MAC-Adresse gebildet (`netspeaker-xxxxxx`), unabhängig vom Hostnamen. Der Import von Einstellungen übernimmt den Hostnamen nicht, damit zwei Geräte nicht denselben Namen bekommen.
- **SD-Karte:** MP3-Dateien von einer Micro-SD-Karte (FAT32, SD bis 2 GB und SDHC bis 32 GB, SPI-Modul mit 3,3 V). Standard-Pins MISO = GPIO 10, SCK (CLK) = GPIO 11, MOSI = GPIO 12, CS = GPIO 13, im Tab *Audio* änderbar. Im Tab *SD-Karte* gibt es eine Dateiliste der Karte mit Ordnern; `file=sd:/Ordner/Datei.mp3` per REST und MQTT, mit `/` am Ende der ganze Ordner (in die Warteschlange, bis zu 8). Ohne Karte läuft alles andere weiter; nach dem Einstecken bindet der Knopf *Neu einbinden* die Karte ein.+öschen oder hochladen.
- **Ordner abspielen:** Ein Ordner der SD-Karte (`sd:/Ordner/`) spielt alle MP3-Dateien nacheinander. Mit `shuffle=1` (Knopf *Zufall* im Tab *SD-Karte*) ist die Reihenfolge je Sitzung zufällig, jede Datei kommt einmal vor und die erste Datei ist nie die der letzten Sitzung dieses Ordners; mit `loop=1` (Häkchen *Endlos*) beginnt danach eine neue Runde, neu gemischt und nie mit der zuletzt gespielten Datei. Ansagen dazwischen kommen zuerst; Stopp oder ein neuer Sender beendet die Sitzung. REST: `curl -d "file=sd:/Musik/&shuffle=1&loop=1" http://<ip>/api/play`, MQTT: `{"file":"sd:/Musik/","shuffle":1,"loop":1}`.
- **Warteschlange:** Ansagen und MP3-Dateien reihen sich ein (bis zu 8) und werden nacheinander abgespielt, auch wenn sie schnell hintereinander eintreffen. Ohne Webradio blendet jede Ansage oder Datei ein und aus (Dauer wie *Radio aus-/einblenden*, bei kurzen Dateien kürzer). Mit Webradio läuft es leiser weiter und bleibt zwischen den Einträgen abgesenkt. Ein Stopp leert die Warteschlange, ein Sender wechselt sofort. Ist sie voll, kommt der Fehler "Warteschlange voll"; `/api/status` meldet die Zahl der Wartenden als `queue`.
- **Zustand wiederherstellen:** Das Speichern im Tab *Audio* ändert die laufende Wiedergabe nur noch, wenn die *Standard-Lautstärke* geändert wurde. Verbindet sich ein Webradio neu (Abriss, Aussetzer), behält es die zuletzt eingestellte Lautstärke. Mit dem Haken *Zuletzt gespielten Sender nach einem Neustart fortsetzen* (Standard: aus) merkt sich das Gerät den laufenden Sender samt Lautstärke und spielt ihn nach einem Neustart oder Stromausfall wieder ab; ein Stopp löscht den Merker.
- **Lautstärke:** Änderungen der Lautstärke (Regler, REST, MQTT, Titelregel) bei laufendem Ton gehen sanft zum neuen Wert, in 100 Stufen. Die Dauer (Standard 1500 ms) steht im Tab *Audio* und lässt sich je Aufruf angeben: REST `v=60&ms=3000`, MQTT `{"v":60,"ms":3000}`. Auch Ein- und Ausblenden der Sender sind dadurch feiner abgestuft.
- **Ansagen über Radio und Dateien:** Läuft ein Webradio, werden Ansagen und MP3-Dateien leise über das Radio gemischt; läuft eine MP3-Datei (zum Beispiel ein Ordner der SD-Karte), wird eine Ansage ebenso über die Datei gemischt und sie wird nicht gestoppt, nur abgesenkt. Das gilt auch für eine MP3-Datei aus dem Flash, die während einer Datei der SD-Karte aufgerufen wird: Sie springt in der Warteschlange nach vorn und startet sofort über der SD-Datei, die SD-Datei wird abgesenkt und danach wieder aufgeblendet. **Rangfolge** von hoch nach niedrig: 1. MP3 aus dem Flash, 2. Sprechen (`say`), 3. MP3 von der SD-Karte, 4. Webradio. 1 und 2 spielen nacheinander in der Reihenfolge des Aufrufs und senken 3 und 4 ab; eine niedrigere Stufe unterbricht nie eine höhere. Läuft eine SD-Datei als eigene Mischspur über dem Radio, kommt eine Flash-MP3 oder ein Text sofort auf die zweite Mischspur darüber; SD-Datei und Radio werden abgesenkt und spielen danach weiter (zwei Mischspuren gleichzeitig). Das Radio läuft ohne Unterbrechung und ohne Neuverbinden weiter und ist während der Ansage auf einen einstellbaren Anteil (Standard 35 %) abgesenkt; Ab- und Aufblenden dauern so lange wie die Einstellung *Radio aus-/einblenden* (Standard 1200 ms). Schalter im Tab *Audio* (Standard: an). Gemischt werden nur MP3-Dateien und VoiceRSS-Ansagen; ist das Radio noch nicht bereit oder der Schalter aus, gilt der alte Weg (Radio stoppt, danach neu verbinden).
- **MP3-Lautstärke angleichen:** Beim ersten Abspielen einer MP3-Datei (Gerät oder SD-Karte) misst das Gerät im Hintergrund ihre Lautheit (Bloecke zu 400 ms mit Gate, Ziel etwa -16 dBFS, Spitze unter -1 dBFS) und speichert den Ausgleich in `/loudness.bin`; die Datei selbst bleibt unverändert. Während der Messung (einige Sekunden bis eine Minute) läuft die Datei unverändert und wird dann sanft angepasst; danach gilt der Wert sofort. Ansagen und Radio bleiben unverändert. Haken im Tab *Audio* (Standard: an); der Ausgleich steht im Status als `gainDb`.
- **Stream-Info:** Im Tab *Webradio* zeigt ein aufklappbarer Bereich *Stream-Info* unter dem Namen, was der Sender mitschickt: Name, Titel, Genre, Beschreibung, Webseite, Logo-Adresse, Bitrate und Server, dazu Codec, Abtastrate und Kanäle. Angezeigt wird alles davon, was der Sender liefert; die Werte stehen auch im Status (`genre`, `desc`, `web`, `logo`, `bitrate`, `host`).
- **Starton:** Beim Einschalten und nach einem Neustart über die Weboberfläche spielt das Gerät einen kurzen Ton („buub“) in der eingestellten Lautstärke, direkt über den I2S-Ausgang. Nach einem Neustart per MQTT, nach eigenen Neustarts des Geräts (WLAN lange weg, Speicher knapp), nach einem Absturz und im abgesicherten Modus bleibt er aus. Haken *Kurzer Ton beim Start* im Tab *Audio* (Standard: an).
- **Sprachen:** Die Weboberfläche ist deutsch; eine Sprachdatei (`lang/en.json` für Englisch) lässt sich im Tab *Status* hochladen und wird im Browser angewendet. Ein Knopf in der Statuszeile wechselt zwischen DE und EN. Die Übersetzung liegt im Speicher des Geräts, nicht in der Firmware. Die Datei liegt im Ordner `lang/` und als Anhang jedes Releases; der Knopf *Von GitHub laden* im Tab *Status* holt sie direkt vom Projekt auf GitHub (das Gerät braucht dafür Internet). Wer die `merged.bin` des Releases bei Adresse 0x0 flasht, löscht damit das Dateisystem und muss die Sprachdatei danach neu hochladen. Weitere Sprachen: `lang/README.md`.
- **NTP-Server:** Im Tab *Zeit* lassen sich bis zu 5 NTP-Server eintragen; es erscheint immer ein leeres Feld mehr, bis alle fünf belegt sind. Die Netzwerk-Bibliothek fragt höchstens drei gleichzeitig: Es gelten die ersten drei belegten, kommt innerhalb von 45 s keine Zeit an, probiert das Gerät die nächsten.
- **MQTT-Anmeldung:** Im Tab *MQTT* lassen sich optional Benutzer und Passwort für den Broker eintragen (leer = ohne Anmeldung). Das Passwort wird nicht angezeigt, nicht exportiert und nie über die API ausgegeben; `/api/mqtt` nimmt optional `user` und `pass`.
- **Gruppen-Topics:** Im Tab *MQTT* lassen sich bis zu 5 zusätzliche Topics eintragen (Standard `netspeaker/all`, immer ein leeres Feld sichtbar). Das Gerät abonniert `<Topic>/#` und nimmt darunter dieselben Befehle an wie unter dem Basis-Topic; `mosquitto_pub -t netspeaker/all/stop -n` stoppt zum Beispiel alle Geräte. Ein leeres Feld entfernt das Abo.
- **Dateisuche:** Ein Dateiname ohne `sd:` (REST `/api/file`, MQTT `…/file` und `…/play`) wird zuerst im Flash gesucht, dann auf der SD-Karte (Pfad ab Wurzel); `sd:/Pfad.mp3` sucht nur auf der Karte.
- **Senderliste sichern und laden:** Im Tab *Webradio* speichert *Liste speichern* die gespeicherten Sender (Name, URL, Lautstärke) als Textdatei, *Liste laden* ersetzt die Liste durch eine solche Datei. Per REST: `GET /stations.txt` und `POST /importstations` (Text im Body).
- **Log:** Das Schreiben des Logs in die Datei (Tab *Log*) schaltet sich nach einer einstellbaren Zeit selbst aus (Standard 60 Minuten, 0 = nie), damit der Flash geschont wird.
- **Freie Pins:** Alle zugelassenen Pins (GPIO 1-18, 21, 33-42, 47, 48), die nicht im Tab *Audio* eingestellt sind (Verstärker, SD-Karte), keine Strapping-Pins (GPIO 3, 45, 46) sind und nicht die Status-LED tragen, bekommen einen Pull-up (High), damit offene Pins nicht schweben. Das gilt nach dem Start und nach jeder Änderung der SD-Pins.
- **Eingänge:** Der Tab *Eingänge* listet die freien Pins mit Pull-up und ihren Zustand (High oder Low). Je Pin lässt sich eine Aktion festlegen: Bekommt der Pin Kontakt nach GND (zum Beispiel durch eine Taste), spielt das Gerät eine Ansage (VoiceRSS), eine MP3-Datei, einen Ordner der SD-Karte, eine URL oder einen gespeicherten Webradio-Sender (Name oder Platznummer). Der Pin muss etwa 40 ms auf GND bleiben, es wird einmal je Tastendruck ausgelöst; die Aktion kommt wie ein REST-Aufruf in die Warteschlange. REST: `GET /api/pins` liefert die Pins, `POST /api/pins` mit `pin`, `type` (say, file oder station), `text` und optional `vol` speichert.
- **Titel zur Musik:** Der Titel eines Webradios kommt mit dem Datenstrom, die Musik dazu spielt erst, wenn der Eingangspuffer abgespielt ist. Das Gerät zeigt den Titel deshalb um die Dauer des gepufferten Tons verzögert an (Schätzung aus Füllstand und Bitrate); die Titelregel folgt derselben Verzögerung.
- **Formate:** Es werden nur MP3-Dateien hochgeladen und abgespielt; Webradios als MP3 oder Vorbis. FLAC, AAC, WAV und Opus sind ausgebaut (kleinere Firmware); dazu liegt eine angepasste Kopie der Bibliothek in lib\ESP32-audioI2S, die `buildAll.bat` benutzt.
- **Hochladen:** Im Tab MP3 (Flash) lassen sich mehrere Dateien auf einmal wählen; sie werden nacheinander hochgeladen, die Rückmeldung nennt Erfolge und Fehler.
- **Hinweise (ⓘ):** Erklärungen zu Feldern, Häkchen und Auswahlen stehen hinter einem ⓘ direkt an der Beschriftung (Mouseover zeigt den Text), wie bei der Uhr. Symbol-Knöpfe, Statuspunkte und die Tabs behalten ihren kurzen Tooltip.
- **Titelregel:** Enthält der Titel des Webradios eines der Stichworte aus dem Tab *Audio* (Standard "Cambodia", Lautstärke 86), wird die Lautstärke angehoben und beim nächsten Titel oder Ende des Senders wieder gesenkt. Mehrere Stichworte stehen durch Komma oder Semikolon getrennt; mit  Ein Sender meldet das Ende eines Titels nicht; mit *höchstens … s* (Standard 0 = bis zum nächsten Titel) endet die Regel nach einer festen Zeit, damit Werbung nach dem Titel nicht laut bleibt.
- **Start:** Das Gerät startet immer stumm. Mit dem Haken „Beim Start Sender 1 abspielen“ im Tab *Audio* (Standard: aus) spielt es nach dem Start, wenn das WLAN seit 2 s steht und der Stream von Platz 1 antwortet, diesen Sender (mit Einblenden). Antwortet der Stream nicht, versucht es bis zu 10-mal im Abstand von 6 s; wurde inzwischen selbst etwas gestartet, entfällt der Autostart.
- **Download der Ansage:** Mit Zeitlimit (25 s), Größenlimit und Watchdog-Fütterung; hängendes Laden wird nach 60 s abgebrochen.
- **Selbstschutz:** Watchdog (30 s), Neustart bei dauerhaft knappem Speicher (unter 15 KB länger als 60 s), Sicherheitsmodus nach 3 Abstürzen in Folge (ohne MQTT und Radio-Neuverbindung, die Weboberfläche bleibt). Der Tab *Status* zeigt Neustartgrund, Abstürze, WLAN- und MQTT-Wiederverbindungen und den kleinsten freien Speicher.

---

## Lizenz und Drittkomponenten

Das Projekt steht unter der **GNU General Public License v3.0** (Datei `LICENSE`). Das gilt, weil die Firmware die Bibliothek ESP32-audioI2S (GPL-3.0) einbindet; wer die Firmware weitergibt, muss auch den Quelltext unter denselben Bedingungen bereitstellen.

| Komponente | Lizenz | Hinweis |
|---|---|---|
| [ESP32-audioI2S](https://github.com/schreibfaul1/ESP32-audioI2S) 4.0.0 (schreibfaul1) | GPL-3.0 | Geändert in `lib/ESP32-audioI2S`: nur MP3- und Vorbis-Decoder werden angelegt (WAV, AAC, FLAC, Opus ausgebaut), dazu ein Einsprung für das Mischen. Die Dekoderquellen für AAC und FLAC liegen unverändert im Ordner, werden aber nicht verwendet. |
| Vorbis-Decoder (Xiph.Org) in der Bibliothek | BSD-Stil | Lizenztext in den Quelldateien |
| Arduino-ESP32 (Espressif) | LGPL-2.1 | wird beim Bauen eingebunden, nicht mitgeliefert |
| PubSubClient (Nick O'Leary) | MIT | wird beim Bauen eingebunden, nicht mitgeliefert |
| esptool (Espressif) | GPL-2.0+ | liegt als `esptool.exe` nur im Anhang `netspeaker_flash.zip` der Releases, Quelltext: https://github.com/espressif/esptool |

**VoiceRSS** ist ein fremder Dienst: Es gelten dessen Nutzungsbedingungen und Tageslimits, der API-Schlüssel ist persönlich. **Webradio-Streams** unterliegen den Rechten der Sender. Das Repository enthält keine Sounds und keine Produktfotos (Rechte nicht geklärt); eigene MP3-Dateien werden über den Tab *MP3* hochgeladen.

## Kontakt

E-Mail: [carleaner@gmx.de](mailto:carleaner@gmx.de)  
GitHub: <https://github.com/holgiw/ESP32-netspeaker>

## English

Voice output and MP3 player for the **ESP32-S3** (4 MB Flash, 2 MB PSRAM) with a **MAX98357A** I2S amplifier. Text is converted to speech by [VoiceRSS](https://www.voicerss.org/); MP3 files and web radio streams play directly. Control via a web interface, an HTTP API and MQTT.

> **Finished firmware (`.bin`):** The ZIP under *Code → Download ZIP* contains the firmware of that state in `build_netspeaker/esp32s3/` and the flash tool. Double-clicking `build_netspeaker/flashESP.bat` is enough; if `esptool` is missing, the script downloads it. The complete `merged.bin` and the tools ZIP with `esptool.exe` are in the [newest release](https://github.com/holgiw/ESP32-netspeaker/releases/latest) under *Assets*. See *Flashing the finished firmware*.

### Hardware

| MAX98357A | ESP32-S3 (default) |
|---|---|
| VIN | 5 V |
| GND | GND |
| DIN | GPIO 2 |
| BCLK | GPIO 3 |
| LRC | GPIO 4 |
| SD | open (mono mix L+R) |
| GAIN | open (9 dB) |

Speaker 4–8 Ω on the two outputs, neither to GND. A 100–470 µF capacitor near VIN prevents crackling. Pins can be changed in the *Audio* tab (applies after restart); allowed are 1–18, 21, 38–42, 47, 48.

### Parts (examples)

| Part | Example |
|---|---|
| Microcontroller | [ESP32-S3 Mini development board](https://www.amazon.de/dp/B0F1CR5RHV) (4 MB flash, 2 MB PSRAM) |
| Amplifier | [MAX98357A I2S DAC amplifier module for ESP32](https://www.amazon.de/dp/B0H5VLLTHY) |
| SD card reader (optional) | [Create idea micro SD card reader module, 3.3 V, SPI, SD card slot](https://www.amazon.de/dp/B0DRXBF5RW) |
| Speaker | 4–8 Ω, 3 W or more |

The links are only examples; other modules with the same connections work as well.

### SD card reader (optional)

> **Note:** SD card support has only been tested briefly: the module is detected and mounted with a FAT32 card. Long-term use, speed, the file manager and random playback have not been checked thoroughly yet.

The module works at 3.3 V over SPI and needs no level shifter. It takes micro SD cards up to 2 GB and micro SDHC cards up to 32 GB, formatted as FAT32.

| SD module | ESP32-S3 (default) |
|---|---|
| 3V3 | 3V3 |
| GND | GND |
| MISO | GPIO 10 |
| CLK (SCK) | GPIO 11 |
| MOSI | GPIO 12 |
| CS | GPIO 13 |

The pins can be changed in the *Audio* tab (applies immediately). The wiring picture ([verkabelung.svg](verkabelung.svg)) is also in the *Doku* tab of the web interface. Keep the wires short (under 10 cm). Without a card everything else works unchanged.

### Flashing the finished firmware

Without the Arduino IDE and without cloning the repository. There are two ways:

**A. Tools zip from the release (small):**

1. Download `netspeaker_flash.zip` from the [newest release](https://github.com/holgiw/ESP32-netspeaker/releases/latest) and unpack the **whole** zip into a folder (`esptool.exe` has to be next to `flashESP.bat`).
2. Connect the ESP32-S3 via USB and double-click `flashESP.bat`.

**B. ZIP of the repository (*Code → Download ZIP*):** unpack it and double-click `build_netspeaker/flashESP.bat`. If `esptool.exe` is missing (no Arduino IDE), the script downloads it from `netspeaker_flash.zip` of the release (about 14 MB, checksum checked) and puts it next to the file.

In both cases the script finds the ESP, downloads `netspeaker-esp32s3.zip` from the newest release (SHA-256 checked, internet needed), flashes it and then asks for the WiFi (see below). The settings in the device stay, including the language file. If the virus scanner reports `esptool.exe` (false alarm), add an exception for the folder.

The folder `build_netspeaker/` holds the flash tools (`flashESP.bat`, `voicesetup.ps1`, `publish_builds.ps1`, `cleanup_history.ps1`, `readme.txt`) and, in `esp32s3/`, the firmware of that state (`cleanup_history.ps1` removes it from all older commits after a new build so the repository does not grow); `esptool.exe` and the complete `merged.bin` are not in it, they are attached to the releases (`netspeaker-esp32s3.zip`, `netspeaker-esp32s3.sha256`, `netspeaker_flash.zip` and the complete `netspeaker-<version>-esp32s3-merged.bin`). With the `merged.bin` you can also flash by hand: `esptool --chip esp32s3 --port COM3 write_flash 0x0 netspeaker-<version>-esp32s3-merged.bin` (this erases the file system; upload the language file again afterwards).

### Build

With the Arduino IDE (or `buildAll.bat`):

- Board: **ESP32S3 Dev Module** with 4 MB flash, partition scheme **No OTA (2MB APP/2MB SPIFFS)**, enable PSRAM (QSPI) and USB CDC On Boot. The firmware is about 1.9 MB and does not fit two OTA slots; there is therefore no OTA, updates only via USB. The file system has about 1.9 MB for MP3 files.
- Libraries: **ESP32-audioI2S** by schreibfaul1 (version 4.0.0) and **PubSubClient**.

`buildAll.bat` builds with `arduino-cli` and puts the files in `build_netspeaker\esp32s3`. `build_netspeaker\flashESP.bat` flashes one device via USB, with your own build from `esp32s3` or, if there is none, with the package from the newest release. `flashAll.bat` checks the devices on the network and flashes connected devices via USB (no OTA updates). `build_netspeaker\publish_builds.ps1` creates the packages for a release (`-Upload` uploads them).

**Setting up the WiFi while flashing:** Both scripts ask for the WiFi after flashing (with `flashAll.bat -Yes` the question is skipped). They show the visible 2.4 GHz networks by signal strength; if the PC is connected to one of them, one key press takes over its name and the password stored in the PC. Otherwise you enter the number or the name and the password twice, hidden (8 to 63 characters, empty for an open network). Enter leaves the WiFi unchanged. The data goes to the device over the USB cable; the access point and the browser are not needed, and the other settings stay. If the device sends no confirmation, set up the WiFi in the browser as described under *First start*.

### First start

Without WiFi credentials the device opens the open access point `netspeaker-setup-…`, web interface at `http://192.168.4.1/`. Enter up to three networks in the *WLAN* tab. After that: `http://<hostname>.local/`.

### Web interface

Tabs: **MP3 (Flash)** (upload, play and delete files in the device), **Sprechen** (speak text, stop, volume), **SD-Karte** (file manager: play, upload, new folder, rename, delete, download), **Webradio** (play URL, save, play, move and delete 30 stations), **Audio** (connections with a uniform pin selection for amplifier and SD card, volume, fading and mixing, title rule, start, speech output VoiceRSS with API key, language, voice, rate, format), **MQTT**, **WLAN** (hostname, three networks), **Zeit** (NTP, time zone), **Status**, **Log**. The top status bar shows MQTT, VoiceRSS, audio and time; export/import of the settings (without WiFi passwords, with the API key) and restart are icons there.

The free API key is available at voicerss.org. VoiceRSS errors (wrong key, daily limit) show up in the log and in the *Sprechen* tab. The key is not part of the settings export and must be entered again on a new device (an import only takes it from old export files).

### Screenshots

The screenshots in the German section above (folder `docs/screenshots/`) show the web interface; [`08-webradio-english.jpg`](docs/screenshots/08-webradio-english.jpg) shows it in English after loading `lang/en.json`.

### HTTP API and MQTT

See the German section above; the commands and topics are the same. Parameters can come as a form, query or flat JSON body; MQTT `say`/`play` accept plain text or JSON, plus the extra topics `url`, `file` and `station` (stored slot 1–30 or name). The calls `pins` (free pins with actions) and `lang` (language files) are new. Up to 30 web radio/URL slots are stored in the Webradio tab; the Doku tab lists all calls with copy buttons.

### Notes

- If a web radio is playing and an announcement or MP3 file arrives, the radio is faded out gently (*Audio* tab, default 700 ms, 0 = immediately) and then stopped for it; after the announcement it is faded in again with the same duration, the announcement plays and the radio reconnects afterwards with the old volume (live stream: continues live). The audio library plays only one source at a time, so fading with mixing is not possible. `/api/stop` also ends the radio.
- The speech is stored as MP3 in `/tts.mp3` in the flash file system and then played (at most 500 characters per announcement).
- The TLS connection to VoiceRSS does not verify the certificate; the API key is sent encrypted but without server verification.
- Web radio start, stop and change: a web radio is faded in slowly when started from idle and faded out slowly on stop (`/api/stop`, MQTT `…/stop`). If another station arrives while a radio is playing (new URL, `/api/station`, MQTT `…/station`), the old one is faded out slowly, the new one is started silent and faded in slowly (*Audio* tab, "Radio Start/Stopp/Wechsel (ms)", default 1500, 0 = immediately). The volume of the station is the target. If the station that is already running is chosen, it simply keeps playing (no fade, no restart). Announcements and files stop immediately.
- Volume: explicit value (`vol`) before the volume of the stored station before the general volume. Every playback sets it anew; the general volume does not change because of it.
- Logging to the file is off by default (*Log* tab, or `/api/log`); the serial output always runs.
- The device has no login and is meant for the home network. There is no OTA (no OTA partition, no open update port); updates only via USB.

**Preset station:** A new device puts the station *RSA* into slot 1 (`DEFAULT_STATION_NAME` and `DEFAULT_STATION_URL` in `config.h`). This applies as long as slot 1 is empty (new device or deleted slot); an occupied slot stays as it is.

## Reliability

- **WiFi:** The driver reconnects by itself after a drop. If the connection stays away for more than 6 s, the device tries all stored networks in turn (10 s each) without blocking the main loop, with delays from 2 to 15 s. After 10 minutes without WiFi it restarts; if it opened the access point `netspeaker-setup-…`, that stays. After the WiFi is back, mDNS and MQTT are rebuilt immediately.
- **MQTT:** The broker address is resolved only once, connecting has a TCP timeout of 0.8 s and the keepalive is 15 s. Retry delays 1, 2, 4, 8, then 10 s; at least 8 s while playing so the sound is rarely disturbed.
- **Web radio:** If a live stream drops (WiFi or server gone), it is reconnected automatically (delays 3 to 30 s) until someone stops it. Finite MP3 URLs are not repeated.
- **Stream without sound:** If neither sound nor any new chunk of data arrives for 20 s after the start or after a connection loss (the server stays silent while the audio library waits without a limit), the device reconnects the stream itself, even if it is the same URL. A slow server that still delivers data is not interrupted; after each unsuccessful attempt the wait doubles (20, 40, 80, 90 s).
- **Identity:** The MQTT client ID is always built from the MAC address (`netspeaker-xxxxxx`), independent of the hostname. Importing settings does not take over the hostname, so two devices never share a name.
- **SD card:** MP3 files from a micro SD card (FAT32, SD up to 2 GB and SDHC up to 32 GB, SPI module with 3.3 V). Default pins MISO = GPIO 10, SCK (CLK) = GPIO 11, MOSI = GPIO 12, CS = GPIO 13, changeable in the *Audio* tab. The *SD-Karte* tab has a file list of the card with folders; `file=sd:/folder/file.mp3` via REST and MQTT, with `/` at the end the whole folder (into the queue, up to 8). Without a card everything else keeps running; after inserting, the button *Neu einbinden* mounts the card. File manager in the *SD-Karte* tab: upload (several files) into the shown folder, new folder, rename, delete (folder with content, with confirmation) and download; also via REST (`/upload?target=sd&dir=...`, `/api/sdmkdir`, `/api/sdrename`, `/api/sddelete`, `/api/sdfile`). Names on the card are cleaned (umlauts transliterated, spaces become underscores), MP3 only.
- **Playing a folder:** A folder of the SD card (`sd:/folder/`) plays all MP3 files one after another. With `shuffle=1` (button *Zufall* in the *SD-Karte* tab) the order is random per session, every file appears once and the first file is never the one the last session of this folder started with; with `loop=1` (checkbox *Endlos*) a new round follows, reshuffled and never starting with the file played last. Announcements in between come first; stop or a new station ends the session. REST: `curl -d "file=sd:/Musik/&shuffle=1&loop=1" http://<ip>/api/play`, MQTT: `{"file":"sd:/Musik/","shuffle":1,"loop":1}`.
- **Queue:** Announcements and MP3 files line up (up to 8) and are played one after another, even if they arrive in quick succession. Without a web radio each announcement or file fades in and out (duration like *Radio aus-/einblenden*, shorter for short files). With a web radio it keeps playing quieter and stays lowered between the entries. A stop empties the queue, a station change takes effect at once. If it is full the error "Warteschlange voll" is returned; `/api/status` reports the number of waiting entries as `queue`.
- **Restore the state:** Saving in the *Audio* tab changes the running playback only if the *Standard-Lautstärke* was changed. If a web radio reconnects (drop, dropout) it keeps the volume last set. With the checkbox *Zuletzt gespielten Sender nach einem Neustart fortsetzen* (default: off) the device remembers the running station with its volume and plays it again after a restart or power failure; a stop deletes the memory.
- **Volume:** Changes of the volume (slider, REST, MQTT, title rule) while sound is playing move gently to the new value, in 100 steps. The duration (default 1500 ms) is set in the *Audio* tab and can be given per call: REST `v=60&ms=3000`, MQTT `{"v":60,"ms":3000}`. Fading stations in and out is also finer as a result.
- **Announcements over radio and files:** While a web radio is running, announcements and MP3 files are mixed quietly over it; while an MP3 file runs (for example a folder of the SD card), an announcement is mixed over the file the same way and the file is only lowered, not stopped. The same holds for an MP3 file from the flash that is called while a file of the SD card is playing: it jumps ahead in the queue and starts right away over the SD file, the SD file is lowered and faded back up afterwards. **Priority** from high to low: 1. MP3 from the flash, 2. speech (`say`), 3. MP3 from the SD card, 4. web radio. 1 and 2 play one after the other in the order of the calls and lower 3 and 4; a lower level never interrupts a higher one. If an SD file runs as its own mix track over the radio, a flash MP3 or a text goes right away onto the second mix track above it; SD file and radio are lowered and continue afterwards (two mix tracks at the same time). The radio keeps playing without interruption or reconnecting and is lowered to an adjustable share (default 35 %) during the announcement; lowering and raising take as long as the setting *Radio aus-/einblenden* (default 1200 ms). Switch in the *Audio* tab (default: on). Only MP3 files and VoiceRSS announcements are mixed; if the radio is not ready yet or the switch is off, the old way applies (radio stops, then reconnects).
- **Match MP3 volume:** On the first play of an MP3 file (device or SD card) the device measures its loudness in the background (400 ms blocks with a gate, target about -16 dBFS, peak below -1 dBFS) and stores the correction in `/loudness.bin`; the file itself stays unchanged. During the measurement (a few seconds up to a minute) the file plays unchanged and is then adjusted gently; afterwards the value applies at once. Announcements and radio stay unchanged. Checkbox in the *Audio* tab (default: on); the correction is in the status as `gainDb`.
- **Stream info:** In the *Webradio* tab a collapsible *Stream-Info* area under the name shows what the station sends: name, title, genre, description, website, logo address, bitrate and server, plus codec, sample rate and channels. Everything of this that the station delivers is shown; the values are also in the status (`genre`, `desc`, `web`, `logo`, `bitrate`, `host`).
- **Start tone:** On power-up and after a restart from the web interface the device plays a short tone ("buub") at the set volume, directly through the I2S output. After a restart via MQTT, after restarts the device triggers itself (WiFi lost for long, low memory), after a crash and in safe mode it stays off. Checkbox *Kurzer Ton beim Start* in the *Audio* tab (default: on).
- **Languages:** The web interface is in German; a language file (`lang/en.json` for English) can be uploaded in the *Status* tab and is applied in the browser. A button in the status bar switches between DE and EN. The translation lives in the memory of the device, not in the firmware. The file is in the `lang/` folder and attached to every release; the button *Von GitHub laden* in the *Status* tab fetches it directly from the project on GitHub (the device needs internet for this). Flashing the `merged.bin` of a release at address 0x0 erases the file system, so the language file has to be uploaded again afterwards. More languages: `lang/README.md`.
- **NTP servers:** In the *Zeit* tab up to 5 NTP servers can be entered; there is always one more empty field until all five are used. The network library asks three at most at the same time: the first three used ones apply, and if no time arrives within 45 s the device tries the next ones.
- **MQTT login:** In the *MQTT* tab a user and password for the broker can optionally be entered (empty = no login). The password is not shown, not exported and never output by the API; `/api/mqtt` optionally takes `user` and `pass`.
- **Group topics:** In the *MQTT* tab up to 5 additional topics can be entered (default `netspeaker/all`, one empty field always shown). The device subscribes to `<topic>/#` and accepts the same commands under it as under the base topic; `mosquitto_pub -t netspeaker/all/stop -n` stops all devices, for example. An empty field removes the subscription.
- **File lookup:** A file name without `sd:` (REST `/api/file`, MQTT `…/file` and `…/play`) is looked up in the flash first and then on the SD card (path from the root); `sd:/path.mp3` looks on the card only.
- **Saving and loading the station list:** In the *Webradio* tab *Liste speichern* (save list) stores the saved stations (name, URL, volume) as a text file, *Liste laden* (load list) replaces the list with such a file. Via REST: `GET /stations.txt` and `POST /importstations` (text in the body).
- **Log:** Writing the log to the file (*Log* tab) switches itself off after an adjustable time (default 60 minutes, 0 = never) to spare the flash.
- **Free pins:** All allowed pins (GPIO 1-18, 21, 33-42, 47, 48) that are not configured in the *Audio* tab (amplifier, SD card), are no strapping pins (GPIO 3, 45, 46) and do not carry the status LED get a pull-up (high), so that open pins do not float. This applies after the start and after every change of the SD pins.
- **Inputs:** The *Eingänge* tab lists the free pins with pull-up and their state (high or low). An action can be set per pin: when the pin gets contact to GND (for example through a button), the device plays an announcement (VoiceRSS), an MP3 file, a folder of the SD card, a URL or a saved web radio station (name or slot number). The pin must stay at GND for about 40 ms, it triggers once per button press; the action enters the queue like a REST call. REST: `GET /api/pins` returns the pins, `POST /api/pins` with `pin`, `type` (say, file or station), `text` and optionally `vol` saves.
- **Title with the music:** The title of a web radio comes with the data stream, the music for it plays only when the input buffer has been played. The device therefore shows the title delayed by the duration of the buffered sound (estimated from fill level and bitrate); the title rule follows the same delay.
- **Formats:** Only MP3 files are uploaded and played; web radios as MP3 or Vorbis. FLAC, AAC, WAV and Opus are removed (smaller firmware); a modified copy of the library in lib\ESP32-audioI2S is used by `buildAll.bat` for that.
- **Upload:** In the MP3 (Flash) tab several files can be selected at once; they are uploaded one after another and the result lists successes and failures.
- **Hints (ⓘ):** Explanations of fields, checkboxes and selections are behind a ⓘ right at the label (mouse-over shows the text), like on the clock. Symbol buttons, status dots and the tabs keep their short tooltip.
- **Title rule:** If the title of the web radio contains one of the keywords from the *Audio* tab (default "Cambodia", volume 86), the volume is raised and lowered again with the next title or when the station ends. Several keywords are separated by comma or semicolon; with `=value` a keyword gets its own volume, for example `Cambodia=90, Thunder` (without it the volume of the rule applies). A station does not report the end of a title; with *höchstens … s* (default 0 = until the next title) the rule ends after a fixed time so that ads after the title do not stay loud.
- **Start:** The device always starts silent. With the checkbox "Beim Start Sender 1 abspielen" in the *Audio* tab (default: off) it plays station 1 after the start once the WiFi has been up for 2 s and the stream of slot 1 answers (with fade-in). If the stream does not answer it tries up to 10 times at 6 s intervals; if something was started by hand in the meantime, the autostart is dropped.
- **Announcement download:** With a deadline (25 s), a size limit and watchdog feeding; hanging loads are aborted after 60 s.
- **Self protection:** Watchdog (30 s), restart if free memory stays low (under 15 KB for more than 60 s), safe mode after 3 crashes in a row (without MQTT and radio reconnect, the web interface stays). The *Status* tab shows the reset reason, crashes, WiFi and MQTT reconnects and the smallest free memory.

## License and third-party components

The project is licensed under the **GNU General Public License v3.0** (file `LICENSE`). This is because the firmware links the library ESP32-audioI2S (GPL-3.0); whoever passes on the firmware must also provide the source under the same terms.

| Component | License | Note |
|---|---|---|
| [ESP32-audioI2S](https://github.com/schreibfaul1/ESP32-audioI2S) 4.0.0 (schreibfaul1) | GPL-3.0 | Modified in `lib/ESP32-audioI2S`: only MP3 and Vorbis decoders are created (WAV, AAC, FLAC, Opus removed), plus a hook for mixing. The AAC and FLAC decoder sources remain unchanged in the folder but are not used. |
| Vorbis decoder (Xiph.Org) inside the library | BSD-style | license text in the source files |
| Arduino-ESP32 (Espressif) | LGPL-2.1 | pulled in at build time, not shipped |
| PubSubClient (Nick O'Leary) | MIT | pulled in at build time, not shipped |
| esptool (Espressif) | GPL-2.0+ | included as `esptool.exe` only in the release attachment `netspeaker_flash.zip`, source: https://github.com/espressif/esptool |

**VoiceRSS** is a third-party service: its terms of use and daily limits apply, and the API key is personal. **Web radio streams** are subject to the broadcasters' rights. The repository contains no sounds and no product photos (rights not clarified); upload your own MP3 files in the *MP3* tab.

## Contact

E-mail: [carleaner@gmx.de](mailto:carleaner@gmx.de)  
GitHub: <https://github.com/holgiw/ESP32-netspeaker>

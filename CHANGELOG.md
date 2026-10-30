# Changelog

## 2026-10-30 - Version 1.3.9

**Deutsch**

- Der Opus-Dekoder ist ausgebaut: Die Firmware wird um rund 160 KB kleiner (1,93 statt 2,09 MB) und lässt wieder Platz für Neues. Webradios und Dateien im Opus-Format spielen nicht mehr; MP3 und Vorbis laufen wie bisher.
- Zwei Mischspuren: Läuft eine SD-Datei über dem Radio, kommt eine Flash-MP3 oder ein Text auf eine zweite Spur darüber. SD-Datei und Radio werden abgesenkt und spielen danach weiter; die SD-Datei wird nicht mehr durch die Ansage ersetzt.
- Die Tabs stehen in der Reihenfolge der Rangfolge: MP3 (Flash), Sprechen, SD-Karte, Webradio, danach die übrigen. Beim Öffnen der Seite ist der erste Tab, MP3 (Flash), aktiv.

**English**

- The Opus decoder is removed: the firmware gets about 160 KB smaller (1.93 instead of 2.09 MB) and leaves room for new features again. Web radios and files in Opus format no longer play; MP3 and Vorbis work as before.
- Two mix tracks: if an SD file runs over the radio, a flash MP3 or a text goes onto a second track above it. SD file and radio are lowered and continue afterwards; the SD file is no longer replaced by the announcement.
- The tabs follow the playback priority: MP3 (Flash), Sprechen, SD-Karte, Webradio, then the others. When the page opens, the first tab, MP3 (Flash), is active.

## 2026-10-30 - Version 1.3.8

**Deutsch**

- Rangfolge der Wiedergabe von hoch nach niedrig: 1. MP3 aus dem Flash, 2. Sprechen, 3. MP3 von der SD-Karte, 4. Webradio. Flash-MP3 und Sprechen spielen nacheinander in der Reihenfolge des Aufrufs und senken SD-Datei und Radio ab; ein Text springt nicht mehr vor eine Flash-MP3, die allein läuft. Läuft eine SD-Datei als Mischspur über dem Radio, ersetzt eine Flash-MP3 oder ein Text diese Spur (kurz ausgeblendet), das Radio bleibt abgesenkt und kehrt danach zurück.
- Absturz behoben: Der erste Namensabruf nach dem Start (zum Beispiel beim ersten Senderstart) konnte das Gerät mit einem Fehler in `sys_untimeout` neu starten, wenn gerade die Zeitabfrage lief. Jetzt wird vor der Zeitabfrage einmal abgerufen.

**English**

- Playback priority from high to low: 1. MP3 from the flash, 2. speech, 3. MP3 from the SD card, 4. web radio. Flash MP3 and speech play one after the other in the order of the calls and lower the SD file and the radio; a text no longer jumps ahead of a flash MP3 that plays alone. If an SD file runs as a mix layer over the radio, a flash MP3 or a text replaces that layer (faded out briefly), the radio stays lowered and returns afterwards.
- Crash fixed: the first name lookup after the start (for example the first station start) could restart the device with an error in `sys_untimeout` if the time query was running at that moment. Now one lookup is done before the time query.

## 2026-10-30 - Version 1.3.7

**Deutsch**

- Webradio: bis zu 30 statt 15 gespeicherte Plätze (Tab *Webradio*, REST `/api/station`, MQTT `…/station`, Eingänge). Bereits gespeicherte Plätze bleiben erhalten; der MQTT-Puffer ist für die längere Senderliste auf 8 KB gewachsen.
- Senderliste sichern und laden: Im Tab *Webradio* speichern *Liste speichern* und *Liste laden* die Liste als Textdatei (`/stations.txt`, `POST /importstations`); das Laden ersetzt die ganze Liste.
- Eine MP3-Datei aus dem Flash, die während einer Datei der SD-Karte aufgerufen wird, startet sofort: Die SD-Datei wird abgesenkt (wie bei einer Ansage), die Flash-Datei läuft darüber und danach wird die SD-Datei wieder aufgeblendet. Bisher wartete sie, bis die SD-Datei zu Ende war. Auch beim Sprechen sinkt das Radio oder die Datei jetzt schon beim Aufruf und nicht erst, wenn der Text geladen ist; die Ansage ist dadurch früher zu hören.
- Dateisuche: Ein Dateiname ohne `sd:` (REST und MQTT) wird zuerst im Flash gesucht, dann auf der SD-Karte (Pfad ab Wurzel).

**English**

- Web radio: up to 30 instead of 15 stored slots (*Webradio* tab, REST `/api/station`, MQTT `…/station`, inputs). Already stored slots are kept; the MQTT buffer grew to 8 KB for the longer station list.
- Saving and loading the station list: in the *Webradio* tab *Liste speichern* (save list) and *Liste laden* (load list) store the list as a text file (`/stations.txt`, `POST /importstations`); loading replaces the whole list.
- An MP3 file from the flash called while a file of the SD card is playing starts right away: the SD file is lowered (like for an announcement), the flash file plays over it and afterwards the SD file is faded back up. Before, it waited until the SD file had ended. Speaking works the same way: the radio or file is lowered when the call comes in, not only when the text has been loaded, so the announcement is heard earlier.
- File lookup: a file name without `sd:` (REST and MQTT) is looked up in the flash first and then on the SD card (path from the root).

## 2026-10-30 - Version 1.3.6

**Deutsch**

- MQTT: bis zu 5 zusätzliche Abos (Gruppen-Topics) im Tab *MQTT*, Standard `netspeaker/all`, immer ein leeres Feld sichtbar. Das Gerät nimmt unter `<Topic>/#` dieselben Befehle an wie unter dem Basis-Topic (`say`, `play`, `url`, `file`, `station`, `list`, `stop`, `volume`, `restart`), so erreicht ein Befehl mehrere Geräte. Export und Import der Einstellungen enthalten die Felder (`mq_g1` bis `mq_g5`); per REST setzt `/api/mqtt` sie mit `group1` bis `group5` und liefert sie als `groups`. Doku-Seite, README und `en.json` sind ergänzt.
- Sprachdatei von GitHub laden: Das Eingabefeld ist mit `en.json` vorbelegt und breiter; ein angehängtes `.json` wird vor dem Laden entfernt.
- Das Repository-ZIP enthält die Firmware in `build_netspeaker/esp32s3/`; das Werkzeug-ZIP enthält den Lizenztext von `esptool`.

**English**

- MQTT: up to 5 additional subscriptions (group topics) in the *MQTT* tab, default `netspeaker/all`, one empty field always shown. Under `<topic>/#` the device accepts the same commands as under the base topic (`say`, `play`, `url`, `file`, `station`, `list`, `stop`, `volume`, `restart`), so one command reaches several devices. Export and import of the settings contain the fields (`mq_g1` to `mq_g5`); via REST `/api/mqtt` sets them with `group1` to `group5` and returns them as `groups`. Documentation page, README and `en.json` are updated.
- Loading a language file from GitHub: the input field is prefilled with `en.json` and wider; a trailing `.json` is removed before loading.
- The repository ZIP contains the firmware in `build_netspeaker/esp32s3/`; the tools ZIP contains the license text of `esptool`.

## 2026-10-30 - Version 1.3.5

**Deutsch**

- Das ZIP des Repositorys reicht zum Flashen: Fehlt `esptool.exe` (keine Arduino IDE), lädt `voicesetup.ps1` es aus `netspeaker_flash.zip` des neuesten Releases (SHA-256 geprüft, neu: `netspeaker_flash.sha256`) und legt es neben das Skript. Das Repository-ZIP enthält jetzt auch die Firmware in `build_netspeaker/esp32s3/` (ohne `merged.bin`, die liegt am Release). `netspeaker_flash.zip` enthält zusätzlich den Lizenztext von `esptool` (`esptool-LICENSE.txt`, GPL-2.0+); die README ist angepasst. Die Firmware ist unverändert.
- `build_netspeaker/cleanup_history.ps1` entfernt die Firmware nach einem neuen Build aus allen älteren Commits (nur der neueste Commit behält sie), damit das Repository nicht um ~2 MB je Version wächst.

**English**

- The ZIP of the repository is enough for flashing: if `esptool.exe` is missing (no Arduino IDE), `voicesetup.ps1` downloads it from `netspeaker_flash.zip` of the newest release (SHA-256 checked, new: `netspeaker_flash.sha256`) and puts it next to the script. The repository ZIP now also contains the firmware in `build_netspeaker/esp32s3/` (without `merged.bin`, which is attached to the release). `netspeaker_flash.zip` additionally contains the `esptool` license text (`esptool-LICENSE.txt`, GPL-2.0+); the README is adjusted. The firmware is unchanged.
- `build_netspeaker/cleanup_history.ps1` removes the firmware from all older commits after a new build (only the newest commit keeps it), so the repository does not grow by ~2 MB per version.

## 2026-10-30 - Version 1.3.4

**Deutsch**

- Flash-Werkzeuge wie bei der Uhr: `flashESP.bat` und `voicesetup.ps1` liegen jetzt im Ordner `build_netspeaker/` des Repositorys (nur die Werkzeuge, die Binärdateien bleiben ausgeschlossen). Fehlt ein lokaler Build, lädt `flashESP.bat` das Paket `netspeaker-esp32s3.zip` vom neuesten Release und prüft die SHA-256-Summe. Neu: `publish_builds.ps1` erzeugt die Pakete (`netspeaker-esp32s3.zip`, `.sha256` und `netspeaker_flash.zip` mit `esptool.exe`) und lädt sie mit `-Upload` ins Release. Neuer README-Abschnitt *Fertige Firmware flashen*. Die Firmware ist unverändert.

**English**

- Flash tools as with the clock: `flashESP.bat` and `voicesetup.ps1` now live in the folder `build_netspeaker/` of the repository (only the tools, the binaries stay excluded). Without a local build `flashESP.bat` downloads the package `netspeaker-esp32s3.zip` from the newest release and checks the SHA-256 sum. New: `publish_builds.ps1` creates the packages (`netspeaker-esp32s3.zip`, `.sha256` and `netspeaker_flash.zip` with `esptool.exe`) and uploads them to the release with `-Upload`. New README section *Flashing the finished firmware*. The firmware is unchanged.

## 2026-10-30 - Version 1.3.3

**Deutsch**

- `flashAll.bat` fragt nach dem Flashen eines Geräts am USB auch das WLAN ab (Netze zur Auswahl, WLAN des PCs mit gespeichertem Passwort zum Übernehmen), wie bisher nur `flashESP.bat`; mit `-Yes` entfällt die Frage. Die README beschreibt die WLAN-Einrichtung beim Flashen. Die Firmware ist unverändert.

**English**

- `flashAll.bat` now also asks for the WiFi after flashing a device on USB (networks to choose from, the WiFi of the PC with its stored password to take over), as only `flashESP.bat` did before; with `-Yes` the question is skipped. The README describes setting up the WiFi while flashing. The firmware is unchanged.

## 2026-10-30 - Version 1.3.2

**Deutsch**

- Tab *Sprechen*: Was gerade läuft (Sender oder Datei, mit Fortschrittsbalken und Stopp-Knopf) steht jetzt oben wie in den anderen Tabs.
- Tab *Zeit*: Bis zu 5 NTP-Server; es wird immer ein leeres Feld mit angezeigt. Das Gerät fragt die ersten drei belegten und wechselt nach 45 s ohne Zeit zu den weiteren. Export und Import enthalten `ntp1` bis `ntp5`.
- Die Tabs *SD-Karte* und *MP3 (Flash)* zeigen den belegten Speicher einheitlich oben unter der Überschrift: „Belegt X von Y (Z %)“ mit einem Balken.
- Sprachdateien lassen sich vom Projekt auf GitHub laden (Karte *Sprache / Language* im Tab *Status*, Knopf *Von GitHub laden*; `fetch=en` bei `/api/lang`). Die Adresse ist fest, nur der Sprachcode kommt von außen. Übersetzte HTML-Teile werden im Browser auf Hervorhebungen (`code`, `em`, `strong`, `b`, `kbd`) beschränkt.
- Screenshots der Weboberfläche im Ordner `docs/screenshots/` und in der README. Die englische Sprachdatei übersetzt jetzt auch die Zeilen der Stream-Info (Station, Title, Description, Sample rate, Channels …).

**English**

- *Sprechen* tab: what is playing right now (station or file, with progress bar and stop button) is now shown at the top like in the other tabs.
- *Zeit* tab: up to 5 NTP servers; one empty field is always shown. The device asks the first three used ones and switches to the others after 45 s without time. Export and import contain `ntp1` to `ntp5`.
- The *SD-Karte* and *MP3 (Flash)* tabs show the used memory in the same way at the top under the heading: "Belegt X von Y (Z %)" with a bar.
- Language files can be loaded from the project on GitHub (card *Sprache / Language* in the *Status* tab, button *Von GitHub laden*; `fetch=en` at `/api/lang`). The address is fixed, only the language code comes from outside. Translated HTML parts are limited in the browser to emphasis tags (`code`, `em`, `strong`, `b`, `kbd`).
- Screenshots of the web interface in the folder `docs/screenshots/` and in the README. The English language file now also translates the rows of the stream info (Station, Title, Description, Sample rate, Channels …).

## 2026-10-30 - Version 1.3.1

**Deutsch**

- Das Schreiben des Logs in die Datei schaltet sich nach einer einstellbaren Zeit selbst aus (Tab *Log*, Standard 60 Minuten, 0 = nie), damit der Flash geschont wird. Die Zeit läuft ab dem Einschalten oder ab dem Start des Geräts; im Log steht eine Zeile `[LOG] … switched off automatically`.
- MQTT: Im Tab *MQTT* lassen sich optional Benutzer und Passwort für den Broker eintragen; leer = ohne Anmeldung. Das Passwort wird nicht angezeigt, nicht exportiert und nie über die API ausgegeben; ein leeres Feld behält das gespeicherte. `/api/mqtt` nimmt optional `user` und `pass`.
- Tab *Eingänge*: „GPIO 47“ und der Zustand werden nicht mehr umgebrochen.
- GPIO 33-37 gelten als freie Pins (Tab *Audio*, Tab *Eingänge*), weil der ESP32-S3FH4R2 Quad-PSRAM hat und diese Pins nur bei Octal-PSRAM belegt sind. Welche Pins das Board herausführt, hängt vom Board ab.
- Tab *Eingänge*: Als Aktion lässt sich auch ein gespeicherter Webradio-Sender wählen (Auswahl der gespeicherten Sender, Name oder Platznummer); `type=station` in `/api/pins`.
- Tab *MQTT* kompakter: Broker und Port stehen in einer Zeile, Benutzer und Passwort in der nächsten.
- Doku ergänzt und berichtigt: Die Lautstärke geht überall von 0 bis 100 (statt 0-21); neu sind die Karten *Eingänge (Pins)* und *Sprachen*, die Felder von `/api/status` (Fortschritt, Stream-Info, `gainDb`) und die JSON-Formen von MQTT `/url` und `/file`.

**English**

- Writing the log to the file switches itself off after an adjustable time (*Log* tab, default 60 minutes, 0 = never) to spare the flash. The time runs from switching on or from the start of the device; the log gets a line `[LOG] … switched off automatically`.
- MQTT: in the *MQTT* tab a user and password for the broker can optionally be entered; empty = no login. The password is not shown, not exported and never output by the API; an empty field keeps the saved one. `/api/mqtt` optionally takes `user` and `pass`.
- *Eingänge* tab: "GPIO 47" and the state are no longer wrapped.
- GPIO 33-37 count as free pins (*Audio* tab, *Eingänge* tab), because the ESP32-S3FH4R2 has quad PSRAM and these pins are only taken with octal PSRAM. Which pins the board brings out depends on the board.
- *Eingänge* tab: a saved web radio station can now also be chosen as the action (list of the saved stations, name or slot number); `type=station` in `/api/pins`.
- *MQTT* tab more compact: broker and port are in one line, user and password in the next.
- Docs completed and corrected: the volume goes from 0 to 100 everywhere (instead of 0-21); new are the cards *Eingänge (Pins)* and *Sprachen*, the fields of `/api/status` (progress, stream info, `gainDb`) and the JSON forms of MQTT `/url` and `/file`.

## 2026-10-29 - Version 1.3.0

**Deutsch**

- Die Weboberfläche lässt sich in weiteren Sprachen anzeigen. Eine Sprachdatei (`lang/en.json` für Englisch) wird im Tab *Status* in der Karte *Sprache / Language* hochgeladen, liegt im Speicher des Geräts und wird im Browser angewendet; in der Statuszeile wechselt ein Knopf (EN/DE) die Sprache. Die Firmware wird dafür nur um wenige KB größer, die Übersetzung selbst braucht keinen Platz in der App-Partition. Anleitung und Prüfwerkzeug für fehlende Übersetzungen: `lang/README.md`.
- Nicht eingestellte Pins (alle zugelassenen außer Verstärker, SD-Karte, Strapping-Pins und Status-LED) bekommen einen Pull-up (High), damit offene Pins nicht schweben.
- Neuer Tab *Eingänge*: Je freiem Pin lässt sich eine Aktion festlegen; bei einer MP3-Datei bietet eine Auswahl die Dateien im Flash und auf der SD-Karte an. Bekommt der Pin Kontakt nach GND (zum Beispiel durch eine Taste), spielt das Gerät eine Ansage (VoiceRSS) oder eine MP3-Datei, einen Ordner der SD-Karte oder eine URL. REST: `/api/pins`.
- Der Fortschrittsbalken unter dem Namen wird bei Webradio ausgeblendet, weil ein Stream keine Länge hat.
- Im Tab *Sprechen* steht neben dem Lautstärkeregler der Wert (zum Beispiel 60 / 100); er folgt dem Regler und der tatsächlichen Lautstärke.
- Tab *Status*: Die SD-Karte zeigt jetzt Zustand, Größe, belegt (mit Prozent), frei, ob sie eingeschaltet ist und die vier Pins (CS, SCK, MOSI, MISO).

**English**

- The web interface can be shown in further languages. A language file (`lang/en.json` for English) is uploaded in the *Status* tab, in the card *Sprache / Language*, is stored in the memory of the device and applied in the browser; a button (EN/DE) in the status bar switches the language. The firmware only grows by a few KB, the translation itself needs no space in the app partition. Instructions and a tool for finding missing translations: `lang/README.md`.
- Pins that are not configured (all allowed ones except amplifier, SD card, strapping pins and status LED) get a pull-up (high) so that open pins do not float.
- New tab *Eingänge*: an action can be set per free pin; for an MP3 file a list offers the files in flash and on the SD card. When the pin gets contact to GND (for example through a button), the device plays an announcement (VoiceRSS) or an MP3 file, a folder of the SD card or a URL. REST: `/api/pins`.
- The progress bar under the name is hidden for web radio, because a stream has no length.
- In the *Sprechen* tab the value is shown next to the volume slider (for example 60 / 100); it follows the slider and the actual volume.
- *Status* tab: the SD card now shows state, size, used (with percent), free, whether it is enabled and the four pins (CS, SCK, MOSI, MISO).

## 2026-10-29 - Version 1.2.1

**Deutsch**

- Der Titel eines Webradios wird erst angezeigt (und für die Titelregel gemeldet), wenn die dazu gehörende Musik zu hören ist: Er wird um die Dauer des gepufferten Tons verzögert. Im Log steht bei jedem neuen Titel die Verzögerung (`[TITLE] ... shown in ... ms`).
- Titelregel: Mit *höchstens … s* im Tab *Audio* endet die angehobene Lautstärke nach einer festen Zeit (zum Beispiel 210 s), weil ein Sender das Ende eines Titels nicht meldet und Werbung danach sonst laut bleibt. 0 = bis zum nächsten Titel (Standard).
- Der Tab *MP3* heißt jetzt *MP3 (Flash)*, zur Abgrenzung von der SD-Karte. In der Statuszeile steht als Marke jetzt *SPEAKER* statt *VOICE*.
- *Stream-Info* zeigt zusätzlich Codec, Abtastrate und Kanäle; sie stehen auch im Status als `codec`, `rate`, `ch` und `bits`.
- Kurzer Ton („buub“) beim Start in der eingestellten Lautstärke; nach einem Neustart per MQTT, nach eigenen Neustarts des Geräts (WLAN lange weg, Speicher knapp), nach einem Absturz und im abgesicherten Modus bleibt er aus. Haken *Kurzer Ton beim Start* im Tab *Audio* (Standard: an).

**English**

- The title of a web radio is only shown (and passed to the title rule) when the matching music is audible: it is delayed by the duration of the buffered sound. The log shows the delay for every new title (`[TITLE] ... shown in ... ms`).
- Title rule: with *höchstens … s* in the *Audio* tab the raised volume ends after a fixed time (for example 210 s), because a station does not report the end of a title and ads after it would otherwise stay loud. 0 = until the next title (default).
- The tab *MP3* is now called *MP3 (Flash)*, to tell it apart from the SD card. The brand in the status bar is now *SPEAKER* instead of *VOICE*.
- *Stream-Info* additionally shows codec, sample rate and channels; they are also in the status as `codec`, `rate`, `ch` and `bits`.
- Short tone ("buub") at the start at the set volume; it stays off after a restart via MQTT, after restarts the device triggers itself (WiFi lost for long, low memory), after a crash and in safe mode. Checkbox *Kurzer Ton beim Start* in the *Audio* tab (default: on).

## 2026-10-29 - Version 1.2.0

**Deutsch**

- Eine Ansage stoppt eine laufende MP3-Datei (auch aus einem Ordner der SD-Karte) nicht mehr, sondern wird leise darübergemischt; die Datei läuft abgesenkt weiter. Voraussetzung ist der Haken *Radio unter Ansagen weiterlaufen lassen* im Tab *Audio*. Mehrere Ansagen laufen nacheinander und kommen vor den wartenden Dateien dran.
- Bei laufender MP3-Datei zeigt die Oberfläche unter dem Namen einen Fortschrittsbalken (Position und Länge in der Statusabfrage: `pos`, `dur` in Sekunden). Zufall und Ordner-Knopf gibt es nun auch im Hauptverzeichnis der SD-Karte.
- MP3-Dateien werden beim Start ein- und am Ende ausgeblendet (Dauer: *Radio aus-/einblenden*); Ansagen werden nie ein- oder ausgeblendet.
- MP3-Dateien werden in der Lautstärke angeglichen: Beim ersten Abspielen misst das Gerät im Hintergrund die Lautheit der Datei und speichert den Ausgleich (`/loudness.bin`); Ansagen und Radio bleiben unverändert. Haken *MP3-Lautstärke angleichen* im Tab *Audio* (Standard: an).
- Tab *Webradio*: aufklappbarer Bereich *Stream-Info* mit Sender, Titel, Genre, Beschreibung, Webseite, Logo-Adresse, Bitrate und Server, soweit der Sender sie liefert; die Werte stehen auch im Status.

**English**

- An announcement no longer stops a running MP3 file (including one from a folder of the SD card): it is mixed quietly over it and the file keeps playing, lowered. This needs the checkbox *Radio unter Ansagen weiterlaufen lassen* in the *Audio* tab. Several announcements play one after another and come before the waiting files.
- While an MP3 file plays, the web interface shows a progress bar under the name (position and length in the status query: `pos`, `dur` in seconds). Random and the folder button are now also available in the root of the SD card.
- MP3 files fade in at the start and out at the end (duration: *Radio aus-/einblenden*); announcements are never faded in or out.
- The volume of MP3 files is matched: on the first play the device measures the loudness of the file in the background and stores the correction (`/loudness.bin`); announcements and radio stay unchanged. Checkbox *MP3-Lautstärke angleichen* in the *Audio* tab (default: on).
- *Webradio* tab: collapsible *Stream-Info* area with station, title, genre, description, website, logo address, bitrate and server as far as the station delivers them; the values are also in the status.

## 2026-10-29 - Version 1.1.0

**Deutsch**

- Ordner der SD-Karte spielen mit `shuffle=1` in zufälliger Reihenfolge: jede Datei einmal je Sitzung, die Reihenfolge ist jedes Mal eine andere und beginnt nie mit der Datei der letzten Sitzung dieses Ordners. `loop=1` (Häkchen *Endlos*) startet danach eine neue, neu gemischte Runde ohne die zuletzt gespielte Datei am Anfang. Knopf *Zufall* im Tab *SD-Karte*, Parameter in REST und MQTT (JSON).
- Ordner werden jetzt einzeln nachgeladen statt auf einmal in die Warteschlange gelegt: es gibt keine Grenze von 8 Dateien mehr.
- Ansagen und Dateien werden nie mehr ein- oder ausgeblendet, auch nicht ohne Radio; nur das Radio blendet weiterhin aus, ein und wird abgesenkt.

**English**

- Folders of the SD card play in random order with `shuffle=1`: every file once per session, the order is different every time and never starts with the file the last session of this folder started with. `loop=1` (checkbox *Endlos*) then starts a new, reshuffled round without the file played last at the start. Button *Zufall* in the *SD-Karte* tab, parameters in REST and MQTT (JSON).
- Folders are now fed in one file at a time instead of all at once into the queue: the limit of 8 files is gone.
- Announcements and files are never faded in or out any more, not even without radio; only the radio still fades out, fades in and is ducked.

## 2026-10-28 - Version 1.0.0

**Deutsch**

- Erste Veröffentlichung als **NetSpeaker**: Netzwerk-Lautsprecher für den ESP32-S3 (4 MB Flash, 2 MB PSRAM) mit MAX98357A.
- Ansagen per VoiceRSS, Webradio, MP3-Dateien im Gerät und von der optionalen SD-Karte (mit Dateimanager; noch nicht produktiv getestet).
- Ansagen und Dateien laufen in einer Warteschlange; mit Webradio werden sie über das abgesenkte Radio gemischt, ohne Radio ein- und ausgeblendet.
- Lautstärke 0-100 mit sanften Änderungen, Titelregel mit mehreren Stichworten, Autostart und Wiederherstellen des letzten Zustands.
- Weboberfläche (Tabs Sprechen, Webradio, SD-Karte, MP3, Audio, MQTT, WLAN, Zeit, Status, Log, Doku) mit ``-Hinweisen und Verkabelungsplan, HTTP-API und MQTT.
- Export und Import der Einstellungen ohne WLAN-Passwörter und ohne API-Schlüssel; die Exportdatei trägt den Hostnamen im Namen.
- Platz 1 der Senderliste wird mit *RSA* vorbelegt, solange er leer ist.
- Lizenz GPL-3.0, Drittkomponenten in der README.

**English**

- First release as **NetSpeaker**: network speaker for the ESP32-S3 (4 MB flash, 2 MB PSRAM) with MAX98357A.
- Announcements via VoiceRSS, web radio, MP3 files in the device and from the optional SD card (with file manager; not tested in production yet).
- Announcements and files run in a queue; with web radio they are mixed over the ducked radio, without radio they fade in and out.
- Volume 0-100 with smooth changes, title rule with several keywords, autostart and restoring the last state.
- Web interface (tabs Sprechen, Webradio, SD-Karte, MP3, Audio, MQTT, WLAN, Zeit, Status, Log, Doku) with `` hints and wiring diagram, HTTP API and MQTT.
- Export and import of the settings without WiFi passwords and without API key; the export file carries the hostname in its name.
- Slot 1 of the station list is preset with *RSA* as long as it is empty.
- License GPL-3.0, third-party components in the README.
// NetSpeaker: Texte per VoiceRSS ansagen und MP3 abspielen (ESP32-S3 + MAX98357A I2S-Verstaerker).
// NetSpeaker: speak texts via VoiceRSS and play MP3 (ESP32-S3 + MAX98357A I2S amplifier).
//
// Board: ESP32-S3 mit 4 MB Flash und 2 MB PSRAM. Flash 4MB, Partitionsschema "No OTA (2MB APP/2MB SPIFFS)",
// PSRAM (QSPI) und USB CDC On Boot aktivieren. Es gibt keine OTA-Partition, Updates nur per USB.
// Board: ESP32-S3 with 4 MB flash and 2 MB PSRAM. Flash 4MB, partition scheme "No OTA (2MB APP/2MB SPIFFS)",
// enable PSRAM (QSPI) and USB CDC On Boot. There is no OTA partition, updates only via USB.
//
// Bibliotheken: "ESP32-audioI2S" von schreibfaul1 (Version 4.0.0; audio->loop() muss regelmaessig aus loop() laufen)
// und "PubSubClient" von Nick O'Leary.
// Libraries: "ESP32-audioI2S" by schreibfaul1 (version 4.0.0; audio->loop() must run regularly from loop())
// and "PubSubClient" by Nick O'Leary.
//
// Verdrahtung / wiring: BCLK -> GPIO3, LRC -> GPIO4, DIN -> GPIO2 (aenderbar / changeable), VIN -> 5 V, GND -> GND.

#include <WiFi.h>
#include <WiFiUdp.h>
#include <WiFiClientSecure.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Preferences.h>
#include <LittleFS.h>
#include <SPI.h>
#include <SD.h>
#include <algorithm>
#include <vector>
#include <HTTPClient.h>
#include <PubSubClient.h>
#include <Audio.h>
#include <atomic>
#include "mp3_decoder/mp3_decoder.h"
#include <esp_task_wdt.h>
#include <driver/i2s_std.h>
#include <time.h>

#include "config.h"

// ---------------------------------------------------------------------------
// Einstellungen / settings

struct Settings {
    String host;
    String ssid[3];
    String pass[3];
    String ntp[5];
    String tz;
    String vkey;
    String vlang;
    String vvoice;
    String vfmt;
    String mqttServer;
    String mqttUser;
    String mqttPass;
    String mqttBase;
    String mqttGroup[5];
    int vrate;
    int vol;
    int bclk;
    int lrc;
    int din;
    int mqttPort;
    bool logEnabled;
    int logAutoMin;
    String radioName[RADIO_SLOTS];
    String radioUrl[RADIO_SLOTS];
    int radioVol[RADIO_SLOTS];
    int fadeMs;
    int switchFadeMs;
    bool autoStart;
    bool bootTone;
    bool mixAnn;
    bool loudNorm;
    int duckPct;
    int volRampMs;
    bool resumeLast;
    String lastUrl;
    int lastVol;
    bool sdOn;
    int sdCs;
    int sdSck;
    int sdMosi;
    int sdMiso;
    String titleMatch;
    int titleVol;
    int titleMaxS;
};

Settings cfg;
Preferences prefs;

// ---------------------------------------------------------------------------
// Laufzeitzustand / runtime state

enum PlayState { PS_IDLE, PS_LOADING, PS_PLAYING };
// Eine Mischspur: Ringpuffer, Zeiger und Zustand (siehe Mischer weiter unten); Spur 0 = Datei der SD-Karte ueber dem Radio, Spur 1 = Ansage.
// One mix track: ring buffer, pointers and state (see mixer further down); track 0 = file of the SD card over the radio, track 1 = announcement.

#define MIX_LAYER 0
#define MIX_ANN 1

struct MixTrack {
    int16_t* ring = nullptr;
    std::atomic<uint32_t> wr{0};
    std::atomic<uint32_t> rd{0};
    volatile bool active = false;
    volatile bool inUse = false;
    volatile bool producerDone = false;
    volatile bool abortFlag = false;
    volatile bool taskRunning = false;
    volatile bool drained = false;
    volatile bool inHook = false;
    volatile uint32_t srcRate = 0;
    volatile uint32_t outRate = 44100;
    volatile uint32_t phase = 0;
    volatile int32_t gainQ15 = 0;
    int32_t baseGainQ15 = 0;
    String path;
    bool onSd = false;
    bool pending = false;
    unsigned long activateAt = 0;
    uint32_t lastRd = 0;
    unsigned long lastProgress = 0;
};

enum PendingType { PEND_NONE, PEND_SAY, PEND_URL, PEND_FILE, PEND_STOP };

struct Pending {
    PendingType type = PEND_NONE;
    String text;
    String lang;
    String voice;
    int rate = 99;
    int vol = -1;
    bool resume = false;
    bool force = false;
};

Pending pending;

// Eintrag einer Ordnerliste der SD-Karte und aufgeloester Dateiname; die Typen stehen oben, weil die Arduino-IDE Funktionsprototypen vor die Funktionen setzt.
// Entry of a folder list of the SD card and a resolved file name; the types are up here because the Arduino IDE puts function prototypes before the functions.

struct SdEntry {
    String name;
    bool dir;
    uint32_t size;
};

struct FileRef {
    bool ok = false;
    bool sd = false;
    String path;
    String shown;
};

// Warteschlange fuer Ansagen und MP3-Dateien: sie werden nacheinander abgespielt, Stopp leert sie.
// Queue for announcements and MP3 files: they are played one after another, stop empties it.

Pending annQueue[ANN_QUEUE_LEN];
int annCount = 0;

bool annPush(const Pending& p) {
    if (annCount >= ANN_QUEUE_LEN) return false;
    annQueue[annCount++] = p;
    return true;
}

bool annTake(int idx, Pending& out) {
    if (idx < 0 || idx >= annCount) return false;
    out = annQueue[idx];
    for (int i = idx + 1; i < annCount; i++) annQueue[i - 1] = annQueue[i];
    annCount--;
    annQueue[annCount] = Pending();
    return true;
}

bool annPop(Pending& out) {
    if (annCount == 0) return false;
    out = annQueue[0];
    for (int i = 1; i < annCount; i++) annQueue[i - 1] = annQueue[i];
    annCount--;
    annQueue[annCount] = Pending();
    return true;
}

// Ordner-Sitzung: die MP3-Dateien eines Ordners der SD-Karte werden nacheinander in die Warteschlange gegeben, sobald sie leer ist. Mit shuffle
// ist die Reihenfolge je Sitzung zufaellig und jede Datei kommt einmal vor; mit loop beginnt danach eine neue Runde.
// Folder session: the MP3 files of a folder on the SD card are put into the queue one after another as soon as it is empty. With shuffle the
// order is random per session and every file appears once; with loop a new round starts afterwards.

struct DirSession {
    bool active = false;
    bool shuffle = false;
    bool loop = false;
    int vol = -1;
    String dir;
    std::vector<String> names;
    size_t pos = 0;
    String lastPlayed;
    unsigned long lastFeed = 0;
};

DirSession dirSess;

void annClear() {
    for (int i = 0; i < ANN_QUEUE_LEN; i++) annQueue[i] = Pending();
    annCount = 0;
    dirSess.active = false;
    dirSess.names.clear();
}
volatile PlayState playState = PS_IDLE;
String lastError;
unsigned long playStartMillis = 0;
bool voiceBusy = false;
bool voiceEver = false;
bool voiceLastOk = false;

bool fsReady = false;
bool apActive = false;
unsigned long lastMqttTry = 0;

// Verbindungsueberwachung: Zaehler und Zeitpunkte fuer WLAN und MQTT.
// Connection supervision: counters and timestamps for WiFi and MQTT.

volatile bool gotIpFlag = false;
volatile uint8_t lastDisconnectReason = 0;
unsigned long wifiLostAt = 0;
unsigned long wifiNextTry = 0;
unsigned long wifiBackoff = WIFI_BACKOFF_START_MS;
unsigned long wifiTryStart = 0;
unsigned long wifiReconnects = 0;
int wifiTryIdx = -1;
unsigned long mqttBackoff = MQTT_BACKOFF_START_MS;
unsigned long mqttReconnects = 0;
bool mqttWasConnected = false;
IPAddress mqttIp;
bool mqttIpValid = false;
String mqttIpFor;
unsigned long loadStartMillis = 0;

// Webradio, das laufen soll: wird nach einem Abriss automatisch neu verbunden (nur Live-Streams, nicht nach Stopp).
// Web radio that should be running: reconnected automatically after a drop (live streams only, not after a stop).

String wantRadioUrl;
int wantRadioVol = -1;
bool wantRadioLive = true;
bool streamWaitingReady = false;
unsigned long streamWaitSince = 0;
uint32_t streamLastFill = 0;
int streamStalls = 0;
unsigned long radioRetryAt = 0;
unsigned long radioRetryDelay = RADIO_RETRY_START_MS;

// Selbstschutz.
// Self protection.

bool safeMode = false;
int bootCrashes = 0;
bool bootMarkedStable = false;
unsigned long heapLowSince = 0;
bool mqttEverConnected = false;
String lastPublishedState;

SemaphoreHandle_t logMutex = nullptr;
WebServer webserver(80);
Audio* audio = nullptr;
WiFiClient mqttNet;
PubSubClient mqttClient(mqttNet);

// ---------------------------------------------------------------------------
// Log / log

// Die serielle Ausgabe laeuft immer; in die Logdatei wird nur geschrieben, wenn das Log eingeschaltet ist (Standard: aus).
// The serial output always runs; the log file is written only when logging is switched on (default: off).

void logLine(const String& msg) {
    char stamp[40];
    time_t now = time(nullptr);

    // Ohne gueltige Zeit (vor dem ersten NTP-Abgleich) zaehlt die Zeit seit dem Start.
    // Without a valid time (before the first NTP sync) the time since boot is used.

    if (now > 1700000000L) {
        struct tm timeinfo;
        localtime_r(&now, &timeinfo);
        strftime(stamp, sizeof(stamp), "[%Y-%m-%d %H:%M:%S", &timeinfo);
        snprintf(stamp + strlen(stamp), sizeof(stamp) - strlen(stamp), ".%03lu] ", millis() % 1000);
    } else {
        snprintf(stamp, sizeof(stamp), "[%lu ms] ", millis());
    }
    String line = String(stamp) + msg;
    Serial.println(line);
    if (!cfg.logEnabled || !fsReady || !logMutex) return;
    if (xSemaphoreTake(logMutex, pdMS_TO_TICKS(200)) != pdTRUE) return;
    File f = LittleFS.open(LOG_FILE, FILE_APPEND);
    if (f) {
        if (f.size() > LOG_MAX_BYTES) {
            f.close();
            LittleFS.remove(LOG_OLD_FILE);
            LittleFS.rename(LOG_FILE, LOG_OLD_FILE);
            f = LittleFS.open(LOG_FILE, FILE_WRITE);
        }
        if (f) {
            f.println(line);
            f.close();
        }
    }
    xSemaphoreGive(logMutex);
}

void clearLogFiles() {
    if (!fsReady || !logMutex) return;
    if (xSemaphoreTake(logMutex, pdMS_TO_TICKS(500)) != pdTRUE) return;
    LittleFS.remove(LOG_FILE);
    LittleFS.remove(LOG_OLD_FILE);
    xSemaphoreGive(logMutex);
}
// ---------------------------------------------------------------------------
// Hilfsfunktionen / helpers

String defaultHost() {
    uint64_t mac = ESP.getEfuseMac();
    char buf[8];
    snprintf(buf, sizeof(buf), "%02x%02x%02x", (unsigned)((mac >> 24) & 0xff), (unsigned)((mac >> 32) & 0xff), (unsigned)((mac >> 40) & 0xff));
    return String(PROJECT_NAME) + "-" + buf;
}

// Hostname bereinigen: Kleinbuchstaben, nur a-z 0-9 und Bindestrich, hoechstens 31 Zeichen, leer = Standardname.
// Clean the hostname: lower case, only a-z 0-9 and hyphen, at most 31 characters, empty = default name.

String cleanHost(String s) {
    s.toLowerCase();
    String out;
    for (size_t i = 0; i < s.length() && out.length() < 31; i++) {
        char c = s[i];
        if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '-') out += c;
    }
    while (out.startsWith("-")) out.remove(0, 1);
    while (out.endsWith("-")) out.remove(out.length() - 1);
    if (out.isEmpty()) return defaultHost();
    return out;
}

String urlEncode(const String& s) {
    String out;
    char buf[4];
    for (size_t i = 0; i < s.length(); i++) {
        uint8_t c = (uint8_t)s[i];
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '-' || c == '_' || c == '.' || c == '~') {
            out += (char)c;
        } else {
            snprintf(buf, sizeof(buf), "%%%02X", c);
            out += buf;
        }
    }
    return out;
}

String jsonEscape(const String& s) {
    String out;
    for (size_t i = 0; i < s.length(); i++) {
        char c = s[i];
        if (c == '"' || c == '\\') {
            out += '\\';
            out += c;
        } else if (c == '\n') {
            out += "\\n";
        } else if ((uint8_t)c >= 0x20) {
            out += c;
        }
    }
    return out;
}

bool timeIsValid() {
    return time(nullptr) > 1700000000L;
}

// Minimaler JSON-Leser fuer flache Objekte: liefert den Wert zu key (Text oder Zahl), leer wenn der Schluessel fehlt.
// Minimal JSON reader for flat objects: returns the value for key (text or number), empty if the key is missing.

String jsonGet(const String& body, const String& key) {
    int k = body.indexOf("\"" + key + "\"");
    if (k < 0) return "";
    int colon = body.indexOf(':', k + key.length() + 2);
    if (colon < 0) return "";
    int p = colon + 1;
    while (p < (int)body.length() && (body[p] == ' ' || body[p] == '\t' || body[p] == '\n' || body[p] == '\r')) p++;
    String out;
    if (p < (int)body.length() && body[p] == '"') {
        for (p++; p < (int)body.length() && body[p] != '"'; p++) {
            if (body[p] == '\\' && p + 1 < (int)body.length()) {
                p++;
                out += body[p] == 'n' ? '\n' : body[p];
            } else {
                out += body[p];
            }
        }
    } else {
        for (; p < (int)body.length() && body[p] != ',' && body[p] != '}'; p++) out += body[p];
        out.trim();
    }
    return out;
}
// GPIO 33-37 sind nur frei, wenn kein Octal-PSRAM verbaut ist (der ESP32-S3FH4R2 hat Quad-PSRAM); 26-32 gehoeren dem Flash und PSRAM.
// GPIO 33-37 are only free if no octal PSRAM is fitted (the ESP32-S3FH4R2 has quad PSRAM); 26-32 belong to flash and PSRAM.

bool pinAllowed(int pin) {
#if !defined(CONFIG_SPIRAM_MODE_OCT)
    if (pin >= 33 && pin <= 37) return true;
#endif
    return (pin >= 1 && pin <= 18) || pin == 21 || (pin >= 38 && pin <= 42) || pin == 47 || pin == 48;
}

// Alle zugelassenen Pins, die nicht eingestellt sind (Verstaerker, SD-Karte), keine Strapping-Pins sind und nicht die Status-LED tragen,
// bekommen einen Pull-up (High), damit offene Pins nicht schweben.
// All allowed pins that are not configured (amplifier, SD card), are no strapping pins and do not carry the status LED get a pull-up (high)
// so that open pins do not float.

bool pinIdle(int pin) {
    if (!pinAllowed(pin) || pin == 0 || pin == 3 || pin == 45 || pin == 46) return false;
    int used[] = {cfg.din, cfg.bclk, cfg.lrc, cfg.sdCs, cfg.sdSck, cfg.sdMosi, cfg.sdMiso, -1, -1};
    int n = 7;
#if defined(RGB_BUILTIN)
    int led = RGB_BUILTIN;
    if (led >= SOC_GPIO_PIN_COUNT) led -= SOC_GPIO_PIN_COUNT;
    used[n++] = led;
#endif
#if defined(LED_BUILTIN)
    used[n++] = LED_BUILTIN;
#endif
    for (int i = 0; i < n; i++) {
        if (used[i] == pin) return false;
    }
    return true;
}

void idlePins() {
    for (int pin = 1; pin <= 48; pin++) {
        if (pinIdle(pin)) pinMode(pin, INPUT_PULLUP);
    }
}

// Eingaenge: Ein freier Pin (Pull-up) loest eine Aktion aus, sobald er Kontakt nach GND bekommt (zum Beispiel durch eine Taste).
// Die Aktion ist eine Ansage (VoiceRSS) oder eine MP3-Datei bzw. URL; sie kommt wie ein Aufruf per REST in die Warteschlange.
// Inputs: A free pin (pull-up) triggers an action as soon as it gets contact to GND (for example through a button).
// The action is an announcement (VoiceRSS) or an MP3 file or URL; it enters the queue like a call via REST.

String pinType[49];
String pinText[49];
int pinVol[49];

void loadPinActions() {
    for (int pin = 1; pin <= 48; pin++) {
        if (!pinAllowed(pin)) continue;
        pinType[pin] = prefs.getString(("pt" + String(pin)).c_str(), "");
        pinText[pin] = prefs.getString(("px" + String(pin)).c_str(), "");
        pinVol[pin] = prefs.getInt(("pv" + String(pin)).c_str(), -1);
    }
}

String commandSay(const String& textIn, const String& lang, const String& voice, int rate, int vol);
String commandPlayFile(const String& nameIn, int vol, bool shuffle, bool loop);
String commandPlayUrl(const String& urlIn, int vol);
String commandPlayStation(const String& selIn, int vol);

void pinWatch() {
    static unsigned long lastScan = 0, lowSince[49], lastFire = 0;
    static bool armed[49];
    if (millis() - lastScan < 10) return;
    lastScan = millis();
    for (int pin = 1; pin <= 48; pin++) {
        if (pinType[pin].isEmpty() || !pinIdle(pin)) continue;
        if (digitalRead(pin) != LOW) {
            lowSince[pin] = 0;
            armed[pin] = true;
            continue;
        }
        if (!armed[pin]) continue;
        if (lowSince[pin] == 0) {
            lowSince[pin] = millis();
            continue;
        }
        if (millis() - lowSince[pin] < 40) continue;
        armed[pin] = false;
        if (millis() - lastFire < 300) continue;
        lastFire = millis();
        String err;
        const String& x = pinText[pin];
        if (pinType[pin] == "say") err = commandSay(x, "", "", 99, pinVol[pin]);
        else if (pinType[pin] == "station") err = commandPlayStation(x, pinVol[pin]);
        else if (x.startsWith("http://") || x.startsWith("https://")) err = commandPlayUrl(x, pinVol[pin]);
        else err = commandPlayFile(x, pinVol[pin], false, false);
        logLine("[PIN] GPIO " + String(pin) + " to GND -> " + pinType[pin] + " \"" + x + "\"" + (err.length() > 0 ? ": " + err : String()));
    }
}

String mqttBaseTopic() {
    if (cfg.mqttBase.length() > 0) return cfg.mqttBase;
    return String(PROJECT_NAME) + "/" + cfg.host;
}

// Gruppen-Topic i (0-4) ohne abschliessendes "/" oder "#"; leer, wenn nicht gesetzt, ungueltig (+, #) oder gleich dem Basis-Topic.
// Group topic i (0-4) without a trailing "/" or "#"; empty if unset, invalid (+, #) or equal to the base topic.

String mqttGroupTopic(int i) {
    String g = cfg.mqttGroup[i];
    g.trim();
    while (g.endsWith("#") || g.endsWith("/")) g.remove(g.length() - 1);
    if (g.indexOf('+') >= 0 || g.indexOf('#') >= 0 || g == mqttBaseTopic()) return String();
    return g;
}

// ---------------------------------------------------------------------------
// Einstellungen laden und speichern / load and save settings

void loadSettings() {
    prefs.begin(PROJECT_NVS, false);

    // Frueher lag die Lautstaerke auf der Skala 0 bis 21; gespeicherte Werte einmalig auf 0 bis VOLUME_MAX umrechnen.
    // The volume used to be on the scale 0 to 21; convert stored values once to 0 to VOLUME_MAX.

    int storedScale = prefs.getInt("vscale", 21);
    if (storedScale != VOLUME_MAX && storedScale > 0) {
        const char* volKeys[] = {"vol", "tvol"};
        for (int k = 0; k < 2; k++) {
            if (prefs.isKey(volKeys[k])) prefs.putInt(volKeys[k], (prefs.getInt(volKeys[k], 0) * VOLUME_MAX + storedScale / 2) / storedScale);
        }
        for (int i = 1; i <= RADIO_SLOTS; i++) {
            String key = "rv" + String(i);
            int old = prefs.getInt(key.c_str(), -1);
            if (old >= 0) prefs.putInt(key.c_str(), (old * VOLUME_MAX + storedScale / 2) / storedScale);
        }
        prefs.putInt("vscale", VOLUME_MAX);
    }

    // Gespeicherten Hostnamen mit ungueltigen Zeichen einmalig korrigieren und zurueckschreiben.
    // Correct a stored hostname with invalid characters once and write it back.

    String stored = prefs.getString("host", "");
    if (stored.length() > 0) {
        String cleaned = cleanHost(stored);
        if (cleaned != stored) {
            prefs.putString("host", cleaned);
            logLine("[Hostname] invalid characters corrected: \"" + stored + "\" -> \"" + cleaned + "\"");
            stored = cleaned;
        }
    }
    cfg.host = stored.length() > 0 ? stored : defaultHost();
    for (int i = 0; i < 3; i++) {
        cfg.ssid[i] = prefs.getString(("ssid" + String(i + 1)).c_str(), "");
        cfg.pass[i] = prefs.getString(("pass" + String(i + 1)).c_str(), "");
    }
    cfg.ntp[0] = prefs.getString("ntp1", "pool.ntp.org");
    cfg.ntp[1] = prefs.getString("ntp2", "time.google.com");
    for (int i = 2; i < 5; i++) cfg.ntp[i] = prefs.getString(("ntp" + String(i + 1)).c_str(), "");
    cfg.tz = prefs.getString("tz", "CET-1CEST,M3.5.0,M10.5.0/3");
    cfg.vkey = prefs.getString("vkey", "");
    cfg.vlang = prefs.getString("vlang", "de-de");
    cfg.vvoice = prefs.getString("vvoice", "");
    cfg.vfmt = prefs.getString("vfmt", "22khz_16bit_mono");
    cfg.vrate = prefs.getInt("vrate", 0);
    cfg.vol = constrain(prefs.getInt("vol", 57), 0, VOLUME_MAX);
    cfg.bclk = prefs.getInt("bclk", DEFAULT_PIN_BCLK);
    cfg.lrc = prefs.getInt("lrc", DEFAULT_PIN_LRC);
    cfg.din = prefs.getInt("din", DEFAULT_PIN_DIN);
    cfg.mqttServer = prefs.getString("mq_srv", "");
    cfg.mqttUser = prefs.getString("mq_usr", "");
    cfg.mqttPass = prefs.getString("mq_pw", "");
    cfg.mqttPort = prefs.getInt("mq_port", 1883);
    cfg.mqttBase = prefs.getString("mq_base", "");
    cfg.mqttGroup[0] = prefs.getString("mq_g1", String(PROJECT_NAME) + "/all");
    for (int i = 1; i < 5; i++) cfg.mqttGroup[i] = prefs.getString(("mq_g" + String(i + 1)).c_str(), "");
    cfg.logEnabled = prefs.getBool("log", false);
    cfg.logAutoMin = constrain(prefs.getInt("logmin", 60), 0, 1440);
    cfg.fadeMs = constrain(prefs.getInt("fade", FADE_DEFAULT_MS), 0, FADE_MAX_MS);
    cfg.switchFadeMs = constrain(prefs.getInt("swfade", SWITCH_FADE_DEFAULT_MS), 0, FADE_MAX_MS);
    cfg.autoStart = prefs.getBool("autostart", false);
    cfg.bootTone = prefs.getBool("btone", true);
    cfg.mixAnn = prefs.getBool("mix", true);
    cfg.loudNorm = prefs.getBool("loud", true);
    cfg.duckPct = constrain(prefs.getInt("duck", 35), 0, 100);
    cfg.volRampMs = constrain(prefs.getInt("vramp", VOLUME_RAMP_DEFAULT_MS), 0, VOLUME_RAMP_MAX_MS);
    cfg.resumeLast = prefs.getBool("rlast", false);
    cfg.lastUrl = prefs.getString("rsurl", "");
    cfg.lastVol = prefs.getInt("rsvol", -1);
    cfg.sdOn = prefs.getBool("sdon", true);
    cfg.sdCs = prefs.getInt("sdcs", DEFAULT_SD_CS);
    cfg.sdSck = prefs.getInt("sdsck", DEFAULT_SD_SCK);
    cfg.sdMosi = prefs.getInt("sdmosi", DEFAULT_SD_MOSI);
    cfg.sdMiso = prefs.getInt("sdmiso", DEFAULT_SD_MISO);
    cfg.titleMatch = prefs.getString("tmatch", "Cambodia");
    cfg.titleVol = constrain(prefs.getInt("tvol", 86), 0, VOLUME_MAX);
    cfg.titleMaxS = constrain(prefs.getInt("tmax", 0), 0, 3600);
    for (int i = 0; i < RADIO_SLOTS; i++) {
        cfg.radioName[i] = prefs.getString(("rn" + String(i + 1)).c_str(), "");
        cfg.radioUrl[i] = prefs.getString(("ru" + String(i + 1)).c_str(), "");
        cfg.radioVol[i] = prefs.getInt(("rv" + String(i + 1)).c_str(), -1);
    }

    // Ist Platz 1 leer, wird er mit dem Standardsender vorbelegt.
    // If slot 1 is empty, it is preset with the default station.

    if (cfg.radioUrl[0].isEmpty()) {
        cfg.radioName[0] = DEFAULT_STATION_NAME;
        cfg.radioUrl[0] = DEFAULT_STATION_URL;
    }
}

// ---------------------------------------------------------------------------
// Wiedergabe / playback

void publishStatus(bool force = false);
void onAudioEvent(Audio::msg_t m);
void mixStopAll();

// Die Audio-Bibliothek ruft den Rueckruf aus ihrem eigenen Task auf. Dort wird nur in eine Warteschlange geschrieben,
// die Hauptschleife arbeitet die Ereignisse ab; so greifen nie zwei Tasks auf Strings, MQTT oder den Zustand zu.
// The audio library calls the callback from its own task. There it only writes to a queue, the main loop handles the
// events; this way two tasks never touch strings, MQTT or the state at the same time.

#define AUDIO_EVT_QUEUE_LEN 40

struct AudioEvt {
    uint8_t type;
    char text[168];
};

QueueHandle_t audioEvtQueue = nullptr;

// Das Audio-Objekt belegt viel internen RAM und wird nur zum Abspielen angelegt; sonst fehlt der Platz fuer die TLS-Verbindung.
// The audio object uses a lot of internal RAM and exists only while playing; otherwise there is no room for the TLS connection.

int currentVolume = 57;
unsigned long volChangedAt = 0;
int lastPublishedVol = -1;
int lastPublishedGeneral = -1;

// MQTT-Neustart: Zeitpunkt des angeforderten Neustarts (0 = keiner) und Zeitpunkt des letzten Abonnierens.
// MQTT restart: time of the requested restart (0 = none) and time of the last subscribe.

unsigned long restartAt = 0;
unsigned long mqttSubscribedAt = 0;
const unsigned long MQTT_RESTART_IGNORE_MS = 3000;

// Laeuft ein Webradio und kommt eine Ansage oder Datei, merken wir uns Stream und Lautstaerke und setzen den Stream danach fort.
// If a web radio is playing and an announcement or file arrives, we remember stream and volume and resume the stream afterwards.

String streamUrl;
bool streamActive = false;

// Was gerade laeuft: nowKind ist "radio", "file", "speech" oder leer; nowName/nowTitle kommen aus den Stream-Metadaten.
// What is playing right now: nowKind is "radio", "file", "speech" or empty; nowName/nowTitle come from the stream metadata.

String nowKind;
String nowFile;
String nowName;
String nowTitle;
String nowGenre;
String nowDesc;
String nowWeb;
String nowLogo;
String nowHost;
uint32_t nowBitrate = 0;

// Titel des Senders kommen mit dem Datenstrom, die Musik dazu spielt erst, wenn der Eingangspuffer abgespielt ist. Darum wird ein neuer Titel
// um die Dauer des gepufferten Tons verzoegert angezeigt (und fuer die Titelregel gemeldet).
// Titles of the station come with the data stream, the music for them plays only when the input buffer has been played. So a new title is shown
// (and passed on to the title rule) delayed by the duration of the buffered sound.

#define TITLE_Q_LEN 4
struct TitlePend {
    String text;
    unsigned long due;
};
TitlePend titleQ[TITLE_Q_LEN];
int titleQn = 0;
String lastPublishedNow;
String resumeUrl;
int resumeVolume = 57;

// Einblenden nach der Fortsetzung: wartet auf "stream ready", steigt dann in fadeMs von 0 auf fadeInTarget.
// Fade-in after the resume: waits for "stream ready", then rises from 0 to fadeInTarget within fadeMs.

bool fadeInWaiting = false;
bool fadeInActive = false;
unsigned long fadeInStart = 0;
int fadeInTarget = 0;
int fadeInMs = 700;
// Sanftes Aendern der Radio-Lautstaerke (Ducking): Start, Ziel und Dauer; die Schritte macht rampStep() in der Hauptschleife.
// Gentle change of the radio volume (ducking): start, target and duration; rampStep() makes the steps in the main loop.

struct VolRamp {
    bool on = false;
    int from = 0;
    int to = 0;
    int now = 0;
    unsigned long t0 = 0;
    int ms = 0;
};
VolRamp volRamp;

// Der Radio-Pegel ist wegen einer laufenden Ansage abgesenkt.
// The radio level is lowered because of a running announcement.

bool mixDucked = false;
unsigned long mixDuckAt = 0;

void ensureAudio() {
    if (audio) return;
    audio = new Audio();
    Audio::audio_info_callback = onAudioEvent;
    audio->setPinout(pinAllowed(cfg.bclk) ? cfg.bclk : DEFAULT_PIN_BCLK, pinAllowed(cfg.lrc) ? cfg.lrc : DEFAULT_PIN_LRC, pinAllowed(cfg.din) ? cfg.din : DEFAULT_PIN_DIN);
    audio->setVolumeSteps(VOLUME_MAX * VOL_FINE);
    audio->setVolume(currentVolume * VOL_FINE);
    volRamp.on = false;
    volRamp.now = currentVolume * VOL_FINE;
    logLine("[AUDIO] created, free heap " + String(ESP.getFreeHeap()));
}

void releaseAudio() {
    mixStopAll();
    if (!audio) return;
    audio->stopSong();
    delete audio;
    audio = nullptr;
    if (audioEvtQueue) xQueueReset(audioEvtQueue);
    logLine("[AUDIO] released, free heap " + String(ESP.getFreeHeap()));
}

bool titleBoost = false;
unsigned long titleBoostSince = 0;
int titleSavedVol = 0;
int titleBoostVol = 0;

void applyVolume(int v);

// Beendet eine Lautstaerke-Anhebung durch die Titelregel.
// Ends a volume raise made by the title rule.

void titleBoostEnd() {
    if (!titleBoost) return;
    titleBoost = false;
    applyVolume(titleSavedVol);
}

void applyVolume(int v) {
    if (v < 0) v = 0;
    if (v > VOLUME_MAX) v = VOLUME_MAX;
    currentVolume = v;
    volChangedAt = millis();
    if (streamActive && nowKind == "radio" && !mixDucked) wantRadioVol = v;
    volRamp.on = false;
    volRamp.now = v * VOL_FINE;
    if (audio) audio->setVolume(v * VOL_FINE);
}

// Ziel in feinen Einheiten (Lautstaerke mal VOL_FINE).
// Target in fine units (volume times VOL_FINE).

void volRampTo(int to, int ms) {
    if (!audio) return;
    volRamp.from = volRamp.now;
    volRamp.now = volRamp.from;
    volRamp.to = to;
    volRamp.ms = ms;
    volRamp.t0 = millis();
    volRamp.on = true;
}

void volRampStep() {
    if (!volRamp.on) return;
    if (!audio) {
        volRamp.on = false;
        return;
    }
    unsigned long dt = millis() - volRamp.t0;
    int v = volRamp.to;
    if (volRamp.ms > 0 && dt < (unsigned long)volRamp.ms) v = volRamp.from + (int)((long)(volRamp.to - volRamp.from) * (long)dt / volRamp.ms);
    if (v != volRamp.now) {
        volRamp.now = v;
        audio->setVolume(v);
    }
    if (dt >= (unsigned long)volRamp.ms) volRamp.on = false;
}

// Aendert die Lautstaerke sanft: bei laufendem Ton wandert sie in ms (Standard cfg.volRampMs) zum neuen Wert, sonst gilt der Wert sofort.
// Waehrend einer Ansage bleibt das Radio entsprechend abgesenkt, beim Einblenden wird nur das Ziel geaendert.
// Changes the volume gently: while sound is playing it moves to the new value within ms (default cfg.volRampMs), otherwise the value applies at once.
// During an announcement the radio stays lowered accordingly, during a fade-in only the target changes.

void applyVolumeSmooth(int v, int ms = -1) {
    if (v < 0) v = 0;
    if (v > VOLUME_MAX) v = VOLUME_MAX;
    if (ms < 0) ms = cfg.volRampMs;
    if (ms > VOLUME_RAMP_MAX_MS) ms = VOLUME_RAMP_MAX_MS;
    if (fadeInWaiting || fadeInActive) {
        fadeInTarget = v;
        return;
    }
    if (!audio || playState != PS_PLAYING) {
        applyVolume(v);
        return;
    }
    currentVolume = v;
    volChangedAt = millis();
    if (streamActive && nowKind == "radio" && !mixDucked) wantRadioVol = v;
    volRampTo(mixDucked ? v * VOL_FINE * cfg.duckPct / 100 : v * VOL_FINE, ms);
}

// Anzeigename des Streams: gespeicherter Name des Platzes, sonst der Sendername aus den Metadaten, sonst der Host der URL.
// Display name of the stream: stored name of the slot, otherwise the station name from the metadata, otherwise the host of the URL.

String stationDisplayName() {
    for (int i = 0; i < RADIO_SLOTS; i++) {
        if (cfg.radioUrl[i].length() > 0 && cfg.radioUrl[i] == streamUrl && cfg.radioName[i].length() > 0) return cfg.radioName[i];
    }
    if (nowName.length() > 0) return nowName;
    String host = streamUrl;
    int p = host.indexOf("://");
    if (p >= 0) host = host.substring(p + 3);
    int slash = host.indexOf('/');
    if (slash >= 0) host = host.substring(0, slash);
    return host;
}

String nowText() {
    if (playState == PS_IDLE || nowKind.isEmpty()) return "";
    if (nowKind == "radio") return stationDisplayName() + (nowTitle.length() > 0 ? " - " + nowTitle : "");
    if (nowKind == "file") return nowFile;
    return "Ansage";
}

void queueResumeIfNeeded() {
    if (resumeUrl.isEmpty() || pending.type != PEND_NONE || annCount > 0) return;
    pending.type = PEND_URL;
    pending.text = resumeUrl;
    pending.vol = -1;
    pending.resume = true;
    pending.force = false;
    logLine("[PLAY] resuming stream " + resumeUrl);
}

// Im Leerlauf gilt wieder die allgemeine Lautstaerke, ausser eine Fortsetzung oder ein neuer Befehl wartet schon.
// When idle the general volume applies again, unless a resume or a new command is already waiting.

void settleVolume() {
    if (pending.type == PEND_NONE && resumeUrl.isEmpty() && annCount == 0 && currentVolume != cfg.vol) applyVolume(cfg.vol);
}

void setIdle() {
    mixStopAll();
    titleBoostEnd();
    streamWaitingReady = false;
    playState = PS_IDLE;
    nowKind = "";
    publishStatus();
    queueResumeIfNeeded();
    settleVolume();
}

void stopPlayback() {
    mixStopAll();
    titleBoostEnd();
    if (audio) audio->stopSong();
    if (audioEvtQueue) xQueueReset(audioEvtQueue);
    streamActive = false;
    streamWaitingReady = false;
    nowKind = "";
    playState = PS_IDLE;
}

void failPlayback(const String& msg) {
    lastError = msg;
    logLine("[PLAY] " + msg);
    nowKind = "";
    playState = PS_IDLE;
    publishStatus(true);
    queueResumeIfNeeded();
    settleVolume();
}

// SD-Karte (SPI): Dateien der Karte werden mit "sd:/Ordner/Datei.mp3" angesprochen, ein Name mit "/" am Ende bedeutet den ganzen Ordner.
// SD card (SPI): files of the card are addressed as "sd:/folder/file.mp3", a name ending in "/" means the whole folder.

SPIClass sdSpi(FSPI);
SemaphoreHandle_t sdMutex = nullptr;
bool sdReady = false;
String sdState = "off";

// Bindet die Karte (neu) ein; ohne Karte oder bei ausgeschaltetem SD bleibt sdReady falsch, alles andere laeuft weiter.
// Mounts the card (again); without a card or with SD switched off sdReady stays false, everything else keeps running.

void sdStopAll();

bool sdMount() {
    sdStopAll();
    if (sdMutex) xSemaphoreTake(sdMutex, pdMS_TO_TICKS(3000));
    if (sdReady) SD.end();
    sdReady = false;
    sdSpi.end();
    if (!cfg.sdOn) {
        sdState = "off";
    } else {
        sdSpi.begin(cfg.sdSck, cfg.sdMiso, cfg.sdMosi, cfg.sdCs);
        if (SD.begin(cfg.sdCs, sdSpi, 16000000) && SD.cardType() != CARD_NONE) {
            sdReady = true;
            const char* t = SD.cardType() == CARD_SD ? "SD" : (SD.cardType() == CARD_SDHC ? "SDHC" : "card");
            sdState = String(t);
            logLine("[SD] " + sdState + " mounted, " + String((unsigned long)(SD.cardSize() / 1048576ULL)) + " MB");
        } else {
            SD.end();
            sdSpi.end();
            sdState = "no card";
            logLine("[SD] no card or mount failed (CS " + String(cfg.sdCs) + ", SCK " + String(cfg.sdSck) + ", MOSI " + String(cfg.sdMosi) + ", MISO " + String(cfg.sdMiso) + ")");
        }
    }
    if (sdMutex) xSemaphoreGive(sdMutex);
    idlePins();
    return sdReady;
}

// Listet einen Ordner der Karte (nur Ordner und MP3-Dateien), Ordner zuerst, dann nach Name.
// Lists a folder of the card (only folders and MP3 files), folders first, then by name.

bool sdListDir(String dir, std::vector<SdEntry>& out) {
    if (!sdReady) return false;
    if (!dir.startsWith("/")) dir = "/" + dir;
    if (dir.length() > 1 && dir.endsWith("/")) dir.remove(dir.length() - 1);
    if (dir.indexOf("..") >= 0 || dir.length() > 160) return false;
    if (xSemaphoreTake(sdMutex, pdMS_TO_TICKS(2000)) != pdTRUE) return false;
    File d = SD.open(dir.c_str());
    bool ok = d && d.isDirectory();
    if (ok) {
        File e = d.openNextFile();
        while (e && (int)out.size() < SD_LIST_MAX) {
            String n = String(e.name());
            n = n.substring(n.lastIndexOf('/') + 1);
            bool isDir = e.isDirectory();
            bool show = !n.startsWith(".") && n != "System Volume Information";
            if (show && !isDir) {
                String l = n;
                l.toLowerCase();
                show = l.endsWith(".mp3");
            }
            if (show) out.push_back(SdEntry{n, isDir, (uint32_t)e.size()});
            e.close();
            esp_task_wdt_reset();
            e = d.openNextFile();
        }
        if (e) e.close();
    }
    if (d) d.close();
    xSemaphoreGive(sdMutex);
    std::sort(out.begin(), out.end(), [](const SdEntry& a, const SdEntry& b) {
        if (a.dir != b.dir) return a.dir;
        String x = a.name, y = b.name;
        x.toLowerCase();
        y.toLowerCase();
        return x < y;
    });
    return ok;
}

// Namen auf der Karte: Umlaute werden umschrieben, Leerzeichen werden zu Unterstrichen, sonst nur Buchstaben, Ziffern und . - _ ( ).
// Dateien muessen auf .mp3 enden (zu lange Namen behalten die Endung); leer = nicht erlaubt.
// Names on the card: umlauts are transliterated, spaces become underscores, otherwise only letters, digits and . - _ ( ).
// Files must end in .mp3 (names that are too long keep the extension); empty = not allowed.

String sdCleanName(const String& name, bool isFile) {
    String out;
    for (size_t i = 0; i < name.length() && out.length() < 120; i++) {
        uint8_t c = (uint8_t)name[i];
        if (c == 0xC3 && i + 1 < name.length()) {
            uint8_t d = (uint8_t)name[++i];
            if (d == 0xA4) out += "ae";
            else if (d == 0xB6) out += "oe";
            else if (d == 0xBC) out += "ue";
            else if (d == 0x84) out += "Ae";
            else if (d == 0x96) out += "Oe";
            else if (d == 0x9C) out += "Ue";
            else if (d == 0x9F) out += "ss";
            continue;
        }
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '.' || c == '-' || c == '_' || c == '(' || c == ')') out += (char)c;
        else if (c == ' ') out += '_';
    }
    while (out.startsWith(".")) out.remove(0, 1);
    if (isFile) {
        String l = out;
        l.toLowerCase();
        if (!l.endsWith(".mp3")) return "";
        if (out.length() > 100) out = out.substring(0, 96) + ".mp3";
    } else if (out.length() > 60) {
        out = out.substring(0, 60);
    }
    return out;
}

bool sdPathOk(const String& p) {
    return p.startsWith("/") && p.length() <= 160 && p.indexOf("..") < 0 && p.indexOf("//") < 0 && p.indexOf('\\') < 0;
}

// Ordner samt Inhalt loeschen; der Aufrufer haelt die Sperre.
// Delete a folder together with its content; the caller holds the lock.

bool sdRemoveTree(const String& path, int depth) {
    if (depth > 8) return false;
    File d = SD.open(path.c_str());
    if (!d) return false;
    if (!d.isDirectory()) {
        d.close();
        return SD.remove(path.c_str());
    }
    std::vector<String> names;
    File e = d.openNextFile();
    while (e) {
        String n = String(e.name());
        names.push_back(n.substring(n.lastIndexOf('/') + 1));
        e.close();
        esp_task_wdt_reset();
        e = d.openNextFile();
    }
    d.close();
    bool ok = true;
    for (size_t i = 0; i < names.size(); i++) {
        ok = sdRemoveTree(path + "/" + names[i], depth + 1) && ok;
        esp_task_wdt_reset();
    }
    return SD.rmdir(path.c_str()) && ok;
}

// Loest einen Dateinamen aus einem Befehl auf: "sd:/Pfad.mp3" liegt auf der Karte, alles andere im Ordner /mp3 des Geraets.
// Resolves a file name from a command: "sd:/path.mp3" is on the card, everything else in the /mp3 folder of the device.

FileRef resolveFile(String name) {
    FileRef r;
    name.trim();
    if (name.startsWith("sd:")) {
        r.sd = true;
        name.remove(0, 3);
        if (!name.startsWith("/")) name = "/" + name;
        r.path = name;
        r.shown = "sd:" + name;
        String l = name;
        l.toLowerCase();
        if (!sdReady || name.indexOf("..") >= 0 || name.length() > 160 || !l.endsWith(".mp3")) return r;
        if (xSemaphoreTake(sdMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
            r.ok = SD.exists(name.c_str());
            xSemaphoreGive(sdMutex);
        }
        return r;
    }
    String plain = name;
    if (plain.startsWith("/")) plain.remove(0, 1);
    if (name.startsWith("/")) name.remove(0, 1);
    if (name.startsWith("mp3/")) name.remove(0, 4);
    r.path = String(MP3_DIR) + "/" + name;
    r.shown = name;
    r.ok = fsReady && !name.isEmpty() && name.indexOf("..") < 0 && LittleFS.exists(r.path);

    // Ein Name ohne "sd:" wird zuerst im Flash gesucht, dann auf der SD-Karte (Pfad ab Wurzel).
    // A name without "sd:" is looked up in the flash first, then on the SD card (path from the root).

    if (!r.ok && sdReady && !plain.isEmpty()) {
        FileRef s = resolveFile("sd:/" + plain);
        if (s.ok) return s;
    }
    return r;
}

// Lautstaerke-Angleichung der MP3-Dateien (nicht der Ansagen): Beim ersten Abspielen misst ein Hintergrund-Task die Lautheit der ganzen
// Datei (Bloecke zu 400 ms, Gate wie bei EBU R128, aber ohne K-Filter) und merkt sich den Ausgleich in /loudness.bin; er wirkt als Faktor
// vor der Ausgabe. Das Ziel ist ein Pegel von LOUD_TARGET_DB, der Spitzenwert bleibt unter -1 dBFS.
// Loudness matching of the MP3 files (not of announcements): on the first play a background task measures the loudness of the whole file
// (blocks of 400 ms, gate as in EBU R128 but without the K filter) and stores the correction in /loudness.bin; it acts as a factor before
// the output. The goal is a level of LOUD_TARGET_DB, the peak stays below -1 dBFS.

#define LOUD_TARGET_DB -16.0f
#define LOUD_MAX_ENTRIES 400
#define LOUD_FILE "/loudness.bin"

struct LoudRec {
    uint32_t key;
    int32_t db10;
};

std::vector<LoudRec> loudCache;
bool loudCacheLoaded = false;
volatile bool fileGainOn = false;
volatile int32_t fileGainQ15 = 32768;
float fileGainCurDb = 0.0f;
float fileGainTargetDb = 0.0f;
bool fileGainKnown = false;
uint32_t loudCurKey = 0;
volatile bool loudBusy = false;
volatile bool loudDone = false;
String loudJobPath;
bool loudJobSd = false;
uint32_t loudJobKey = 0;
bool loudResultOk = false;
int loudResultDb10 = 0;
float loudResultRms = 0;
float loudResultPeak = 0;
bool loudPend = false;
String loudPendPath;
bool loudPendSd = false;
uint32_t loudPendKey = 0;

uint32_t loudKey(const String& path, bool sd, size_t size) {
    uint32_t h = 2166136261UL;
    for (size_t i = 0; i < path.length(); i++) h = (h ^ (uint8_t)path[i]) * 16777619UL;
    return (h ^ (uint32_t)size) * 16777619UL + (sd ? 1 : 0);
}

int32_t loudQ15(float db) {
    return (int32_t)(powf(10.0f, db / 20.0f) * 32768.0f);
}

void loudLoad() {
    if (loudCacheLoaded || !fsReady) return;
    loudCacheLoaded = true;
    File f = LittleFS.open(LOUD_FILE, FILE_READ);
    if (!f) return;
    LoudRec r;
    while (f.read((uint8_t*)&r, sizeof(r)) == sizeof(r)) loudCache.push_back(r);
    f.close();
}

bool loudFind(uint32_t key, int& db10) {
    loudLoad();
    for (size_t i = 0; i < loudCache.size(); i++) {
        if (loudCache[i].key == key) {
            db10 = loudCache[i].db10;
            return true;
        }
    }
    return false;
}

void loudAdd(uint32_t key, int db10) {
    loudLoad();
    bool found = false;
    for (size_t i = 0; i < loudCache.size(); i++) {
        if (loudCache[i].key == key) {
            loudCache[i].db10 = db10;
            found = true;
        }
    }
    if (!found) {
        if (loudCache.size() >= LOUD_MAX_ENTRIES) loudCache.erase(loudCache.begin());
        LoudRec r;
        r.key = key;
        r.db10 = db10;
        loudCache.push_back(r);
    }
    if (!fsReady) return;
    File f = LittleFS.open(LOUD_FILE, FILE_WRITE);
    if (!f) return;
    for (size_t i = 0; i < loudCache.size(); i++) f.write((const uint8_t*)&loudCache[i], sizeof(LoudRec));
    f.close();
}

int loudRead(File& f, bool sd, uint8_t* buf, size_t len) {
    if (sd) xSemaphoreTake(sdMutex, portMAX_DELAY);
    int n = f.read(buf, len);
    if (sd) xSemaphoreGive(sdMutex);
    return n;
}

void loudSeek(File& f, bool sd, uint32_t pos) {
    if (sd) xSemaphoreTake(sdMutex, portMAX_DELAY);
    f.seek(pos);
    if (sd) xSemaphoreGive(sdMutex);
}

void loudTask(void*) {
    String path = loudJobPath;
    bool sd = loudJobSd;
    File f = sd ? SD.open(path.c_str(), FILE_READ) : LittleFS.open(path, FILE_READ);
    uint8_t* in = (uint8_t*)malloc(MIX_IN_BYTES);
    int32_t* out = (int32_t*)malloc(1152 * 2 * sizeof(int32_t));
    MP3Decoder* dec = nullptr;
    std::vector<float> blocks;
    float peak = 0;
    if (f && in && out && audio) {
        dec = new MP3Decoder(*audio);
        if (!dec->init()) {
            delete dec;
            dec = nullptr;
        }
    }
    if (dec) {
        uint8_t hdr[10];
        if (loudRead(f, sd, hdr, 10) == 10 && memcmp(hdr, "ID3", 3) == 0) {
            uint32_t sz = ((uint32_t)(hdr[6] & 0x7f) << 21) | ((uint32_t)(hdr[7] & 0x7f) << 14) | ((uint32_t)(hdr[8] & 0x7f) << 7) | (uint32_t)(hdr[9] & 0x7f);
            loudSeek(f, sd, 10 + sz);
        } else {
            loudSeek(f, sd, 0);
        }
        int have = 0;
        bool eof = false;
        int errors = 0;
        uint32_t blockLen = 0;
        uint32_t inBlock = 0;
        double acc = 0;
        int frameCount = 0;
        while (audio) {
            if (have < MIX_REFILL_BELOW && !eof) {
                int n = loudRead(f, sd, in + have, MIX_IN_BYTES - have);
                if (n <= 0) eof = true;
                else have += n;
            }
            if (have < 4) break;
            int32_t sync = dec->findSyncWord(in, have);
            if (sync < 0) {
                have = 0;
                if (eof) break;
                continue;
            }
            if (sync > 0) {
                memmove(in, in + sync, have - sync);
                have -= sync;
            }
            int32_t left = have;
            int32_t res = dec->decode(in, &left, out);
            int used = have - left;
            if (res == 0) {
                uint32_t frames = dec->getOutputSamples();
                if (frames > 1152) frames = 1152;
                if (blockLen == 0 && frames > 0) blockLen = dec->getSampleRate() * 4 / 10;
                for (uint32_t i = 0; i < frames && blockLen > 0; i++) {
                    float l = (float)(out[2 * i] >> 16) / 32768.0f;
                    float r = (float)(out[2 * i + 1] >> 16) / 32768.0f;
                    float a = fabsf(l) > fabsf(r) ? fabsf(l) : fabsf(r);
                    if (a > peak) peak = a;
                    acc += 0.5 * ((double)l * l + (double)r * r);
                    if (++inBlock >= blockLen) {
                        blocks.push_back((float)(acc / blockLen));
                        acc = 0;
                        inBlock = 0;
                    }
                }
                errors = 0;
            } else if (++errors > 100) {
                break;
            }
            if (used <= 0) used = 1;
            if (used > have) used = have;
            have -= used;
            if (have > 0) memmove(in, in + used, have);
            if ((++frameCount & 3) == 0) vTaskDelay(1);
        }
    }
    if (dec) delete dec;
    if (f) f.close();
    free(in);
    free(out);

    // Gate: absolut -60 dBFS, relativ 10 dB unter dem Mittel der Bloecke ueber dem absoluten Gate.
    // Gate: absolute -60 dBFS, relative 10 dB below the mean of the blocks above the absolute gate.

    loudResultOk = false;
    if (blocks.size() >= 3 && peak > 0.0001f) {
        double sum = 0;
        int n = 0;
        for (size_t i = 0; i < blocks.size(); i++) {
            if (blocks[i] > 1e-6f) {
                sum += blocks[i];
                n++;
            }
        }
        if (n > 0) {
            double mean1 = sum / n, sum2 = 0;
            int n2 = 0;
            for (size_t i = 0; i < blocks.size(); i++) {
                if (blocks[i] > 1e-6f && blocks[i] > mean1 * 0.1) {
                    sum2 += blocks[i];
                    n2++;
                }
            }
            double mean = n2 > 0 ? sum2 / n2 : mean1;
            float rmsDb = 10.0f * log10f((float)mean);
            float peakDb = 20.0f * log10f(peak);
            float gain = LOUD_TARGET_DB - rmsDb;
            if (gain > -peakDb - 1.0f) gain = -peakDb - 1.0f;
            if (gain > 12.0f) gain = 12.0f;
            if (gain < -18.0f) gain = -18.0f;
            loudResultDb10 = (int)lroundf(gain * 10.0f);
            loudResultRms = rmsDb;
            loudResultPeak = peakDb;
            loudResultOk = true;
        }
    }
    loudDone = true;
    loudBusy = false;
    vTaskDelete(nullptr);
}

void loudStartJob(const String& path, bool sd, uint32_t key) {
    loudJobPath = path;
    loudJobSd = sd;
    loudJobKey = key;
    loudDone = false;
    loudBusy = true;
    if (xTaskCreatePinnedToCore(loudTask, "loudness", 12288, nullptr, 1, nullptr, 0) != pdPASS) loudBusy = false;
}

void loudRequest(const String& path, bool sd, uint32_t key) {
    if (loudBusy || loudDone) {
        loudPend = true;
        loudPendPath = path;
        loudPendSd = sd;
        loudPendKey = key;
        return;
    }
    loudStartJob(path, sd, key);
}

// Beim Start einer Datei: bekannten Ausgleich sofort setzen, sonst im Hintergrund messen; Ansagen und Radio bleiben ohne Ausgleich.
// At the start of a file: set a known correction at once, otherwise measure in the background; announcements and radio stay uncorrected.

void loudBegin(const String& path, bool sd) {
    fileGainOn = false;
    fileGainQ15 = 32768;
    fileGainCurDb = 0.0f;
    fileGainTargetDb = 0.0f;
    fileGainKnown = false;
    loudCurKey = 0;
    if (!cfg.loudNorm) return;
    File f = sd ? SD.open(path.c_str(), FILE_READ) : LittleFS.open(path, FILE_READ);
    size_t size = f ? f.size() : 0;
    if (f) f.close();
    if (size == 0) return;
    loudCurKey = loudKey(path, sd, size);
    int db10 = 0;
    if (loudFind(loudCurKey, db10)) {
        fileGainTargetDb = db10 / 10.0f;
        fileGainCurDb = fileGainTargetDb;
        fileGainQ15 = loudQ15(fileGainCurDb);
        fileGainOn = true;
        fileGainKnown = true;
        logLine("[LOUD] " + path + ": gain " + String(fileGainCurDb, 1) + " dB (stored)");
    } else {
        loudRequest(path, sd, loudCurKey);
    }
}

// Laeuft in der Hauptschleife: Ergebnis des Messens uebernehmen, den Ausgleich sanft nachfuehren, ausserhalb einer Datei abschalten.
// Runs in the main loop: take over the result of the measurement, follow the correction gently, switch off outside of a file.

void loudWatch() {
    if (loudDone && !loudBusy) {
        loudDone = false;
        if (loudResultOk) {
            loudAdd(loudJobKey, loudResultDb10);
            logLine("[LOUD] " + loudJobPath + ": level " + String(loudResultRms, 1) + " dBFS, peak " + String(loudResultPeak, 1) + " dBFS, gain " + String(loudResultDb10 / 10.0f, 1) + " dB");
            if (loudJobKey == loudCurKey && cfg.loudNorm) {
                fileGainTargetDb = loudResultDb10 / 10.0f;
                fileGainKnown = true;
                fileGainOn = true;
            }
        } else {
            logLine("[LOUD] " + loudJobPath + ": not measurable");
        }
    }
    if (!loudBusy && !loudDone && loudPend) {
        loudPend = false;
        loudStartJob(loudPendPath, loudPendSd, loudPendKey);
    }
    bool fileNow = nowKind == "file" && playState == PS_PLAYING;
    if (!fileNow || !cfg.loudNorm) {
        if (fileGainOn) {
            fileGainOn = false;
            fileGainQ15 = 32768;
            fileGainCurDb = 0.0f;
        }
        return;
    }
    static unsigned long last = 0;
    if (millis() - last < 40) return;
    last = millis();
    float d = fileGainTargetDb - fileGainCurDb;
    if (fabsf(d) < 0.01f) return;
    if (d > 0.25f) d = 0.25f;
    if (d < -0.25f) d = -0.25f;
    fileGainCurDb += d;
    fileGainQ15 = loudQ15(fileGainCurDb);
}

// Ein- und Ausblenden einer MP3-Datei (nicht einer Ansage), die ohne Radio ueber den Audio-Weg laeuft. Die Dauer ist die Einstellung
// "Radio aus-/einblenden", bei kurzen Dateien hoechstens ein Viertel der geschaetzten Laenge.
// Fade-in and fade-out of an MP3 file (not of an announcement) that plays through the audio path without radio. The duration is the
// setting "Radio aus-/einblenden", for short files at most a quarter of the estimated length.

int fileFadeMs = 0;
bool fileFadeOutDone = false;

void fileFadePrepare(const String& path, bool sd) {
    fileFadeOutDone = false;
    fileFadeMs = 0;
    if (cfg.fadeMs <= 0 || !audio) return;
    File f = sd ? SD.open(path.c_str(), FILE_READ) : LittleFS.open(path, FILE_READ);
    size_t size = f ? f.size() : 0;
    if (f) f.close();
    int ms = cfg.fadeMs;
    if (size > 0) {
        int cap = (int)(size / 16) / 4;
        if (cap < 80) cap = 80;
        if (ms > cap) ms = cap;
    }
    fileFadeMs = ms;
    audio->setVolume(0);
    volRamp.on = false;
    volRamp.now = 0;
}

void fileFadeStart() {
    if (fileFadeMs > 0 && audio) volRampTo(currentVolume * VOL_FINE, fileFadeMs);
}

void fileFadeWatch() {
    if (fileFadeMs <= 0 || fileFadeOutDone || !audio || playState != PS_PLAYING || nowKind != "file") return;
    if (millis() - playStartMillis < 300) return;
    uint32_t size = audio->getFileSize();
    uint32_t pos = audio->getAudioFilePosition();
    uint32_t bitrate = audio->getBitRate();
    if (size == 0 || bitrate < 8000) return;
    if (pos < size / 2) return;
    uint32_t bytesLeft = size > pos ? size - pos : 0;
    uint32_t remainMs = (uint32_t)((uint64_t)bytesLeft * 8000ULL / bitrate);
    if (remainMs <= (uint32_t)fileFadeMs) {
        fileFadeOutDone = true;
        volRampTo(0, remainMs > 60 ? (int)remainMs : 60);
    }
}

void startFile(String name) {
    FileRef r = resolveFile(name);
    if (!r.ok) {
        failPlayback(String(r.sd && !sdReady ? "SD card not available: " : "File not found: ") + r.path);
        return;
    }
    String path = r.path;
    stopPlayback();
    lastError = "";
    ensureAudio();
    fileFadePrepare(path, r.sd);
    fs::FS& useFs = r.sd ? (fs::FS&)SD : (fs::FS&)LittleFS;
    if (audio->connecttoFS(useFs, path.c_str())) {
        fileFadeStart();
        loudBegin(path, r.sd);
        playStartMillis = millis();
        playState = PS_PLAYING;
        nowKind = "file";
        nowFile = r.shown;
        logLine("[PLAY] file " + (r.sd ? "sd:" : String()) + path);
        publishStatus(true);
    } else {
        failPlayback("Cannot play file " + path);
    }
}

void startUrl(const String& url) {
    resumeUrl = "";
    if (!url.startsWith("http://") && !url.startsWith("https://")) {
        failPlayback("URL must start with http:// or https://");
        return;
    }
    if (WiFi.status() != WL_CONNECTED) {
        failPlayback("No WiFi connection");
        return;
    }
    stopPlayback();
    lastError = "";
    playState = PS_LOADING;
    loadStartMillis = millis();
    publishStatus(true);
    ensureAudio();
    if (audio->connecttohost(url.c_str())) {
        playStartMillis = millis();
        playState = PS_PLAYING;
        streamUrl = url;
        streamActive = true;
        streamWaitingReady = true;
        streamWaitSince = millis();
        streamLastFill = 0;
        nowKind = "radio";
        nowName = "";
        nowTitle = "";
        nowGenre = "";
        nowDesc = "";
        nowWeb = "";
        nowLogo = "";
        nowHost = "";
        nowBitrate = 0;
        titleQn = 0;
        titleBoost = false;
        logLine("[PLAY] url " + url);
        publishStatus(true);
    } else {
        failPlayback("Cannot open URL");
    }
}

// Laedt den Sprachtext von VoiceRSS als MP3 nach TTS_FILE. VoiceRSS meldet Fehler (falscher Schluessel, Limit) als Text
// "ERROR: ..." mit HTTP 200, deshalb wird der Dateianfang geprueft.

// Downloads the speech from VoiceRSS as MP3 to TTS_FILE. VoiceRSS reports errors (wrong key, limit) as text
// "ERROR: ..." with HTTP 200, so the start of the file is checked.

bool downloadSpeech(const String& url, String& errorOut) {
    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;

    // HTTP/1.0 vermeidet "chunked"-Antworten; so laesst sich die Datei selbst lesen, mit Zeitlimit und Watchdog.
    // HTTP/1.0 avoids "chunked" responses; this way the file can be read by hand, with a deadline and the watchdog.

    http.useHTTP10(true);
    http.setTimeout(TTS_HTTP_TIMEOUT_MS);
    http.setConnectTimeout(TTS_HTTP_TIMEOUT_MS);
    if (!http.begin(client, url)) {
        errorOut = "HTTP begin failed";
        return false;
    }
    esp_task_wdt_reset();
    int code = http.GET();
    esp_task_wdt_reset();
    if (code != HTTP_CODE_OK) {
        errorOut = "VoiceRSS HTTP " + String(code) + (code < 0 ? " (" + http.errorToString(code) + ")" : "");
        http.end();
        return false;
    }
    LittleFS.remove(TTS_FILE);
    File f = LittleFS.open(TTS_FILE, FILE_WRITE);
    if (!f) {
        errorOut = "Cannot write " TTS_FILE;
        http.end();
        return false;
    }
    int total = http.getSize();
    NetworkClient* stream = http.getStreamPtr();
    // Der Puffer liegt auf dem Heap: der Stapel der Hauptschleife wird beim TLS-Verbindungsaufbau schon stark gebraucht.
    // The buffer lives on the heap: the stack of the main loop is already heavily used during the TLS handshake.

    uint8_t* buf = (uint8_t*)malloc(1024);
    if (!buf) {
        errorOut = "Out of memory";
        f.close();
        http.end();
        return false;
    }
    size_t written = 0;
    unsigned long start = millis();
    unsigned long lastData = millis();
    bool ok = true;
    while (http.connected() || stream->available() > 0) {
        esp_task_wdt_reset();
        int avail = stream->available();
        if (avail > 0) {
            int n = stream->readBytes(buf, avail > 1024 ? 1024 : avail);
            if (n <= 0) continue;
            if (f.write(buf, n) != (size_t)n) {
                errorOut = "Flash full or write error";
                ok = false;
                break;
            }
            written += n;
            lastData = millis();
            if (written > TTS_MAX_BYTES) {
                errorOut = "Response too large";
                ok = false;
                break;
            }
            if (total >= 0 && written >= (size_t)total) break;
        } else {
            if (total >= 0 && written >= (size_t)total) break;
            if (millis() - lastData > TTS_HTTP_TIMEOUT_MS) {
                errorOut = "Download stalled";
                ok = false;
                break;
            }
            delay(5);
        }
        if (millis() - start > TTS_DEADLINE_MS) {
            errorOut = "Download too slow";
            ok = false;
            break;
        }
    }
    free(buf);
    f.close();
    http.end();
    esp_task_wdt_reset();
    if (!ok) return false;
    if (written == 0) {
        errorOut = "Empty response";
        return false;
    }
    File r = LittleFS.open(TTS_FILE, FILE_READ);
    char head[96];
    size_t n = r ? r.readBytes(head, sizeof(head) - 1) : 0;
    if (r) r.close();
    head[n] = 0;
    if (n >= 5 && strncmp(head, "ERROR", 5) == 0) {
        errorOut = String(head);
        errorOut.trim();
        return false;
    }
    return true;
}

// Baut die Anfrage-URL fuer VoiceRSS aus Text, Sprache, Stimme und Geschwindigkeit.
// Builds the request URL for VoiceRSS from text, language, voice and rate.

String buildSpeechUrl(const Pending& p) {
    String lang = p.lang.length() > 0 ? p.lang : cfg.vlang;
    String voice = p.voice.length() > 0 ? p.voice : cfg.vvoice;
    int rate = p.rate != 99 ? p.rate : cfg.vrate;
    if (rate < -10) rate = -10;
    if (rate > 10) rate = 10;
    String url = "https://api.voicerss.org/?key=" + urlEncode(cfg.vkey) + "&hl=" + urlEncode(lang) + "&r=" + String(rate) +
                 "&c=MP3&f=" + urlEncode(cfg.vfmt) + "&src=" + urlEncode(p.text);
    if (voice.length() > 0) url += "&v=" + urlEncode(voice);
    logLine("[VOICERSS] free heap " + String(ESP.getFreeHeap()) + ", request lang=" + lang + " voice=" + voice + " chars=" + String(p.text.length()));
    return url;
}

// Mischer: Eine Ansage oder MP3-Datei wird in einem eigenen Task dekodiert und im Audio-Task zum laufenden Webradio addiert.
// Das Radio laeuft ohne Unterbrechung weiter und ist waehrenddessen leiser (Ducking).
// Es gibt zwei Mischspuren: Spur 0 traegt eine Datei der SD-Karte ueber dem Radio, Spur 1 eine Ansage (Flash-MP3 oder Sprechen).
// Kommt eine Ansage, senkt sich Spur 0 ab und spielt danach weiter.
// Mixer: an announcement or MP3 file is decoded in its own task and added to the running web radio in the audio task.
// The radio keeps playing without interruption and is quieter meanwhile (ducking).
// There are two mix tracks: track 0 carries a file of the SD card over the radio, track 1 an announcement (flash MP3 or speech).
// When an announcement comes, track 0 is lowered and continues afterwards.

#define MIX_RING_FRAMES 16384
#define MIX_RING_MASK (MIX_RING_FRAMES - 1)

MixTrack mixT[2];

bool mixAny() {
    return mixT[0].inUse || mixT[1].inUse;
}

// Lautstaerkekurve der Audio-Bibliothek (VOLUME_MAX Stufen, -60 bis 0 dB), damit Ansage und Radio gleich laut klingen.
// Volume curve of the audio library (VOLUME_MAX steps, -60 to 0 dB), so announcement and radio sound equally loud.

int32_t mixGainFor(int vol) {
    if (vol <= 0) return 0;
    float t = (float)vol / (float)VOLUME_MAX;
    if (t > 1.0f) t = 1.0f;
    float dB = -112.0f * t * t * t + 172.0f * t * t - 60.0f;
    if (dB < -60.0f) dB = -60.0f;
    if (dB > 0.0f) dB = 0.0f;
    return (int32_t)(powf(10.0f, dB / 20.0f) * 32768.0f);
}

// Addiert eine Mischspur (umgerechnet auf die Rate des Radios) zu den Samples des Audio-Tasks.
// Adds one mix track (converted to the rate of the radio) to the samples of the audio task.

static void mixAddTrack(MixTrack& m, int32_t* buf, int16_t words) {
    if (!m.active) return;
    m.inHook = true;
    int16_t* ring = m.ring;
    uint32_t srcRate = m.srcRate;
    uint32_t outRate = m.outRate;
    if (!m.active || !ring || srcRate == 0 || outRate == 0) {
        m.inHook = false;
        return;
    }
    uint32_t step = (uint32_t)(((uint64_t)srcRate << 16) / outRate);
    uint32_t rd = m.rd.load(std::memory_order_relaxed);
    uint32_t wr = m.wr.load(std::memory_order_acquire);
    uint32_t phase = m.phase;
    int32_t g = m.gainQ15;
    int frames = words / 2;
    for (int i = 0; i < frames; i++) {
        if (wr - rd < 2) {
            wr = m.wr.load(std::memory_order_acquire);
            if (wr - rd < 2) {
                if (m.producerDone) m.drained = true;
                break;
            }
        }
        const int16_t* a = ring + (rd & MIX_RING_MASK) * 2;
        const int16_t* b = ring + ((rd + 1) & MIX_RING_MASK) * 2;
        int32_t frac = (int32_t)((phase & 0xFFFF) >> 2);
        int32_t l = a[0] + (((int32_t)b[0] - a[0]) * frac >> 14);
        int32_t r = a[1] + (((int32_t)b[1] - a[1]) * frac >> 14);
        int64_t sl = (int64_t)buf[2 * i] + (int64_t)((l * g) << 1);
        int64_t sr = (int64_t)buf[2 * i + 1] + (int64_t)((r * g) << 1);
        if (sl > INT32_MAX) sl = INT32_MAX;
        if (sl < INT32_MIN) sl = INT32_MIN;
        if (sr > INT32_MAX) sr = INT32_MAX;
        if (sr < INT32_MIN) sr = INT32_MIN;
        buf[2 * i] = (int32_t)sl;
        buf[2 * i + 1] = (int32_t)sr;
        phase += step;
        rd += phase >> 16;
        phase &= 0xFFFF;
    }
    m.phase = phase;
    m.rd.store(rd, std::memory_order_release);
    m.inHook = false;
}

// Wird im Audio-Task vor jeder I2S-Ausgabe aufgerufen: addiert beide Mischspuren zu den Radio-Samples.
// Called in the audio task before every I2S output: adds both mix tracks to the radio samples.

void audio_process_i2s(int32_t* buf, int16_t words, bool* continueI2S) {
    int32_t fg = fileGainQ15;
    if (fileGainOn && fg != 32768) {
        for (int i = 0; i < words; i++) {
            int64_t v = ((int64_t)buf[i] * fg) >> 15;
            if (v > INT32_MAX) v = INT32_MAX;
            if (v < INT32_MIN) v = INT32_MIN;
            buf[i] = (int32_t)v;
        }
    }
    mixAddTrack(mixT[MIX_LAYER], buf, words);
    mixAddTrack(mixT[MIX_ANN], buf, words);
}

// Lese-Task: MP3 aus dem Dateisystem lesen, mit dem Dekoder der Bibliothek in PCM wandeln und in den Ringpuffer schreiben.
// Reader task: read the MP3 from the file system, convert it to PCM with the decoder of the library and write it to the ring buffer.

// Lesen aus der Datei des Mischers; auf der SD-Karte unter dem Sperrzeichen, weil die Weboberflaeche sie zugleich auflisten kann.
// Reading from the file of the mixer; on the SD card under the lock because the web interface may list it at the same time.

int mixRead(bool onSd, File& f, uint8_t* buf, size_t len) {
    if (onSd) xSemaphoreTake(sdMutex, portMAX_DELAY);
    int n = f.read(buf, len);
    if (onSd) xSemaphoreGive(sdMutex);
    return n;
}

void mixSeek(bool onSd, File& f, uint32_t pos) {
    if (onSd) xSemaphoreTake(sdMutex, portMAX_DELAY);
    f.seek(pos);
    if (onSd) xSemaphoreGive(sdMutex);
}

void mixTask(void* arg) {
    MixTrack& m = *(MixTrack*)arg;
    File f = m.onSd ? SD.open(m.path.c_str(), FILE_READ) : LittleFS.open(m.path, FILE_READ);
    uint8_t* in = (uint8_t*)malloc(MIX_IN_BYTES);
    int32_t* out = (int32_t*)malloc(1152 * 2 * sizeof(int32_t));
    MP3Decoder* dec = nullptr;
    if (f && in && out && audio) {
        dec = new MP3Decoder(*audio);
        if (!dec->init()) {
            delete dec;
            dec = nullptr;
        }
    }
    if (dec) {
        uint8_t hdr[10];
        if (mixRead(m.onSd, f, hdr, 10) == 10 && memcmp(hdr, "ID3", 3) == 0) {
            uint32_t sz = ((uint32_t)(hdr[6] & 0x7f) << 21) | ((uint32_t)(hdr[7] & 0x7f) << 14) | ((uint32_t)(hdr[8] & 0x7f) << 7) | (uint32_t)(hdr[9] & 0x7f);
            mixSeek(m.onSd, f, 10 + sz);
        } else {
            mixSeek(m.onSd, f, 0);
        }
        int have = 0;
        bool eof = false;
        bool first = true;
        int errors = 0;
        while (!m.abortFlag) {
            if (have < MIX_REFILL_BELOW && !eof) {
                int n = mixRead(m.onSd, f, in + have, MIX_IN_BYTES - have);
                if (n <= 0) eof = true;
                else have += n;
            }
            if (have < 4) break;
            int32_t sync = dec->findSyncWord(in, have);
            if (sync < 0) {
                have = 0;
                if (eof) break;
                continue;
            }
            if (sync > 0) {
                memmove(in, in + sync, have - sync);
                have -= sync;
            }
            int32_t left = have;
            int32_t res = dec->decode(in, &left, out);
            int used = have - left;
            if (res == 0) {
                uint32_t frames = dec->getOutputSamples();
                if (frames > 1152) frames = 1152;
                if (frames > 0) {
                    if (first) {
                        m.srcRate = dec->getSampleRate();
                        first = false;
                    }
                    while (!m.abortFlag && MIX_RING_FRAMES - (m.wr.load(std::memory_order_relaxed) - m.rd.load(std::memory_order_acquire)) < frames + 1) vTaskDelay(3);
                    if (m.abortFlag) break;
                    uint32_t w = m.wr.load(std::memory_order_relaxed);
                    for (uint32_t i = 0; i < frames; i++) {
                        int16_t* d = m.ring + ((w + i) & MIX_RING_MASK) * 2;
                        d[0] = (int16_t)(out[2 * i] >> 16);
                        d[1] = (int16_t)(out[2 * i + 1] >> 16);
                    }
                    m.wr.store(w + frames, std::memory_order_release);
                }
                errors = 0;
            } else if (++errors > 100) {
                break;
            }
            if (used <= 0) used = 1;
            if (used > have) used = have;
            have -= used;
            if (have > 0) memmove(in, in + used, have);
            vTaskDelay(1);
        }
    }
    if (dec) delete dec;
    if (f) f.close();
    free(in);
    free(out);
    m.producerDone = true;
    m.taskRunning = false;
    vTaskDelete(nullptr);
}

// Absenken der Spur 0 (Datei der SD-Karte), solange eine Ansage laeuft: weicher Verlauf ueber die Dauer des Ab- und Aufblendens.
// Lowering of track 0 (file of the SD card) while an announcement runs: smooth ramp over the duration of fading down and up.

struct LayerRamp {
    bool on = false;
    int32_t from = 0;
    int32_t to = 0;
    unsigned long t0 = 0;
    int ms = 0;
};
LayerRamp layRamp;

void layerDuck(bool down) {
    MixTrack& m = mixT[MIX_LAYER];
    if (!m.inUse) return;
    layRamp.from = m.gainQ15;
    layRamp.to = down ? (int32_t)((int64_t)m.baseGainQ15 * cfg.duckPct / 100) : m.baseGainQ15;
    layRamp.t0 = millis();
    layRamp.ms = cfg.fadeMs;
    layRamp.on = true;
}

void layerRampStep() {
    if (!layRamp.on) return;
    MixTrack& m = mixT[MIX_LAYER];
    if (!m.inUse) {
        layRamp.on = false;
        return;
    }
    unsigned long dt = millis() - layRamp.t0;
    int32_t v = layRamp.to;
    if (layRamp.ms > 0 && dt < (unsigned long)layRamp.ms) v = layRamp.from + (int32_t)((int64_t)(layRamp.to - layRamp.from) * (int64_t)dt / layRamp.ms);
    m.gainQ15 = v;
    if (dt >= (unsigned long)layRamp.ms) layRamp.on = false;
}

// Haelt eine Mischspur an: Task beenden, Speicher freigeben; die Radio-Lautstaerke bleibt unveraendert.
// Halts a mix track: end the task, free the memory; the radio volume stays unchanged.

void mixHalt(int t) {
    MixTrack& m = mixT[t];
    if (!m.inUse) return;
    m.active = false;
    m.pending = false;
    m.abortFlag = true;
    for (int i = 0; i < 100 && m.taskRunning; i++) delay(10);
    m.abortFlag = false;
    for (int i = 0; i < 50 && m.inHook; i++) delay(2);
    delay(5);
    if (m.ring) {
        heap_caps_free(m.ring);
        m.ring = nullptr;
    }
    m.inUse = false;
    if (t == MIX_LAYER) layRamp.on = false;
}

// Beendet eine Mischspur; sind beide frei und wartet nichts, blendet das Radio wieder auf die vorherige Lautstaerke auf.
// Ends a mix track; if both are free and nothing is waiting, the radio fades back up to its previous volume.

void mixStop(int t) {
    bool was = mixT[t].inUse;
    mixHalt(t);
    if (t == MIX_ANN && annCount == 0) layerDuck(false);
    bool radioBack = false;
    if (mixDucked && annCount == 0 && !mixAny()) {
        volRampTo(currentVolume * VOL_FINE, cfg.fadeMs);
        mixDucked = false;
        radioBack = true;
    }
    if (!was) return;
    if (t == MIX_LAYER) logLine(radioBack ? "[MIX] card file over the radio finished, radio back to volume " + String(currentVolume) : String("[MIX] card file over the radio finished"));
    else logLine(annCount > 0 ? String("[MIX] announcement finished, next one waiting, radio stays lowered") : "[MIX] announcement finished, radio back to volume " + String(currentVolume));
}

void mixStopAll() {
    mixStop(MIX_ANN);
    mixStop(MIX_LAYER);
}

// Blendet die laufende Ansage kurz aus (100 ms), bevor eine neue sie ersetzt.
// Fades the running announcement out briefly (100 ms) before a new one replaces it.

void mixQuickFade() {
    MixTrack& m = mixT[MIX_ANN];
    if (!m.inUse || !m.active) return;
    int32_t g0 = m.gainQ15;
    for (int i = 9; i >= 0; i--) {
        m.gainQ15 = (int32_t)((int64_t)g0 * i / 10);
        delay(10);
        esp_task_wdt_reset();
    }
}

// Startet eine Mischspur fuer eine MP3-Datei; false, wenn Speicher oder Dekoder fehlen (dann gilt der normale Weg ueber Unterbrechen).
// Starts a mix track for an MP3 file; false if memory or decoder is missing (the normal way via interrupting then applies).

bool mixStart(int t, const String& path, int vol, bool sd = false) {
    MixTrack& m = mixT[t];
    if (t == MIX_ANN) mixQuickFade();
    mixHalt(t);
    if (!audio) return false;
    m.ring = (int16_t*)heap_caps_malloc(MIX_RING_FRAMES * 2 * sizeof(int16_t), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (!m.ring) return false;
    m.inUse = true;
    m.path = path;
    m.onSd = sd;
    m.wr = 0;
    m.rd = 0;
    m.phase = 0;
    m.srcRate = 0;
    m.drained = false;
    m.producerDone = false;
    m.abortFlag = false;
    m.outRate = audio->getSampleRate();
    m.baseGainQ15 = mixGainFor(vol);
    m.gainQ15 = m.baseGainQ15;
    m.taskRunning = true;
    if (m.outRate == 0 || xTaskCreatePinnedToCore(mixTask, t == MIX_LAYER ? "mixlay" : "mixann", 12288, &m, 1, nullptr, 1) != pdPASS) {
        m.taskRunning = false;
        mixStop(t);
        return false;
    }
    unsigned long t0 = millis();
    while (millis() - t0 < 1500 && m.taskRunning && (m.wr.load() - m.rd.load()) < MIX_PREFILL_FRAMES) {
        delay(5);
        esp_task_wdt_reset();
    }
    if (m.wr.load() < 2) {
        mixStop(t);
        return false;
    }
    m.lastRd = 0;
    m.lastProgress = millis();
    int duck = currentVolume * VOL_FINE * cfg.duckPct / 100;

    // War das Radio oder die Datei schon beim Aufruf abgesenkt (Ansage), startet die Ansage ohne weitere Wartezeit.
    // If the radio or file was already lowered when the call came in (announcement), the announcement starts without further delay.

    unsigned long lead = (unsigned long)cfg.fadeMs * 6 / 10;
    unsigned long since = mixDucked ? millis() - mixDuckAt : 0;
    if (!mixDucked) {
        volRampTo(duck, cfg.fadeMs);
        mixDuckAt = millis();
    }
    mixDucked = true;
    if (t == MIX_ANN) layerDuck(true);
    m.pending = true;
    m.activateAt = millis() + (since >= lead ? 0 : lead - since);
    logLine("[MIX] " + path + " mixed over the running audio, " + String(t == MIX_LAYER ? "file" : "announcement") + " volume " + String(vol) + ", radio " + String(currentVolume) + " -> " + String(duck / VOL_FINE) + ", source " + String(m.srcRate) + " Hz, output " + String(m.outRate) + " Hz");
    return true;
}

// Kann eine Ansage oder Datei jetzt ueber das laufende Radio oder eine laufende MP3-Datei gemischt werden?
// Can an announcement or file be mixed over the running radio or a running MP3 file now?

bool mixPossible() {
    if (!cfg.mixAnn || !audio || playState != PS_PLAYING || !audio->isRunning()) return false;
    if (streamActive) return nowKind == "radio" && !streamWaitingReady && !fadeInWaiting && !fadeInActive;
    return nowKind == "file";
}

// Meldet einen Fehler, ohne das laufende Radio anzutasten.
// Reports an error without touching the running radio.

void announceFail(const String& msg) {
    lastError = msg;
    logLine("[PLAY] " + msg);
    if (annCount == 0) layerDuck(false);
    if (mixDucked && annCount == 0 && !mixAny()) {
        volRampTo(currentVolume * VOL_FINE, cfg.fadeMs);
        mixDucked = false;
    }
    publishStatus(true);
}

// Prueft laufend, ob eine Mischspur zu Ende ist oder haengt.
// Continuously checks whether a mix track has ended or is stuck.

void mixWatch() {
    volRampStep();
    layerRampStep();
    for (int t = 0; t < 2; t++) {
        MixTrack& m = mixT[t];
        if (!m.inUse) continue;
        if (m.pending && millis() >= m.activateAt) {
            m.pending = false;
            m.active = true;
            m.lastProgress = millis();
        }
        uint32_t rd = m.rd.load();
        if (rd != m.lastRd) {
            m.lastRd = rd;
            m.lastProgress = millis();
        }
        bool empty = m.producerDone && (m.wr.load() - rd) < 2;
        bool stuck = m.active && (m.wr.load() - rd) > 0 && millis() - m.lastProgress > 4000;
        if (empty || m.drained || stuck || !audio || playState != PS_PLAYING || !(streamActive || nowKind == "file")) mixStop(t);
    }
}

// Waehrend der Text geladen wird, blockiert die Hauptschleife; ein kleiner Task fuehrt das Absenken in dieser Zeit weiter, damit es sanft bleibt.
// While the text is being loaded the main loop is blocked; a small task keeps the lowering going meanwhile so it stays smooth.

volatile bool duckWorkerRun = false;
volatile bool duckWorkerDone = true;

void duckWorker(void*) {
    while (duckWorkerRun) {
        volRampStep();
        layerRampStep();
        vTaskDelay(pdMS_TO_TICKS(20));
    }
    duckWorkerDone = true;
    vTaskDelete(nullptr);
}

void duckWorkerStart() {
    duckWorkerDone = false;
    duckWorkerRun = true;
    if (xTaskCreate(duckWorker, "duck", 4096, nullptr, 1, nullptr) != pdPASS) {
        duckWorkerRun = false;
        duckWorkerDone = true;
    }
}

void duckWorkerStop() {
    duckWorkerRun = false;
    for (int i = 0; i < 50 && !duckWorkerDone; i++) delay(2);
}

// Ansage ueber das Radio mischen: Text laden, dann mit dem Mischer ueber das laufende Radio legen. false = normaler Weg.
// Mix an announcement over the radio: download the text, then lay it over the running radio with the mixer. false = normal way.

bool speakMixed(const Pending& p) {
    if (cfg.vkey.isEmpty()) {
        announceFail("VoiceRSS API key is not set");
        return true;
    }
    if (WiFi.status() != WL_CONNECTED) {
        announceFail("No WiFi connection");
        return true;
    }
    mixQuickFade();
    mixHalt(MIX_ANN);
    lastError = "";

    // Radio oder Datei sofort absenken, damit die Ansage nach dem Laden gleich zu hoeren ist.
    // Lower the radio or file right away so the announcement is audible as soon as it is loaded.

    if (!mixDucked) {
        volRampTo(currentVolume * VOL_FINE * cfg.duckPct / 100, cfg.fadeMs);
        mixDucked = true;
        mixDuckAt = millis();
    }
    layerDuck(true);
    voiceBusy = true;
    voiceEver = true;
    publishStatus(true);
    String err;
    duckWorkerStart();
    bool ok = downloadSpeech(buildSpeechUrl(p), err);
    duckWorkerStop();
    voiceBusy = false;
    voiceLastOk = ok;
    if (!ok) {
        announceFail("VoiceRSS: " + err);
        return true;
    }
    if (!mixPossible()) {
        announceFail("Radio stopped while the announcement was loading");
        return true;
    }
    if (!mixStart(MIX_ANN, TTS_FILE, p.vol >= 0 ? p.vol : cfg.vol)) {
        announceFail("Mixer not available");
        return true;
    }
    publishStatus(true);
    return true;
}

// Datei ueber das Radio mischen; false, wenn die Datei kein Fall fuer den Mischer ist (dann der normale Weg).
// Mix a file over the radio; false if the file is not a case for the mixer (then the normal way).

bool fileMixed(const Pending& p) {
    FileRef r = resolveFile(p.text);
    if (!r.ok) {
        announceFail(String(r.sd && !sdReady ? "SD card not available: " : "File not found: ") + r.path);
        return true;
    }
    lastError = "";

    // Eine Datei der Karte ueber dem Radio liegt auf Spur 0 und wird von einer Ansage abgesenkt; alles andere ist eine Ansage (Spur 1).
    // A file of the card over the radio goes on track 0 and is lowered by an announcement; everything else is an announcement (track 1).

    if (!mixStart((r.sd && streamActive) ? MIX_LAYER : MIX_ANN, r.path, p.vol >= 0 ? p.vol : cfg.vol, r.sd)) return false;

    // Laeuft schon eine Datei, bleibt sie die angezeigte; nur ueber einem Radio steht die gemischte Datei in nowFile.
    // If a file is already playing it stays the one shown; only over a radio the mixed file goes into nowFile.

    if (nowKind != "file") nowFile = r.shown;
    publishStatus(true);
    return true;
}

// Beendet Wiedergabe und Mischer, die gerade von der Karte lesen (vor dem Neu-Einbinden).
// Ends playback and mixer that are reading from the card right now (before mounting again).

void sdStopAll() {
    for (int t = 0; t < 2; t++) {
        if (mixT[t].inUse && mixT[t].onSd) mixStop(t);
    }
    if (nowKind == "file" && playState != PS_IDLE && nowFile.startsWith("sd:")) stopPlayback();
}

// Beendet die Wiedergabe oder den Mischer, wenn die Datei oder eine Datei im Ordner der Karte gerade benutzt wird.
// Ends playback or the mixer if the file, or a file in the folder, on the card is being used right now.

void sdStopIfUsing(const String& path) {
    String shown = "sd:" + path;
    for (int t = 0; t < 2; t++) {
        if (mixT[t].inUse && mixT[t].onSd && (mixT[t].path == path || mixT[t].path.startsWith(path + "/"))) mixStop(t);
    }
    if (nowKind == "file" && playState != PS_IDLE && (nowFile == shown || nowFile.startsWith(shown + "/"))) stopPlayback();
}

// Beendet nur dann die Wiedergabe, wenn genau diese Datei gerade gespielt oder gemischt wird.
// Stops playback only if exactly this file is being played or mixed right now.

void stopIfUsing(const String& name) {
    String path = String(MP3_DIR) + "/" + name;
    for (int t = 0; t < 2; t++) {
        if (mixT[t].inUse && mixT[t].path == path) mixStop(t);
    }
    if (nowKind == "file" && nowFile == name && playState != PS_IDLE) stopPlayback();
}

void speakText(const Pending& p) {
    if (cfg.vkey.isEmpty()) {
        failPlayback("VoiceRSS API key is not set");
        return;
    }
    if (WiFi.status() != WL_CONNECTED) {
        failPlayback("No WiFi connection");
        return;
    }
    stopPlayback();
    releaseAudio();
    lastError = "";
    voiceBusy = true;
    voiceEver = true;
    playState = PS_LOADING;
    loadStartMillis = millis();
    publishStatus(true);
    String url = buildSpeechUrl(p);
    String err;
    bool ok = downloadSpeech(url, err);
    voiceBusy = false;
    voiceLastOk = ok;
    if (!ok) {
        failPlayback("VoiceRSS: " + err);
        return;
    }
    applyVolume(p.vol >= 0 ? p.vol : cfg.vol);
    ensureAudio();
    if (audio->connecttoFS(LittleFS, TTS_FILE)) {
        playStartMillis = millis();
        playState = PS_PLAYING;
        nowKind = "speech";
        logLine("[VOICERSS] playing");
        publishStatus(true);
    } else {
        failPlayback("Cannot play " TTS_FILE);
    }
}

void fadeInStep() {
    if (!fadeInWaiting && !fadeInActive) return;
    if (!audio) {
        fadeInWaiting = false;
        fadeInActive = false;
        return;
    }
    if (fadeInWaiting && millis() - fadeInStart > 10000UL) {
        fadeInWaiting = false;
        applyVolume(fadeInTarget);
        return;
    }
    if (!fadeInActive) return;
    unsigned long dt = millis() - fadeInStart;
    if (fadeInMs <= 0 || dt >= (unsigned long)fadeInMs) {
        fadeInActive = false;
        applyVolume(fadeInTarget);
        return;
    }
    int fine = (int)((long)fadeInTarget * VOL_FINE * (long)dt / fadeInMs);
    if (fine != volRamp.now) {
        volRamp.now = fine;
        audio->setVolume(fine);
        currentVolume = (fine + VOL_FINE / 2) / VOL_FINE;
        volChangedAt = millis();
    }
}

// Blendet den laufenden Stream in ms Millisekunden auf Lautstaerke 0 aus. currentVolume bleibt unveraendert, damit die Fortsetzung die alte Lautstaerke kennt.
// Fades the running stream to volume 0 over ms milliseconds. currentVolume stays unchanged so the resume knows the old volume.

void fadeOutStream(int ms) {
    if (!audio || ms <= 0 || currentVolume <= 0 || playState != PS_PLAYING) return;
    volRamp.on = false;
    int start = volRamp.now;
    unsigned long t0 = millis();
    while (millis() - t0 < (unsigned long)ms) {
        int v = start - (int)((long)start * (long)(millis() - t0) / ms);
        audio->setVolume(v < 0 ? 0 : v);
        audio->loop();
        esp_task_wdt_reset();
        delay(10);
    }
    audio->setVolume(0);
    volRamp.now = 0;
}

// Laeuft gerade keine Ansage oder Datei (und wird keine geladen oder gemischt)?
// Is no announcement or file running right now (and none being loaded or mixed)?

bool announceBusy() {
    return voiceBusy || mixAny() || ((nowKind == "speech" || nowKind == "file") && playState != PS_IDLE);
}

// Ist p eine MP3-Datei aus dem Flash, waehrend gerade eine MP3-Datei von der SD-Karte spielt? Dann wird sie sofort ueber die Karte gemischt.
// Is p an MP3 file from the flash while an MP3 file from the SD card is playing? Then it is mixed over the card file right away.

bool flashMp3(const Pending& p) {
    if (p.type != PEND_FILE || p.text.startsWith("sd:")) return false;
    String lower = p.text;
    lower.toLowerCase();
    if (!lower.endsWith(".mp3")) return false;
    String n = p.text;
    n.trim();
    if (n.startsWith("/")) n.remove(0, 1);
    if (n.startsWith("mp3/")) n.remove(0, 4);
    return fsReady && !n.isEmpty() && n.indexOf("..") < 0 && LittleFS.exists(String(MP3_DIR) + "/" + n);
}

// Rangfolge von hoch nach niedrig: 1. MP3 aus dem Flash, 2. Sprechen, 3. MP3 von der SD-Karte, 4. Webradio.
// 1 und 2 spielen nacheinander und senken 3 und 4 ab. Laeuft eine SD-Datei als Mischspur ueber dem Radio, kommt eine Flash-MP3 oder ein Text
// sofort auf die freie Ansage-Spur; SD-Datei und Radio werden abgesenkt und spielen danach weiter.
// Priority from high to low: 1. MP3 from the flash, 2. speech, 3. MP3 from the SD card, 4. web radio.
// 1 and 2 play one after the other and lower 3 and 4. If an SD file runs as a mix track over the radio, a flash MP3 or a text goes
// onto the free announcement track right away; SD file and radio are lowered and continue afterwards.

bool preempts(const Pending& p) {
    if (!mixT[MIX_LAYER].inUse || mixT[MIX_ANN].inUse || voiceBusy || playState != PS_PLAYING || !mixPossible()) return false;
    return p.type == PEND_SAY || flashMp3(p);
}

bool flashOverSd(const Pending& p) {
    return nowKind == "file" && nowFile.startsWith("sd:") && mixPossible() && flashMp3(p);
}

// Holt den naechsten Eintrag aus der Warteschlange, sobald die vorige Ansage oder Datei zu Ende ist.
// Takes the next entry from the queue as soon as the previous announcement or file has ended.

void processQueue() {
    if (annCount == 0 || pending.type != PEND_NONE || playState == PS_LOADING) return;
    Pending next;
    if (!announceBusy()) {
        if (annPop(next)) pending = next;
        return;
    }

    // Spielt nur eine MP3-Datei (keine Ansage, kein Mischer), kommt die erste Ansage der Warteschlange vor und wird leise darueber gemischt;
    // ebenso eine Flash-MP3-Datei vor einer laufenden Datei der SD-Karte.
    // If only an MP3 file is playing (no announcement, no mixer), the first announcement of the queue jumps ahead and is mixed quietly over it;
    // so does a flash MP3 file ahead of a running file of the SD card.

    for (int i = 0; i < annCount; i++) {
        if (preempts(annQueue[i]) && annTake(i, next)) {
            pending = next;
            return;
        }
    }

    if (!voiceBusy && !mixAny() && nowKind == "file" && mixPossible()) {
        for (int i = 0; i < annCount; i++) {
            if (((annQueue[i].type == PEND_SAY && nowFile.startsWith("sd:")) || flashOverSd(annQueue[i])) && annTake(i, next)) {
                pending = next;
                return;
            }
        }
    }
}

void processPending() {
    if (pending.type == PEND_NONE) return;
    Pending p = pending;
    pending.type = PEND_NONE;
    pending.resume = false;
    pending.force = false;

    // Wird der Sender gewaehlt, der schon laeuft, spielt er einfach weiter: kein Ausblenden, kein Neustart. Ein ausdruecklicher Wert (vol) gilt trotzdem.
    // If the station that is already running is chosen, it simply keeps playing: no fade, no restart. An explicit value (vol) still applies.

    if (p.type == PEND_URL && !p.resume && !p.force && streamActive && playState == PS_PLAYING && p.text == streamUrl) {
        if (p.vol >= 0) {
            applyVolumeSmooth(p.vol);
        }
        wantRadioUrl = p.text;
        logLine("[PLAY] same station selected, keeps playing");
        return;
    }
    if (!p.resume) {
        fadeInWaiting = false;
        fadeInActive = false;
    }

    // Laeuft ein Webradio, wird eine Ansage oder MP3-Datei leise darueber gemischt; das Radio laeuft ohne Unterbrechung weiter.
    // If a web radio is running, an announcement or MP3 file is mixed over it quietly; the radio keeps playing without interruption.

    bool mixable = p.type == PEND_SAY;
    if (p.type == PEND_FILE && streamActive) {
        String lowerName = p.text;
        lowerName.toLowerCase();
        mixable = lowerName.endsWith(".mp3");
    }
    if (flashOverSd(p)) mixable = true;

    if (mixAny() && !(mixable && mixPossible())) {
        mixHalt(MIX_LAYER);
        mixHalt(MIX_ANN);
        mixDucked = false;
        volRamp.on = false;
    }
    if (mixable && mixPossible()) {
        bool handled = p.type == PEND_SAY ? speakMixed(p) : fileMixed(p);
        if (handled) return;
    }

    if (mixDucked) {
        mixDucked = false;
        volRamp.on = false;
    }

    // Stream merken, bevor Ansage oder Datei ihn stoppen; die Lautstaerke der Ansage ersetzt nur zeitweise die des Streams.
    // Remember the stream before an announcement or file stops it; the announcement volume only temporarily replaces the stream volume.

    if ((p.type == PEND_SAY || p.type == PEND_FILE) && streamActive && playState != PS_IDLE) {
        resumeUrl = streamUrl;
        resumeVolume = (fadeInWaiting || fadeInActive) ? fadeInTarget : currentVolume;
        logLine("[PLAY] stream paused for announcement");
        fadeOutStream(cfg.fadeMs);
    }
    bool switching = false;
    if (p.type == PEND_URL && !p.resume && !p.force && streamActive && playState == PS_PLAYING && cfg.switchFadeMs > 0) {
        fadeOutStream(cfg.switchFadeMs);
        switching = true;
        logLine("[PLAY] station change, faded out");
    }
    if (p.resume) {
        if (cfg.fadeMs > 0 && resumeVolume > 0) {
            fadeInMs = cfg.fadeMs;
            fadeInTarget = resumeVolume;
            applyVolume(0);
            fadeInWaiting = true;
            fadeInActive = false;
            fadeInStart = millis();
        } else {
            applyVolume(resumeVolume);
        }
    } else if (p.type == PEND_URL || p.type == PEND_FILE) {

        // Reihenfolge der Lautstaerke: ausdruecklicher Wert, sonst Wert des Senders (steckt schon in p.vol), sonst die allgemeine.
        // Order of the volume: explicit value, otherwise the value of the station (already in p.vol), otherwise the general one.

        int target = p.vol >= 0 ? p.vol : cfg.vol;

        // Jeder Start einer URL (aus dem Leerlauf oder als Senderwechsel nach dem Ausblenden) beginnt leise und blendet ein.
        // Every start of a URL (from idle or as a station change after the fade-out) begins silent and fades in.

        if (p.type == PEND_URL && cfg.switchFadeMs > 0 && target > 0) {
            fadeInMs = cfg.switchFadeMs;
            fadeInTarget = target;
            applyVolume(0);
            fadeInWaiting = true;
            fadeInActive = false;
            fadeInStart = millis();
        } else {
            applyVolume(target);
        }
    }
    switch (p.type) {
        case PEND_SAY: speakText(p); break;
        case PEND_URL:
            wantRadioUrl = p.text;
            wantRadioVol = p.vol;
            wantRadioLive = true;
            startUrl(p.text);
            break;
        case PEND_FILE: startFile(p.text); break;
        case PEND_STOP:
            saveRadioState("", -1);
            annClear();
            resumeUrl = "";
            wantRadioUrl = "";
            lastError = "";

            // Ein laufendes Webradio wird vor dem Stopp langsam ausgeblendet; Ansagen und Dateien stoppen sofort.
            // A running web radio is faded out slowly before the stop; announcements and files stop immediately.

            if (streamActive && playState == PS_PLAYING) fadeOutStream(cfg.switchFadeMs);
            stopPlayback();
            logLine("[PLAY] stopped");
            publishStatus(true);
            settleVolume();
            break;
        default: break;
    }
}

// Meldet das Ende einer Wiedergabe, auch wenn die Audio-Bibliothek keinen Rueckruf liefert.
// Detects the end of a playback even if the audio library gives no callback.

void playbackWatch() {
    if (playState == PS_PLAYING && millis() - playStartMillis > 1500 && audio && !audio->isRunning()) {
        logLine("[PLAY] finished");
        setIdle();
    }

    // Kein Ton (Dekoder nicht bereit) nach Start oder "Stream lost": Der Server liefert nichts, die Bibliothek wartet ohne Zeitlimit.
    // Dann den Stream selbst neu verbinden (ausdruecklich erzwungen, auch wenn es dieselbe URL ist).
    // No sound (decoder not ready) after the start or "Stream lost": the server delivers nothing and the library waits without a limit.
    // Then reconnect the stream ourselves (explicitly forced, even if it is the same URL).

    if (streamWaitingReady && streamActive && playState == PS_PLAYING && audio) {

        // Kommen noch Daten an (der Eingangspuffer aendert sich), ist der Server nur langsam: weiter warten.
        // If data is still arriving (the input buffer changes), the server is just slow: keep waiting.

        uint32_t fill = audio->inBufferFilled();
        if (fill != streamLastFill) {
            streamLastFill = fill;
            streamWaitSince = millis();
        }
    }
    unsigned long stallLimit = STREAM_READY_TIMEOUT_MS << (streamStalls > 3 ? 3 : streamStalls);
    if (stallLimit > STREAM_READY_TIMEOUT_MAX_MS) stallLimit = STREAM_READY_TIMEOUT_MAX_MS;
    if (streamWaitingReady && streamActive && playState == PS_PLAYING && pending.type == PEND_NONE && millis() - streamWaitSince > stallLimit) {
        logLine("[PLAY] no sound or data for " + String(stallLimit / 1000) + " s, reconnecting " + streamUrl);
        streamStalls++;
        streamWaitingReady = false;
        pending.type = PEND_URL;
        pending.text = streamUrl;
        pending.vol = wantRadioVol;
        pending.resume = false;
        pending.force = true;
    }

    // Haengt das Laden (Verbindung, Download) laenger als LOAD_TIMEOUT_MS, wird abgebrochen und das Audio-Objekt neu aufgebaut.
    // If loading (connection, download) hangs longer than LOAD_TIMEOUT_MS, it is aborted and the audio object is rebuilt.

    if (playState == PS_LOADING && millis() - loadStartMillis > LOAD_TIMEOUT_MS) {
        releaseAudio();
        failPlayback("Timeout while loading");
    }
}

// Reisst ein Live-Webradio ab (WLAN weg, Server weg), wird es mit wachsendem Abstand neu verbunden, bis jemand stoppt.
// If a live web radio drops (WiFi gone, server gone), it is reconnected with growing delays until someone stops it.

void radioKeepAlive() {
    if (safeMode || wantRadioUrl.isEmpty() || !wantRadioLive) return;
    if (playState == PS_PLAYING && millis() - playStartMillis > RADIO_STABLE_MS) radioRetryDelay = RADIO_RETRY_START_MS;
    if (playState != PS_IDLE || !resumeUrl.isEmpty() || pending.type != PEND_NONE || WiFi.status() != WL_CONNECTED) {
        radioRetryAt = 0;
        return;
    }
    unsigned long now = millis();
    if (radioRetryAt == 0) {
        radioRetryAt = now + radioRetryDelay;
        return;
    }
    if (now < radioRetryAt) return;
    radioRetryAt = 0;
    radioRetryDelay = radioRetryDelay * 2 > RADIO_RETRY_MAX_MS ? RADIO_RETRY_MAX_MS : radioRetryDelay * 2;
    logLine("[PLAY] radio reconnect " + wantRadioUrl);
    pending.type = PEND_URL;
    pending.text = wantRadioUrl;
    pending.vol = wantRadioVol;
    pending.resume = false;
    pending.force = false;
}
// Ereignisse der Audio-Bibliothek (kommen aus audio->loop()): ins Log schreiben, Ende der Wiedergabe melden.
// Events of the audio library (they come from audio->loop()): write them to the log, report the end of playback.

// Titelregel: enthaelt der Titel des Webradios eines der eingestellten Stichworte, wird die Lautstaerke angehoben; beim naechsten Titel
// oder beim Ende des Senders gilt wieder die vorherige Lautstaerke. Stichworte stehen durch Komma oder Semikolon getrennt, jedes
// kann mit "=Wert" seine eigene Lautstaerke haben (zum Beispiel "Cambodia=90, Thunder"), sonst gilt die Lautstaerke der Regel.
// Title rule: if the title of the web radio contains one of the configured keywords, the volume is raised; with the next title or
// at the end of the station the previous volume applies again. Keywords are separated by comma or semicolon, each can have its
// own volume with "=value" (for example "Cambodia=90, Thunder"), otherwise the volume of the rule applies.

bool titleRuleMatch(const String& title, String& keywordOut, int& volOut) {
    String t = title;
    t.toLowerCase();
    String rest = cfg.titleMatch;
    while (rest.length() > 0) {
        int cut = -1;
        for (size_t i = 0; i < rest.length(); i++) {
            if (rest[i] == ',' || rest[i] == ';') {
                cut = (int)i;
                break;
            }
        }
        String token = cut >= 0 ? rest.substring(0, cut) : rest;
        rest = cut >= 0 ? rest.substring(cut + 1) : String();
        token.trim();
        int vol = cfg.titleVol;
        int eq = token.lastIndexOf('=');
        if (eq > 0 && eq < (int)token.length() - 1) {
            String num = token.substring(eq + 1);
            num.trim();
            bool digits = num.length() > 0;
            for (size_t i = 0; i < num.length(); i++) {
                if (num[i] < '0' || num[i] > '9') digits = false;
            }
            if (digits) {
                vol = constrain((int)num.toInt(), 0, VOLUME_MAX);
                token = token.substring(0, eq);
                token.trim();
            }
        }
        if (token.isEmpty()) continue;
        String k = token;
        k.toLowerCase();
        if (t.indexOf(k) >= 0) {
            keywordOut = token;
            volOut = vol;
            return true;
        }
    }
    return false;
}

void titleRule(const String& title) {
    if (mixAny()) return;
    String keyword;
    int vol = cfg.titleVol;
    bool hit = nowKind == "radio" && cfg.titleMatch.length() > 0 && title.length() > 0 && titleRuleMatch(title, keyword, vol);
    if (hit && !titleBoost) {
        titleBoost = true;
        titleBoostSince = millis();

        // Kommt der Titel schon beim Einblenden, ist currentVolume noch niedrig: dann gilt das Ziel des Einblendens als vorherige Lautstaerke.
        // If the title arrives during the fade-in, currentVolume is still low: then the target of the fade-in counts as the previous volume.

        titleSavedVol = (fadeInWaiting || fadeInActive) ? fadeInTarget : currentVolume;
        titleBoostVol = vol;
        applyVolumeSmooth(vol);
        logLine("[TITLE] \"" + keyword + "\" matched, volume " + String(titleSavedVol) + " -> " + String(vol));
        publishStatus(true);
    } else if (hit && titleBoost && vol != titleBoostVol) {
        titleBoostVol = vol;
        applyVolumeSmooth(vol);
        logLine("[TITLE] \"" + keyword + "\" matched, volume -> " + String(vol));
        publishStatus(true);
    } else if (!hit && titleBoost) {
        titleBoost = false;
        applyVolumeSmooth(titleSavedVol);
        logLine("[TITLE] title changed, volume back to " + String(titleSavedVol));
        publishStatus(true);
    }
}

void onAudioEvent(Audio::msg_t m) {
    if (!audioEvtQueue) return;
    AudioEvt e;
    e.type = (uint8_t)m.e;
    strlcpy(e.text, m.msg ? m.msg : "", sizeof(e.text));
    xQueueSend(audioEvtQueue, &e, 0);
}

void handleAudioEvent(uint8_t type, const String& text) {
    if (type == (uint8_t)Audio::evt_eof) {
        logLine("[AUDIO] end: " + text);
        setIdle();
    } else if (type == (uint8_t)Audio::evt_name) {
        nowName = text;
        publishStatus();
    } else if (type == (uint8_t)Audio::evt_streamtitle) {
        if (nowKind == "radio" && audio && titleQn < TITLE_Q_LEN) {
            uint32_t br = audio->getBitRate();
            if (br < 8000) br = 128000;
            unsigned long delayMs = (unsigned long)((uint64_t)audio->inBufferFilled() * 8000ULL / br) + 150;
            if (delayMs > 20000) delayMs = 20000;
            unsigned long due = millis() + delayMs;
            if (titleQn > 0 && (long)(due - titleQ[titleQn - 1].due) < 0) due = titleQ[titleQn - 1].due;
            titleQ[titleQn].text = text;
            titleQ[titleQn].due = due;
            titleQn++;
            logLine("[TITLE] \"" + text + "\" shown in " + String(delayMs) + " ms (buffered sound)");
        } else {
            nowTitle = text;
            titleRule(text);
            publishStatus();
        }
    } else if (type == (uint8_t)Audio::evt_genre) {
        if (nowKind == "radio") nowGenre = text;
    } else if (type == (uint8_t)Audio::evt_icydescription) {
        if (nowKind == "radio") nowDesc = text;
    } else if (type == (uint8_t)Audio::evt_icyurl) {
        if (nowKind == "radio") nowWeb = text;
    } else if (type == (uint8_t)Audio::evt_icylogo) {
        if (nowKind == "radio") nowLogo = text;
    } else if (type == (uint8_t)Audio::evt_lasthost) {
        if (nowKind == "radio") nowHost = text;
    } else if (type == (uint8_t)Audio::evt_bitrate) {
        if (nowKind == "radio") nowBitrate = (uint32_t)text.toInt();
    } else if (type == (uint8_t)Audio::evt_info || type == (uint8_t)Audio::evt_log) {
        if (text.indexOf("Stream lost") >= 0 && streamActive) {
            streamWaitingReady = true;
            streamWaitSince = millis();
        }
        if (text.indexOf("stream ready") >= 0 || text.indexOf("MP3Decoder has been initialized") >= 0) {
            streamWaitingReady = false;
            streamStalls = 0;
        }
        int lenAt = text.indexOf("Audio-Length:");
        if (lenAt >= 0 && nowKind == "radio") wantRadioLive = (text.substring(lenAt + 13).toInt() == 0);
        if (fadeInWaiting && text.indexOf("stream ready") >= 0) {
            fadeInWaiting = false;
            fadeInActive = true;
            fadeInStart = millis();
        }
        logLine("[AUDIO] " + text);
    }
}

// Arbeitet die Ereignisse der Audio-Bibliothek in der Hauptschleife ab (hoechstens 16 pro Durchlauf).
// Handles the events of the audio library in the main loop (at most 16 per pass).

void titleWatch() {

    // Ein Sender meldet das Ende eines Titels nicht; darum kann die Titelregel nach einer festen Zeit enden (Werbung nach dem Titel bleibt sonst laut).
    // A station does not report the end of a title; so the title rule can end after a fixed time (otherwise ads after the title stay loud).

    if (titleBoost && cfg.titleMaxS > 0 && millis() - titleBoostSince > (unsigned long)cfg.titleMaxS * 1000UL) {
        titleBoost = false;
        applyVolumeSmooth(titleSavedVol);
        logLine("[TITLE] time limit of " + String(cfg.titleMaxS) + " s reached, volume back to " + String(titleSavedVol));
        publishStatus(true);
    }
    if (titleQn == 0 || (long)(millis() - titleQ[0].due) < 0) return;
    String text = titleQ[0].text;
    for (int i = 1; i < titleQn; i++) titleQ[i - 1] = titleQ[i];
    titleQn--;
    titleQ[titleQn].text = "";
    if (nowKind != "radio") return;
    nowTitle = text;
    titleRule(text);
    publishStatus();
}

void audioEventPump() {
    if (!audioEvtQueue) return;
    AudioEvt e;
    for (int n = 0; n < 16 && xQueueReceive(audioEvtQueue, &e, 0) == pdTRUE; n++) handleAudioEvent(e.type, String(e.text));
}

// ---------------------------------------------------------------------------
// WLAN / WiFi

bool wifiHasCredentials() {
    for (int i = 0; i < 3; i++) {
        if (cfg.ssid[i].length() > 0) return true;
    }
    return false;
}

void startMdns() {
    MDNS.end();
    if (MDNS.begin(cfg.host.c_str())) {
        MDNS.addService("http", "tcp", 80);
        logLine("[mDNS] http://" + cfg.host + ".local/");
    }
}

bool connectWifi() {
    for (int i = 0; i < 3; i++) {
        if (cfg.ssid[i].isEmpty()) continue;
        logLine("[WiFi] Connect to: " + cfg.ssid[i]);
        WiFi.begin(cfg.ssid[i].c_str(), cfg.pass[i].c_str());
        unsigned long start = millis();
        while (millis() - start < WIFI_CONNECT_TIMEOUT_MS && WiFi.status() != WL_CONNECTED) {
            esp_task_wdt_reset();
            delay(250);
        }
        if (WiFi.status() == WL_CONNECTED) {
            WiFi.setSleep(false);
            logLine("[WiFi] Connected to: " + cfg.ssid[i]);
            logLine("[WiFi] IP address: " + WiFi.localIP().toString());
            startMdns();
            return true;
        }
        logLine("[WiFi] Connection failed or timed out");
        WiFi.disconnect();
    }
    return false;
}

void startAccessPoint() {
    String name = String(PROJECT_NAME) + "-setup-" + cfg.host.substring(cfg.host.length() > 6 ? cfg.host.length() - 6 : 0);
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP(name.c_str());
    apActive = true;
    logLine("[WiFi] Access point " + name + " started, http://192.168.4.1/");
}

// Ereignisse des WLAN-Treibers: hier nur Merker setzen, die Arbeit macht wifiWatch() in der Hauptschleife.
// Events of the WiFi driver: only set flags here, wifiWatch() does the work in the main loop.

void onWifiEvent(WiFiEvent_t event, WiFiEventInfo_t info) {
    if (event == ARDUINO_EVENT_WIFI_STA_GOT_IP) gotIpFlag = true;
    if (event == ARDUINO_EVENT_WIFI_STA_DISCONNECTED) lastDisconnectReason = info.wifi_sta_disconnected.reason;
}

int firstWifiSlot(int from) {
    for (int k = from; k < 3; k++) {
        if (cfg.ssid[k].length() > 0) return k;
    }
    return -1;
}

void startWifiAttempt(int idx) {
    wifiTryIdx = idx;
    wifiTryStart = millis();
    logLine("[WiFi] Connect to: " + cfg.ssid[idx]);
    WiFi.disconnect(false);
    WiFi.begin(cfg.ssid[idx].c_str(), cfg.pass[idx].c_str());
}

// Sofort neu versuchen, zum Beispiel nach geaenderten Zugangsdaten.
// Retry immediately, for example after changed credentials.

void wifiRetryNow() {
    wifiNextTry = 0;
    wifiTryIdx = -1;
    wifiBackoff = WIFI_BACKOFF_START_MS;
}

// Haelt das WLAN: der Treiber verbindet selbst neu; bleibt die Verbindung weg, probiert dies der Reihe nach alle gespeicherten
// Netze ohne die Hauptschleife zu blockieren, mit wachsendem Abstand. Nach WIFI_RESTART_AFTER_MS ohne WLAN: Neustart.
// Keeps the WiFi: the driver reconnects by itself; if the connection stays away, this tries all stored networks in turn
// without blocking the main loop, with growing delays. After WIFI_RESTART_AFTER_MS without WiFi: restart.

void wifiWatch() {
    unsigned long now = millis();
    if (WiFi.status() == WL_CONNECTED) {
        if (gotIpFlag || wifiLostAt != 0) {
            gotIpFlag = false;
            if (wifiLostAt != 0) {
                wifiReconnects++;
                logLine("[WiFi] reconnected after " + String((now - wifiLostAt) / 1000) + " s, IP " + WiFi.localIP().toString());
            }
            wifiLostAt = 0;
            wifiTryIdx = -1;
            wifiBackoff = WIFI_BACKOFF_START_MS;
            WiFi.setSleep(false);
            startMdns();

            // Nach dem Wiederkommen sofort MQTT neu verbinden.
            // Reconnect MQTT right away once the WiFi is back.

            lastMqttTry = 0;
            mqttBackoff = MQTT_BACKOFF_START_MS;
        }
        if (apActive) {
            WiFi.softAPdisconnect(true);
            WiFi.mode(WIFI_STA);
            apActive = false;
            logLine("[WiFi] Access point stopped");
        }
        return;
    }
    if (!wifiHasCredentials()) return;
    if (wifiLostAt == 0) {
        wifiLostAt = now;
        wifiNextTry = now + WIFI_FIRST_RETRY_MS;
        logLine("[WiFi] connection lost, reason " + String((int)lastDisconnectReason));
    }
    if (!apActive && now - wifiLostAt > WIFI_RESTART_AFTER_MS) {
        logLine("[WiFi] no connection for " + String((now - wifiLostAt) / 1000) + " s, restarting");
        delay(200);
        prefs.putBool("quiet", true);
        ESP.restart();
    }
    if (wifiTryIdx >= 0) {
        if (now - wifiTryStart < WIFI_ATTEMPT_TIMEOUT_MS) return;
        logLine("[WiFi] attempt to " + cfg.ssid[wifiTryIdx] + " timed out");
        int next = firstWifiSlot(wifiTryIdx + 1);
        if (next >= 0) {
            startWifiAttempt(next);
        } else {
            wifiTryIdx = -1;
            wifiNextTry = now + wifiBackoff;
            wifiBackoff = wifiBackoff * 2 > WIFI_BACKOFF_MAX_MS ? WIFI_BACKOFF_MAX_MS : wifiBackoff * 2;
        }
        return;
    }
    if (now >= wifiNextTry) startWifiAttempt(firstWifiSlot(0));
}
// Zugangsdaten per USB: Zeile "WIFI:<SSID><Tab><Passwort>"; nur wenn noch kein Passwort gespeichert ist.
// Credentials via USB: line "WIFI:<SSID><Tab><password>"; only while no password is stored yet.

void checkSerialWifi() {
    if (cfg.pass[0].length() > 0) return;
    unsigned long start = millis();
    String line;
    while (millis() - start < SERIAL_PROVISION_WINDOW_MS) {
        esp_task_wdt_reset();
        while (Serial.available()) {
            char c = (char)Serial.read();
            if (c != '\n') {
                if (c != '\r' && line.length() < 120) line += c;
                continue;
            }
            if (line.startsWith("WIFI:")) {
                int tab = line.indexOf('\t');
                String ssid = tab > 0 ? line.substring(5, tab) : line.substring(5);
                String pass = tab > 0 ? line.substring(tab + 1) : "";
                if (ssid.length() >= 1 && ssid.length() <= 32 && (pass.isEmpty() || (pass.length() >= 8 && pass.length() <= 63))) {
                    cfg.ssid[0] = ssid;
                    cfg.pass[0] = pass;
                    prefs.putString("ssid1", ssid);
                    prefs.putString("pass1", pass);
                    Serial.println("[PROV] ok");
                    logLine("[PROV] WiFi saved: " + ssid);
                    return;
                }
                Serial.println("[PROV] error");
            }
            line = "";
        }
        delay(20);
    }
}

// ---------------------------------------------------------------------------
// Befehle / commands

// Gemeinsame Befehle fuer REST-API und MQTT: pruefen, in die Warteschlange stellen; Rueckgabe leer = angenommen, sonst Fehlertext.
// Shared commands for the REST API and MQTT: validate, queue; empty return = accepted, otherwise the error text.

String commandSay(const String& textIn, const String& lang, const String& voice, int rate, int vol) {
    String text = textIn;
    text.trim();
    if (text.isEmpty()) return "Kein Text angegeben";
    if (text.length() > TTS_MAX_CHARS) return "Text zu lang (max. " + String(TTS_MAX_CHARS) + " Zeichen)";
    if (cfg.vkey.isEmpty()) return "Kein VoiceRSS-API-Schluessel gespeichert";
    Pending q;
    q.type = PEND_SAY;
    q.text = text;
    q.lang = lang;
    q.voice = voice;
    q.rate = rate;
    q.vol = vol;
    if (!annPush(q)) return "Warteschlange voll (max. " + String(ANN_QUEUE_LEN) + ")";
    return "";
}

String commandPlayUrl(const String& urlIn, int vol) {
    String url = urlIn;
    url.trim();
    if (!url.startsWith("http://") && !url.startsWith("https://")) return "URL muss mit http:// oder https:// beginnen";
    dirSess.active = false;
    pending.type = PEND_URL;
    pending.text = url;
    pending.vol = vol;
    pending.resume = false;
    pending.force = false;
    return "";
}

// Mischt die Dateien eines Ordners (Fisher-Yates mit dem Hardware-Zufallsgenerator). Die erste Datei ist nie die, mit der die letzte
// Sitzung dieses Ordners begann (avoid), und bei einer neuen Runde nie die zuletzt gespielte; so ist die Reihenfolge jedes Mal eine andere.
// Shuffles the files of a folder (Fisher-Yates with the hardware random generator). The first file is never the one the last session of this
// folder started with (avoid), and in a new round never the one played last; so the order is different every time.

void dirShuffle(std::vector<String>& v, const String& avoid) {
    for (size_t i = v.size(); i > 1; i--) {
        size_t j = esp_random() % i;
        std::swap(v[i - 1], v[j]);
    }
    if (v.size() > 1 && avoid.length() > 0 && v[0] == avoid) {
        size_t j = 1 + esp_random() % (v.size() - 1);
        std::swap(v[0], v[j]);
    }
}

// Beginnt eine Ordner-Sitzung (shuffle = zufaellige Reihenfolge, loop = endlos); die Dateien kommen einzeln in die Warteschlange.
// Starts a folder session (shuffle = random order, loop = endless); the files are put into the queue one at a time.

String commandPlayDir(const String& refIn, int vol, bool shuffle = false, bool loop = false) {
    String dir = refIn.substring(3);
    if (!dir.startsWith("/")) dir = "/" + dir;
    std::vector<SdEntry> list;
    if (!sdReady) return "SD-Karte nicht verfuegbar";
    if (!sdListDir(dir, list)) return "Ordner nicht gefunden";
    if (!dir.endsWith("/")) dir += "/";
    DirSession s;
    for (size_t i = 0; i < list.size(); i++) {
        if (!list[i].dir) s.names.push_back("sd:" + dir + list[i].name);
    }
    if (s.names.empty()) return "Keine MP3-Dateien im Ordner";
    s.active = true;
    s.shuffle = shuffle;
    s.loop = loop;
    s.vol = vol;
    s.dir = dir;
    if (shuffle) {
        String last = prefs.getString("dirfirst", "");
        String avoid = last.startsWith(dir + "|") ? last.substring(dir.length() + 1) : String();
        dirShuffle(s.names, avoid);
        prefs.putString("dirfirst", dir + "|" + s.names[0]);
    }
    logLine("[DIR] session " + dir + ": " + String(s.names.size()) + " files" + (shuffle ? ", shuffled, first " + s.names[0] : String()) + (loop ? ", loop" : String()));
    dirSess = s;
    return "";
}

// Gibt der Warteschlange die naechste Datei der Ordner-Sitzung, sobald nichts mehr wartet oder laeuft; Ansagen dazwischen kommen zuerst.
// Gives the queue the next file of the folder session as soon as nothing is waiting or running; announcements in between come first.

void dirFeed() {
    if (!dirSess.active || annCount > 0 || pending.type != PEND_NONE || announceBusy() || playState == PS_LOADING) return;
    if (millis() - dirSess.lastFeed < 400) return;
    if (!sdReady) {
        dirSess.active = false;
        return;
    }
    if (dirSess.pos >= dirSess.names.size()) {
        if (!dirSess.loop) {
            logLine("[DIR] session finished");
            dirSess.active = false;
            return;
        }
        dirSess.pos = 0;
        if (dirSess.shuffle) dirShuffle(dirSess.names, dirSess.lastPlayed);
        logLine("[DIR] new round" + String(dirSess.shuffle ? ", shuffled" : ""));
    }
    Pending q;
    q.type = PEND_FILE;
    q.text = dirSess.names[dirSess.pos++];
    q.vol = dirSess.vol;
    dirSess.lastPlayed = q.text;
    dirSess.lastFeed = millis();
    annPush(q);
}
String commandPlayFile(const String& nameIn, int vol, bool shuffle = false, bool loop = false) {
    String name = nameIn;
    name.trim();
    if (name.isEmpty()) return "Kein Dateiname angegeben";
    if (name.startsWith("sd:") && name.endsWith("/")) return commandPlayDir(name, vol, shuffle, loop);
    Pending q;
    q.type = PEND_FILE;
    q.text = name;
    q.vol = vol;
    if (!annPush(q)) return "Warteschlange voll (max. " + String(ANN_QUEUE_LEN) + ")";
    return "";
}

// Gespeicherten Platz abspielen: sel ist die Nummer 1 bis RADIO_SLOTS oder der Name (Gross-/Kleinschreibung egal).
// Play a stored slot: sel is the number 1 to RADIO_SLOTS or the name (case does not matter).

String commandPlayStation(const String& selIn, int vol) {
    String sel = selIn;
    sel.trim();
    if (sel.isEmpty()) return "Platznummer oder Name angeben";
    int slot = -1;
    if (sel.toInt() >= 1 && sel.toInt() <= RADIO_SLOTS && String(sel.toInt()) == sel) {
        slot = sel.toInt() - 1;
    } else {
        for (int i = 0; i < RADIO_SLOTS; i++) {
            if (cfg.radioName[i].length() > 0 && cfg.radioName[i].equalsIgnoreCase(sel)) slot = i;
        }
    }
    if (slot < 0) return "Platz nicht gefunden";
    if (cfg.radioUrl[slot].isEmpty()) return "Platz " + String(slot + 1) + " ist leer";
    return commandPlayUrl(cfg.radioUrl[slot], vol >= 0 ? vol : cfg.radioVol[slot]);
}

// Liste der gespeicherten Sender als JSON, fuer REST (/api/stations) und MQTT (.../stations).
// List of the stored stations as JSON, for REST (/api/stations) and MQTT (.../stations).

String stationsJson() {
    String j = "{\"stations\":[";
    bool first = true;
    for (int i = 0; i < RADIO_SLOTS; i++) {
        if (cfg.radioUrl[i].isEmpty()) continue;
        if (!first) j += ",";
        first = false;
        j += "{\"n\":" + String(i + 1) + ",\"name\":\"" + jsonEscape(cfg.radioName[i]) + "\",\"url\":\"" + jsonEscape(cfg.radioUrl[i]) + "\",\"vol\":" + (cfg.radioVol[i] >= 0 ? String(cfg.radioVol[i]) : String("null")) + "}";
    }
    return j + "]}";
}

void commandStop() {
    annClear();
    pending.type = PEND_STOP;
}

void commandVolume(int v, int ms = -1) {
    cfg.vol = constrain(v, 0, VOLUME_MAX);
    prefs.putInt("vol", cfg.vol);
    applyVolumeSmooth(cfg.vol, ms);
}

// Zahl aus Text; leer = Standardwert def.
// Number from text; empty = default value def.

int numberOr(const String& s, int def) {
    return s.isEmpty() ? def : (int)s.toInt();
}

// Schalter aus Text: 1, true, on oder yes.
// Switch from text: 1, true, on or yes.

bool flagOn(const String& s) {
    String t = s;
    t.trim();
    t.toLowerCase();
    return t == "1" || t == "true" || t == "on" || t == "yes";
}
// ---------------------------------------------------------------------------
// MQTT

String playStateName() {
    if (playState == PS_LOADING) return "loading";
    if (playState == PS_PLAYING) return "playing";
    return "idle";
}

void publishStatus(bool force) {
    String now = nowText();
    if (now != lastPublishedNow && mqttClient.connected()) {
        lastPublishedNow = now;
        mqttClient.publish((mqttBaseTopic() + "/now").c_str(), now.c_str(), true);
    }
    String state = playStateName();
    if (lastError.length() > 0 && playState == PS_IDLE) state = "error: " + lastError;
    if (!force && state == lastPublishedState) return;
    lastPublishedState = state;
    if (mqttClient.connected()) mqttClient.publish((mqttBaseTopic() + "/status").c_str(), state.c_str(), true);
}

// Befehle per MQTT: say (Text oder JSON), play (URL oder Dateiname, Text oder JSON), url, file, station (Platz oder Name), stop, volume. Fehler erscheinen in status.
// Commands via MQTT: say (text or JSON), play (URL or file name, text or JSON), url, file, station (slot or name), stop, volume. Errors show up in status.

void rejectCommand(const String& why) {
    lastError = why;
    logLine("[MQTT] rejected: " + why);
    publishStatus(true);
}

// Veroeffentlicht die Senderliste (retained) unter <Basis>/stations; auch nach jeder Aenderung der Liste.
// Publishes the station list (retained) under <base>/stations; also after every change of the list.

void publishStations() {
    if (!mqttClient.connected()) return;
    if (!mqttClient.publish((mqttBaseTopic() + "/stations").c_str(), stationsJson().c_str(), true)) logLine("[MQTT] station list not published (too large for the buffer?)");
}

// Meldet die Lautstaerken per MQTT (retained): <Basis>/volume/state = gerade wirkende, <Basis>/volume/general = allgemeine.
// Reports the volumes via MQTT (retained): <base>/volume/state = currently effective, <base>/volume/general = general.

void publishVolumes(bool force) {
    if (!mqttClient.connected()) return;
    if (force || currentVolume != lastPublishedVol) {
        lastPublishedVol = currentVolume;
        mqttClient.publish((mqttBaseTopic() + "/volume/state").c_str(), String(currentVolume).c_str(), true);
    }
    if (force || cfg.vol != lastPublishedGeneral) {
        lastPublishedGeneral = cfg.vol;
        mqttClient.publish((mqttBaseTopic() + "/volume/general").c_str(), String(cfg.vol).c_str(), true);
    }
}

void onMqttMessage(char* topic, uint8_t* payload, unsigned int length) {
    String t = String(topic);
    String body;
    body.reserve(length);
    for (unsigned int i = 0; i < length; i++) body += (char)payload[i];
    body.trim();
    bool isJson = body.startsWith("{");
    String base = mqttBaseTopic();
    String err;

    // Befehle an ein Gruppen-Topic gelten wie Befehle an das eigene Basis-Topic.
    // Commands sent to a group topic count like commands sent to the own base topic.

    if (!t.startsWith(base + "/")) {
        for (int i = 0; i < 5; i++) {
            String g = mqttGroupTopic(i);
            if (g.length() > 0 && t.startsWith(g + "/")) {
                t = base + t.substring(g.length());
                break;
            }
        }
    }
    if (t == base + "/say") {
        if (isJson) err = commandSay(jsonGet(body, "text"), jsonGet(body, "lang"), jsonGet(body, "voice"), numberOr(jsonGet(body, "rate"), 99), numberOr(jsonGet(body, "vol"), -1));
        else err = commandSay(body, "", "", 99, -1);
    } else if (t == base + "/play" || t == base + "/url" || t == base + "/file") {
        String url = body, file = body;
        int v = -1;
        bool shuffle = false, loop = false;
        if (isJson) {
            url = jsonGet(body, "url");
            file = jsonGet(body, "file");
            v = numberOr(jsonGet(body, "vol"), -1);
            shuffle = flagOn(jsonGet(body, "shuffle"));
            loop = flagOn(jsonGet(body, "loop"));
        }
        bool wantUrl = t == base + "/url" || (t == base + "/play" && (url.startsWith("http://") || url.startsWith("https://")));
        if (t == base + "/file") wantUrl = false;
        if (wantUrl) err = commandPlayUrl(url, v);
        else err = commandPlayFile(file, v, shuffle, loop);
    } else if (t == base + "/station") {
        if (isJson) err = commandPlayStation(jsonGet(body, "name").length() > 0 ? jsonGet(body, "name") : jsonGet(body, "n"), numberOr(jsonGet(body, "vol"), -1));
        else err = commandPlayStation(body, -1);
    } else if (t == base + "/list") {
        publishStations();
    } else if (t == base + "/stop") {
        commandStop();
    } else if (t == base + "/volume") {
        if (isJson) commandVolume(numberOr(jsonGet(body, "v"), cfg.vol), numberOr(jsonGet(body, "ms"), -1));
        else commandVolume(body.toInt());
    } else if (t == base + "/restart") {
        // Eine beim Verbinden ausgelieferte (retained) Nachricht wuerde sonst eine Neustartschleife ausloesen.
        // A (retained) message delivered right after connecting would otherwise cause a restart loop.

        if (millis() - mqttSubscribedAt < MQTT_RESTART_IGNORE_MS) err = "restart ignored right after connecting (retained?)";
        else {
            logLine("[MQTT] restart requested");
            restartAt = millis() + 300;
        }
    }
    if (err.length() > 0) rejectCommand(err);
}

// Startet das Geraet neu, sobald der per MQTT angeforderte Zeitpunkt erreicht ist (die Antwort geht vorher raus).
// Restarts the device once the time requested via MQTT is reached (the reply goes out first).

void restartWatch() {
    if (restartAt != 0 && (long)(millis() - restartAt) >= 0) {
        if (mqttClient.connected()) mqttClient.disconnect();
        prefs.putBool("quiet", true);
        ESP.restart();
    }
}
// Adresse des Brokers einmal aufloesen und merken; ein Hostname wird nur bei Bedarf neu aufgeloest (DNS kann blockieren).
// Resolve the broker address once and remember it; a host name is only resolved again when needed (DNS can block).

bool resolveBroker() {
    if (mqttIpValid && mqttIpFor == cfg.mqttServer) return true;
    IPAddress ip;
    if (!ip.fromString(cfg.mqttServer)) {
        if (!WiFi.hostByName(cfg.mqttServer.c_str(), ip)) return false;
    }
    mqttIp = ip;
    mqttIpFor = cfg.mqttServer;
    mqttIpValid = true;
    return true;
}

void mqttFailed(const String& why) {
    logLine("[MQTT] " + why);
    mqttIpValid = false;
    mqttBackoff = mqttBackoff * 2 > MQTT_BACKOFF_MAX_MS ? MQTT_BACKOFF_MAX_MS : mqttBackoff * 2;
}

// Verbindungsaufbau mit kurzem TCP-Zeitlimit, damit die Hauptschleife (und damit der Ton) hoechstens kurz stockt.
// Connecting with a short TCP timeout so the main loop (and with it the sound) only stalls briefly.

void mqttConnect() {
    if (!resolveBroker()) {
        mqttFailed("cannot resolve " + cfg.mqttServer);
        return;
    }
    mqttClient.setServer(mqttIp, cfg.mqttPort);
    mqttNet.setConnectionTimeout(MQTT_TCP_TIMEOUT_MS);
    if (!mqttNet.connected() && !mqttNet.connect(mqttIp, cfg.mqttPort)) {
        mqttFailed("broker " + mqttIp.toString() + " not reachable");
        return;
    }
    String base = mqttBaseTopic();
    bool withLogin = cfg.mqttUser.length() > 0;
    if (mqttClient.connect(defaultHost().c_str(), withLogin ? cfg.mqttUser.c_str() : nullptr, withLogin ? cfg.mqttPass.c_str() : nullptr, (base + "/availability").c_str(), 0, true, "offline")) {
        mqttEverConnected = true;
        mqttBackoff = MQTT_BACKOFF_START_MS;
        if (mqttWasConnected) mqttReconnects++;
        mqttWasConnected = true;
        mqttClient.publish((base + "/availability").c_str(), "online", true);
        mqttClient.subscribe((base + "/say").c_str());
        mqttClient.subscribe((base + "/play").c_str());
        mqttClient.subscribe((base + "/url").c_str());
        mqttClient.subscribe((base + "/file").c_str());
        mqttClient.subscribe((base + "/station").c_str());
        mqttClient.subscribe((base + "/list").c_str());
        mqttClient.subscribe((base + "/stop").c_str());
        mqttClient.subscribe((base + "/volume").c_str());
        mqttClient.subscribe((base + "/restart").c_str());
        for (int i = 0; i < 5; i++) {
            String g = mqttGroupTopic(i);
            bool again = false;
            for (int j = 0; j < i; j++) {
                if (g == mqttGroupTopic(j)) again = true;
            }
            if (g.length() > 0 && !again) {
                mqttClient.subscribe((g + "/#").c_str());
                logLine("[MQTT] Group topic " + g + "/#");
            }
        }
        mqttSubscribedAt = millis();
        logLine("[MQTT] Connected to " + cfg.mqttServer + ":" + String(cfg.mqttPort) + ", base " + base);
        publishStatus(true);
        publishStations();
        publishVolumes(true);
    } else {
        mqttFailed("connect failed, rc=" + String(mqttClient.state()));
    }
}

void mqttReconnectNow() {
    if (mqttClient.connected()) mqttClient.disconnect();
    mqttIpValid = false;
    mqttBackoff = MQTT_BACKOFF_START_MS;
    lastMqttTry = 0;
}

void mqttWatch() {
    if (safeMode || cfg.mqttServer.isEmpty() || WiFi.status() != WL_CONNECTED) return;
    if (mqttClient.connected()) {
        mqttClient.loop();

        // Erst veroeffentlichen, wenn die Lautstaerke 0,4 s ruhig ist (beim Ein- und Ausblenden aendert sie sich laufend).
        // Publish only when the volume has been quiet for 0.4 s (it changes continuously while fading).

        if (millis() - volChangedAt > 400) publishVolumes(false);
        return;
    }
    if (mqttWasConnected) {
        mqttWasConnected = false;
        mqttNet.stop();
        lastMqttTry = 0;
        mqttBackoff = MQTT_BACKOFF_START_MS;
        logLine("[MQTT] connection lost, rc=" + String(mqttClient.state()));
    }

    // Beim Abspielen seltener versuchen, damit jeder Versuch den Ton moeglichst selten stoeren kann.
    // Try less often while playing so an attempt rarely disturbs the sound.

    unsigned long interval = mqttBackoff;
    if (playState == PS_PLAYING && interval < MQTT_BACKOFF_PLAYING_MS) interval = MQTT_BACKOFF_PLAYING_MS;
    if (lastMqttTry != 0 && millis() - lastMqttTry < interval) return;
    lastMqttTry = millis();
    if (lastMqttTry == 0) lastMqttTry = 1;
    mqttConnect();
}
// ---------------------------------------------------------------------------
// Zeit und Statusanzeigen / time and status indicators

// Bis zu fuenf NTP-Server; die Netzwerk-Bibliothek fragt aber nur drei gleichzeitig. Es gelten die ersten drei belegten; kommt nach 45 s
// keine Zeit an, wechselt ntpWatch() zu den naechsten drei (der Reihe nach, mit Wiederholung).
// Up to five NTP servers; the network library only asks three at a time. The first three used ones apply; if no time arrives after 45 s,
// ntpWatch() switches to the next three (in turn, wrapping around).

int ntpOffset = 0;

int ntpCount() {
    int n = 0;
    for (int i = 0; i < 5; i++) {
        if (!cfg.ntp[i].isEmpty()) n++;
    }
    return n;
}

void ntpApply() {
    String list[5];
    int n = 0;
    for (int i = 0; i < 5; i++) {
        if (!cfg.ntp[i].isEmpty()) list[n++] = cfg.ntp[i];
    }
    if (n == 0) list[n++] = "pool.ntp.org";
    int m = n < 3 ? n : 3;
    String s[3];
    for (int i = 0; i < m; i++) s[i] = list[(ntpOffset + i) % n];
    configTzTime(cfg.tz.c_str(), s[0].c_str(), m > 1 ? s[1].c_str() : nullptr, m > 2 ? s[2].c_str() : nullptr);
}

void setupTime() {
    ntpOffset = 0;
    ntpApply();
}

void ntpWatch() {
    static unsigned long since = 0;
    if (timeIsValid() || WiFi.status() != WL_CONNECTED) {
        since = 0;
        return;
    }
    if (since == 0) {
        since = millis();
        return;
    }
    int n = ntpCount();
    if (n <= 3 || millis() - since < 45000UL) return;
    since = millis();
    ntpOffset = (ntpOffset + 3) % n;
    ntpApply();
    logLine("[TIME] no time yet, trying the next NTP servers (from number " + String(ntpOffset + 1) + ")");
}

// Zustand der Punkte in der Statusleiste: "ok", "syncing" (gelb blinkend), "playing" (gruen blinkend, nur Audio), "idle" (grau, nur Audio), "bad" oder "na".
// State of the dots in the status bar: "ok", "syncing" (blinking amber), "playing" (blinking green, audio only), "idle" (gray, audio only), "bad" or "na".

String audioDotState() {
    if (playState == PS_PLAYING) return "playing";
    if (playState == PS_LOADING) return "syncing";
    if (lastError.length() > 0) return "bad";
    return "idle";
}

String voiceDotState() {
    if (voiceBusy) return "syncing";
    if (!voiceEver) return "na";
    return voiceLastOk ? "ok" : "bad";
}

String mqttDotState() {
    return mqttClient.connected() ? "ok" : "bad";
}

String timeDotState() {
    return timeIsValid() ? "ok" : "na";
}

// Zustand nach einem Neustart: Mit dem Haken "zuletzt gespielten Sender fortsetzen" merkt sich das Geraet den laufenden Sender und seine
// Lautstaerke (erst wenn sie 3 s ruhig ist) und spielt ihn nach dem Start wieder ab; ein Stopp loescht den Merker.
// State after a restart: with the checkbox "resume the last played station" the device remembers the running station and its volume
// (once it has been quiet for 3 s) and plays it again after the start; a stop deletes the memory.

void saveRadioState(const String& url, int vol) {
    if (url == cfg.lastUrl && vol == cfg.lastVol) return;
    cfg.lastUrl = url;
    cfg.lastVol = vol;
    prefs.putString("rsurl", url);
    prefs.putInt("rsvol", vol);
}

void resumeStateWatch() {
    static unsigned long lastCheck = 0;
    if (millis() - lastCheck < 2000) return;
    lastCheck = millis();
    if (!cfg.resumeLast) return;
    if (streamActive && nowKind == "radio" && playState == PS_PLAYING && !fadeInWaiting && !fadeInActive && !mixDucked && !titleBoost && millis() - volChangedAt > 3000 && wantRadioUrl == streamUrl) {
        saveRadioState(streamUrl, currentVolume);
    }
}

// ---------------------------------------------------------------------------
// Autostart: Sender 1 der Liste abspielen, wenn das Haekchen gesetzt ist (Standard: aus). Das Geraet startet sonst immer stumm.
// Autostart: play station 1 of the list if the checkbox is set (default: off). Otherwise the device always starts silent.

bool autostartDone = false;
unsigned long autostartNextTry = 0;
int autostartTries = 0;
unsigned long autostartWifiSince = 0;

// Prueft kurz, ob der Stream antwortet (HTTP-Kopf, Weiterleitungen werden verfolgt); der Ton wird dabei nicht gelesen.
// Briefly checks whether the stream answers (HTTP header, redirects are followed); the audio is not read.

bool probeStream(const String& url) {
    HTTPClient http;
    WiFiClient plain;
    WiFiClientSecure secure;
    secure.setInsecure();
    bool began = url.startsWith("https://") ? http.begin(secure, url) : http.begin(plain, url);
    if (!began) return false;
    http.setConnectTimeout(3000);
    http.setTimeout(3000);
    http.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
    http.setRedirectLimit(3);
    esp_task_wdt_reset();
    int code = http.GET();
    http.end();
    esp_task_wdt_reset();
    return code >= 200 && code < 300;
}

void autostartWatch() {
    if ((!cfg.autoStart && !cfg.resumeLast) || autostartDone || safeMode) return;
    if (WiFi.status() != WL_CONNECTED) {
        autostartWifiSince = 0;
        return;
    }
    unsigned long now = millis();
    if (autostartWifiSince == 0) {
        autostartWifiSince = now;
        autostartNextTry = now + AUTOSTART_DELAY_MS;
        return;
    }

    // Hat jemand inzwischen selbst etwas gestartet, entfaellt der Autostart.
    // If somebody has started something in the meantime, the autostart is dropped.

    if (playState != PS_IDLE || pending.type != PEND_NONE || wantRadioUrl.length() > 0) {
        autostartDone = true;
        logLine("[AUTOSTART] skipped, something was started already");
        return;
    }
    if (now < autostartNextTry) return;
    String asUrl = cfg.radioUrl[0];
    int asVol = cfg.radioVol[0];
    String asName = "station 1: " + cfg.radioName[0];
    if (cfg.resumeLast && cfg.lastUrl.length() > 0) {
        asUrl = cfg.lastUrl;
        asVol = cfg.lastVol;
        asName = "the last played station";
    } else if (!cfg.autoStart) {
        autostartDone = true;
        return;
    }
    if (asUrl.isEmpty()) {
        autostartDone = true;
        logLine("[AUTOSTART] station 1 is empty, nothing to play");
        return;
    }
    if (probeStream(asUrl)) {
        autostartDone = true;
        logLine("[AUTOSTART] stream ok, playing " + asName);
        commandPlayUrl(asUrl, asVol);
        return;
    }
    autostartTries++;
    logLine("[AUTOSTART] stream not ready (try " + String(autostartTries) + " of " + String(AUTOSTART_MAX_TRIES) + ")");
    if (autostartTries >= AUTOSTART_MAX_TRIES) {
        autostartDone = true;
        logLine("[AUTOSTART] given up");
        return;
    }
    autostartNextTry = millis() + AUTOSTART_RETRY_MS;
}

// ---------------------------------------------------------------------------
// Selbstschutz / self protection

String resetReasonName() {
    switch (esp_reset_reason()) {
        case ESP_RST_POWERON: return "power-on";
        case ESP_RST_SW: return "software restart";
        case ESP_RST_PANIC: return "panic (crash)";
        case ESP_RST_INT_WDT: return "interrupt watchdog";
        case ESP_RST_TASK_WDT: return "task watchdog";
        case ESP_RST_WDT: return "other watchdog";
        case ESP_RST_BROWNOUT: return "brownout";
        case ESP_RST_DEEPSLEEP: return "deep sleep";
        case ESP_RST_USB: return "usb reset";
        default: return "other (" + String((int)esp_reset_reason()) + ")";
    }
}

bool isCrashReset() {
    esp_reset_reason_t r = esp_reset_reason();
    return r == ESP_RST_PANIC || r == ESP_RST_INT_WDT || r == ESP_RST_TASK_WDT || r == ESP_RST_WDT;
}

// Nach STABLE_BOOT_MS ohne Absturz gilt der Start als gut (Zaehler zurueck). Bleibt der freie Speicher lange zu knapp: Neustart.
// After STABLE_BOOT_MS without a crash the start counts as good (counter reset). If free memory stays too low for long: restart.

void healthWatch() {
    unsigned long now = millis();
    if (!bootMarkedStable && now > STABLE_BOOT_MS) {
        bootMarkedStable = true;
        if (bootCrashes > 0) {
            bootCrashes = 0;
            prefs.putInt("crashes", 0);
            logLine("[BOOT] stable, crash counter reset");
        }
    }
    if (ESP.getFreeHeap() < HEAP_LOW_BYTES) {
        if (heapLowSince == 0) heapLowSince = now;
        if (now - heapLowSince > HEAP_LOW_RESTART_MS) {
            logLine("[HEALTH] free heap below " + String(HEAP_LOW_BYTES) + " bytes for " + String(HEAP_LOW_RESTART_MS / 1000) + " s, restarting");
            delay(200);
            prefs.putBool("quiet", true);
            ESP.restart();
        }
    } else {
        heapLowSince = 0;
    }
}

// ---------------------------------------------------------------------------
// Web-Oberflaeche / web interface

#include "web_ui.h"

// ---------------------------------------------------------------------------
// Arduino

// Board-LED leuchtet, solange eine Ansage geladen oder gesprochen wird (RGB-LED des Boards, sonst einfache LED).
// Board LED is lit while an announcement is being fetched or spoken (RGB LED of the board, otherwise a plain LED).

// Das Schreiben des Logs in die Datei schaltet sich nach logAutoMin Minuten selbst aus (0 = nie), damit der Flash geschont wird. Die Zeit
// laeuft ab dem Einschalten oder ab dem Start des Geraets.
// Writing the log to the file switches itself off after logAutoMin minutes (0 = never) to spare the flash. The time runs from switching on or
// from the start of the device.

void logWatch() {
    static bool was = false;
    static unsigned long since = 0;
    if (!cfg.logEnabled) {
        was = false;
        return;
    }
    if (!was) {
        was = true;
        since = millis();
        return;
    }
    if (cfg.logAutoMin > 0 && millis() - since > (unsigned long)cfg.logAutoMin * 60000UL) {
        logLine("[LOG] file logging switched off automatically after " + String(cfg.logAutoMin) + " min");
        cfg.logEnabled = false;
        prefs.putBool("log", false);
        was = false;
    }
}

void ledWatch() {
    static int last = -1;
    int on = (voiceBusy || mixAny() || (nowKind == "speech" && playState != PS_IDLE)) ? 1 : 0;
    if (on == last) return;
    last = on;
#if defined(RGB_BUILTIN)
    neopixelWrite(RGB_BUILTIN, on ? 0 : 0, on ? 40 : 0, on ? 40 : 0);
#elif defined(LED_BUILTIN)
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, on ? HIGH : LOW);
#endif
}

// Kurzer Ton "buub" beim Start, direkt ueber den I2S-Treiber (die Audio-Bibliothek wird erst bei Bedarf angelegt). Er ist so laut wie die
// allgemeine Lautstaerke; nach einem Absturz, im abgesicherten Modus und nach einem Neustart per MQTT bleibt er aus.
// Short tone "buub" at the start, directly through the I2S driver (the audio library is only created when needed). It is as loud as the
// general volume; after a crash, in safe mode and after a restart via MQTT it stays off.

void bootTone() {
    int vol = cfg.vol;
    if (vol <= 0) return;
    int bclk = pinAllowed(cfg.bclk) ? cfg.bclk : DEFAULT_PIN_BCLK;
    int lrc = pinAllowed(cfg.lrc) ? cfg.lrc : DEFAULT_PIN_LRC;
    int din = pinAllowed(cfg.din) ? cfg.din : DEFAULT_PIN_DIN;
    i2s_chan_handle_t tx = nullptr;
    i2s_chan_config_t cc = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);
    cc.auto_clear = true;
    if (i2s_new_channel(&cc, &tx, nullptr) != ESP_OK) return;
    i2s_std_config_t sc = {};
    sc.clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(22050);
    sc.slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO);
    sc.gpio_cfg.mclk = I2S_GPIO_UNUSED;
    sc.gpio_cfg.bclk = (gpio_num_t)bclk;
    sc.gpio_cfg.ws = (gpio_num_t)lrc;
    sc.gpio_cfg.dout = (gpio_num_t)din;
    sc.gpio_cfg.din = I2S_GPIO_UNUSED;
    if (i2s_channel_init_std_mode(tx, &sc) != ESP_OK || i2s_channel_enable(tx) != ESP_OK) {
        i2s_del_channel(tx);
        return;
    }

    // Zwei kurze, abfallende Toene (hoch-tief), je mit weichem Ein- und Ausklang; die Amplitude folgt der Lautstaerkekurve, hoechstens halbe Aussteuerung.
    // Two short falling tones (high-low), each with a soft attack and release; the amplitude follows the volume curve, at most half scale.

    float amp = (float)mixGainFor(vol) / 32768.0f * 0.5f;
    struct Seg {
        float f0, f1;
        int ms;
    };
    const Seg segs[] = {{300.0f, 240.0f, 130}, {0.0f, 0.0f, 40}, {220.0f, 150.0f, 200}};
    const float rate = 22050.0f;
    int16_t buf[256 * 2];
    float phase = 0;
    for (size_t s = 0; s < sizeof(segs) / sizeof(segs[0]); s++) {
        int total = (int)(rate * segs[s].ms / 1000.0f);
        int done = 0;
        while (done < total) {
            int n = total - done > 256 ? 256 : total - done;
            for (int i = 0; i < n; i++) {
                float x = (float)(done + i) / (float)total;
                float v = 0;
                if (segs[s].f0 > 0) {
                    float f = segs[s].f0 + (segs[s].f1 - segs[s].f0) * x;
                    phase += 2.0f * 3.14159265f * f / rate;
                    if (phase > 6.2831853f) phase -= 6.2831853f;
                    float env = x < 0.1f ? x / 0.1f : (x > 0.6f ? (1.0f - x) / 0.4f : 1.0f);
                    v = (sinf(phase) + 0.25f * sinf(2.0f * phase)) / 1.25f * env * amp;
                }
                int16_t o = (int16_t)(v * 32767.0f);
                buf[2 * i] = o;
                buf[2 * i + 1] = o;
            }
            size_t written = 0;
            i2s_channel_write(tx, buf, n * 4, &written, 1000);
            done += n;
        }
    }
    memset(buf, 0, sizeof(buf));
    size_t written = 0;
    i2s_channel_write(tx, buf, sizeof(buf), &written, 1000);
    i2s_channel_disable(tx);
    i2s_del_channel(tx);
    logLine("[BOOT] start tone played");
}

void setup() {
    Serial.begin(115200);

    // Ohne angeschlossenen Monitor darf die serielle Ausgabe nicht blockieren, sonst steht die Hauptschleife und der Ton bleibt aus.
    // Without a connected monitor the serial output must not block, otherwise the main loop stalls and there is no sound.

    Serial.setTxTimeoutMs(0);
    delay(1500);
    Serial.println("[BOOT] start, reset reason " + resetReasonName());
    logMutex = xSemaphoreCreateMutex();
    sdMutex = xSemaphoreCreateMutex();
    audioEvtQueue = xQueueCreate(AUDIO_EVT_QUEUE_LEN, sizeof(AudioEvt));
    fsReady = LittleFS.begin(true);
    if (fsReady) LittleFS.mkdir(MP3_DIR);
    loadSettings();
    loadPinActions();
    logLine("[BOOT] " PROJECT_TITLE " " + String(FIRMWARE_MARKER + FIRMWARE_MARKER_PREFIX_LEN));
    bootCrashes = prefs.getInt("crashes", 0);
    if (isCrashReset()) {
        bootCrashes++;
        prefs.putInt("crashes", bootCrashes);
    }
    safeMode = bootCrashes >= CRASH_LIMIT;
    logLine("[BOOT] reset reason: " + resetReasonName() + ", crashes in a row: " + String(bootCrashes) + (safeMode ? ", SAFE MODE (no MQTT, no radio reconnect)" : ""));
    logLine("[BOOT] chip " + String(ESP.getChipModel()) + ", PSRAM " + String(ESP.getPsramSize()) + " bytes, LittleFS " + (fsReady ? "ok" : "failed"));

#if ESP_ARDUINO_VERSION_MAJOR >= 3
    esp_task_wdt_config_t wdtConfig = {.timeout_ms = WDT_TIMEOUT_S * 1000, .idle_core_mask = 0, .trigger_panic = true};
    esp_task_wdt_reconfigure(&wdtConfig);
#else
    esp_task_wdt_init(WDT_TIMEOUT_S, true);
#endif
    esp_task_wdt_add(NULL);

    bool quietStart = prefs.getBool("quiet", false);
    if (quietStart) prefs.putBool("quiet", false);
    if (cfg.bootTone && !quietStart && !safeMode && !isCrashReset()) bootTone();
    else logLine("[BOOT] start tone skipped");

    checkSerialWifi();
    WiFi.setHostname(cfg.host.c_str());
    WiFi.persistent(false);
    WiFi.setAutoReconnect(true);
    WiFi.onEvent(onWifiEvent);
    WiFi.mode(WIFI_STA);
    if (!wifiHasCredentials() || !connectWifi()) startAccessPoint();
    gotIpFlag = false;

    // Der erste Namensabruf nach dem Verbinden leert den DNS-Cache des Stacks; liefe da schon eine Zeitserver-Abfrage, stuerzt das Geraet ab
    // (assert in sys_untimeout). Deshalb jetzt einmal abrufen, bevor die Zeitabfrage startet.
    // The first name lookup after connecting clears the DNS cache of the stack; if a time server lookup were already running the device
    // would crash (assert in sys_untimeout). So do one lookup now, before the time query starts.

    IPAddress warmUp;
    if (WiFi.status() == WL_CONNECTED) WiFi.hostByName("localhost", warmUp);
    setupTime();
    setupWeb();
    sdMount();


    applyVolume(cfg.vol);
    logLine("[AUDIO] I2S pins BCLK=" + String(cfg.bclk) + " LRC=" + String(cfg.lrc) + " DIN=" + String(cfg.din) + ", free heap " + String(ESP.getFreeHeap()));

    mqttClient.setBufferSize(8192);
    mqttClient.setSocketTimeout(1);
    mqttClient.setKeepAlive(MQTT_KEEPALIVE_S);
    mqttClient.setCallback(onMqttMessage);
    logLine("[BOOT] ready");
}

void loop() {
    esp_task_wdt_reset();
    if (audio) audio->loop();
    audioEventPump();
    titleWatch();
    fadeInStep();
    webserver.handleClient();
    dirFeed();
    processQueue();
    processPending();
    playbackWatch();
    radioKeepAlive();
    wifiWatch();
    mqttWatch();
    restartWatch();
    healthWatch();
    ledWatch();
    autostartWatch();
    mixWatch();
    fileFadeWatch();
    loudWatch();
    pinWatch();
    logWatch();
    ntpWatch();
    resumeStateWatch();
    delay(2);
}
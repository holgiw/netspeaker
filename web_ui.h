#pragma once

// Web-Oberflaeche: eine Hub-Seite mit Tabs, Statusleiste, JSON-API, Datei-Upload, Export/Import.
// Web interface: one hub page with tabs, status bar, JSON API, file upload, export/import.

// ---------------------------------------------------------------------------
// Stil, Seitenkopf / style, page head

static const char PAGE_CSS[] PROGMEM = R"css(
:root{--bg:#10151a;--panel:#1a2129;--panel-border:#2a333c;--text:#e8edf2;--muted:#8f9ba7;--accent:#f5a623;--accent-dim:#7a530f;--ok:#3ddc84;--bad:#ff5c5c;--sans:system-ui,-apple-system,"Segoe UI",Roboto,sans-serif;--mono:ui-monospace,"SFMono-Regular",Menlo,Consolas,monospace;}
body{font-family:Arial,Helvetica,sans-serif;text-align:center;margin:0;background:var(--bg);color:var(--text);}
.page{padding:0 8px 40px;}
input,select,button,textarea{margin:10px;padding:10px;width:80%;box-sizing:border-box;background:#0d1216;color:var(--text);border:1px solid var(--panel-border);border-radius:6px;font-family:inherit;}
input[type=checkbox],input[type=radio]{width:auto;}
button,button[type=submit],input[type=submit]{background:var(--accent);color:#1a1200;font-weight:bold;border:none;cursor:pointer;}
button:hover,input[type=submit]:hover{background:#ffb84d;}
h1,h2,h3{color:var(--text);}
hr{border:0;height:1px;background-color:var(--panel-border);margin:20px 0;}
table{margin:auto;border-collapse:collapse;}
th,td{padding:8px 10px;text-align:left;border:1px solid var(--panel-border);}
a{color:var(--accent);}
small{color:var(--muted);}
.topbar{display:flex;flex-wrap:wrap;align-items:center;gap:.6rem 1.2rem;padding:.9rem 1rem;border-bottom:1px solid var(--panel-border);position:sticky;top:0;background:var(--bg);z-index:5;}
.brand{display:flex;align-items:baseline;gap:.5rem;margin-right:auto;}
.brand-mark{font-family:var(--mono);color:var(--accent);font-size:.8rem;letter-spacing:.05em;}
.topbar h1{font-size:1.15rem;margin:0;letter-spacing:.02em;}
h1 a.host-link{color:inherit;text-decoration:none;border-bottom:1px dashed var(--muted);}
h1 a.host-link:hover{color:var(--accent);border-bottom-color:var(--accent);}
.ip-hint{font-family:var(--mono);font-size:.7rem;color:var(--muted);}
.ip-hint a{color:inherit;font-size:inherit;text-decoration:none;border-bottom:1px dashed var(--muted);}
.ip-hint a:hover{color:var(--accent);border-bottom-color:var(--accent);}
.status-strip{display:flex;gap:.9rem;flex-wrap:wrap;}
.status{display:flex;align-items:center;gap:.4rem;font-size:.8rem;color:var(--muted);}
.status[hidden]{display:none;}
.statval{font-family:var(--mono);color:var(--text);}
.dot{width:.55rem;height:.55rem;border-radius:50%;background:var(--bad);box-shadow:0 0 0 3px rgba(255,92,92,.15);}
.dot.ok{background:var(--ok);box-shadow:0 0 0 3px rgba(61,220,132,.18);}
.dot.na,.dot.idle{background:var(--muted);box-shadow:0 0 0 3px rgba(143,155,167,.18);}
.dot.syncing{background:var(--accent);box-shadow:0 0 0 3px rgba(245,166,35,.18);animation:dotBlink 1s ease-in-out infinite;}
.dot.playing{background:var(--ok);box-shadow:0 0 0 3px rgba(61,220,132,.18);animation:dotBlink 1s ease-in-out infinite;}
@keyframes dotBlink{0%,100%{opacity:1;}50%{opacity:.25;}}
@media (prefers-reduced-motion:reduce){.dot.syncing,.dot.playing{animation:none;opacity:.6;}}
.datetime{font-family:var(--mono);font-size:.8rem;color:var(--muted);}
.offline-hint{display:none;color:var(--bad);font-size:.75rem;margin-left:.3rem;}
.offline-hint.show{display:inline;}
.refresh-btn{display:inline-flex;align-items:center;justify-content:center;box-sizing:border-box;background:transparent;border:1px solid var(--panel-border);color:var(--text);border-radius:.4rem;width:2rem;height:2rem;padding:0;margin:0;font-size:1rem;line-height:1;cursor:pointer;text-decoration:none;}
.refresh-btn:hover{background:transparent;border-color:var(--accent);color:var(--accent);}
.refresh-btn:focus-visible{outline:2px solid var(--accent);outline-offset:1px;}
.reset-btn{background:transparent;border:1px solid var(--panel-border);color:var(--bad);border-radius:.4rem;width:2rem;height:2rem;padding:0;margin:0;font-size:1rem;line-height:1;cursor:pointer;}
.reset-btn:hover{background:rgba(255,92,92,.12);border-color:var(--bad);}
.reset-btn:focus-visible{outline:2px solid var(--bad);outline-offset:1px;}
.tabctrl{display:none;}
.tabnav{margin:20px auto;max-width:900px;display:flex;flex-wrap:wrap;justify-content:center;gap:4px;}
.tabnav label{background:var(--panel);border:1px solid var(--panel-border);color:var(--muted);padding:8px 16px;border-radius:8px 8px 0 0;cursor:pointer;font-weight:bold;}
.tabpanel{display:none;}
.card{background:var(--panel);border:1px solid var(--panel-border);border-radius:10px;max-width:500px;margin:15px auto;padding:12px 16px;text-align:left;}
.card h3{margin:.2rem 0 .6rem;text-align:center;}
.card.wide{max-width:700px;}
.sub{border:1px solid var(--panel-border);border-radius:8px;padding:4px 12px 10px;margin:10px 0;}
.sub h4{margin:.5rem 0 .6rem;font-size:.95rem;color:var(--accent);text-align:left;}
.pinrow{justify-content:flex-start;}
.pinlabel{display:inline-block;width:11rem;text-align:right;color:var(--muted);}
select.pinsel.dup{outline:2px solid var(--bad);}
.pinwarn{display:block;color:var(--bad);min-height:1.2em;}
.row{display:flex;gap:6px;align-items:center;flex-wrap:wrap;justify-content:center;}
.row input,.row select,.row button{margin:6px;}
.flash{background:#d4edda;color:#155724;border:1px solid #c3e6cb;border-radius:6px;padding:8px 12px;margin:10px auto;max-width:500px;}
.codewrap{position:relative;}
.copy-btn{position:absolute;top:6px;right:6px;width:auto;margin:0;padding:3px 10px;font-size:.75rem;font-weight:normal;background:var(--panel);border:1px solid var(--panel-border);color:var(--text);border-radius:.4rem;cursor:pointer;}
.copy-btn:hover{background:var(--panel);border-color:var(--accent);color:var(--accent);}
.doc p,.doc li{font-size:.9rem;line-height:1.45;}
.doc table{width:100%;font-size:.85rem;}
.doc td,.doc th{text-align:left;}
.mqtt{font-size:1.05rem;}
.mqtt h3{font-size:1.3rem;}
.mqtt input{font-size:1.05rem;}
.mqtt label{display:block;margin:.6rem 0 .1rem;font-weight:bold;}
.mqtt table{width:100%;margin:12px 0;}
.mqtt th{background:var(--bg);font-size:.95rem;}
.mqtt td{vertical-align:top;padding:9px 10px;font-size:1rem;}
.mqtt code{font-family:var(--mono);font-size:.95rem;color:var(--accent);overflow-wrap:anywhere;word-break:normal;}
.mqtt td:first-child{min-width:310px;}
.mqtt .ex{display:block;margin:2px 0 6px;color:var(--text);}
.mqtt .kind{display:inline-block;padding:1px 8px;border-radius:10px;font-size:.8rem;border:1px solid var(--panel-border);color:var(--muted);}
.mqtt .kind.cmd{color:var(--accent);border-color:var(--accent-dim);}
.mqtt .kind.msg{color:var(--ok);border-color:var(--ok);}
.res-ok{background:#d4edda;color:#155724;border:1px solid #c3e6cb;border-radius:6px;padding:8px 12px;margin:10px auto;max-width:400px;}
.res-bad{background:#f8d7da;color:#721c24;border:1px solid #f5c6cb;border-radius:6px;padding:8px 12px;margin:10px auto;max-width:400px;}
textarea{height:110px;resize:vertical;}
pre.code{text-align:left;white-space:pre-wrap;word-break:break-all;font-family:var(--mono);font-size:.75rem;color:var(--muted);}
)css";

#include "wiring_svg.h"

static const char TAB_KEYS[][12] = {"mp3", "sprechen", "sd", "radio", "pins", "audio", "mqtt", "wlan", "zeit", "status", "log", "doku"};
static const char* const TAB_LABELS[] = {"MP3 (Flash)", "Sprechen", "SD-Karte", "Webradio", "Eing&auml;nge", "Audio", "MQTT", "WLAN", "Zeit", "Status", "Log", "Doku"};
static const char* const TAB_TIPS[] = {"MP3-Dateien im Ger&auml;t hochladen und abspielen", "Text ansagen, stoppen, Lautst&auml;rke", "Dateien der SD-Karte abspielen, hochladen und verwalten", "Webradio-URL abspielen und bis zu 30 Sender speichern", "Freie Pins mit Pull-up: bei Kontakt nach GND eine Ansage oder MP3-Datei abspielen", "Anschl&uuml;sse (Pins von Verst&auml;rker und SD-Karte), Lautst&auml;rke, &Uuml;berblenden, Start und Sprachausgabe (VoiceRSS)", "MQTT-Broker und Basis-Topic", "Hostname und WLAN-Zugangsdaten", "Zeitserver und Zeitzone", "Zustand des Ger&auml;ts", "Ereignisprotokoll", "Alle REST- und MQTT-Aufrufe zum Kopieren"};
static const int TAB_COUNT = 12;

// ⓘ-Hinweis mit Erklaerung als Tooltip (title) - der Punkt am Satzende kommt hier dazu; im Text keine einfachen Anfuehrungszeichen.
// ⓘ note with an explanation as tooltip (title) - the full stop at the end is added here; no single quotes in the text.

String infoTip(const String& text) {
    return "<span title='" + text + ".' style='cursor:help;'>&#9432;</span>";
}

String htmlEsc(const String& s) {
    String out;
    for (size_t i = 0; i < s.length(); i++) {
        char c = s[i];
        if (c == '&') out += "&amp;";
        else if (c == '<') out += "&lt;";
        else if (c == '>') out += "&gt;";
        else if (c == '\'') out += "&#39;";
        else if (c == '"') out += "&quot;";
        else out += c;
    }
    return out;
}

// Text fuer Tooltip und aria-label eines Punktes; ohne Umlaute, weil JavaScript keine HTML-Entities aufloest.
// Text for the tooltip and aria-label of a dot; without umlauts because JavaScript does not decode HTML entities.

String dotStatusText(const String& label, const String& state) {
    String t = "Nicht verfuegbar";
    if (state == "ok") t = "OK";
    else if (state == "syncing") t = "Aktiv";
    else if (state == "playing") t = "Spielt";
    else if (state == "idle") t = "Leerlauf";
    else if (state == "bad") t = "Fehler";
    return label + ": " + t;
}

String statusDotHtml(const String& key, const String& label, const String& state, bool gated, bool present) {
    String title = htmlEsc(dotStatusText(label, state));
    String cls = state == "na" ? " na" : (state == "ok" ? " ok" : (state == "syncing" ? " syncing" : (state == "playing" ? " playing" : (state == "idle" ? " idle" : ""))));
    String h = "<span class='status' id='status-" + key + "'" + ((gated && !present) ? " hidden" : "") + ">";
    h += "<i id='dot-" + key + "' class='dot" + cls + "' role='img' title='" + title + "' aria-label='" + title + "'></i>" + label + "</span>";
    return h;
}

String formatNow() {
    if (!timeIsValid()) return "";
    char buf[24];
    time_t now = time(nullptr);
    struct tm timeinfo;
    localtime_r(&now, &timeinfo);
    strftime(buf, sizeof(buf), "%d.%m.%Y %H:%M", &timeinfo);
    return String(buf);
}

bool mqttGatedPresent() {
    return mqttEverConnected;
}

bool voiceGatedPresent() {
    return voiceEver;
}

// Die Punkte hier und in handleTopbarStatus() muessen dieselben Bedingungen nutzen, sonst stimmen Erstansicht und Abfrage nicht ueberein.
// The dots here and in handleTopbarStatus() must use the same conditions, otherwise first paint and poll disagree.

String topbarHtml() {
    String ip = apActive && WiFi.status() != WL_CONNECTED ? WiFi.softAPIP().toString() : WiFi.localIP().toString();
    String h = "<header class='topbar'><div class='brand'><span class='brand-mark'>" PROJECT_TAG "</span>";
    h += "<h1><a class='host-link' href='http://" + cfg.host + ".local/'>" + htmlEsc(cfg.host) + "</a></h1>";
    h += "<span class='ip-hint'><a href='http://" + ip + "/'>" + ip + "</a></span></div>";
    h += "<div class='status-strip'>";
    h += "<span class='status' title='Aktuelle Lautst&auml;rke (0 bis 100)'>Lautst&auml;rke: <span id='value-vol' class='statval'>" + String(cfg.vol) + "/" + String(VOLUME_MAX) + "</span></span>";
    if (cfg.mqttServer.length() > 0) h += statusDotHtml("mqtt", "MQTT", mqttDotState(), true, mqttGatedPresent());
    if (cfg.vkey.length() > 0) h += statusDotHtml("voice", "VoiceRSS", voiceDotState(), true, voiceGatedPresent());
    h += statusDotHtml("audio", "Audio", audioDotState(), false, true);
    h += statusDotHtml("time", "Zeit", timeDotState(), false, true);
    h += "</div><div id='topbar-datetime' class='datetime'>" + formatNow() + "</div>";
    h += "<span id='topbar-offline-hint' class='offline-hint'>&#9888; Verbindung verloren</span>";
    h += "<a class='refresh-btn' id='langBtn' href='#' style='display:none' title='Sprache / Language'></a>";
    h += "<a class='refresh-btn' href='/export' title='Einstellungen als Datei exportieren'>&#11123;</a>";
    h += "<label class='refresh-btn' title='Einstellungen importieren'>&#11121;<input type='file' accept='.txt,text/plain' style='display:none' onchange='importSettings(this)'></label>";
    h += "<button type='button' class='reset-btn' onclick=\"if(confirm('ESP32 jetzt neu starten?')){fetch('/restart',{method:'POST'});setTimeout(function(){location.reload();},5000);}\" title='Neu starten'>&#9211;</button>";
    h += "</header>";
    return h;
}

static const char TOPBAR_JS[] PROGMEM = R"js(
(function(){
  function setStatusDot(id,state,title){var el=document.getElementById(id);if(!el)return;
    el.classList.toggle('ok',state==='ok');el.classList.toggle('syncing',state==='syncing');el.classList.toggle('playing',state==='playing');el.classList.toggle('idle',state==='idle');el.classList.toggle('na',state==='na');
    if(title){el.title=title;el.setAttribute('aria-label',title);}}
  function setPresent(id,p){var el=document.getElementById(id);if(el)el.hidden=!p;}
  function setValue(id,t){var el=document.getElementById(id);if(el)el.textContent=t;}
  function setOnline(ok){var h=document.getElementById('topbar-offline-hint');if(h)h.classList.toggle('show',!ok);}
  var fails=0;
  function poll(){
    fetch('/api/topbarStatus',{cache:'no-store'}).then(function(r){return r.json();}).then(function(s){
      fails=0;setOnline(true);
      setStatusDot('dot-mqtt',s.mqtt,s.mqttTitle);setPresent('status-mqtt',s.mqttPresent);
      setStatusDot('dot-voice',s.voice,s.voiceTitle);setPresent('status-voice',s.voicePresent);
      setStatusDot('dot-audio',s.audio,s.audioTitle);
      setStatusDot('dot-time',s.time,s.timeTitle);
      setValue('value-vol',s.volValue);
      var dt=document.getElementById('topbar-datetime');if(dt)dt.textContent=s.datetime;
    }).catch(function(){fails++;if(fails>=2)setOnline(false);});
  }
  var timer=null;
  function start(){if(timer)return;poll();timer=setInterval(poll,5000);}
  function stop(){if(!timer)return;clearInterval(timer);timer=null;}
  document.addEventListener('visibilitychange',function(){if(document.hidden)stop();else start();});
  if(!document.hidden)start();
})();
function importSettings(inp){
  var f=inp.files[0];if(!f)return;
  if(!confirm('Bestehende Einstellungen werden ueberschrieben (WLAN-Passwoerter bleiben). Fortfahren?')){inp.value='';return;}
  var rd=new FileReader();
  rd.onload=function(){fetch('/import',{method:'POST',headers:{'Content-Type':'text/plain'},body:rd.result}).then(function(r){return r.text();}).then(function(t){
    alert(t.indexOf('OK|')===0?t.substring(3):('Fehler: '+t.substring(5)));location.reload();});};
  rd.readAsText(f);
}
function loadStations(inp){
  var f=inp.files[0];if(!f)return;
  if(!confirm('Die gespeicherte Senderliste wird durch die Datei ersetzt. Fortfahren?')){inp.value='';return;}
  var rd=new FileReader();
  rd.onload=function(){fetch('/importstations',{method:'POST',headers:{'Content-Type':'text/plain'},body:rd.result}).then(function(r){return r.text();}).then(function(t){
    alert(t.indexOf('OK|')===0?t.substring(3):('Fehler: '+t.substring(5)));location.reload();});};
  rd.readAsText(f);
}
)js";

// ---------------------------------------------------------------------------
// Tabs / tabs

String selectOptions(const char* const* values, const char* const* labels, int count, const String& current) {
    String h;
    for (int i = 0; i < count; i++) {
        h += "<option value='" + String(values[i]) + "'" + (current == values[i] ? " selected" : "") + ">" + labels[i] + "</option>";
    }
    return h;
}

static const char* const LANG_VALUES[] = {"de-de", "en-gb", "en-us", "fr-fr", "es-es", "it-it", "nl-nl", "pt-pt", "pl-pl", "sv-se", "da-dk"};
static const char* const LANG_LABELS[] = {"Deutsch (de-de)", "English UK (en-gb)", "English US (en-us)", "Fran&ccedil;ais (fr-fr)", "Espa&ntilde;ol (es-es)", "Italiano (it-it)", "Nederlands (nl-nl)", "Portugu&ecirc;s (pt-pt)", "Polski (pl-pl)", "Svenska (sv-se)", "Dansk (da-dk)"};
static const int LANG_COUNT = 11;

static const char* const FMT_VALUES[] = {"8khz_16bit_mono", "22khz_16bit_mono", "44khz_16bit_mono", "44khz_16bit_stereo"};
static const char* const FMT_LABELS[] = {"8 kHz mono (klein)", "22 kHz mono (Standard)", "44 kHz mono", "44 kHz stereo"};

// Die Auswahl der Stimmen wird im Browser je nach gewaehlter Sprache gefuellt (VOICES in PAGE_JS); "Andere ..." erlaubt eigene Namen.
// The voice choices are filled in the browser depending on the selected language (VOICES in PAGE_JS); "Andere ..." allows own names.

String nowPlayingRow();

String panelSpeak() {
    String h = nowPlayingRow();
    h += "<div class='card'><h3>Text ansagen</h3>";
    h += "<div class='row'>Text zum Ansagen " + infoTip("Text, der &uuml;ber VoiceRSS angesagt wird (h&ouml;chstens 500 Zeichen)") + "</div>";
    h += "<textarea id='sayText' maxlength='" + String(TTS_MAX_CHARS) + "' placeholder='Text zum Ansagen'></textarea>";
    h += "<div class='row'><select id='sayLang' onchange=\"fillVoiceSel('sayVoice',this.value,false)\" style='width:210px'><option value=''>Standard (" + htmlEsc(cfg.vlang) + ")</option>" + selectOptions(LANG_VALUES, LANG_LABELS, LANG_COUNT, "") + "</select> " + infoTip("Sprache der Ansage; Standard ist die Sprache aus dem Tab Audio") + "";
    h += "<select id='sayVoice' onchange='voiceChosen(this)' style='width:210px'><option value=''>Standard (VoiceRSS)</option></select> " + infoTip("Stimme f&uuml;r diese Ansage; Standard = gespeicherte Stimme bzw. Standardstimme der Sprache") + "</div>";
    h += "<div class='row'><button type='button' onclick='sayNow()' style='width:150px'>Sprechen</button><button type='button' onclick='stopNow()' style='width:150px'>Stopp</button> " + infoTip("Sprechen: sagt den Text jetzt an, ein laufendes Webradio l&auml;uft danach weiter. Stopp: beendet die Wiedergabe sofort, auch ein Webradio, ohne Fortsetzung") + "</div>";
    h += "<div class='row'><label>Lautst&auml;rke <input type='range' id='vol' min='0' max='" + String(VOLUME_MAX) + "' value='" + String(cfg.vol) + "' style='width:200px' oninput='volShow(this.value)' onchange='setVol(this.value)'> <span id='volShow' style='display:inline-block;min-width:76px'>" + String(cfg.vol) + " / " + String(VOLUME_MAX) + "</span></label> " + infoTip("Lautst&auml;rke 0 bis 100; wird sofort &uuml;bernommen und gespeichert") + "</div>";
    h += "<div id='sayResult'></div><div class='row'><small id='playState'></small></div></div>";
    return h;
}

// Zeile mit der Anzeige, was gerade laeuft, und dem Stopp-Knopf; steht oben in jedem Tab, der Wiedergabe startet.
// Row with the display of what is playing right now and the stop button; at the top of every tab that starts playback.

String nowPlayingRow() {
    return "<div class='row' style='min-height:2.4rem'><strong class='nowplaying' style='color:var(--ok)'></strong><button type='button' class='stopbtn' onclick='stopNow()' style='width:auto;padding:6px 16px;margin:0 8px' disabled>&#9632; Stopp</button></div>";
}

String panelRadio() {
    String h = nowPlayingRow();
    h += "<details id='radioInfoBox' style='display:none;max-width:500px;margin:0 auto 10px;text-align:left'><summary style='cursor:pointer;color:var(--muted)'>Stream-Info</summary><table id='radioInfo' style='width:100%;margin-top:6px'></table></details>";
    h += "<div class='card'><h3>Webradio-URL abspielen</h3><div class='row'>";
    h += "<input id='playUrl' placeholder='https://...' style='width:90%'><button type='button' onclick='playUrlNow()' style='width:150px'>URL abspielen</button><button type='button' class='stopbtn' onclick='stopNow()' style='width:150px' disabled>&#9632; Stopp</button> " + infoTip("Adresse eines Webradios oder einer MP3-Datei (http:// oder https://); sie wird jetzt abgespielt, aber nicht gespeichert. Stopp beendet die Wiedergabe, auch ein Webradio, ohne Fortsetzung") + "</div><div id='radioResult'></div></div>";

    // Belegte Plaetze plus der erste freie werden angezeigt; nach dem Speichern erscheint der naechste freie.
    // Used slots plus the first free one are shown; after saving the next free one appears.

    int firstEmpty = -1;
    int used = 0;
    for (int i = 0; i < RADIO_SLOTS; i++) {
        if (cfg.radioUrl[i].isEmpty()) {
            if (firstEmpty < 0) firstEmpty = i;
        } else {
            used++;
        }
    }
    h += "<div class='card' style='max-width:900px'><h3>Gespeicherte Webradios / URLs " + infoTip("Pro Platz: Name (beliebig, auch per REST und MQTT abrufbar), URL (Webradio oder MP3; leer lassen und speichern gibt den Platz frei) und Lautst&auml;rke des Senders (0-100, leer = allgemeine Lautst&auml;rke)") + "</h3><small>" + String(used) + " von " + String(RADIO_SLOTS) + " Pl&auml;tzen belegt</small><form method='POST' action='/save'><input type='hidden' name='sect' value='radio'>";
    for (int i = 0; i < RADIO_SLOTS; i++) {
        if (cfg.radioUrl[i].isEmpty() && i != firstEmpty) continue;
        String n = String(i + 1);
        h += "<div class='row stationrow' data-n='" + n + "' data-url='" + htmlEsc(cfg.radioUrl[i]) + "'><span style='width:1.6rem;text-align:right;color:var(--muted)'>" + n + "</span>";
        h += "<input name='rn" + n + "' maxlength='" + String(RADIO_NAME_MAX) + "' placeholder='Name' value='" + htmlEsc(cfg.radioName[i]) + "' style='width:150px'>";
        h += "<input name='ru" + n + "' maxlength='" + String(RADIO_URL_MAX) + "' placeholder='http://...' value='" + htmlEsc(cfg.radioUrl[i]) + "' style='width:300px'>";
        h += "<input type='number' name='rv" + n + "' min='0' max='" + String(VOLUME_MAX) + "' placeholder='Std' value='" + (cfg.radioVol[i] >= 0 ? String(cfg.radioVol[i]) : String()) + "' style='width:64px'>";
        String off = cfg.radioUrl[i].isEmpty() ? " disabled" : "";
        h += "<button type='button' class='copy-btn' style='position:static;margin:2px' title='Nach oben schieben (speichert sofort)' onclick='moveStation(" + n + ",-1)'" + off + ">&#9650;</button>";
        h += "<button type='button' class='copy-btn' style='position:static;margin:2px' title='Nach unten schieben (speichert sofort)' onclick='moveStation(" + n + ",1)'" + off + ">&#9660;</button>";
        h += "<button type='button' class='playbtn' onclick='playStation(" + n + ")' title='Abspielen' style='width:auto;padding:8px 14px'" + off + ">&#9654;</button>";
        h += "<button type='button' class='copy-btn' style='position:static;margin:2px' title='REST-Link kopieren' onclick='copyText(stationLink(" + n + "),this)'" + off + ">URL</button>";
        h += "<button type='button' class='copy-btn' style='position:static;margin:2px' title='Platz l&ouml;schen (Name und URL); die folgenden Pl&auml;tze r&uuml;cken auf' onclick='delStation(" + n + ")'" + off + ">&#10006;</button></div>";
    }
    h += "<small>Das Feld rechts neben der URL ist die Lautst&auml;rke des Senders (0-100, leer = allgemeine Lautst&auml;rke). Speichern, bevor du &#9654; dr&uuml;ckst. &#9650;&#9660; verschieben den Sender und speichern sofort. Der URL-Knopf kopiert den REST-Aufruf <code>/api/station?n=Platz</code>; auch der Name geht (<code>?name=...</code>).</small>";
    h += "<button type='submit'>Speichern</button><div id='stResult'></div></form>";
    h += "<div class='row'><button type='button' onclick=\"location.href='/stations.txt'\" style='width:auto;padding:8px 14px'>Liste speichern</button> " + infoTip("Speichert die Senderliste (Name, URL, Lautst&auml;rke) als Textdatei auf deinem Rechner") + " ";
    h += "<button type='button' onclick=\"document.getElementById('stFile').click()\" style='width:auto;padding:8px 14px'>Liste laden</button><input type='file' id='stFile' accept='.txt' style='display:none' onchange='loadStations(this)'> " + infoTip("Ersetzt die gespeicherte Senderliste durch eine zuvor gespeicherte Datei") + "</div></div>";
    return h;
}

String panelSd() {
    String h = nowPlayingRow();
    h += "<div class='card' style='max-width:900px'><h3>SD-Karte</h3><small id='sdInfo'>Lade &hellip;</small><div id='sdMem'></div><div id='sdPath' style='margin:6px 0;color:var(--muted)'></div><div id='sdList'></div>";
    h += "<div class='row'><input type='file' id='sdUpFile' multiple accept='.mp3' style='width:55%'><button type='button' onclick='sdUploadNow()' style='width:auto;padding:8px 14px'>Hochladen</button> " + infoTip("Eine oder mehrere MP3-Dateien w&auml;hlen; sie werden nacheinander in den gezeigten Ordner der Karte hochgeladen") + "</div>";
    h += "<div class='row'><button type='button' onclick='sdMkdir()' style='width:auto;padding:8px 14px'>Neuer Ordner</button><button type='button' onclick='sdRemount()' style='width:auto;padding:8px 14px'>Neu einbinden</button> <label style='display:inline-flex;align-items:center;gap:4px'><input type='checkbox' id='sdLoop'> Endlos</label> " + infoTip("Neuer Ordner: legt einen Ordner im gezeigten Ordner an. Neu einbinden: bindet die Karte neu ein, zum Beispiel nach dem Einstecken. Endlos: nach der letzten Datei eines Ordners beginnt eine neue Runde. Zufall (Knopf am Ordner): jede Datei kommt je Sitzung einmal, die Reihenfolge ist jedes Mal eine andere und beginnt nie mit der Datei der letzten Sitzung") + "</div><div id='sdResult'></div></div>";
    return h;
}

String panelPins() {
    String h = "<div class='card' style='max-width:900px'><h3>Eing&auml;nge (Pull-up)</h3>";
    h += "<p>Alle freien Pins liegen mit Pull-up auf High. Bekommt ein Pin Kontakt nach GND (zum Beispiel durch eine Taste), spielt das Ger&auml;t die gew&auml;hlte Ansage, MP3-Datei oder den gew&auml;hlten Webradio-Sender. Nicht angezeigt werden die Pins von Verst&auml;rker und SD-Karte, die Strapping-Pins und die Status-LED. " + infoTip("Aktion: Ansage = Text, der &uuml;ber VoiceRSS gesprochen wird. MP3-Datei = Dateiname aus dem Tab MP3 (Flash), sd:/Pfad.mp3 oder sd:/Ordner/ von der SD-Karte oder eine URL (http:// oder https://). Webradio = gespeicherter Sender, Name oder Platznummer aus dem Tab Webradio. Die Lautst&auml;rke ist optional (0-100, leer = allgemeine). Der Pin muss etwa 40 ms auf GND bleiben; ausgel&ouml;st wird einmal je Tastendruck") + "</p>";
    h += "<table id='pinTable' style='width:100%'></table><div id='pinResult'></div></div>";
    return h;
}

String panelMp3() {
    String h = nowPlayingRow();
    h += "<div class='card'><h3>Dateien im Ger&auml;t</h3><div id='fsMem'></div><div id='fileList'>Lade &hellip;</div><hr>";
    h += "<div class='row'><input type='file' id='upFile' multiple accept='.mp3' style='width:90%'><button type='button' onclick='uploadNow()' style='width:150px'>Hochladen</button> " + infoTip("Eine oder mehrere MP3-Dateien vom Rechner w&auml;hlen; sie werden nacheinander in den Speicher des Ger&auml;ts geladen") + "</div>";
    h += "<div id='upResult'></div></div>";
    return h;
}

// Auswahlfeld fuer einen Pin: alle erlaubten GPIOs des ESP32-S3; ein gespeicherter, nicht erlaubter Wert bleibt sichtbar.
// Select field for a pin: all allowed GPIOs of the ESP32-S3; a stored value that is not allowed stays visible.

String pinSelect(const char* name, int value) {
    String s = String("<select class='pinsel' name='") + name + "' onchange='checkPins()' style='width:130px'>";
    bool found = false;
    for (int p = 0; p <= 48; p++) {
        if (!pinAllowed(p)) continue;
        if (p == value) found = true;
        s += "<option value='" + String(p) + "'" + (p == value ? " selected" : "") + ">GPIO " + String(p) + "</option>";
    }
    if (!found) s += "<option value='" + String(value) + "' selected>GPIO " + String(value) + " (nicht erlaubt)</option>";
    return s + "</select>";
}

String pinRow(const char* label, const char* name, int value, const char* title) {
    return String("<div class='row pinrow'><span class='pinlabel'>") + label + "</span>" + pinSelect(name, value) + " " + infoTip(title) + "</div>";
}

// Der Tab Audio fasst alles zusammen, was Ton und Sprache betrifft, in getrennten Karten; ein Speichern gilt fuer alle.
// The Audio tab gathers everything about sound and speech in separate cards; one save applies to all.

String panelAudio() {
    String h = "<form method='POST' action='/save'><input type='hidden' name='sect' value='audio'>";
    h += "<div class='card wide'><h3>Anschl&uuml;sse</h3>";
    h += "<div class='sub'><h4>Verst&auml;rker (I2S, MAX98357A)</h4>";
    h += pinRow("DIN (Daten)", "din", cfg.din, "GPIO f&uuml;r die Daten (DIN) zum Verst&auml;rker; wirkt nach einem Neustart");
    h += pinRow("BCLK (Bit-Takt)", "bclk", cfg.bclk, "GPIO f&uuml;r den Bit-Takt (BCLK) des Verst&auml;rkers; wirkt nach einem Neustart");
    h += pinRow("LRC (Wort-Takt)", "lrc", cfg.lrc, "GPIO f&uuml;r den Wort-Takt (LRC/WS) des Verst&auml;rkers; wirkt nach einem Neustart");
    h += "<small>&Auml;nderungen an diesen Pins wirken nach einem Neustart.</small></div>";
    h += "<div class='sub'><h4>SD-Karte (SPI)</h4>";
    h += "<div class='row'><label style='display:flex;align-items:center;gap:8px;cursor:pointer;'><input type='checkbox' name='sdon' value='1'" + String(cfg.sdOn ? " checked" : "") + " style='width:auto;margin:0;'>SD-Karte verwenden</label> " + infoTip("SD-Karte im SPI-Modus verwenden; ohne Karte l&auml;uft alles andere weiter") + "</div>";
    h += pinRow("MISO (Daten von der Karte)", "sdmiso", cfg.sdMiso, "Daten von der Karte (MISO/DO); &Auml;nderungen wirken sofort");
    h += pinRow("SCK (Takt)", "sdsck", cfg.sdSck, "Takt (CLK) der SD-Karte; &Auml;nderungen wirken sofort");
    h += pinRow("MOSI (Daten zur Karte)", "sdmosi", cfg.sdMosi, "Daten zur Karte (MOSI/DI); &Auml;nderungen wirken sofort");
    h += pinRow("CS (Chip-Select)", "sdcs", cfg.sdCs, "Chip-Select der SD-Karte; &Auml;nderungen wirken sofort");
    h += "<small>&Auml;nderungen an den SD-Pins wirken sofort, die Karte wird neu eingebunden.</small></div>";
    h += "<small id='pinWarn' class='pinwarn'></small>";
    h += "<small>Erlaubte Pins am ESP32-S3: 1-18, 21, 33-42, 47, 48. Jeder Pin darf nur einmal vergeben sein; sonst werden die Pins nicht gespeichert.</small></div>";
    h += "<div class='card wide'><h3>Lautst&auml;rke</h3>";
    h += "<div class='row'>Standard-Lautst&auml;rke (0-" + String(VOLUME_MAX) + ") " + infoTip("Lautst&auml;rke, die nach dem Start gilt und wenn nichts anderes eingestellt ist") + " <input type='number' name='vol' min='0' max='" + String(VOLUME_MAX) + "' value='" + String(cfg.vol) + "' style='width:90px'></div>";
    h += "<div class='row'>Lautst&auml;rke &auml;ndern (ms) " + infoTip("Dauer, in der eine ge&auml;nderte Lautst&auml;rke (Regler, REST, MQTT) bei laufendem Ton zum neuen Wert wandert; 0 = sofort. Per REST und MQTT l&auml;sst sich die Dauer je Aufruf mit ms angeben") + " <input type='number' name='vramp' min='0' max='" + String(VOLUME_RAMP_MAX_MS) + "' step='100' value='" + String(cfg.volRampMs) + "' style='width:90px'></div>";
    h += "</div>";
    h += "<div class='card wide'><h3>&Uuml;berblenden und Mischen</h3>";
    h += "<div class='sub'><h4>Webradio ein- und ausblenden</h4>";
    h += "<div class='row'>Radio aus-/einblenden (ms) " + infoTip("Dauer, in der ein laufendes Webradio vor einer Ansage oder MP3-Datei auf Lautst&auml;rke 0 ausgeblendet und nach der Fortsetzung wieder eingeblendet wird; auch das Absenken beim Mischen nutzt diese Zeit; 0 = sofort") + " <input type='number' name='fade' min='0' max='" + String(FADE_MAX_MS) + "' step='100' value='" + String(cfg.fadeMs) + "' style='width:90px'></div>";
    h += "<div class='row'>Radio Start/Stopp/Wechsel (ms) " + infoTip("Dauer des Einblendens beim Start eines Webradios, des Ausblendens beim Stopp und beider beim Senderwechsel; 0 = sofort") + " <input type='number' name='swfade' min='0' max='" + String(FADE_MAX_MS) + "' step='100' value='" + String(cfg.switchFadeMs) + "' style='width:90px'></div>";
    h += "</div><div class='sub'><h4>Ansagen &uuml;ber dem Radio</h4>";
    h += "<div class='row'><label style='display:flex;align-items:center;gap:8px;cursor:pointer;'><input type='checkbox' name='mix' value='1'" + String(cfg.mixAnn ? " checked" : "") + " style='width:auto;margin:0;'>Radio unter Ansagen weiterlaufen lassen</label> " + infoTip("Ansagen (und bei Webradio auch MP3-Dateien) werden leise &uuml;ber das laufende Webradio oder eine laufende MP3-Datei gelegt, die ohne Unterbrechung weiterl&auml;uft. Aus: das Radio stoppt f&uuml;r die Ansage und wird danach neu verbunden; eine MP3-Datei stoppt ebenfalls") + " Radio dabei auf <input type='number' name='duck' min='0' max='100' value='" + String(cfg.duckPct) + "' style='width:64px'> % " + infoTip("Lautst&auml;rke des Radios w&auml;hrend der Ansage in Prozent der eingestellten Lautst&auml;rke") + "</div>";
    h += "<div class='row'><label style='display:flex;align-items:center;gap:8px;cursor:pointer;'><input type='checkbox' name='loud' value='1'" + String(cfg.loudNorm ? " checked" : "") + " style='width:auto;margin:0;'>MP3-Lautst&auml;rke angleichen</label> " + infoTip("Beim ersten Abspielen einer MP3-Datei misst das Ger&auml;t im Hintergrund ihre Lautst&auml;rke und gleicht sie auf einen einheitlichen Pegel an; der Ausgleich wird gespeichert, die Datei selbst bleibt unver&auml;ndert. W&auml;hrend der Messung (einige Sekunden bis eine Minute) l&auml;uft die Datei noch unver&auml;ndert und wird dann sanft angepasst. Ansagen und Radio bleiben unver&auml;ndert") + "</div>";
    h += "</div><div class='sub'><h4>Titelregel</h4>";
    h += "<div class='row'>Titelregel: Titel enth&auml;lt " + infoTip("Stichworte der Titelregel:&#10;&#8226; Mehrere Stichworte mit Komma oder Semikolon trennen.&#10;&#8226; Gro&szlig;-/Kleinschreibung ist egal; es gen&uuml;gt, wenn der Titel das Stichwort enth&auml;lt.&#10;&#8226; Mit =Wert (0-100) bekommt ein Stichwort eine eigene Lautst&auml;rke, zum Beispiel: Cambodia=90, Thunder&#10;&#8226; Leer = Regel aus") + " <input name='tmatch' maxlength='120' value='" + htmlEsc(cfg.titleMatch) + "' placeholder='z. B. Cambodia, Thunder=90' style='width:280px'> Lautst&auml;rke <input type='number' name='tvol' min='0' max='" + String(VOLUME_MAX) + "' value='" + String(cfg.titleVol) + "' style='width:64px'> " + infoTip("Lautst&auml;rke (0-100) f&uuml;r Stichworte ohne eigenen Wert; sie gilt, solange der Titel l&auml;uft, danach wieder die vorherige") + " h&ouml;chstens <input type='number' name='tmax' min='0' max='3600' value='" + String(cfg.titleMaxS) + "' style='width:70px'> s " + infoTip("Ein Sender meldet das Ende eines Titels nicht. Mit einem Wert gr&ouml;&szlig;er als 0 endet die Titelregel nach so vielen Sekunden (zum Beispiel 210 f&uuml;r einen Titel von 3:30), damit Werbung nach dem Titel nicht laut bleibt; 0 = bis zum n&auml;chsten Titel") + "</div>";
    h += "</div></div>";
    h += "<div class='card wide'><h3>Start</h3>";
    h += "<div class='row'><label style='display:flex;align-items:center;gap:8px;cursor:pointer;'><input type='checkbox' name='autostart' value='1'" + String(cfg.autoStart ? " checked" : "") + " style='width:auto;margin:0;'>Beim Start Sender 1 abspielen</label> " + infoTip("Spielt nach dem Start nach einigen Sekunden Sender 1 der Liste ab, sobald das WLAN steht und der Stream antwortet. Ausgeschaltet bleibt das Ger&auml;t nach dem Start stumm") + "</div>";
    h += "<div class='row'><label style='display:flex;align-items:center;gap:8px;cursor:pointer;'><input type='checkbox' name='btone' value='1'" + String(cfg.bootTone ? " checked" : "") + " style='width:auto;margin:0;'>Kurzer Ton beim Start</label> " + infoTip("Beim Einschalten und nach einem Neustart &uuml;ber die Weboberfl&auml;che spielt das Ger&auml;t einen kurzen Ton (&bdquo;buub&ldquo;) in der eingestellten Lautst&auml;rke. Nach einem Neustart per MQTT, nach einem Absturz und im abgesicherten Modus bleibt er aus") + "</div>";
    h += "<small>Aus (Standard): Das Ger&auml;t startet stumm. An: Nach dem Start spielt es nach einigen Sekunden Sender 1 der Liste im Tab Webradio, sobald der Stream antwortet (bis zu 10 Versuche im Abstand von 6 s).</small><br>";
    h += "<div class='row'><label style='display:flex;align-items:center;gap:8px;cursor:pointer;'><input type='checkbox' name='resume' value='1'" + String(cfg.resumeLast ? " checked" : "") + " style='width:auto;margin:0;'>Zuletzt gespielten Sender nach einem Neustart fortsetzen</label> " + infoTip("Merkt sich den laufenden Sender mit seiner Lautst&auml;rke und spielt ihn nach einem Neustart oder Stromausfall wieder ab; ein Stopp l&ouml;scht den Merker") + "</div>";
    h += "<small>Das Ger&auml;t merkt sich Sender und Lautst&auml;rke (erst wenn die Lautst&auml;rke 3 s unver&auml;ndert ist). Nach einem Neustart spielt es ihn wieder ab, mit derselben Lautst&auml;rke; ein Stopp l&ouml;scht den Merker, dann startet das Ger&auml;t stumm. Hat dieser Haken Vorrang vor &bdquo;Beim Start Sender 1 abspielen&ldquo;.</small>";
    h += "</div>";
    h += "<div class='card wide'><h3>Sprachausgabe (VoiceRSS)</h3>";
    h += "<div class='row'><input type='text' name='vkey' maxlength='64' placeholder='API-Schl&uuml;ssel' value='" + htmlEsc(cfg.vkey) + "' autocomplete='off' style='width:260px'> " + infoTip("API-Schl&uuml;ssel von voicerss.org; ein leeres Feld l&ouml;scht ihn") + "</div>";
    h += "<small>Der Schl&uuml;ssel wird angezeigt; ein leeres Feld l&ouml;scht ihn. Er steht nicht im Export der Einstellungen und muss auf einem neuen Ger&auml;t neu eingetragen werden. Kostenloser Schl&uuml;ssel: voicerss.org</small>";
    h += "<div class='row'><select name='vlang' id='cfgLang' onchange=\"fillVoiceSel('cfgVoice',this.value,false)\" style='width:260px'>" + selectOptions(LANG_VALUES, LANG_LABELS, LANG_COUNT, cfg.vlang) + "</select> " + infoTip("Standardsprache f&uuml;r Ansagen") + "</div>";
    h += "<div class='row'><select name='vvoice' id='cfgVoice' onchange='voiceChosen(this)' style='width:260px'><option value=''>Standard (VoiceRSS)</option>" + (cfg.vvoice.length() > 0 ? "<option value='" + htmlEsc(cfg.vvoice) + "' selected>" + htmlEsc(cfg.vvoice) + "</option>" : String()) + "</select> " + infoTip("Standardstimme f&uuml;r Ansagen; Standard = Standardstimme der Sprache") + "</div>";
    h += "<div class='row'>Tempo (-10 bis 10): " + infoTip("Sprechtempo: -10 = sehr langsam, 0 = normal, 10 = sehr schnell") + " <input type='number' name='vrate' min='-10' max='10' value='" + String(cfg.vrate) + "' style='width:90px'></div>";
    h += "<div class='row'><select name='vfmt' style='width:260px'>" + selectOptions(FMT_VALUES, FMT_LABELS, 4, cfg.vfmt) + "</select> " + infoTip("Audioformat der Sprachdatei: kleiner = schneller geladen, gr&ouml;&szlig;er = bessere Qualit&auml;t") + "</div>";
    h += "</div>";
    h += "<div style='text-align:center'><button type='submit'>Speichern</button></div></form>";
    return h;
}

// Eine Zeile der Topic-Tabelle: Topic mit Kopierknopf, Art (Befehl = senden, Meldung = lesen), Nutzlast, Beispiele.
// One row of the topic table: topic with copy button, kind (command = send, message = read), payload, examples.

String mqttRow(const String& base, const char* suffix, bool command, const char* payloadText, const char* const* examples, int exampleCount) {
    String topic = base + suffix;
    String h = "<tr><td><code>" + htmlEsc(topic) + "</code><br><button type='button' class='copy-btn' style='position:static;margin:4px 0 0' data-copy='" + htmlEsc(topic) + "' onclick='copyText(this.getAttribute(\"data-copy\"),this)' title='Topic kopieren'>Kopieren</button></td>";
    h += String("<td><span class='kind ") + (command ? "cmd'>senden" : "msg'>lesen") + "</span></td><td>" + payloadText + "</td><td>";
    for (int i = 0; i < exampleCount; i++) h += "<code class='ex'>" + htmlEsc(examples[i]) + "</code>";
    return h + "</td></tr>";
}

String panelMqtt() {
    String base = mqttBaseTopic();
    String h = "<div class='card mqtt' style='max-width:1000px'><h3>MQTT</h3><form method='POST' action='/save'><input type='hidden' name='sect' value='mqtt'>";
    h += "<div class='row' style='justify-content:flex-start;gap:6px 18px'>";
    h += "<span style='display:inline-flex;align-items:center;gap:6px'><label for='mq_srv' style='white-space:nowrap'>Broker " + infoTip("Adresse des MQTT-Brokers; leer = MQTT aus") + "</label><input id='mq_srv' name='mq_srv' placeholder='z. B. 192.168.0.70 (leer = MQTT aus)' value='" + htmlEsc(cfg.mqttServer) + "' style='width:min(100%,300px);margin:4px 0'></span>";
    h += "<span style='display:inline-flex;align-items:center;gap:6px'><label for='mq_port' style='white-space:nowrap'>Port " + infoTip("Port des Brokers, meist 1883") + "</label><input id='mq_port' type='number' name='mq_port' min='1' max='65535' value='" + String(cfg.mqttPort) + "' style='width:100px;margin:4px 0'></span></div>";
    h += "<div class='row' style='justify-content:flex-start;gap:6px 18px'>";
    h += "<span style='display:inline-flex;align-items:center;gap:6px'><label for='mq_user' style='white-space:nowrap'>Benutzer " + infoTip("Benutzername f&uuml;r den MQTT-Broker (optional); leer = ohne Anmeldung") + "</label><input id='mq_user' name='mq_user' autocomplete='off' placeholder='optional' value='" + htmlEsc(cfg.mqttUser) + "' style='width:min(100%,200px);margin:4px 0'></span>";
    h += "<span style='display:inline-flex;align-items:center;gap:6px'><label for='mq_pass' style='white-space:nowrap'>Passwort " + infoTip("Passwort f&uuml;r den MQTT-Broker (optional); es wird nicht angezeigt. Leer lassen, um das gespeicherte zu behalten; ohne Benutzer wird es gel&ouml;scht") + "</label><input id='mq_pass' name='mq_pass' type='password' autocomplete='new-password' placeholder='" + String(cfg.mqttPass.length() > 0 ? "gespeichert (leer lassen = behalten)" : "optional") + "' value='' style='width:min(100%,260px);margin:4px 0'></span></div>";
    h += "<div class='row' style='justify-content:flex-start'><span style='display:inline-flex;align-items:center;gap:6px;flex-wrap:wrap'><label for='mq_base' style='white-space:nowrap'>Basis-Topic " + infoTip("Basis-Topic; darunter liegen say, play, url, file, station, stop, volume, restart, status") + "</label><input id='mq_base' name='mq_base' placeholder='" + htmlEsc(String(PROJECT_NAME) + "/" + cfg.host) + "' value='" + htmlEsc(cfg.mqttBase) + "' style='width:min(100%,420px);margin:4px 0'></span></div>";
    int grpVisible = 1;
    for (int i = 0; i < 5; i++) {
        if (!cfg.mqttGroup[i].isEmpty()) grpVisible = i + 2;
    }
    if (grpVisible > 5) grpVisible = 5;
    for (int i = 1; i <= 5; i++) {
        String n = String(i);
        h += "<div id='grpRow" + n + "' class='row' style='justify-content:flex-start;" + String(i > grpVisible ? "display:none" : "") + "'><span style='display:inline-flex;align-items:center;gap:6px;flex-wrap:wrap'><label for='mq_g" + n + "' style='white-space:nowrap'>Gruppen-Topic " + n + " " + infoTip("Zus&auml;tzliches Abo, um mehrere Ger&auml;te mit einem Befehl anzusprechen: Das Ger&auml;t nimmt unter diesem Topic dieselben Befehle an wie unter dem Basis-Topic (say, play, url, file, station, list, stop, volume, restart), zum Beispiel netspeaker/all/stop. Bis zu 5, leere Felder werden &uuml;bersprungen") + "</label><input id='mq_g" + n + "' name='mq_g" + n + "' placeholder='z. B. netspeaker/all' value='" + htmlEsc(cfg.mqttGroup[i - 1]) + "' oninput='grpGrow(" + n + ")' style='width:min(100%,420px);margin:4px 0'></span></div>";
    }
    h += "<p><small style='font-size:.9rem'>Leer = <code>netspeaker/&lt;Hostname&gt;</code>. Bereits gespeicherte Topics folgen einem neuen Hostnamen nicht automatisch. Status: <strong>" + String(mqttClient.connected() ? "verbunden" : (cfg.mqttServer.isEmpty() ? "aus" : "nicht verbunden")) + "</strong>.</small></p>";
    h += "<button type='submit' style='width:auto;padding:10px 28px'>Speichern</button></form>";
    h += "<h3 style='margin-top:1.4rem'>Topics</h3><p>Befehle sendest du an die Topics mit <span class='kind cmd'>senden</span>, Meldungen liest du von denen mit <span class='kind msg'>lesen</span> (die meisten sind <em>retained</em>, du bekommst den letzten Stand sofort beim Abonnieren). Als Nutzlast geht Text oder JSON.</p>";
    h += "<table><tr><th>Topic</th><th>Art</th><th>Nutzlast</th><th>Beispiele</th></tr>";
    static const char* const exSay[] = {"Das Abendessen ist fertig", "{\"text\":\"Es klingelt an der T\xC3\xBCr\",\"lang\":\"de-de\",\"voice\":\"Hanna\",\"vol\":15}"};
    static const char* const exPlay[] = {"https://dispatcher.rndfnk.com/br/br3/live/mp3/mid", "klingel.mp3", "{\"file\":\"klingel.mp3\",\"vol\":18}"};
    static const char* const exUrl[] = {"https://dispatcher.rndfnk.com/br/br3/live/mp3/mid"};
    static const char* const exFile[] = {"klingel.mp3"};
    static const char* const exStation[] = {"2", "BR3", "{\"name\":\"BR3\",\"vol\":8}"};
    static const char* const exStop[] = {"(leer)"};
    static const char* const exVol[] = {"50", "{\"v\":60,\"ms\":3000}"};
    static const char* const exList[] = {"(leer)"};
    static const char* const exRestart[] = {"(leer)"};
    static const char* const exStatus[] = {"idle", "playing", "error: File not found: /mp3/klingel.mp3"};
    static const char* const exNow[] = {"BR3 - K\xC3\xBCnstler - Titel", "klingel.mp3", "Ansage"};
    static const char* const exStations[] = {"{\"stations\":[{\"n\":1,\"name\":\"BR3\",\"url\":\"https://...\",\"vol\":null}]}"};
    static const char* const exVolState[] = {"12"};
    static const char* const exAvail[] = {"online", "offline"};
    h += mqttRow(base, "/say", true, "Text, der angesagt wird. Oder JSON mit <code>text</code> und optional <code>lang</code>, <code>voice</code>, <code>rate</code>, <code>vol</code>.", exSay, 2);
    h += mqttRow(base, "/play", true, "URL (Webradio, MP3) <em>oder</em> Dateiname aus dem Tab MP3 (Flash) oder <code>sd:/Pfad.mp3</code> von der SD-Karte. Das Ger&auml;t erkennt selbst, was es ist. JSON: <code>url</code> oder <code>file</code>, optional <code>vol</code>; bei einem Ordner (<code>sd:/Pfad/</code>) auch <code>shuffle</code> (zuf&auml;llige Reihenfolge) und <code>loop</code> (endlos), z. B. <code>{&quot;file&quot;:&quot;sd:/Musik/&quot;,&quot;shuffle&quot;:1,&quot;loop&quot;:1}</code>.", exPlay, 3);
    h += mqttRow(base, "/url", true, "URL (http:// oder https://). JSON: <code>url</code>, optional <code>vol</code>.", exUrl, 1);
    h += mqttRow(base, "/file", true, "Dateiname einer hochgeladenen Datei, <code>sd:/Pfad.mp3</code> oder ein Ordner <code>sd:/Ordner/</code>. JSON: <code>file</code>, optional <code>vol</code>, <code>shuffle</code> und <code>loop</code>.", exFile, 1);
    h += mqttRow(base, "/station", true, "Gespeicherter Sender: Platznummer 1-30 oder Name. JSON: <code>n</code> oder <code>name</code>, optional <code>vol</code>.", exStation, 3);
    h += mqttRow(base, "/stop", true, "Keine Nutzlast: beendet die Wiedergabe, auch ein Webradio.", exStop, 1);
    h += mqttRow(base, "/volume", true, "Allgemeine Lautst&auml;rke 0 bis 100 (wird gespeichert); sie wandert sanft zum neuen Wert. Als JSON l&auml;sst sich die Dauer je Aufruf in Millisekunden angeben: <code>{\"v\":60,\"ms\":3000}</code>.", exVol, 2);
    h += mqttRow(base, "/list", true, "Keine Nutzlast: sendet die Senderliste erneut.", exList, 1);
    h += mqttRow(base, "/restart", true, "Keine Nutzlast: startet das Ger&auml;t neu. <strong>Nicht retained senden</strong>, sonst wird ein gespeicherter Befehl nach dem Verbinden ignoriert.", exRestart, 1);
    h += mqttRow(base, "/status", false, "Zustand: <code>idle</code>, <code>loading</code>, <code>playing</code> oder <code>error: ...</code>.", exStatus, 3);
    h += mqttRow(base, "/now", false, "Was gerade l&auml;uft: Sendername und Titel, Dateiname oder &quot;Ansage&quot;; leer, wenn nichts l&auml;uft.", exNow, 3);
    h += mqttRow(base, "/stations", false, "Liste der gespeicherten Sender als JSON; wird nach jeder &Auml;nderung neu gesendet.", exStations, 1);
    h += mqttRow(base, "/volume/state", false, "Gerade wirkende Lautst&auml;rke (kann die eines Senders sein).", exVolState, 1);
    h += mqttRow(base, "/volume/general", false, "Allgemeine Lautst&auml;rke.", exVolState, 1);
    h += mqttRow(base, "/availability", false, "Ger&auml;t erreichbar (<code>online</code>) oder weg (<code>offline</code>, kommt vom Broker).", exAvail, 2);
    h += "</table><p><small style='font-size:.9rem'>Fertige <code>mosquitto_pub</code>-Aufrufe zum Kopieren stehen im Tab <strong>Doku</strong>.</small></p></div>";
    return h;
}
String panelWlan() {
    String h = "<div class='card'><h3>WLAN und Hostname</h3><form method='POST' action='/save'><input type='hidden' name='sect' value='wlan'>";
    h += "<div class='row'><input name='host' maxlength='31' placeholder='Hostname' value='" + htmlEsc(cfg.host) + "' style='width:260px'> " + infoTip("Name im Netzwerk (http://Name.local/); nur a-z, 0-9, Bindestrich; wirkt nach einem Neustart") + "</div>";
    h += "<small>Name im Netzwerk (http://Hostname.local/, OTA, MQTT-Standardtopics). Erlaubt: a-z, 0-9, Bindestrich. Wirkt nach einem Neustart.</small><hr>";
    h += "<div class='row'><button type='button' onclick='scanWifi()' style='width:200px'>Netzwerke suchen</button> " + infoTip("Sucht Funknetze in der N&auml;he und bietet sie in den Listen an") + "</div><div class='row'><strong>Gespeicherte Netzwerke</strong> " + infoTip("Bis zu drei Netze; sie werden in dieser Reihenfolge probiert. Links ein gefundenes Netz ausw&auml;hlen, es wird in das Feld SSID &uuml;bernommen. Ein leeres Passwortfeld beh&auml;lt das gespeicherte Passwort") + "</div><small id='scanInfo'></small>";
    for (int i = 0; i < 3; i++) {
        String n = String(i + 1);
        h += "<div class='row'><select id='ssid_select" + n + "' onchange=\"document.getElementById('ssid" + n + "').value=this.value\" style='max-width:180px;width:180px'></select>";
        h += "<input name='ssid" + n + "' id='ssid" + n + "' placeholder='SSID " + n + "' value='" + htmlEsc(cfg.ssid[i]) + "' style='width:110px'>";
        h += "<input name='pass" + n + "' type='password' placeholder='Passwort' value='' style='width:110px'>";
        if (WiFi.status() == WL_CONNECTED && WiFi.SSID() == cfg.ssid[i] && cfg.ssid[i].length() > 0) h += "<small>aktiv verbunden</small>";
        h += "</div>";
    }
    h += "<small>Das Passwort wird nicht angezeigt. Leer lassen, um das gespeicherte zu behalten.</small><br>";
    h += "<button type='submit'>Speichern</button></form></div>";
    return h;
}

String panelTime() {
    static const char* const tzValues[] = {"CET-1CEST,M3.5.0,M10.5.0/3", "CEST-2", "CET-1", "GMT0BST,M3.5.0/1,M10.5.0", "UTC0", "EST5EDT,M3.2.0,M11.1.0", "EET-2EEST,M3.5.0/3,M10.5.0/4"};
    static const char* const tzLabels[] = {"Mitteleuropa (DST auto)", "Mitteleuropa (feste Sommerzeit)", "Mitteleuropa (feste Winterzeit)", "Gro&szlig;britannien (DST auto)", "UTC", "US Ost (DST auto)", "Osteuropa (DST auto)"};
    String h = "<div class='card'><h3>Zeit</h3><form method='POST' action='/save'><input type='hidden' name='sect' value='time'>";
    int ntpVisible = 1;
    for (int i = 0; i < 5; i++) {
        if (!cfg.ntp[i].isEmpty()) ntpVisible = i + 2;
    }
    if (ntpVisible > 5) ntpVisible = 5;
    for (int i = 1; i <= 5; i++) {
        String n = String(i);
        h += "<div id='ntpRow" + n + "'" + (i > ntpVisible ? " style='display:none'" : "") + "><div class='row'>NTP Server" + n + " " + infoTip("Zeitserver (NTP), zum Beispiel pool.ntp.org") + " : <input type='text' id='ntpServerInput" + n + "' name='ntp" + n + "' value='" + htmlEsc(cfg.ntp[i - 1]) + "' oninput='ntpGrow(" + n + ")' style='width:180px'>";
        h += "<button type='button' onclick='testNtp(" + n + ")' style='width:100px'>Test</button> " + infoTip("Fragt diesen Server ab und zeigt die empfangene UTC-Zeit") + "</div><div id='ntpTestResult" + n + "'></div></div>";
    }
    h += "<small>Bis zu 5 Server, leere Felder werden &uuml;bersprungen. Das Ger&auml;t fragt die ersten drei; kommt innerhalb von 45 s keine Zeit an, probiert es die weiteren.</small>";
    h += "<div class='row'><select id='tz_select' style='width:300px' onchange=\"document.getElementById('tz_input').value=this.value\">";
    for (int i = 0; i < 7; i++) {
        h += "<option value='" + String(tzValues[i]) + "'" + (cfg.tz == tzValues[i] ? " selected" : "") + ">" + tzLabels[i] + " (" + tzValues[i] + ")</option>";
    }
    h += "</select> " + infoTip("Zeitzone aus der Liste w&auml;hlen; der Wert wird in das Feld darunter &uuml;bernommen") + "</div><div class='row'><input type='text' id='tz_input' name='tz' value='" + htmlEsc(cfg.tz) + "' style='width:300px'> " + infoTip("Zeitzone als POSIX-Text, zum Beispiel CET-1CEST,M3.5.0,M10.5.0/3") + "</div>";
    h += "<small>F&uuml;r eigene Zeitzonen eine Vorgabe w&auml;hlen oder den Wert oben selbst eintragen.</small><br>";
    h += "<button type='submit'>Speichern</button></form></div>";
    return h;
}

String statusRow(const String& label, const String& value) {
    return "<tr><td>" + label + "</td><td>" + htmlEsc(value) + "</td></tr>";
}

String panelStatus() {
    char up[32];
    unsigned long s = millis() / 1000;
    snprintf(up, sizeof(up), "%lud %02lu:%02lu:%02lu", s / 86400, (s / 3600) % 24, (s / 60) % 60, s % 60);
    String h = "<div class='card'><h3>Status</h3><table>";
    h += statusRow("Chip", String(ESP.getChipModel()));
    h += statusRow("Chip Revision", String(ESP.getChipRevision()));
    h += statusRow("Firmware", String(FIRMWARE_MARKER + FIRMWARE_MARKER_PREFIX_LEN));
    h += statusRow("Uptime", String(up));
    h += statusRow("Free Heap", String(ESP.getFreeHeap()) + " bytes (minimum since boot " + String(ESP.getMinFreeHeap()) + ")");
    h += statusRow("Reset Reason", resetReasonName());
    h += statusRow("Crashes in a Row", String(bootCrashes));
    h += statusRow("Safe Mode", safeMode ? String("ON (no MQTT, no radio reconnect)") : String("off"));
    h += statusRow("WiFi Reconnects", String(wifiReconnects));
    h += statusRow("MQTT Reconnects", String(mqttReconnects));
    h += statusRow("PSRAM", String(ESP.getPsramSize()) + " bytes (free " + String(ESP.getFreePsram()) + ")");
    h += statusRow("Flash Size", String(ESP.getFlashChipSize()) + " bytes");
    h += statusRow("LittleFS", fsReady ? String(LittleFS.usedBytes()) + " / " + String(LittleFS.totalBytes()) + " bytes" : String("not mounted"));
    h += statusRow("SD Card", sdState);
    if (sdReady) {
        unsigned long sdTotal = (unsigned long)(SD.totalBytes() / 1048576ULL);
        unsigned long sdUsed = (unsigned long)(SD.usedBytes() / 1048576ULL);
        unsigned long sdFree = sdTotal > sdUsed ? sdTotal - sdUsed : 0;
        h += statusRow("SD Size", String(sdTotal) + " MB (card " + String((unsigned long)(SD.cardSize() / 1048576ULL)) + " MB)");
        h += statusRow("SD Used", String(sdUsed) + " MB" + (sdTotal > 0 ? " (" + String(sdUsed * 100UL / sdTotal) + " %)" : String()));
        h += statusRow("SD Free", String(sdFree) + " MB");
    }
    h += statusRow("SD Enabled", cfg.sdOn ? String("yes") : String("no"));
    h += statusRow("SD CS GPIO", String(cfg.sdCs));
    h += statusRow("SD SCK GPIO", String(cfg.sdSck));
    h += statusRow("SD MOSI GPIO", String(cfg.sdMosi));
    h += statusRow("SD MISO GPIO", String(cfg.sdMiso));
    h += statusRow("WiFi SSID", WiFi.status() == WL_CONNECTED ? WiFi.SSID() : String("not connected"));
    h += statusRow("WiFi RSSI", WiFi.status() == WL_CONNECTED ? String(WiFi.RSSI()) + " dBm" : String("-"));
    h += statusRow("IP Address", WiFi.localIP().toString());
    h += statusRow("MAC Address", WiFi.macAddress());
    h += statusRow("Audio DIN GPIO", String(cfg.din));
    h += statusRow("Audio BCLK GPIO", String(cfg.bclk));
    h += statusRow("Audio LRC GPIO", String(cfg.lrc));
    h += statusRow("Audio State", playStateName());
    h += statusRow("Audio Last Error", lastError.length() > 0 ? lastError : String("-"));
    h += statusRow("VoiceRSS Key", cfg.vkey.length() > 0 ? String("set") : String("not set"));
    h += statusRow("MQTT Status", cfg.mqttServer.isEmpty() ? String("disabled") : (mqttClient.connected() ? String("connected") : String("disconnected")));
    h += "</table></div>";
    h += "<div class='card'><h3>Sprache / Language</h3><p>Weitere Sprachen kommen als Datei (zum Beispiel <code>en.json</code>) in den Speicher des Ger&auml;ts und werden im Browser angewendet. Mit der Schaltfl&auml;che in der Statuszeile wechselst du die Sprache.</p><div id='langList'></div>";
    h += "<div class='row'><input type='file' id='langFile' accept='.json,application/json' style='width:60%'><button type='button' onclick='langUpload()' style='width:auto;padding:8px 14px'>Hochladen</button></div>";
    h += "<div class='row'><input type='text' id='langCode' value='en.json' maxlength='24' style='width:130px'><button type='button' onclick='langFetch()' style='width:auto;padding:8px 14px'>Von GitHub laden</button> " + infoTip("L&auml;dt die Sprachdatei (zum Beispiel en.json) aus dem Ordner lang des Projekts auf GitHub (holgiw/ESP32-netspeaker) in das Ger&auml;t; dazu braucht das Ger&auml;t Internet. Eine vorhandene Datei gleichen Namens wird ersetzt") + "</div><div id='langResult'></div></div>";
    return h;
}

String panelLog() {
    String h = "<div class='card' style='max-width:900px'><form method='POST' action='/save'><input type='hidden' name='sect' value='log'>";
    h += "<div class='row'><label style='display:flex;align-items:center;gap:6px;cursor:pointer;'><input type='checkbox' name='log' value='1'" + String(cfg.logEnabled ? " checked" : "") + " style='width:auto;margin:0;'>Log in Datei schreiben (Standard: aus)</label> " + infoTip("Schreibt Ereignisse in eine Datei im Flash (nur zur Fehlersuche; Standard: aus)") + " automatisch aus nach <input type='number' name='logmin' min='0' max='1440' value='" + String(cfg.logAutoMin) + "' style='width:70px'> min " + infoTip("Schaltet das Schreiben des Logs in die Datei nach so vielen Minuten von selbst aus, damit der Flash geschont wird; 0 = nie. Die Zeit l&auml;uft ab dem Einschalten oder ab dem Start des Ger&auml;ts") + "";
    h += "<button type='submit' style='width:auto;padding:8px 16px;margin:6px;'>Speichern</button>";
    h += "<button type='button' onclick='clearLog()' style='width:auto;padding:8px 16px;margin:6px;background:var(--panel);border:1px solid var(--panel-border);color:var(--text);font-weight:normal;'>Log leeren</button></div>";
    h += "<small>Ausgeschaltet wird nichts in den Flash geschrieben; die serielle Ausgabe l&auml;uft immer.</small><div id='logResult'></div></form></div>";
    h += "<div class='card' style='max-width:900px'>";

    // Karte so breit wie das Log-Feld darunter (max-width:900px); beide Werte gehoeren zusammen.
    // Card as wide as the log box below (max-width:900px); both values belong together.

    h += "<div style='display:flex;align-items:center;gap:14px;flex-wrap:wrap;'>";
    h += "<label style='display:flex;align-items:center;gap:6px;white-space:nowrap;cursor:pointer;'><input type='checkbox' id='logAutoRefresh' style='width:auto;margin:0;'>Auto-Aktualisierung (10 s)</label> " + infoTip("Holt das Log alle 10 Sekunden neu") + "";
    h += "<button type='button' id='logRefreshNow' style='background:var(--panel);border:1px solid var(--panel-border);color:var(--text);border-radius:.4rem;padding:4px 12px;font-size:.8rem;cursor:pointer;width:auto;margin:0;font-weight:normal;'>Jetzt aktualisieren</button>";
    h += "<span id='logErrorHint' class='offline-hint'>&#9888; Aktualisierung fehlgeschlagen - letzter Stand wird angezeigt</span></div>";
    h += "<pre id='logContent' style='background:var(--panel);border:1px solid var(--panel-border);border-radius:10px;max-width:900px;height:400px;overflow-y:auto;margin:15px auto;padding:12px 16px;text-align:left;white-space:pre-wrap;word-break:break-word;font-family:monospace;font-size:.85rem;'>Lade &hellip;</pre></div>";
    return h;
}

// Doku-Tab: Aufrufe von REST-API und MQTT; @HOST@ ersetzt das Skript durch die Adresse, unter der die Seite gerade geoeffnet ist.
// Docs tab: calls of the REST API and MQTT; the script replaces @HOST@ with the address the page is currently opened under.

String docCode(const String& code, bool withHost = true) {
    return "<div class='codewrap'><button type='button' class='copy-btn' onclick='copyPre(this)'>Kopieren</button><pre class='code" + String(withHost ? " codehost" : "") + "'>" + htmlEsc(code) + "</pre></div>";
}

String docCard(const String& title, const String& text, const String& code, bool withHost = true) {
    String h = "<div class='card doc' style='max-width:900px'><h3>" + title + "</h3>";
    if (text.length() > 0) h += "<p>" + text + "</p>";
    if (code.length() > 0) h += docCode(code, withHost);
    return h + "</div>";
}

String panelDoc() {
    String base = mqttBaseTopic();
    String h = docCard("Allgemein",
        "Alle Aufrufe gehen an <code>http://" + htmlEsc(cfg.host) + ".local/</code> oder an die IP-Adresse. Parameter k&ouml;nnen als Formular (<code>-d</code>), als Query (<code>?text=...</code>) oder als flaches JSON im Body kommen; Aufrufe per GET sind erlaubt, zum Beispiel f&uuml;r Home-Automation. "
        "Antwort: <code>{\"ok\":true,\"msg\":\"...\"}</code> oder bei einem Fehler HTTP 400 mit <code>{\"ok\":false,\"error\":\"...\"}</code>. Ansagen und MP3-Dateien kommen in eine Warteschlange (bis zu 8) und laufen nacheinander; ein Sender-Befehl wirkt sofort, ein Stopp leert die Warteschlange.", "");
    {
        // Verkabelungsbild: die GPIO-Nummern im Bild sind die eingestellten Pins.
        // Wiring picture: the GPIO numbers in the picture are the configured pins.

        String svg = FPSTR(WIRING_SVG);
        svg.replace("GPIO 3 (BCLK)", "GPIO " + String(cfg.bclk) + " (BCLK)");
        svg.replace("GPIO 4 (LRC)", "GPIO " + String(cfg.lrc) + " (LRC)");
        svg.replace("GPIO 2 (DIN)", "GPIO " + String(cfg.din) + " (DIN)");
        svg.replace("GPIO 10 (MISO)", "GPIO " + String(cfg.sdMiso) + " (MISO)");
        svg.replace("GPIO 11 (CLK)", "GPIO " + String(cfg.sdSck) + " (CLK)");
        svg.replace("GPIO 12 (MOSI)", "GPIO " + String(cfg.sdMosi) + " (MOSI)");
        svg.replace("GPIO 13 (CS)", "GPIO " + String(cfg.sdCs) + " (CS)");
        h += "<div class='card doc' style='max-width:900px'><h3>Verkabelung</h3><p>Anschl&uuml;sse von ESP32-S3, MAX98357A-Verst&auml;rker und SD-Kartenleser (optional). Die GPIO-Nummern im Bild sind die im Tab Audio eingestellten Pins.</p>";
        h += "<div style='background:#fff;border-radius:8px;padding:6px;overflow-x:auto'>" + svg + "</div></div>";
    }
    h += docCard("Text ansagen (VoiceRSS)",
        "<code>text</code> (Pflicht, h&ouml;chstens 500 Zeichen), optional <code>lang</code> (z. B. de-de, en-gb), <code>voice</code> (z. B. Hanna), <code>rate</code> (-10 bis 10), <code>vol</code> (0-100). Ohne Angabe gelten die Werte aus dem Tab VoiceRSS. Der API-Schl&uuml;ssel muss gespeichert sein.",
        R"d(curl -d "text=Hallo Welt" http://@HOST@/api/say
curl "http://@HOST@/api/say?text=Hallo%20Welt&lang=en-gb&voice=Alice&vol=10"
curl -H "Content-Type: application/json" -d '{"text":"Hallo Welt","lang":"de-de","voice":"Hanna","rate":0,"vol":12}' http://@HOST@/api/say)d");
    h += docCard("Webradio oder MP3 per URL",
        "<code>url</code> muss mit http:// oder https:// beginnen. <code>/api/play</code> nimmt <code>url</code> oder <code>file</code>, <code>/api/url</code> nur eine URL. Optional <code>vol</code>. Webradio &uuml;ber https braucht viel RAM, http-Streams sind unkritisch. L&auml;uft ein Webradio und kommt eine Ansage oder eine MP3-Datei, wird das Radio daf&uuml;r ausgeblendet und gestoppt (also stumm), die Ansage gespielt und das Radio danach mit der alten Lautst&auml;rke neu verbunden; vorher wird es sanft ausgeblendet und beim Fortsetzen wieder eingeblendet (Dauer im Tab Audio, Standard 700 ms). Ein Webradio wird beim Start (aus dem Leerlauf) langsam eingeblendet, beim Stopp langsam ausgeblendet, und bei einem Senderwechsel wird der alte langsam aus- und der neue langsam eingeblendet (Tab Audio, Standard 1500 ms). Wird derselbe Sender noch einmal gew&auml;hlt, spielt er einfach weiter; ein Live-Stream geht live weiter, nicht an derselben Stelle. <code>/api/stop</code> beendet das Radio ohne Fortsetzung, ein neuer URL-Befehl ersetzt es.",
        R"d(curl -d "url=http://stream.example.com/radio.mp3" http://@HOST@/api/play
curl -d "url=http://stream.example.com/radio.mp3" http://@HOST@/api/url
curl "http://@HOST@/api/url?url=http://stream.example.com/radio.mp3&vol=10")d");
    h += docCard("SD-Karte",
        "Dateien der SD-Karte (FAT32, nur MP3) werden mit dem Namen <code>sd:/Ordner/Datei.mp3</code> angesprochen, in <code>file</code> von <code>/api/play</code> und <code>/api/file</code> und ebenso per MQTT (<code>/file</code>, <code>/play</code>). Endet der Name mit <code>/</code>, spielt das Ger&auml;t alle MP3-Dateien des Ordners nacheinander (nach Name sortiert); sie kommen einzeln in die Warteschlange, Ansagen dazwischen kommen zuerst. Mit <code>shuffle=1</code> ist die Reihenfolge je Sitzung zuf&auml;llig und jede Datei kommt einmal vor, die erste ist nie die der letzten Sitzung dieses Ordners; mit <code>loop=1</code> beginnt nach der letzten Datei eine neue Runde (bei shuffle neu gemischt, die erste Datei ist nie die zuletzt gespielte). Ein neuer Sender oder Stopp beendet die Sitzung. <code>/api/sd?dir=/Ordner</code> listet einen Ordner, <code>/api/sdmount</code> bindet die Karte neu ein. Dateimanager: Hochladen mit <code>/upload?target=sd&amp;dir=/Ordner</code>, <code>/api/sdmkdir</code> (dir, name), <code>/api/sdrename</code> (path, to), <code>/api/sddelete</code> (path; ein Ordner wird samt Inhalt gel&ouml;scht) und <code>/api/sdfile?path=...</code> zum Herunterladen. Namen werden bereinigt (Umlaute umgeschrieben, Leerzeichen zu Unterstrichen).",
        R"d(curl -d "file=sd:/Musik/lied.mp3" http://@HOST@/api/play
curl -d "file=sd:/Musik/Album/" http://@HOST@/api/play
curl -d "file=sd:/Musik/&shuffle=1&loop=1" http://@HOST@/api/play
curl "http://@HOST@/api/sd?dir=/Musik"
curl -F "file=@lied.mp3" "http://@HOST@/upload?target=sd&dir=/Musik"
curl -d "dir=/Musik&name=Neu" http://@HOST@/api/sdmkdir
curl -d "path=/Musik/lied.mp3&to=neu.mp3" http://@HOST@/api/sdrename
curl -d "path=/Musik/lied.mp3" http://@HOST@/api/sddelete
curl -o lied.mp3 "http://@HOST@/api/sdfile?path=/Musik/lied.mp3"
curl -X POST http://@HOST@/api/sdmount)d");
    h += docCard("Gespeicherte Webradios / URLs",
        "Im Tab Webradio lassen sich 30 Pl&auml;tze mit Name, URL und eigener Lautst&auml;rke speichern (leer = allgemeine Lautst&auml;rke; ein <code>vol</code> im Aufruf hat Vorrang). <code>n</code> ist die Platznummer (1-30), alternativ <code>name</code> (Gro&szlig;-/Kleinschreibung egal). <code>/api/stations</code> liefert die belegten Pl&auml;tze (JSON mit n, name, url, vol), <code>/api/stationdelete</code> l&ouml;scht einen Platz (Name und URL), die folgenden Pl&auml;tze r&uuml;cken nach oben. <code>/stations.txt</code> l&auml;dt die Liste als Textdatei, <code>POST /importstations</code> ersetzt sie durch eine solche Datei (im Tab Webradio: Liste speichern / Liste laden).",
        R"d(curl -d "n=3" http://@HOST@/api/station
curl "http://@HOST@/api/station?name=Radio%20Eins&vol=10"
curl http://@HOST@/api/stations
curl http://@HOST@/stations.txt -o stations.txt
curl --data-binary @stations.txt -H "Content-Type: text/plain" http://@HOST@/importstations
curl -d "n=3" http://@HOST@/api/stationdelete)d");
    h += docCard("Hochgeladene MP3-Datei abspielen",
        "<code>file</code> ist der Dateiname aus dem Tab MP3 (Flash). Dort gibt es f&uuml;r jede Datei Kn&ouml;pfe, die den fertigen Aufruf kopieren. Mit <code>/api/files</code> kommt die Liste, mit <code>/api/delete</code> wird eine Datei gel&ouml;scht. Hochladen geht per <code>/upload</code> (Formular, Feld <code>file</code>). Ein Name ohne <code>sd:</code> wird zuerst im Flash und dann auf der SD-Karte gesucht (<code>sd:/Pfad.mp3</code> nur auf der Karte).",
        R"d(curl -d "file=glocke.mp3" http://@HOST@/api/file
curl "http://@HOST@/api/file?file=glocke.mp3&vol=15"
curl http://@HOST@/api/files
curl -d "file=glocke.mp3" http://@HOST@/api/delete
curl -F "file=@glocke.mp3" http://@HOST@/upload)d");
    h += docCard("Stopp, Lautst&auml;rke, Status",
        "<code>v</code> ist 0-100 und wird gespeichert; die Lautst&auml;rke wandert sanft zum neuen Wert, die Dauer in Millisekunden steht im Tab Audio und l&auml;sst sich je Aufruf mit <code>ms</code> &uuml;berschreiben. Ohne <code>v</code> meldet <code>/api/volume</code> nur: <code>volume</code> (allgemein) und <code>playVolume</code> (gerade wirkend). <code>/api/status</code> liefert <code>state</code> (idle, loading, playing), <code>error</code>, <code>queue</code> (wartende Ansagen und Dateien), <code>volume</code> (allgemein), <code>playVolume</code> (die gerade wirkende Lautst&auml;rke), den freien Speicher und was gerade l&auml;uft: <code>source</code> (radio, file, speech), <code>name</code> (Sendername oder Dateiname), <code>title</code> (aktueller Titel, wenn der Sender ihn meldet) und <code>url</code>. Bei einer laufenden Datei kommen <code>pos</code> und <code>dur</code> (Sekunden), <code>pm</code> (Fortschritt in Promille) und <code>rate</code> (Promille je Sekunde) dazu, bei einem Webradio <code>genre</code>, <code>desc</code>, <code>web</code>, <code>logo</code>, <code>host</code>, <code>bitrate</code>, <code>codec</code>, <code>rate</code> (Abtastrate in Hz), <code>ch</code> und <code>bits</code>, und mit der Lautst&auml;rke-Angleichung <code>gainDb</code>.",
        R"d(curl -X POST http://@HOST@/api/stop
curl -d "v=50" http://@HOST@/api/volume
curl -d "v=60&ms=3000" http://@HOST@/api/volume
curl http://@HOST@/api/volume
curl http://@HOST@/api/status)d");
    h += docCard("Log",
        "<code>enable</code> schaltet das Log in die Datei ein (1) oder aus (0), Standard ist aus. <code>clear=1</code> leert die Datei. Ohne Parameter liefert GET den Zustand. <code>/log</code> gibt den Inhalt als Text zur&uuml;ck.",
        R"d(curl http://@HOST@/api/log
curl -d "enable=1" http://@HOST@/api/log
curl -d "clear=1" http://@HOST@/api/log
curl http://@HOST@/log)d");
    h += docCard("MQTT-Broker per API setzen",
        "Setzt nur Broker und Port (optional auch <code>user</code> und <code>pass</code>), alles andere bleibt unver&auml;ndert. GET liefert Broker, Port, <code>user</code> und <code>connected</code>; das Passwort wird nie ausgegeben. Mit <code>group1</code> bis <code>group5</code> lassen sich die Gruppen-Topics setzen (leer = entfernt); GET liefert sie als <code>groups</code>.",
        R"d(curl -d "broker=192.168.0.70&port=1883" http://@HOST@/api/mqtt
curl -d "group1=netspeaker/all&group2=netspeaker/haus" http://@HOST@/api/mqtt
curl http://@HOST@/api/mqtt)d");
    h += docCard("Eing&auml;nge (Pins)",
        "<code>GET /api/pins</code> liefert die freien Pins: <code>p</code> (Nummer), <code>l</code> (Zustand, 1 = High, 0 = Low), <code>t</code> (Aktion), <code>x</code> (Text) und <code>v</code> (Lautst&auml;rke, -1 = allgemeine). <code>POST /api/pins</code> mit <code>pin</code>, <code>type</code> (<code>say</code>, <code>file</code>, <code>station</code> oder leer = keine Aktion), <code>text</code> (Ansage, Dateiname, <code>sd:</code>-Pfad, URL oder Sender) und optional <code>vol</code> (0-100) speichert die Aktion eines Pins. Sie l&ouml;st aus, sobald der Pin etwa 40 ms Kontakt nach GND hat.",
        R"d(curl http://@HOST@/api/pins
curl -d "pin=5&type=say&text=Die Tür ist offen" http://@HOST@/api/pins
curl -d "pin=6&type=file&text=gong.mp3&vol=60" http://@HOST@/api/pins
curl -d "pin=7&type=station&text=RSA" http://@HOST@/api/pins
curl -d "pin=5&type=" http://@HOST@/api/pins)d");
    h += docCard("Sprachen",
        "Sprachdateien (zum Beispiel <code>en.json</code>) liegen im Speicher des Ger&auml;ts. <code>/upload?target=lang</code> (Formular, Feld <code>file</code>) l&auml;dt eine hoch, <code>GET /api/lang</code> listet die vorhandenen Sprachen, <code>POST /api/lang</code> mit <code>fetch=en</code> l&auml;dt die Datei vom Projekt auf GitHub, mit <code>delete=en</code> l&ouml;scht sie, und <code>/lang.json?l=en</code> liefert die Datei.",
        R"d(curl http://@HOST@/api/lang
curl -F "file=@en.json" "http://@HOST@/upload?target=lang"
curl -d "fetch=en" http://@HOST@/api/lang
curl -d "delete=en" http://@HOST@/api/lang
curl "http://@HOST@/lang.json?l=en")d");
    h += docCard("Weitere Aufrufe",
        "<code>/api/scan</code> (WLAN-Suche), <code>/api/testNtp?server=...</code> (NTP-Test, nur im Tab Zeit n&ouml;tig), <code>/api/topbarStatus</code> (Statusleiste), <code>/api/version</code> (Firmware-Stand), <code>/export</code> und <code>/import</code> (Einstellungen ohne WLAN-Passw&ouml;rter, mit API-Schl&uuml;ssel), <code>/restart</code> (POST, Neustart).",
        R"d(curl http://@HOST@/api/version
curl -o netspeaker-settings.txt http://@HOST@/export
curl -X POST http://@HOST@/restart)d");
    h += docCard("MQTT",
        "Basis-Topic <code>" + htmlEsc(base) + "</code> (im Tab MQTT &auml;nderbar). Nutzlast als Text oder als JSON. <code>/say</code>: Text oder <code>{\"text\":\"...\",\"lang\":\"..\",\"voice\":\"..\",\"rate\":0,\"vol\":60}</code>. <code>/play</code>: URL oder Dateiname, auch <code>{\"url\":\"...\"}</code> oder <code>{\"file\":\"...\"}</code>. <code>/url</code> nur URL, <code>/file</code> nur Dateiname, <code>/station</code> Platz 1-30 oder Name (auch JSON <code>{\"n\":3}</code> oder <code>{\"name\":\"..\"}</code>). <code>/stop</code> ohne Nutzlast. <code>/volume</code> 0-100. <code>/restart</code> ohne Nutzlast startet das Ger&auml;t neu (nicht retained senden; eine Nachricht in den ersten 3 s nach dem Verbinden wird ignoriert). "
        "Zur&uuml;ck kommen <code>/status</code> (idle, loading, playing, error: ...), <code>/now</code> (was gerade l&auml;uft, zum Beispiel &quot;Sendername - Titel&quot;) und <code>/availability</code> (online, offline). <code>/volume/state</code> (gerade wirkende Lautst&auml;rke) und <code>/volume/general</code> (allgemeine) werden bei jeder &Auml;nderung gesendet. <code>/stations</code> enth&auml;lt die Senderliste (retained, wird nach jeder &Auml;nderung neu gesendet); ein Aufruf von <code>/list</code> sendet sie erneut; ein ung&uuml;ltiger Befehl erscheint in <code>/status</code> als Fehler. Zus&auml;tzliche Gruppen-Topics (bis zu 5, im Tab MQTT) nehmen dieselben Befehle an, zum Beispiel <code>netspeaker/all/stop</code> f&uuml;r alle Ger&auml;te; Standard ist <code>netspeaker/all</code>.",
        "mosquitto_pub -h <broker> -t " + base + "/say -m \"Hallo Welt\"\n"
        "mosquitto_pub -h <broker> -t " + base + "/say -m '{\"text\":\"Hallo\",\"lang\":\"en-gb\",\"vol\":10}'\n"
        "mosquitto_pub -h <broker> -t " + base + "/play -m \"http://stream.example.com/radio.mp3\"\n"
        "mosquitto_pub -h <broker> -t " + base + "/file -m \"glocke.mp3\"\n"
        "mosquitto_pub -h <broker> -t " + base + "/station -m 3\n"
        "mosquitto_pub -h <broker> -t " + base + "/station -m \"Radio Eins\"\n"
        "mosquitto_pub -h <broker> -t " + base + "/stop -n\n"
        "mosquitto_pub -h <broker> -t " + base + "/volume -m 50\n"
        "mosquitto_pub -h <broker> -t " + base + "/list -n\n"
        "mosquitto_pub -h <broker> -t " + base + "/restart -n\n"
        "mosquitto_sub -h <broker> -t " + base + "/stations -C 1\n"
        "mosquitto_sub -h <broker> -t \"" + base + "/#\" -v", false);
    h += docCard("Kontakt", "E-Mail: <a href='mailto:carleaner@gmx.de'>carleaner@gmx.de</a><br>GitHub: <a href='https://github.com/holgiw/ESP32-netspeaker' target='_blank' rel='noopener'>github.com/holgiw/ESP32-netspeaker</a>", "");
    return h;
}
static const char PINS_JS[] PROGMEM = R"js(
(function(){
  var picks=[],spicks=[],fl=[],sdl=[],stl=[];
  function el(n,c){var e=document.createElement(n);if(c)e.className=c;return e;}
  function lvl(j){var s=document.getElementById('pinlvl'+j.p);if(!s)return;s.textContent=j.l?'High':'Low (GND)';s.style.color=j.l?'var(--muted)':'var(--ok)';s.style.fontWeight=j.l?'normal':'bold';}
  function fillPick(p){
    p.textContent='';var o=el('option');o.value='';o.textContent='Datei w\u00e4hlen \u2026';p.appendChild(o);
    if(fl.length){var g=el('optgroup');g.label='Flash';fl.forEach(function(f){var x=el('option');x.value=f;x.textContent=f;g.appendChild(x);});p.appendChild(g);}
    if(sdl.length){var g2=el('optgroup');g2.label='SD-Karte';sdl.forEach(function(s){var x=el('option');x.value=s[0];x.textContent=s[1];g2.appendChild(x);});p.appendChild(g2);}
  }
  function fillSta(p){p.textContent='';var o=el('option');o.value='';o.textContent='Sender w\u00e4hlen \u2026';p.appendChild(o);stl.forEach(function(s){var x=el('option');x.value=s[0];x.textContent=s[1];p.appendChild(x);});}
  function lists(cb){
    Promise.all([
      fetch('/api/files',{cache:'no-store'}).then(function(r){return r.json();}).catch(function(){return {};}),
      fetch('/api/sd?dir=/',{cache:'no-store'}).then(function(r){return r.json();}).catch(function(){return {};}),
      fetch('/api/stations',{cache:'no-store'}).then(function(r){return r.json();}).catch(function(){return {};})
    ]).then(function(a){
      stl=((a[2]&&a[2].stations)||[]).map(function(s){return [s.name||String(s.n),s.n+': '+(s.name||s.url)];});
      fl=(a[0].files||[]).map(function(f){return f.name;});
      sdl=[];var s=a[1];
      if(s&&s.mounted){sdl.push(['sd:/','\uD83D\uDCC1 / (Hauptverzeichnis)']);(s.entries||[]).forEach(function(e){sdl.push(e.d?['sd:/'+e.n+'/','\uD83D\uDCC1 '+e.n]:['sd:/'+e.n,e.n]);});}
      picks.forEach(fillPick);spicks.forEach(fillSta);if(cb)cb();
    });
  }
  function build(list){
    var tb=document.getElementById('pinTable');if(!tb)return;tb.textContent='';picks=[];spicks=[];
    var h=el('tr');['GPIO','Zustand','Aktion','Text oder Datei','Lautst\u00e4rke',''].forEach(function(t){var c=el('th');c.textContent=t;h.appendChild(c);});tb.appendChild(h);
    list.forEach(function(j){
      var tr=el('tr'),c1=el('td'),c2=el('td'),c3=el('td'),c4=el('td'),c5=el('td'),c6=el('td');
      c1.textContent='GPIO '+j.p;c1.style.whiteSpace='nowrap';c2.style.whiteSpace='nowrap';
      var st=el('span');st.id='pinlvl'+j.p;c2.appendChild(st);
      var sel=el('select');sel.style.cssText='width:auto;margin:2px';[['','Keine Aktion'],['say','Ansage (VoiceRSS)'],['file','MP3-Datei / URL'],['station','Webradio']].forEach(function(o){var op=el('option');op.value=o[0];op.textContent=o[1];sel.appendChild(op);});sel.value=j.t||'';c3.appendChild(sel);
      var tx=el('input');tx.type='text';tx.maxLength=300;tx.value=j.x||'';tx.placeholder='z. B. Die T\u00fcr ist offen / gong.mp3';tx.style.cssText='width:95%;margin:2px';c4.appendChild(tx);
      var pk=el('select');pk.style.cssText='width:95%;margin:2px';fillPick(pk);picks.push(pk);c4.appendChild(pk);
      pk.onchange=function(){if(pk.value){tx.value=pk.value;pk.value='';}};
      var ps=el('select');ps.style.cssText='width:95%;margin:2px';fillSta(ps);spicks.push(ps);c4.appendChild(ps);
      ps.onchange=function(){if(ps.value){tx.value=ps.value;ps.value='';}};
      function vis(){pk.style.display=sel.value==='file'?'':'none';ps.style.display=sel.value==='station'?'':'none';}
      sel.onchange=vis;vis();
      var vo=el('input');vo.type='number';vo.min=0;vo.max=100;vo.value=j.v>=0?j.v:'';vo.placeholder='Std';vo.style.cssText='width:64px;margin:2px';c5.appendChild(vo);
      var bt=el('button');bt.type='button';bt.className='copy-btn';bt.style.cssText='position:static;margin:2px';bt.textContent='Speichern';
      bt.onclick=function(){postApi('/api/pins',{pin:j.p,type:sel.value,text:tx.value,vol:vo.value},'pinResult');};c6.appendChild(bt);
      [c1,c2,c3,c4,c5,c6].forEach(function(c){tr.appendChild(c);});tb.appendChild(tr);lvl(j);
    });
    if(!list.length){var r=el('tr'),c=el('td');c.textContent='Keine freien Pins';r.appendChild(c);tb.appendChild(r);}
  }
  function load(first){fetch('/api/pins',{cache:'no-store'}).then(function(r){return r.json();}).then(function(j){if(first)build(j.pins||[]);else(j.pins||[]).forEach(lvl);}).catch(function(){});}
  lists(function(){load(true);});
  var tab=document.getElementById('tab-pins');if(tab)tab.addEventListener('change',function(){if(tab.checked)lists();});
  setInterval(function(){var t=document.getElementById('tab-pins');if(document.hidden||!t||!t.checked)return;load(false);},1000);
})();
)js";
static const char LANG_JS[] PROGMEM = R"js(
(function(){
  var code='';try{code=localStorage.getItem('uilang')||'';}catch(e){}
  var T=null,R=[],mo=null,SKIP=/^(SCRIPT|STYLE|CODE|PRE|TEXTAREA|SVG)$/,INL=/^(CODE|EM|STRONG|B|KBD)$/,ATTR=['title','placeholder','aria-label','label'];
  function norm(s){return s.replace(/\s+/g,' ').trim();}
  function look(k){var kk=UI_HOST?k.split(UI_HOST).join('{host}'):k,v=T[kk];
    if(v===undefined){for(var i=0;i<R.length;i++){if(R[i][0].test(kk)){v=kk.replace(R[i][0],R[i][1]);break;}}}
    return v===undefined?undefined:(UI_HOST?v.split('{host}').join(UI_HOST):v);}
  function trText(s){var k=norm(s);if(!k)return s;var v=look(k);if(v===undefined)return s;return s.match(/^\s*/)[0]+v+s.match(/\s*$/)[0];}
  function safe(v){var tp=document.createElement('template');tp.innerHTML=v;[].forEach.call(tp.content.querySelectorAll('*'),function(e){if(!INL.test(e.nodeName.toUpperCase())){e.remove();return;}while(e.attributes.length)e.removeAttribute(e.attributes[0].name);});return tp.innerHTML;}
  function trAttrs(n){ATTR.forEach(function(a){if(n.hasAttribute&&n.hasAttribute(a)){var v=n.getAttribute(a),t=trText(v);if(t!==v)n.setAttribute(a,t);}});}
  function inlineMixed(e){var el=0,tx=0;for(var c=e.firstChild;c;c=c.nextSibling){if(c.nodeType===1){if(!INL.test(c.nodeName.toUpperCase())||c.id||c.querySelector('[id]'))return 0;el=1;}else if(c.nodeType===3){if(/[A-Za-zÄÖÜäöüß]/.test(c.nodeValue))tx=1;}else if(c.nodeType!==8)return 0;}return el&&tx;}
  function walk(n){
    if(n.nodeType===3){var p=n.parentNode;if(p&&SKIP.test(p.nodeName.toUpperCase()))return;var t=trText(n.nodeValue);if(t!==n.nodeValue)n.nodeValue=t;return;}
    if(n.nodeType!==1||SKIP.test(n.nodeName.toUpperCase()))return;
    trAttrs(n);
    if(inlineMixed(n)){var v=look(norm(n.innerHTML));if(v!==undefined){n.innerHTML=safe(v);return;}}
    for(var c=n.firstChild;c;c=c.nextSibling)walk(c);
  }
  function watch(){mo.observe(document.body,{childList:true,subtree:true,characterData:true,attributes:true,attributeFilter:ATTR});}
  function onMut(ms){mo.disconnect();ms.forEach(function(m){if(m.type==='childList')m.addedNodes.forEach(walk);else if(m.type==='characterData')walk(m.target);else trAttrs(m.target);});watch();}
  function dialogs(){['confirm','prompt','alert'].forEach(function(n){var o=window[n];window[n]=function(m,d){return o.call(window,typeof m==='string'?trText(m):m,d);};});}
  function langList(){fetch('/api/lang',{cache:'no-store'}).then(function(r){return r.json();}).then(function(j){
    var list=j.langs||[],b=document.getElementById('langBtn'),box=document.getElementById('langList');
    if(b){if(list.length||code){b.style.display='';b.textContent=code?'DE':list[0].toUpperCase();b.onclick=function(){try{if(code)localStorage.removeItem('uilang');else localStorage.setItem('uilang',list[0]);}catch(e){}location.reload();return false;};}else b.style.display='none';}
    if(box){box.textContent='';list.forEach(function(c){var d=document.createElement('div');d.className='row';var s=document.createElement('span');s.textContent=c+'.json';d.appendChild(s);
      var u=document.createElement('button');u.type='button';u.className='copy-btn';u.style.cssText='position:static;margin:2px';u.textContent='\u2716';u.title='Sprache l\u00f6schen';u.onclick=function(){if(confirm(c+'.json l\u00f6schen?'))postApi('/api/lang',{'delete':c},'langResult').then(langList);};d.appendChild(u);box.appendChild(d);});
      if(!list.length)box.textContent='Keine Sprachdatei im Ger\u00e4t';}
    if(mo&&T){mo.disconnect();if(b)walk(b);if(box)walk(box);watch();}
  }).catch(function(){});}
  window.langFetch=function(){var c=(document.getElementById('langCode').value||'').trim().toLowerCase().replace(/\.json$/,'');if(!c){showRes('langResult',{ok:false,error:'Keine Datei gew&auml;hlt'});return;}
    showRes('langResult',{ok:true,msg:'Lade von GitHub &hellip;'});postApi('/api/lang',{fetch:c},'langResult').then(langList);};
  window.langUpload=function(){var f=document.getElementById('langFile').files[0];if(!f){showRes('langResult',{ok:false,error:'Keine Datei gew&auml;hlt'});return;}
    var fd=new FormData();fd.append('file',f,f.name);
    fetch('/upload?target=lang',{method:'POST',body:fd}).then(function(r){return r.json();}).then(function(j){showRes('langResult',j);document.getElementById('langFile').value='';langList();}).catch(function(){showRes('langResult',{ok:false,error:'Fehler'});});};
  if(code){
    fetch('/lang.json?l='+encodeURIComponent(code),{cache:'no-cache'}).then(function(r){return r.json();}).then(function(j){
      T=j.t||{};R=(j.r||[]).map(function(x){return [new RegExp(x[0]),x[1]];});
      dialogs();mo=new MutationObserver(onMut);walk(document.body);document.title=trText(document.title);document.documentElement.lang=code;watch();langList();
    }).catch(function(){langList();});
  }else langList();
})();
)js";
static const char PAGE_JS[] PROGMEM = R"js(
function showRes(id,r){var el=document.getElementById(id);if(!el)return;
  el.className=r.ok?'res-ok':'res-bad';el.innerHTML=(r.ok?'&#10004; ':'&#10008; ')+(r.ok?(r.msg||'OK'):(r.error||'Fehler'));}
function postApi(path,params,resId){
  return fetch(path,{method:'POST',body:new URLSearchParams(params)}).then(function(r){return r.json();}).then(function(j){showRes(resId,j);return j;})
    .catch(function(){showRes(resId,{ok:false,error:'Keine Antwort vom Ger&auml;t'});});
}
function sayNow(){postApi('/api/say',{text:document.getElementById('sayText').value,lang:document.getElementById('sayLang').value,voice:document.getElementById('sayVoice').value},'sayResult');}
function clearLog(){if(!confirm('Log l\u00f6schen?'))return;postApi('/api/log',{clear:'1'},'logResult').then(function(){var p=document.getElementById('logContent');if(p)p.textContent='';});}
function copyText(t,btn){
  function done(ok){if(btn){var o=btn.getAttribute('data-o')||btn.textContent;btn.setAttribute('data-o',o);btn.textContent=ok?'Kopiert':'Fehler';setTimeout(function(){btn.textContent=o;},1500);}}
  if(navigator.clipboard&&window.isSecureContext){navigator.clipboard.writeText(t).then(function(){done(true);},function(){done(false);});return;}
  var ta=document.createElement('textarea');ta.value=t;ta.style.position='fixed';ta.style.opacity='0';document.body.appendChild(ta);ta.select();
  var ok=false;try{ok=document.execCommand('copy');}catch(e){}document.body.removeChild(ta);done(ok);}
function copyPre(btn){copyText(btn.parentNode.querySelector('pre').textContent,btn);}
function playStation(n){postApi('/api/station',{n:n},'stResult');}
function moveStation(n,d){var a=document.querySelector('.stationrow[data-n="'+n+'"]'),b=document.querySelector('.stationrow[data-n="'+(n+d)+'"]');if(!a||!b||!b.getAttribute('data-url')&&d>0)return;['rn','ru','rv'].forEach(function(p){var x=a.querySelector('[name="'+p+n+'"]'),y=b.querySelector('[name="'+p+(n+d)+'"]'),t=x.value;x.value=y.value;y.value=t;});a.closest('form').submit();}
function delStation(n){if(!confirm('Platz '+n+' (Name und URL) l\u00f6schen?'))return;postApi('/api/stationdelete',{n:n},'stResult').then(function(j){if(j&&j.ok)location.href='/?tab=radio';});}
function stationLink(n){return location.origin+'/api/station?n='+n;}
function apiLink(n){return location.origin+'/api/file?file='+encodeURIComponent(n);}
var VOICES={'de-de':['Hanna','Lina','Jonas'],'en-gb':['Alice','Nancy','Lily','Harry'],'en-us':['Linda','Amy','Mary','John','Mike'],'fr-fr':['Bette','Iva','Zola'],'es-es':['Camila','Sofia','Luna'],'it-it':['Bria','Mia','Pietro'],'pl-pl':['Julia','Jan'],'sv-se':['Molly','Hugo']};
function fillVoiceSel(id,lang,keep){var s=document.getElementById(id);if(!s)return;var cur=s.value;var v=VOICES[lang||DEF_LANG]||[];s.innerHTML='';
  function add(val,txt){var o=document.createElement('option');o.value=val;o.textContent=txt;s.appendChild(o);}
  add('','Standard (VoiceRSS)');v.forEach(function(n){add(n,n);});
  if(keep&&cur&&cur!=='__custom__'&&v.indexOf(cur)<0)add(cur,cur);
  add('__custom__','Andere \u2026');
  s.value=((keep||v.indexOf(cur)>=0)&&cur!=='__custom__')?cur:'';}
function voiceChosen(s){if(s.value!=='__custom__')return;
  var n=(prompt('Name der Stimme (wie bei VoiceRSS):')||'').replace(/[^A-Za-z0-9_-]/g,'');
  if(!n){s.value='';return;}
  var o=document.createElement('option');o.value=n;o.textContent=n;s.insertBefore(o,s.querySelector('option[value="__custom__"]'));s.value=n;}function stopNow(){postApi('/api/stop',{},'sayResult');}
function volShow(v){var e=document.getElementById('volShow'),s=document.getElementById('vol');if(e&&s)e.textContent=v+' / '+s.max;}
function setVol(v){volShow(v);postApi('/api/volume',{v:v},'sayResult');}
function playUrlNow(){postApi('/api/play',{url:document.getElementById('playUrl').value},'radioResult');}
var sdCur='/';
function checkPins(){var s=document.querySelectorAll('select.pinsel'),seen={},dup=false;s.forEach(function(e){e.classList.remove('dup');});
  s.forEach(function(e){var v=e.value;if(seen[v]){seen[v].classList.add('dup');e.classList.add('dup');dup=true;}else seen[v]=e;});
  var w=document.getElementById('pinWarn');if(w)w.textContent=dup?'Ein Pin ist doppelt vergeben; so werden die Pins nicht gespeichert.':'';}
function memShow(id,used,total){var e=document.getElementById(id);if(!e)return;e.textContent='';if(!(total>0))return;
  var u=total<10485760?['KB',1024]:(total<10737418240?['MB',1048576]:['GB',1073741824]),pct=Math.round(100*used/total);
  var s=document.createElement('small');s.textContent='Belegt '+Math.round(used/u[1])+' von '+Math.round(total/u[1])+' '+u[0]+' ('+pct+' %)';
  var bar=document.createElement('div');bar.style.cssText='height:6px;margin:4px 0 8px;background:#3a4147;border-radius:3px;overflow:hidden';
  var f=document.createElement('div');f.style.cssText='height:100%;background:var(--ok);width:'+Math.min(100,pct)+'%';bar.appendChild(f);e.appendChild(s);e.appendChild(bar);}
function sdLoad(dir){var box=document.getElementById('sdList');if(!box)return;dir=dir||'/';
  fetch('/api/sd?dir='+encodeURIComponent(dir),{cache:'no-store'}).then(function(r){return r.json();}).then(function(j){
    var info=document.getElementById('sdInfo'),pth=document.getElementById('sdPath');box.textContent='';sdCur=j.dir||'/';
    if(!j.mounted){info.textContent=j.state==='off'?'SD-Karte ist ausgeschaltet (Tab Audio)':'Keine Karte gefunden';pth.textContent='';memShow('sdMem',0,0);return;}
    info.textContent='Karte '+j.state;memShow('sdMem',j.used*1048576,j.size*1048576);pth.textContent=j.dir;
    var cur=j.dir==='/'?'':j.dir,tb=document.createElement('table');tb.style.width='100%';
    function cell(tr,t){var td=document.createElement('td');if(t!==null)td.textContent=t;tr.appendChild(td);return td;}
    function btn(td,txt,tip,fn){var b=document.createElement('button');b.type='button';b.className='copy-btn';b.style.cssText='position:static;margin:2px';b.textContent=txt;b.title=tip;b.onclick=fn;td.appendChild(b);}
    function dl(td,p){var a=document.createElement('a');a.className='copy-btn';a.style.cssText='position:static;margin:2px;display:inline-block;text-decoration:none';a.textContent='\u2B07';a.title='Datei herunterladen';a.href='/api/sdfile?path='+encodeURIComponent(p);td.appendChild(a);}
    if(j.dir!=='/'){var up=document.createElement('tr'),ut=cell(up,null),ua=document.createElement('a');ua.href='#';ua.textContent='\u2191 ..';ua.onclick=function(){sdLoad(j.dir.replace(/\/[^\/]*$/,'')||'/');return false;};ut.appendChild(ua);cell(up,'');cell(up,'');tb.appendChild(up);}
    if(j.entries.some(function(e){return !e.d;})){var hr=document.createElement('tr'),hc=cell(hr,null),ha;hc.textContent=j.dir==='/'?'📁 / (Hauptverzeichnis)':'📁 dieser Ordner';cell(hr,'');ha=cell(hr,null);
      btn(ha,'▶ Ordner','Alle MP3-Dateien dieses Ordners nacheinander abspielen',function(){playFile('sd:'+cur+'/','sdResult',{loop:sdLoopOn()});});
      btn(ha,'🔀 Zufall','Alle MP3-Dateien dieses Ordners in zufälliger Reihenfolge abspielen, jede einmal je Sitzung',function(){playFile('sd:'+cur+'/','sdResult',{shuffle:1,loop:sdLoopOn()});});
      tb.appendChild(hr);}
    j.entries.forEach(function(e){var tr=document.createElement('tr'),td=cell(tr,null),p=cur+'/'+e.n,act;
      if(e.d){var a=document.createElement('a');a.href='#';a.textContent='\uD83D\uDCC1 '+e.n;a.onclick=function(){sdLoad(p);return false;};td.appendChild(a);cell(tr,'');act=cell(tr,null);
        btn(act,'\u25B6 Ordner','Alle MP3-Dateien des Ordners nacheinander abspielen',function(){playFile('sd:'+p+'/','sdResult',{loop:sdLoopOn()});});
        btn(act,'\uD83D\uDD00 Zufall','Alle MP3-Dateien des Ordners in zuf\u00e4lliger Reihenfolge abspielen, jede einmal je Sitzung',function(){playFile('sd:'+p+'/','sdResult',{shuffle:1,loop:sdLoopOn()});});}
      else{td.textContent=e.n;cell(tr,Math.round(e.s/1024)+' KB');act=cell(tr,null);
        btn(act,'\u25B6','Abspielen (reiht sich in die Warteschlange ein)',function(){playFile('sd:'+p,'sdResult');});dl(act,p);}
      btn(act,'\u270E','Umbenennen',function(){sdRen(p,e.n,e.d);});
      btn(act,'\u2716',e.d?'Ordner mit Inhalt l\u00f6schen':'Datei l\u00f6schen',function(){sdDel(p,e.d);});
      tb.appendChild(tr);});
    if(!j.entries.length)tb.appendChild(document.createElement('tr')).appendChild(document.createElement('td')).textContent='Keine MP3-Dateien oder Ordner';
    box.appendChild(tb);}).catch(function(){});}
function sdRemount(){postApi('/api/sdmount',{},'sdResult').then(function(){sdLoad('/');});}
function sdMkdir(){var n=prompt('Name des neuen Ordners:');if(!n)return;postApi('/api/sdmkdir',{dir:sdCur,name:n},'sdResult').then(function(){sdLoad(sdCur);});}
function sdDel(p,isDir){if(!confirm((isDir?'Ordner mit allem Inhalt ':'Datei ')+p+' l\u00f6schen?'))return;postApi('/api/sddelete',{path:p},'sdResult').then(function(){sdLoad(sdCur);});}
function sdRen(p,old,isDir){var n=prompt('Neuer Name:',old);if(!n||n===old)return;postApi('/api/sdrename',{path:p,to:n},'sdResult').then(function(){sdLoad(sdCur);});}
function sdUploadNow(){var fs=Array.prototype.slice.call(document.getElementById('sdUpFile').files);if(!fs.length){showRes('sdResult',{ok:false,error:'Keine Datei gew&auml;hlt'});return;}
  var i=0,okc=0,bad=[];
  function next(){if(i>=fs.length){document.getElementById('sdUpFile').value='';sdLoad(sdCur);
      showRes('sdResult',bad.length?{ok:false,error:okc+' von '+fs.length+' hochgeladen; Fehler: '+bad.join(', ')}:{ok:true,msg:okc+(okc==1?' Datei':' Dateien')+' hochgeladen'});return;}
    var f=fs[i],fn=f.name.replace(/[<>&"']/g,'_');showRes('sdResult',{ok:true,msg:'Lade hoch ('+(i+1)+'/'+fs.length+'): '+fn+' &hellip;'});
    var fd=new FormData();fd.append('file',f,f.name);
    fetch('/upload?target=sd&dir='+encodeURIComponent(sdCur),{method:'POST',body:fd}).then(function(r){return r.json();}).then(function(j){if(j&&j.ok)okc++;else bad.push(fn+' ('+((j&&j.error)||'Fehler')+')');})
      .catch(function(){bad.push(fn);}).then(function(){i++;next();});}
  next();}
function playFile(n,res,ex){var p={file:n};if(ex)for(var k in ex)p[k]=ex[k];postApi('/api/play',p,res||'upResult');}
function sdLoopOn(){var c=document.getElementById('sdLoop');return c&&c.checked?'1':'0';}
function delFile(n){if(!confirm('Datei '+n+' löschen?'))return;postApi('/api/delete',{file:n},'upResult').then(loadFiles);}
function loadFiles(){fetch('/api/files',{cache:'no-store'}).then(function(r){return r.json();}).then(function(j){
  var h='<table style="width:100%">';
  j.files.forEach(function(f){h+='<tr><td>'+f.name+'</td><td>'+Math.round(f.size/1024)+' KB</td><td><a href="#" onclick="playFile(\''+f.name+'\');return false;" title="Abspielen">&#9654;</a> <a href="#" onclick="delFile(\''+f.name+'\');return false;" title="L\u00f6schen">&#10006;</a></td><td><button type="button" class="copy-btn" style="position:static;margin:2px" title="'+apiLink(f.name)+'" onclick="copyText(apiLink(\''+f.name+'\'),this)">URL</button><button type="button" class="copy-btn" style="position:static;margin:2px" title="curl-Aufruf kopieren" onclick="copyText(\'curl &quot;\'+apiLink(\''+f.name+'\')+\'&quot;\',this)">curl</button></td></tr>';});
  if(!j.files.length)h+='<tr><td>Keine Dateien</td></tr>';
  document.getElementById('fileList').innerHTML=h+'</table>';
  memShow('fsMem',j.used,j.total);
  }).catch(function(){});}
function uploadNow(){var fs=Array.prototype.slice.call(document.getElementById('upFile').files);if(!fs.length){showRes('upResult',{ok:false,error:'Keine Datei gew&auml;hlt'});return;}
  var i=0,okc=0,bad=[];
  function next(){if(i>=fs.length){var inp=document.getElementById('upFile');inp.value='';loadFiles();
      showRes('upResult',bad.length?{ok:false,error:okc+' von '+fs.length+' hochgeladen; Fehler: '+bad.join(', ')}:{ok:true,msg:okc+(okc==1?' Datei':' Dateien')+' hochgeladen'});return;}
    var f=fs[i],fn=f.name.replace(/[<>&"']/g,'_');showRes('upResult',{ok:true,msg:'Lade hoch ('+(i+1)+'/'+fs.length+'): '+fn+' &hellip;'});
    var fd=new FormData();fd.append('file',f,f.name);
    fetch('/upload',{method:'POST',body:fd}).then(function(r){return r.json();}).then(function(j){if(j&&j.ok)okc++;else bad.push(fn+' ('+((j&&j.error)||'Fehler')+')');})
      .catch(function(){bad.push(fn);}).then(function(){i++;next();});}
  next();}
var nbs={on:false,jump:true,name:'',pm:0,rate:0,dur:0,t0:0};
function mmss(x){x=Math.round(x);return Math.floor(x/60)+':'+('0'+x%60).slice(-2);}
function barTick(){var pm=nbs.on?Math.min(1000,nbs.pm+nbs.rate*(Date.now()-nbs.t0)/1000):0;
  document.querySelectorAll('.nowbar').forEach(function(bar){var f=bar.firstChild;f.style.transition=nbs.jump?'none':'width .25s linear';f.style.width=(pm/10)+'%';
    bar.title=nbs.on&&nbs.dur>0?mmss(pm/1000*nbs.dur)+' / '+mmss(nbs.dur):'';});nbs.jump=false;}
setInterval(barTick,250);
function pollPlay(){if(document.hidden)return;fetch('/api/status',{cache:'no-store'}).then(function(r){return r.json();}).then(function(j){
  var el=document.getElementById('playState');if(el)el.textContent='Zustand: '+j.state+(j.error?' ('+j.error+')':'');
  var vs=document.getElementById('vol');if(vs&&document.activeElement!==vs&&j.volume!==undefined&&+vs.value!==j.volume){vs.value=j.volume;volShow(j.volume);}
  {var t='';
    if(j.source==='radio'){t='\u266a Webradio: '+(j.name||j.url)+(j.title?' \u2013 '+j.title:'');}
    else if(j.source==='file'){t='\u266a Datei: '+j.name;}
    else if(j.source==='speech'){t='\u266a Ansage';}
    document.querySelectorAll('.nowplaying').forEach(function(np){np.textContent=t;np.title=j.url||'';
      var row=np.parentNode,bar=row.nextSibling&&row.nextSibling.className==='nowbar'?row.nextSibling:null;
      if(!bar){bar=document.createElement('div');bar.className='nowbar';bar.style.cssText='height:6px;margin:2px auto 8px;max-width:100%;background:#3a4147;border-radius:3px;overflow:hidden';bar.innerHTML='<div style="height:100%;width:0;background:var(--ok)"></div>';row.parentNode.insertBefore(bar,row.nextSibling);}
      var card=row.parentNode.querySelector('.card');if(card&&card.offsetWidth)bar.style.width=card.offsetWidth+'px';
      bar.style.display=j.source==='radio'?'none':'';});
    var on=j.source==='file'&&j.pm!==undefined;
    nbs={on:on,jump:!on||nbs.name!==j.name||j.pm<nbs.pm-50,name:j.name,pm:on?j.pm:0,rate:on?(+j.rate||0):0,dur:on?(+j.dur||0):0,t0:Date.now()};
    barTick();
    var ib=document.getElementById('radioInfoBox');
    if(ib){var tb=document.getElementById('radioInfo'),rows=[];
      if(j.source==='radio'){rows=[['Sender',j.name],['Titel',j.title],['Genre',j.genre],['Beschreibung',j.desc],['Webseite',j.web,1],['Logo',j.logo,1],['Bitrate',j.bitrate?(j.bitrate>=1000?Math.round(j.bitrate/1000):j.bitrate)+' kbit/s':''],['Server',j.host],['Codec',j.codec],['Abtastrate',j.rate?j.rate+' Hz':''],['Kan\u00e4le',j.ch],['Bits',j.bits]].filter(function(r){return r[1];});}
      ib.style.display=rows.length?'':'none';
      var key=JSON.stringify(rows);
      if(tb.getAttribute('data-k')!==key){tb.setAttribute('data-k',key);tb.textContent='';
        rows.forEach(function(r){var tr=document.createElement('tr'),a=document.createElement('td'),b=document.createElement('td');a.style.cssText='color:var(--muted);width:30%;vertical-align:top;word-break:break-all';a.textContent=r[0];
          b.style.wordBreak='break-all';
          if(r[2]&&/^https?:\/\//i.test(r[1])){var l=document.createElement('a');l.href=r[1];l.target='_blank';l.rel='noopener noreferrer';l.textContent=r[1];b.appendChild(l);}else b.textContent=r[1];
          tr.appendChild(a);tr.appendChild(b);tb.appendChild(tr);});}}
    document.querySelectorAll('.stationrow').forEach(function(r){var on=j.source==='radio'&&j.url&&r.getAttribute('data-url')===j.url;var n=r.firstChild;n.style.color=on?'var(--ok)':'var(--muted)';n.style.fontWeight=on?'bold':'normal';var b=r.querySelector('.playbtn');if(b&&r.getAttribute('data-url')){var num=r.getAttribute('data-n');
      if(on){b.innerHTML='&#9632;';b.title='Stopp';b.onclick=function(){stopNow();};}
      else{b.innerHTML='&#9654;';b.title='Abspielen';b.onclick=function(){playStation(num);};}}});
    document.querySelectorAll('.stopbtn').forEach(function(sb){sb.disabled=(j.state==='idle');});}}).catch(function(){});}
function scanWifi(){var info=document.getElementById('scanInfo');info.textContent='Suche läuft …';
  fetch('/api/scan',{cache:'no-store'}).then(function(r){return r.json();}).then(function(list){
    for(var i=1;i<=3;i++){var s=document.getElementById('ssid_select'+i);s.innerHTML='<option value="">-</option>';
      list.forEach(function(n){var o=document.createElement('option');o.value=n;o.textContent=n;s.appendChild(o);});}
    info.textContent=list.length+' Netzwerke gefunden';}).catch(function(){info.textContent='Suche fehlgeschlagen';});}
function grpGrow(n){var v=document.getElementById('mq_g'+n).value.trim(),r=document.getElementById('grpRow'+(n+1));if(v&&r)r.style.display='';}
function ntpGrow(n){var v=document.getElementById('ntpServerInput'+n).value.trim(),r=document.getElementById('ntpRow'+(n+1));if(v&&r)r.style.display='';}
function testNtp(n){var box=document.getElementById('ntpTestResult'+n);var v=document.getElementById('ntpServerInput'+n).value;
  box.innerHTML='';
  fetch('/api/testNtp?server='+encodeURIComponent(v)).then(function(r){return r.text();}).then(function(t){
    if(t.indexOf('OK|')===0){box.className='res-ok';box.innerHTML='&#10004; '+t.substring(3);}
    else{box.className='res-bad';box.innerHTML='&#10008; Server nicht erreichbar';}
  }).catch(function(){box.className='res-bad';box.innerHTML='&#10008; Server nicht erreichbar';});}
fillVoiceSel('sayVoice',document.getElementById('sayLang').value,true);fillVoiceSel('cfgVoice',(document.getElementById('cfgLang')||{}).value,true);
document.querySelectorAll('.codehost').forEach(function(p){p.textContent=p.textContent.replace(/@HOST@/g,location.host);});
(function(){var m=location.search.match(/[?&]tab=([a-zA-Z0-9]+)/);if(m){var el=document.getElementById('tab-'+m[1]);if(el)el.checked=true;}
  loadFiles();sdLoad('/');pollPlay();checkPins();setInterval(pollPlay,3000);if(location.search.indexOf('tab=wlan')>=0)scanWifi();})();
(function(){
  var cb=document.getElementById('logAutoRefresh');var pre=document.getElementById('logContent');
  var errEl=document.getElementById('logErrorHint');var btn=document.getElementById('logRefreshNow');var timer=null;
  var stored=null;try{stored=localStorage.getItem('netspeakerLogAutoRefresh');}catch(e){}
  cb.checked=(stored==='1');
  function refreshLog(){fetch('/log',{cache:'no-store'}).then(function(r){return r.text();}).then(function(t){
    errEl.classList.remove('show');pre.textContent=t||'(leer)';pre.scrollTop=pre.scrollHeight;}).catch(function(){errEl.classList.add('show');});}
  function stopTimer(){if(timer){clearInterval(timer);timer=null;}}
  function applyState(){stopTimer();if(cb.checked&&!document.hidden){timer=setInterval(refreshLog,10000);}}
  btn.addEventListener('click',refreshLog);
  cb.addEventListener('change',function(){try{localStorage.setItem('netspeakerLogAutoRefresh',cb.checked?'1':'0');}catch(e){}if(cb.checked)refreshLog();applyState();});
  document.addEventListener('visibilitychange',function(){if(document.hidden){stopTimer();}else{if(cb.checked)refreshLog();applyState();}});
  refreshLog();applyState();
})();
)js";

String flashText(const String& code) {
    if (code == "saved") return "Gespeichert";
    if (code == "pinserr") return "Pins nicht gespeichert: ein Pin ist doppelt vergeben oder nicht erlaubt (alles andere wurde gespeichert)";
    if (code == "restart") return "Gespeichert - Neustart n&ouml;tig (Hostname oder Pins)";
    return "";
}

void sendChunk(const String& s) {
    webserver.sendContent(s);
}

void handleRoot() {
    webserver.setContentLength(CONTENT_LENGTH_UNKNOWN);
    webserver.sendHeader("Cache-Control", "no-store");
    webserver.send(200, "text/html; charset=utf-8", "");
    sendChunk("<!DOCTYPE html><html lang='de'><head><meta charset='utf-8'><meta name='viewport' content='width=device-width,initial-scale=1'><title>" PROJECT_TITLE "</title><style>");
    sendChunk(FPSTR(PAGE_CSS));
    String tabCss;
    for (int i = 0; i < TAB_COUNT; i++) {
        tabCss += String("#tab-") + TAB_KEYS[i] + ":checked ~ .tabnav label[for='tab-" + TAB_KEYS[i] + "']{background:var(--accent);color:#1a1200;border-color:var(--accent);}";
        tabCss += String("#tab-") + TAB_KEYS[i] + ":checked ~ .panel-" + TAB_KEYS[i] + "{display:block;}";
    }
    sendChunk(tabCss + "</style></head><body>");
    sendChunk(topbarHtml());
    sendChunk("<noscript><p>Diese Seite braucht JavaScript.</p></noscript><div class='page'>");
    String msg = flashText(webserver.arg("msg"));
    if (msg.length() > 0) {
        sendChunk("<div class='flash' id='flashBox'>" + msg + "</div><script>setTimeout(function(){var b=document.getElementById('flashBox');if(b)b.style.display='none';},4000);</script>");
    }

    // Die Tab-Eingaben, .tabnav und alle .tabpanel muessen direkte Geschwister bleiben (CSS-Selektor :checked ~).
    // The tab inputs, .tabnav and all .tabpanel must stay direct siblings (CSS selector :checked ~).

    String tabs;
    for (int i = 0; i < TAB_COUNT; i++) {
        tabs += String("<input type='radio' name='tabs' id='tab-") + TAB_KEYS[i] + "' class='tabctrl'" + (i == 0 ? " checked" : "") + ">";
    }
    tabs += "<div class='tabnav'>";
    for (int i = 0; i < TAB_COUNT; i++) {
        tabs += String("<label for='tab-") + TAB_KEYS[i] + "' title='" + TAB_TIPS[i] + "'>" + TAB_LABELS[i] + "</label>";
    }
    tabs += "</div>";
    sendChunk(tabs);
    sendChunk("<div class='tabpanel panel-sprechen'>" + panelSpeak() + "</div>");
    sendChunk("<div class='tabpanel panel-radio'>" + panelRadio() + "</div>");
    sendChunk("<div class='tabpanel panel-sd'>" + panelSd() + "</div>");
    sendChunk("<div class='tabpanel panel-mp3'>" + panelMp3() + "</div>");
    sendChunk("<div class='tabpanel panel-pins'>" + panelPins() + "</div>");
    sendChunk("<div class='tabpanel panel-audio'>" + panelAudio() + "</div>");
    sendChunk("<div class='tabpanel panel-mqtt'>" + panelMqtt() + "</div>");
    sendChunk("<div class='tabpanel panel-wlan'>" + panelWlan() + "</div>");
    sendChunk("<div class='tabpanel panel-zeit'>" + panelTime() + "</div>");
    sendChunk("<div class='tabpanel panel-status'>" + panelStatus() + "</div>");
    sendChunk("<div class='tabpanel panel-log'>" + panelLog() + "</div>");
    sendChunk("<div class='tabpanel panel-doku'>" + panelDoc() + "</div></div>");
    sendChunk("<script>");
    sendChunk("var DEF_LANG='" + jsonEscape(cfg.vlang) + "';");
    sendChunk("var UI_HOST='" + jsonEscape(cfg.host) + "';");
    sendChunk(FPSTR(TOPBAR_JS));
    sendChunk(FPSTR(PAGE_JS));
    sendChunk(FPSTR(PINS_JS));
    sendChunk(FPSTR(LANG_JS));
    sendChunk("</script></body></html>");
    webserver.sendContent("");
}

// ---------------------------------------------------------------------------
// Speichern, Export, Import / save, export, import

void redirectTab(const char* tab, const char* msg) {
    webserver.sendHeader("Location", String("/?tab=") + tab + "&msg=" + msg);
    webserver.send(303);
}

// Entfernt Luecken in der Liste der gespeicherten Plaetze: belegte Plaetze ruecken nach oben, alles wird neu gespeichert.
// Removes gaps in the list of stored slots: used slots move up, everything is stored again.

void compactStations() {
    int dst = 0;
    for (int i = 0; i < RADIO_SLOTS; i++) {
        if (cfg.radioUrl[i].isEmpty()) continue;
        if (dst != i) {
            cfg.radioName[dst] = cfg.radioName[i];
            cfg.radioUrl[dst] = cfg.radioUrl[i];
            cfg.radioVol[dst] = cfg.radioVol[i];
        }
        dst++;
    }
    for (int i = dst; i < RADIO_SLOTS; i++) {
        cfg.radioName[i] = "";
        cfg.radioUrl[i] = "";
        cfg.radioVol[i] = -1;
    }
    for (int i = 0; i < RADIO_SLOTS; i++) {
        String n = String(i + 1);
        prefs.putString(("rn" + n).c_str(), cfg.radioName[i]);
        prefs.putString(("ru" + n).c_str(), cfg.radioUrl[i]);
        prefs.putInt(("rv" + n).c_str(), cfg.radioVol[i]);
    }
    publishStations();
}

void handleSave() {
    String sect = webserver.arg("sect");
    bool restart = false;
    bool pinsBad = false;
    if (sect == "wlan") {
        String host = cleanHost(webserver.arg("host"));
        if (host != cfg.host) restart = true;
        cfg.host = host;
        prefs.putString("host", host);
        for (int i = 0; i < 3; i++) {
            String n = String(i + 1);
            String ssid = webserver.arg("ssid" + n);
            ssid.trim();
            String pass = webserver.arg("pass" + n);

            // Leeres Passwortfeld behaelt das gespeicherte Passwort derselben SSID.
            // An empty password field keeps the stored password of the same SSID.

            if (pass.isEmpty() && ssid == cfg.ssid[i]) pass = cfg.pass[i];
            if (ssid.isEmpty()) pass = "";
            cfg.ssid[i] = ssid;
            cfg.pass[i] = pass;
            prefs.putString(("ssid" + n).c_str(), ssid);
            prefs.putString(("pass" + n).c_str(), pass);
        }
        wifiRetryNow();
    } else if (sect == "voice") {
        String key = webserver.arg("vkey");
        key.trim();
        cfg.vkey = key;
        prefs.putString("vkey", key);
        cfg.vlang = webserver.arg("vlang");
        cfg.vvoice = webserver.arg("vvoice");
        cfg.vvoice.trim();
        cfg.vfmt = webserver.arg("vfmt");
        cfg.vrate = constrain(webserver.arg("vrate").toInt(), -10, 10);
        prefs.putString("vlang", cfg.vlang);
        prefs.putString("vvoice", cfg.vvoice);
        prefs.putString("vfmt", cfg.vfmt);
        prefs.putInt("vrate", cfg.vrate);
    } else if (sect == "audio") {
        // Alle sieben Pins (Verstaerker und SD-Karte) werden zusammen geprueft: erlaubt und paarweise verschieden, sonst bleibt alles beim Alten.
        // All seven pins (amplifier and SD card) are checked together: allowed and pairwise different, otherwise everything stays as it was.

        int* pinTargets[7] = {&cfg.din, &cfg.bclk, &cfg.lrc, &cfg.sdCs, &cfg.sdSck, &cfg.sdMosi, &cfg.sdMiso};
        const char* pinKeys[7] = {"din", "bclk", "lrc", "sdcs", "sdsck", "sdmosi", "sdmiso"};
        int pinNew[7];
        bool pinsOk = true;
        for (int i = 0; i < 7; i++) {
            pinNew[i] = webserver.arg(pinKeys[i]).toInt();
            if (!pinAllowed(pinNew[i])) pinsOk = false;
            for (int j = 0; j < i; j++) {
                if (pinNew[i] == pinNew[j]) pinsOk = false;
            }
        }
        pinsBad = !pinsOk;
        bool sdOldOn = cfg.sdOn;
        int sdOld[4] = {cfg.sdCs, cfg.sdSck, cfg.sdMosi, cfg.sdMiso};
        if (pinsOk) {
            for (int i = 0; i < 7; i++) {
                if (pinNew[i] == *pinTargets[i]) continue;
                if (i < 3) restart = true;
                *pinTargets[i] = pinNew[i];
                prefs.putInt(pinKeys[i], pinNew[i]);
            }
        }
        int volOld = cfg.vol;
        cfg.vol = constrain(webserver.arg("vol").toInt(), 0, VOLUME_MAX);
        prefs.putInt("vol", cfg.vol);
        if (cfg.vol != volOld) applyVolumeSmooth(cfg.vol);
        cfg.volRampMs = constrain(webserver.arg("vramp").toInt(), 0, VOLUME_RAMP_MAX_MS);
        prefs.putInt("vramp", cfg.volRampMs);
        cfg.fadeMs = constrain(webserver.arg("fade").toInt(), 0, FADE_MAX_MS);
        prefs.putInt("fade", cfg.fadeMs);
        cfg.switchFadeMs = constrain(webserver.arg("swfade").toInt(), 0, FADE_MAX_MS);
        prefs.putInt("swfade", cfg.switchFadeMs);
        cfg.titleMatch = webserver.arg("tmatch");
        cfg.titleMatch.trim();
        cfg.titleVol = constrain(webserver.arg("tvol").toInt(), 0, VOLUME_MAX);
        prefs.putString("tmatch", cfg.titleMatch);
        prefs.putInt("tvol", cfg.titleVol);
        cfg.titleMaxS = constrain(webserver.arg("tmax").toInt(), 0, 3600);
        prefs.putInt("tmax", cfg.titleMaxS);
        cfg.mixAnn = webserver.hasArg("mix");
        cfg.loudNorm = webserver.hasArg("loud");
        prefs.putBool("loud", cfg.loudNorm);
        cfg.duckPct = constrain(webserver.arg("duck").toInt(), 0, 100);
        prefs.putBool("mix", cfg.mixAnn);
        prefs.putInt("duck", cfg.duckPct);
        cfg.sdOn = webserver.hasArg("sdon");
        prefs.putBool("sdon", cfg.sdOn);
        if (sdOldOn != cfg.sdOn || sdOld[0] != cfg.sdCs || sdOld[1] != cfg.sdSck || sdOld[2] != cfg.sdMosi || sdOld[3] != cfg.sdMiso) sdMount();
        cfg.autoStart = webserver.hasArg("autostart");
        cfg.bootTone = webserver.hasArg("btone");
        prefs.putBool("btone", cfg.bootTone);
        prefs.putBool("autostart", cfg.autoStart);
        cfg.resumeLast = webserver.hasArg("resume");
        prefs.putBool("rlast", cfg.resumeLast);
        if (!cfg.resumeLast) saveRadioState("", -1);
        if (webserver.hasArg("vlang")) {
            String key = webserver.arg("vkey");
            key.trim();
            cfg.vkey = key;
            prefs.putString("vkey", key);
            cfg.vlang = webserver.arg("vlang");
            cfg.vvoice = webserver.arg("vvoice");
            cfg.vvoice.trim();
            cfg.vfmt = webserver.arg("vfmt");
            cfg.vrate = constrain(webserver.arg("vrate").toInt(), -10, 10);
            prefs.putString("vlang", cfg.vlang);
            prefs.putString("vvoice", cfg.vvoice);
            prefs.putString("vfmt", cfg.vfmt);
            prefs.putInt("vrate", cfg.vrate);
        }
    } else if (sect == "mqtt") {
        cfg.mqttServer = webserver.arg("mq_srv");
        cfg.mqttServer.trim();
        int port = webserver.arg("mq_port").toInt();
        cfg.mqttPort = (port >= 1 && port <= 65535) ? port : 1883;
        cfg.mqttBase = webserver.arg("mq_base");
        cfg.mqttBase.trim();
        prefs.putString("mq_srv", cfg.mqttServer);
        prefs.putInt("mq_port", cfg.mqttPort);
        prefs.putString("mq_base", cfg.mqttBase);
        String groups[5];
        int gm = 0;
        for (int i = 1; i <= 5; i++) {
            String v = webserver.arg("mq_g" + String(i));
            v.trim();
            if (!v.isEmpty()) groups[gm++] = v;
        }
        for (int i = 0; i < 5; i++) {
            cfg.mqttGroup[i] = i < gm ? groups[i] : String();
            prefs.putString(("mq_g" + String(i + 1)).c_str(), cfg.mqttGroup[i]);
        }
        cfg.mqttUser = webserver.arg("mq_user");
        cfg.mqttUser.trim();
        if (webserver.arg("mq_pass").length() > 0) cfg.mqttPass = webserver.arg("mq_pass");
        if (cfg.mqttUser.isEmpty()) cfg.mqttPass = "";
        prefs.putString("mq_usr", cfg.mqttUser);
        prefs.putString("mq_pw", cfg.mqttPass);
        mqttReconnectNow();
    } else if (sect == "radio") {
        for (int i = 0; i < RADIO_SLOTS; i++) {
            String n = String(i + 1);
            if (!webserver.hasArg("ru" + n)) continue;
            String name = webserver.arg("rn" + n);
            String url = webserver.arg("ru" + n);
            name.trim();
            url.trim();
            if (name.length() > RADIO_NAME_MAX) name = name.substring(0, RADIO_NAME_MAX);
            if (!(url.startsWith("http://") || url.startsWith("https://")) || url.length() > RADIO_URL_MAX) url = "";
            if (url.isEmpty()) name = "";
            String rv = webserver.arg("rv" + n);
            rv.trim();
            int vol = (rv.isEmpty() || url.isEmpty()) ? -1 : constrain(rv.toInt(), 0, VOLUME_MAX);
            cfg.radioVol[i] = vol;
            prefs.putInt(("rv" + n).c_str(), vol);
            cfg.radioName[i] = name;
            cfg.radioUrl[i] = url;
            prefs.putString(("rn" + n).c_str(), name);
            prefs.putString(("ru" + n).c_str(), url);
        }
        compactStations();
    } else if (sect == "log") {
        cfg.logEnabled = webserver.hasArg("log");
        prefs.putBool("log", cfg.logEnabled);
        cfg.logAutoMin = constrain(webserver.arg("logmin").toInt(), 0, 1440);
        prefs.putInt("logmin", cfg.logAutoMin);
    } else if (sect == "time") {
        String list[5];
        int m = 0;
        for (int i = 1; i <= 5; i++) {
            String v = webserver.arg("ntp" + String(i));
            v.trim();
            if (!v.isEmpty()) list[m++] = v;
        }
        if (m == 0) list[m++] = "pool.ntp.org";
        for (int i = 0; i < 5; i++) {
            cfg.ntp[i] = i < m ? list[i] : String();
            prefs.putString(("ntp" + String(i + 1)).c_str(), cfg.ntp[i]);
        }
        cfg.tz = webserver.arg("tz");
        if (cfg.tz.isEmpty()) cfg.tz = "UTC0";
        prefs.putString("tz", cfg.tz);
        setupTime();
    }
    logLine("[WEB] settings saved: " + sect);
    static const char* const tabForSect[] = {"wlan", "audio", "audio", "mqtt", "zeit", "log", "radio"};
    static const char* const sects[] = {"wlan", "voice", "audio", "mqtt", "time", "log", "radio"};
    const char* tab = "sprechen";
    for (int i = 0; i < 7; i++) {
        if (sect == sects[i]) tab = tabForSect[i];
    }
    redirectTab(tab, pinsBad ? "pinserr" : (restart ? "restart" : "saved"));
}

void handleExport() {
    String t = "# " PROJECT_NAME " settings export (WiFi passwords, MQTT password and API key omitted)\n";
    t += "host=" + cfg.host + "\n";
    for (int i = 0; i < 3; i++) t += "ssid" + String(i + 1) + "=" + cfg.ssid[i] + "\n";
    for (int i = 0; i < 5; i++) t += "ntp" + String(i + 1) + "=" + cfg.ntp[i] + "\n";
    t += "tz=" + cfg.tz + "\n";
    t += "vlang=" + cfg.vlang + "\nvvoice=" + cfg.vvoice + "\nvfmt=" + cfg.vfmt + "\nvrate=" + String(cfg.vrate) + "\n";
    t += "vol=" + String(cfg.vol) + "\nbclk=" + String(cfg.bclk) + "\nlrc=" + String(cfg.lrc) + "\ndin=" + String(cfg.din) + "\n";
    t += "mq_srv=" + cfg.mqttServer + "\nmq_port=" + String(cfg.mqttPort) + "\nmq_user=" + cfg.mqttUser + "\nmq_base=" + cfg.mqttBase + "\n";
    for (int i = 0; i < 5; i++) t += "mq_g" + String(i + 1) + "=" + cfg.mqttGroup[i] + "\n";
    t += "fade=" + String(cfg.fadeMs) + "\nswfade=" + String(cfg.switchFadeMs) + "\nautostart=" + String(cfg.autoStart ? 1 : 0) + "\nbtone=" + String(cfg.bootTone ? 1 : 0) + "\nresume=" + String(cfg.resumeLast ? 1 : 0) + "\nsdon=" + String(cfg.sdOn ? 1 : 0) + "\nsdcs=" + String(cfg.sdCs) + "\nsdsck=" + String(cfg.sdSck) + "\nsdmosi=" + String(cfg.sdMosi) + "\nsdmiso=" + String(cfg.sdMiso) + "\nvolmax=" + String(VOLUME_MAX) + "\nvramp=" + String(cfg.volRampMs) + "\nmix=" + String(cfg.mixAnn ? 1 : 0) + "\nloud=" + String(cfg.loudNorm ? 1 : 0) + "\nduck=" + String(cfg.duckPct) + "\ntmatch=" + cfg.titleMatch + "\ntvol=" + String(cfg.titleVol) + "\ntmax=" + String(cfg.titleMaxS) + "\n";
    t += "log=" + String(cfg.logEnabled ? 1 : 0) + "\nlogmin=" + String(cfg.logAutoMin) + "\n";
    for (int i = 0; i < RADIO_SLOTS; i++) t += "rn" + String(i + 1) + "=" + cfg.radioName[i] + "\nru" + String(i + 1) + "=" + cfg.radioUrl[i] + "\nrv" + String(i + 1) + "=" + (cfg.radioVol[i] >= 0 ? String(cfg.radioVol[i]) : String()) + "\n";
    webserver.sendHeader("Content-Disposition", "attachment; filename=" + cfg.host + "-settings.txt");
    webserver.sendHeader("Cache-Control", "no-store");
    webserver.send(200, "text/plain; charset=utf-8", t);
}

// Importierte Lautstaerken: Dateien ohne volmax=100 stammen von der alten Skala 0 bis 21 und werden umgerechnet.
// Imported volumes: files without volmax=100 come from the old scale 0 to 21 and are converted.

bool importNewScale = true;

int importVol(int x) {
    return importNewScale ? x : (x * VOLUME_MAX + 10) / 21;
}

void handleImport() {
    String body = webserver.arg("plain");
    if (!body.startsWith("# " PROJECT_NAME " settings export")) {
        webserver.send(200, "text/plain", "FAIL|Das ist keine Exportdatei dieses Ger\xC3\xA4ts.");
        return;
    }
    importNewScale = body.indexOf("volmax=100") >= 0;
    int count = 0;
    bool needRestart = false;
    String oldSsid[3] = {cfg.ssid[0], cfg.ssid[1], cfg.ssid[2]};
    String oldPass[3] = {cfg.pass[0], cfg.pass[1], cfg.pass[2]};
    int pos = 0;
    while (pos < (int)body.length()) {
        int end = body.indexOf('\n', pos);
        if (end < 0) end = body.length();
        String line = body.substring(pos, end);
        pos = end + 1;
        line.trim();
        int eq = line.indexOf('=');
        if (line.startsWith("#") || eq <= 0) continue;
        String k = line.substring(0, eq);
        String v = line.substring(eq + 1);
        if (k == "host") {

            // Der Hostname gehoert zum Geraet und wird nicht uebernommen, damit zwei Geraete nie denselben Namen bekommen.
            // The hostname belongs to the device and is not imported, so two devices never get the same name.

            continue;
        } else if (k == "ssid1" || k == "ssid2" || k == "ssid3") {
            int i = k.substring(4).toInt() - 1;
            cfg.ssid[i] = v;

            // Das Passwort bleibt nur, wenn dieselbe SSID schon gespeichert war.
            // The password stays only if the same SSID was already stored.

            cfg.pass[i] = (v == oldSsid[i]) ? oldPass[i] : "";
            prefs.putString(k.c_str(), v);
            prefs.putString(("pass" + String(i + 1)).c_str(), cfg.pass[i]);
        } else if (k.length() == 4 && k.startsWith("ntp") && k[3] >= '1' && k[3] <= '5') {
            int i = k[3] - '1';
            cfg.ntp[i] = (i == 0 && v.isEmpty()) ? String("pool.ntp.org") : v;
            prefs.putString(k.c_str(), cfg.ntp[i]);
        } else if (k == "tz") { cfg.tz = v.isEmpty() ? "UTC0" : v; prefs.putString("tz", cfg.tz);
        } else if (k == "vlang") { cfg.vlang = v; prefs.putString("vlang", v);
        } else if (k == "vvoice") { cfg.vvoice = v; prefs.putString("vvoice", v);
        } else if (k == "vfmt") { cfg.vfmt = v; prefs.putString("vfmt", v);
        } else if (k == "vrate") { cfg.vrate = constrain(v.toInt(), -10, 10); prefs.putInt("vrate", cfg.vrate);
        } else if (k == "vol") { cfg.vol = constrain(importVol(v.toInt()), 0, VOLUME_MAX); prefs.putInt("vol", cfg.vol);
        } else if (k == "bclk" || k == "lrc" || k == "din") {
            int pin = v.toInt();
            if (!pinAllowed(pin)) continue;
            int* target = k == "bclk" ? &cfg.bclk : (k == "lrc" ? &cfg.lrc : &cfg.din);
            if (*target != pin) needRestart = true;
            *target = pin;
            prefs.putInt(k.c_str(), pin);
        } else if (k == "mq_user") { cfg.mqttUser = v; prefs.putString("mq_usr", v);
        } else if (k == "mq_pass") { cfg.mqttPass = v; prefs.putString("mq_pw", v);
        } else if (k == "mq_srv") { cfg.mqttServer = v; prefs.putString("mq_srv", v);
        } else if (k == "mq_port") { int p = v.toInt(); cfg.mqttPort = (p >= 1 && p <= 65535) ? p : 1883; prefs.putInt("mq_port", cfg.mqttPort);
        } else if (k == "mq_base") { cfg.mqttBase = v; prefs.putString("mq_base", v);
        } else if (k.length() == 5 && k.startsWith("mq_g") && k[4] >= '1' && k[4] <= '5') { cfg.mqttGroup[k[4] - '1'] = v; prefs.putString(k.c_str(), v);
        } else if (k == "sdon") { cfg.sdOn = v.toInt() != 0; prefs.putBool("sdon", cfg.sdOn);
        } else if (k == "sdcs" || k == "sdsck" || k == "sdmosi" || k == "sdmiso") {
            int pin = v.toInt();
            if (!pinAllowed(pin)) continue;
            int* target = k == "sdcs" ? &cfg.sdCs : (k == "sdsck" ? &cfg.sdSck : (k == "sdmosi" ? &cfg.sdMosi : &cfg.sdMiso));
            *target = pin;
            prefs.putInt(k.c_str(), pin);
            needRestart = true;
        } else if (k == "vramp") { cfg.volRampMs = constrain(v.toInt(), 0, VOLUME_RAMP_MAX_MS); prefs.putInt("vramp", cfg.volRampMs);
        } else if (k == "mix") { cfg.mixAnn = v.toInt() != 0; prefs.putBool("mix", cfg.mixAnn);
        } else if (k == "loud") { cfg.loudNorm = v.toInt() != 0; prefs.putBool("loud", cfg.loudNorm);
        } else if (k == "duck") { cfg.duckPct = constrain(v.toInt(), 0, 100); prefs.putInt("duck", cfg.duckPct);
        } else if (k == "tmatch") { cfg.titleMatch = v; prefs.putString("tmatch", v);
        } else if (k == "tmax") { cfg.titleMaxS = constrain(v.toInt(), 0, 3600); prefs.putInt("tmax", cfg.titleMaxS);
        } else if (k == "tvol") { cfg.titleVol = constrain(importVol(v.toInt()), 0, VOLUME_MAX); prefs.putInt("tvol", cfg.titleVol);
        } else if (k == "resume") { cfg.resumeLast = v.toInt() != 0; prefs.putBool("rlast", cfg.resumeLast);
        } else if (k == "btone") { cfg.bootTone = v.toInt() != 0; prefs.putBool("btone", cfg.bootTone);
        } else if (k == "autostart") { cfg.autoStart = v.toInt() != 0; prefs.putBool("autostart", cfg.autoStart);
        } else if (k == "swfade") { cfg.switchFadeMs = constrain(v.toInt(), 0, FADE_MAX_MS); prefs.putInt("swfade", cfg.switchFadeMs);
        } else if (k == "fade") { cfg.fadeMs = constrain(v.toInt(), 0, FADE_MAX_MS); prefs.putInt("fade", cfg.fadeMs);
        } else if (k == "vkey" && v.length() > 0) { cfg.vkey = v; prefs.putString("vkey", v);
        } else if (k == "logmin") { cfg.logAutoMin = constrain(v.toInt(), 0, 1440); prefs.putInt("logmin", cfg.logAutoMin);
        } else if (k == "log") { cfg.logEnabled = v.toInt() != 0; prefs.putBool("log", cfg.logEnabled);
        } else if (k.startsWith("rv") && k.substring(2).toInt() >= 1 && k.substring(2).toInt() <= RADIO_SLOTS) {
            int slot = k.substring(2).toInt() - 1;
            cfg.radioVol[slot] = v.isEmpty() ? -1 : constrain(importVol(v.toInt()), 0, VOLUME_MAX);
            prefs.putInt(k.c_str(), cfg.radioVol[slot]);
        } else if ((k.startsWith("rn") || k.startsWith("ru")) && k.substring(2).toInt() >= 1 && k.substring(2).toInt() <= RADIO_SLOTS) {
            int slot = k.substring(2).toInt() - 1;
            if (k.startsWith("rn")) cfg.radioName[slot] = v.substring(0, RADIO_NAME_MAX);
            else cfg.radioUrl[slot] = ((v.startsWith("http://") || v.startsWith("https://")) && v.length() <= RADIO_URL_MAX) ? v : "";
            prefs.putString(k.c_str(), k.startsWith("rn") ? cfg.radioName[slot] : cfg.radioUrl[slot]);
        } else {
            continue;
        }
        count++;
    }
    applyVolume(cfg.vol);
    setupTime();
    mqttReconnectNow();
    wifiRetryNow();
    publishStations();
    logLine("[WEB] settings imported: " + String(count) + " values");
    webserver.send(200, "text/plain", "OK|" + String(count) + " Werte \xC3\xBCbernommen." + (needRestart ? " Neustart n\xC3\xB6tig (Hostname oder Pins)." : ""));
}

// ---------------------------------------------------------------------------
// JSON-API / JSON API

void sendJson(int code, const String& json) {
    webserver.sendHeader("Cache-Control", "no-store");
    webserver.send(code, "application/json", json);
}

void sendOk(const String& msg) {
    sendJson(200, "{\"ok\":true,\"msg\":\"" + jsonEscape(msg) + "\"}");
}

void sendErr(int code, const String& msg) {
    sendJson(code, "{\"ok\":false,\"error\":\"" + jsonEscape(msg) + "\"}");
}

// Parameter aus Formular, Query oder einem flachen JSON-Body, zum Beispiel {"text":"Hallo","vol":12}.
// Parameter from form, query or a flat JSON body, for example {"text":"Hallo","vol":12}.

String apiArg(const String& name) {
    if (webserver.hasArg(name)) return webserver.arg(name);
    String body = webserver.arg("plain");
    body.trim();
    if (body.startsWith("{")) return jsonGet(body, name);
    return "";
}

void handleApiSay() {
    String err = commandSay(apiArg("text"), apiArg("lang"), apiArg("voice"), numberOr(apiArg("rate"), 99), numberOr(apiArg("vol"), -1));
    if (err.length() > 0) return sendErr(400, err);
    sendOk("Wird angesagt");
}

void handleApiPlay() {
    String url = apiArg("url");
    String file = apiArg("file");
    int vol = numberOr(apiArg("vol"), -1);
    String err = url.length() > 0 ? commandPlayUrl(url, vol) : (file.length() > 0 ? commandPlayFile(file, vol, flagOn(apiArg("shuffle")), flagOn(apiArg("loop"))) : String("url oder file angeben"));
    if (err.length() > 0) return sendErr(400, err);
    sendOk("Wird abgespielt");
}

void handleApiUrl() {
    String err = commandPlayUrl(apiArg("url"), numberOr(apiArg("vol"), -1));
    if (err.length() > 0) return sendErr(400, err);
    sendOk("Wird abgespielt");
}

void handleApiFile() {
    String err = commandPlayFile(apiArg("file"), numberOr(apiArg("vol"), -1), flagOn(apiArg("shuffle")), flagOn(apiArg("loop")));
    if (err.length() > 0) return sendErr(400, err);
    sendOk("Wird abgespielt");
}

void handleApiStation() {
    String sel = apiArg("n");
    if (sel.isEmpty()) sel = apiArg("name");
    String err = commandPlayStation(sel, numberOr(apiArg("vol"), -1));
    if (err.length() > 0) return sendErr(400, err);
    sendOk("Wird abgespielt");
}

void handleApiStationDelete() {
    int n = apiArg("n").toInt();
    if (n < 1 || n > RADIO_SLOTS) return sendErr(400, "Platz 1-" + String(RADIO_SLOTS) + " angeben");
    cfg.radioName[n - 1] = "";
    cfg.radioUrl[n - 1] = "";
    prefs.putString(("rn" + String(n)).c_str(), "");
    prefs.putString(("ru" + String(n)).c_str(), "");
    cfg.radioVol[n - 1] = -1;
    compactStations();
    logLine("[WEB] station slot " + String(n) + " deleted, following slots moved up");
    sendOk("Platz " + String(n) + " gel\xC3\xB6scht");
}

// Senderliste als Textdatei sichern und wieder laden; das Laden ersetzt die ganze Liste.
// Save the station list as a text file and load it again; loading replaces the whole list.

void handleStationsExport() {
    String t = "# " PROJECT_NAME " stations\n";
    int n = 0;
    for (int i = 0; i < RADIO_SLOTS; i++) {
        if (cfg.radioUrl[i].isEmpty()) continue;
        n++;
        String nm = cfg.radioName[i];
        nm.replace("\r", " ");
        nm.replace("\n", " ");
        t += "rn" + String(n) + "=" + nm + "\nru" + String(n) + "=" + cfg.radioUrl[i] + "\nrv" + String(n) + "=" + (cfg.radioVol[i] >= 0 ? String(cfg.radioVol[i]) : String()) + "\n";
    }
    webserver.sendHeader("Content-Disposition", "attachment; filename=" + cfg.host + "-stations.txt");
    webserver.sendHeader("Cache-Control", "no-store");
    webserver.send(200, "text/plain; charset=utf-8", t);
}

void handleStationsImport() {
    String body = webserver.arg("plain");
    if (body.startsWith("\xEF\xBB\xBF")) body.remove(0, 3);
    if (!body.startsWith("# " PROJECT_NAME " stations")) {
        webserver.send(200, "text/plain", "FAIL|Das ist keine Senderliste dieses Ger\xC3\xA4ts.");
        return;
    }

    // Erst in Hilfsfelder lesen: eine Datei ohne gueltigen Sender laesst die gespeicherte Liste unberuehrt.
    // Read into helper arrays first: a file without a valid station leaves the stored list untouched.

    String names[RADIO_SLOTS], urls[RADIO_SLOTS];
    int vols[RADIO_SLOTS];
    for (int i = 0; i < RADIO_SLOTS; i++) vols[i] = -1;
    int pos = 0;
    while (pos < (int)body.length()) {
        int e = body.indexOf('\n', pos);
        if (e < 0) e = body.length();
        String line = body.substring(pos, e);
        pos = e + 1;
        line.trim();
        int eq = line.indexOf('=');
        if (eq < 3 || line.startsWith("#")) continue;
        String k = line.substring(0, eq);
        String v = line.substring(eq + 1);
        v.trim();
        int slot = k.substring(2).toInt() - 1;
        if (slot < 0 || slot >= RADIO_SLOTS) continue;
        if (k.startsWith("rn")) names[slot] = v.substring(0, RADIO_NAME_MAX);
        else if (k.startsWith("ru")) urls[slot] = ((v.startsWith("http://") || v.startsWith("https://")) && v.length() <= RADIO_URL_MAX) ? v : "";
        else if (k.startsWith("rv")) vols[slot] = v.isEmpty() ? -1 : constrain(v.toInt(), 0, VOLUME_MAX);
    }
    int count = 0;
    for (int i = 0; i < RADIO_SLOTS; i++) {
        if (!urls[i].isEmpty()) count++;
    }
    if (count == 0) {
        webserver.send(200, "text/plain", "FAIL|Die Datei enth\xC3\xA4lt keinen g\xC3\xBCltigen Sender (URL mit http:// oder https://).");
        return;
    }
    for (int i = 0; i < RADIO_SLOTS; i++) {
        bool used = !urls[i].isEmpty();
        cfg.radioName[i] = used ? names[i] : String();
        cfg.radioUrl[i] = used ? urls[i] : String();
        cfg.radioVol[i] = used ? vols[i] : -1;
    }
    compactStations();
    logLine("[WEB] station list loaded: " + String(count) + " stations");
    webserver.send(200, "text/plain", "OK|" + String(count) + " Sender geladen.");
}

void handleApiStations() {
    sendJson(200, stationsJson());
}

void handleApiStop() {
    commandStop();
    sendOk("Gestoppt");
}

void handleApiVolume() {
    String v = apiArg("v");

    // Ohne v wird nur gemeldet: volume = allgemeine, playVolume = gerade wirkende Lautstaerke.
    // Without v it only reports: volume = general, playVolume = currently effective volume.

    if (!v.isEmpty()) commandVolume(v.toInt(), numberOr(apiArg("ms"), -1));
    sendJson(200, String("{\"ok\":true,\"volume\":") + String(cfg.vol) + ",\"playVolume\":" + String(currentVolume) + "}");
}

// Log ein- oder ausschalten und leeren: enable=1|0, clear=1 (POST); GET liefert den Zustand.
// Switch logging on or off and clear it: enable=1|0, clear=1 (POST); GET returns the state.

void handleApiLog() {
    if (webserver.method() == HTTP_POST) {
        String en = apiArg("enable");
        if (en.length() > 0) {
            cfg.logEnabled = (en == "1" || en == "true");
            prefs.putBool("log", cfg.logEnabled);
        }
        String cl = apiArg("clear");
        if (cl == "1" || cl == "true") clearLogFiles();
    }
    sendJson(200, String("{\"enabled\":") + (cfg.logEnabled ? "true" : "false") + "}");
}
void handleApiStatus() {
    String j = "{\"state\":\"" + playStateName() + "\",\"error\":\"" + jsonEscape(lastError) + "\",\"volume\":" + String(cfg.vol) + ",\"playVolume\":" + String(currentVolume) + ",\"queue\":" + String(annCount) + ",\"sd\":\"" + sdState + "\",\"sdMounted\":" + (sdReady ? "true" : "false") + ",\"resume\":" + (resumeUrl.length() > 0 ? "true" : "false");
    bool idleNow = playState == PS_IDLE || nowKind.isEmpty();
    j += ",\"source\":\"" + String(idleNow ? "" : nowKind.c_str()) + "\",\"name\":\"" + jsonEscape(idleNow ? String() : (nowKind == "radio" ? stationDisplayName() : (nowKind == "file" ? nowFile : String()))) + "\",\"title\":\"" + jsonEscape(nowKind == "radio" && !idleNow ? nowTitle : String()) + "\",\"url\":\"" + jsonEscape(nowKind == "radio" && !idleNow ? streamUrl : String()) + "\"";
    if (!idleNow && nowKind == "file" && playState == PS_PLAYING && audio) j += ",\"pos\":" + String(audio->getAudioCurrentTime()) + ",\"dur\":" + String(audio->getAudioFileDuration());
    if (!idleNow && nowKind == "radio") {
        j += ",\"genre\":\"" + jsonEscape(nowGenre) + "\",\"desc\":\"" + jsonEscape(nowDesc) + "\",\"web\":\"" + jsonEscape(nowWeb) + "\",\"logo\":\"" + jsonEscape(nowLogo) + "\",\"host\":\"" + jsonEscape(nowHost) + "\",\"bitrate\":" + String(nowBitrate);
        if (audio && playState == PS_PLAYING) j += ",\"codec\":\"" + jsonEscape(String(audio->getCodecname())) + "\",\"rate\":" + String(audio->getSampleRate()) + ",\"ch\":" + String(audio->getChannels()) + ",\"bits\":" + String(audio->getBitsPerSample());
    }
    if (!idleNow && nowKind == "file" && playState == PS_PLAYING && cfg.loudNorm) j += ",\"gainDb\":" + (fileGainKnown ? String(fileGainCurDb, 1) : String("null"));
    if (!idleNow && nowKind == "file" && playState == PS_PLAYING && audio && audio->getFileSize() > 0) {
        uint32_t fsz = audio->getFileSize();
        uint32_t fpos = audio->getAudioFilePosition();
        j += ",\"pm\":" + String((uint32_t)((uint64_t)(fpos > fsz ? fsz : fpos) * 1000ULL / fsz)) + ",\"rate\":" + String((double)audio->getBitRate() / 8.0 * 1000.0 / (double)fsz, 3);
    }
    j += ",\"uptime\":" + String(millis() / 1000) + ",\"resetReason\":\"" + resetReasonName() + "\",\"wifiReconnects\":" + String(wifiReconnects) + ",\"mqttReconnects\":" + String(mqttReconnects) + ",\"minFreeHeap\":" + String(ESP.getMinFreeHeap()) + ",\"safeMode\":" + (safeMode ? "true" : "false");
    j += ",\"heap\":" + String(ESP.getFreeHeap()) + ",\"fsUsed\":" + String(fsReady ? LittleFS.usedBytes() : 0);
    j += ",\"fsTotal\":" + String(fsReady ? LittleFS.totalBytes() : 0) + "}";
    sendJson(200, j);
}

// Dateiname bereinigen: nur Buchstaben, Ziffern, Punkt, Bindestrich, Unterstrich; erlaubte Endung mp3.
// Clean the file name: only letters, digits, dot, hyphen, underscore; allowed extension mp3.

String cleanFileName(String name) {
    int slash = max(name.lastIndexOf('/'), name.lastIndexOf('\\'));
    if (slash >= 0) name = name.substring(slash + 1);
    String out;
    for (size_t i = 0; i < name.length() && out.length() < 120; i++) {
        char c = name[i];
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '.' || c == '-' || c == '_') out += c;
        else if (c == ' ') out += '_';
    }
    String lower = out;
    lower.toLowerCase();
    bool okExt = lower.endsWith(".mp3");
    if (!okExt || out.startsWith(".")) return "";
    if (out.length() > 40) out = out.substring(0, 36) + ".mp3";
    return out;
}

// Inhalt eines Ordners der SD-Karte als JSON: mounted, state, size, used (MB), dir und entries (n = Name, d = Ordner, s = Groesse).
// Content of a folder on the SD card as JSON: mounted, state, size, used (MB), dir and entries (n = name, d = folder, s = size).

void handleApiSd() {
    String dir = webserver.arg("dir");
    if (dir.isEmpty()) dir = "/";
    std::vector<SdEntry> list;
    bool ok = sdListDir(dir, list);
    if (!dir.startsWith("/")) dir = "/" + dir;
    String j = String("{\"mounted\":") + (sdReady ? "true" : "false") + ",\"state\":\"" + jsonEscape(sdState) + "\"";
    if (sdReady) j += ",\"size\":" + String((unsigned long)(SD.totalBytes() / 1048576ULL)) + ",\"used\":" + String((unsigned long)(SD.usedBytes() / 1048576ULL));
    j += ",\"dir\":\"" + jsonEscape(dir) + "\",\"ok\":" + (ok ? "true" : "false") + ",\"entries\":[";
    for (size_t i = 0; i < list.size(); i++) {
        if (i) j += ",";
        j += "{\"n\":\"" + jsonEscape(list[i].name) + "\",\"d\":" + (list[i].dir ? "1" : "0") + ",\"s\":" + String((unsigned long)list[i].size) + "}";
    }
    sendJson(200, j + "]}");
}

void handleApiSdMount() {
    bool ok = sdMount();
    if (!ok) return sendErr(400, "SD: " + sdState);
    sendOk("SD-Karte eingebunden");
}

// Dateimanager der SD-Karte: neuer Ordner, loeschen (Datei oder Ordner mit Inhalt), umbenennen und herunterladen.
// File manager of the SD card: new folder, delete (file or folder with content), rename and download.

void handleApiSdMkdir() {
    if (!sdReady) return sendErr(400, "SD-Karte nicht verf\xC3\xBCgbar");
    String dir = apiArg("dir");
    if (dir.isEmpty()) dir = "/";
    if (dir.length() > 1 && dir.endsWith("/")) dir.remove(dir.length() - 1);
    String name = sdCleanName(apiArg("name"), false);
    if (name.isEmpty() || !sdPathOk(dir)) return sendErr(400, "Name oder Ordner nicht erlaubt");
    String path = (dir == "/" ? String() : dir) + "/" + name;
    if (xSemaphoreTake(sdMutex, pdMS_TO_TICKS(3000)) != pdTRUE) return sendErr(400, "SD-Karte ist besch\xC3\xA4" "ftigt");
    bool exists = SD.exists(path.c_str());
    bool ok = !exists && SD.mkdir(path.c_str());
    xSemaphoreGive(sdMutex);
    if (exists) return sendErr(400, "Existiert schon");
    if (!ok) return sendErr(400, "Ordner konnte nicht angelegt werden");
    logLine("[SD] folder created " + path);
    sendOk("Ordner angelegt");
}

void handleApiSdDelete() {
    if (!sdReady) return sendErr(400, "SD-Karte nicht verf\xC3\xBCgbar");
    String path = apiArg("path");
    if (path.length() > 1 && path.endsWith("/")) path.remove(path.length() - 1);
    if (!sdPathOk(path) || path == "/") return sendErr(400, "Pfad nicht erlaubt");
    sdStopIfUsing(path);
    if (xSemaphoreTake(sdMutex, pdMS_TO_TICKS(3000)) != pdTRUE) return sendErr(400, "SD-Karte ist besch\xC3\xA4" "ftigt");
    bool exists = SD.exists(path.c_str());
    bool ok = exists && sdRemoveTree(path, 0);
    xSemaphoreGive(sdMutex);
    if (!exists) return sendErr(404, "Nicht gefunden");
    if (!ok) return sendErr(400, "Konnte nicht gel\xC3\xB6scht werden");
    logLine("[SD] deleted " + path);
    sendOk("Gel\xC3\xB6scht");
}

void handleApiSdRename() {
    if (!sdReady) return sendErr(400, "SD-Karte nicht verf\xC3\xBCgbar");
    String path = apiArg("path");
    if (path.length() > 1 && path.endsWith("/")) path.remove(path.length() - 1);
    if (!sdPathOk(path) || path == "/") return sendErr(400, "Pfad nicht erlaubt");
    if (xSemaphoreTake(sdMutex, pdMS_TO_TICKS(3000)) != pdTRUE) return sendErr(400, "SD-Karte ist besch\xC3\xA4" "ftigt");
    File f = SD.open(path.c_str());
    bool exists = f;
    bool isDir = f && f.isDirectory();
    if (f) f.close();
    String name = sdCleanName(apiArg("to"), !isDir);
    String parent = path.substring(0, path.lastIndexOf('/'));
    String target = parent + "/" + name;
    bool bad = name.isEmpty();
    bool taken = !bad && SD.exists(target.c_str());
    bool ok = exists && !bad && !taken;
    if (ok) {
        xSemaphoreGive(sdMutex);
        sdStopIfUsing(path);
        xSemaphoreTake(sdMutex, portMAX_DELAY);
        ok = SD.rename(path.c_str(), target.c_str());
    }
    xSemaphoreGive(sdMutex);
    if (!exists) return sendErr(404, "Nicht gefunden");
    if (bad) return sendErr(400, "Neuer Name nicht erlaubt (Dateien enden auf .mp3)");
    if (taken) return sendErr(400, "Der neue Name existiert schon");
    if (!ok) return sendErr(400, "Umbenennen fehlgeschlagen");
    logLine("[SD] renamed " + path + " -> " + target);
    sendOk("Umbenannt");
}

void handleApiSdFile() {
    String path = webserver.arg("path");
    String lower = path;
    lower.toLowerCase();
    if (!sdReady || !sdPathOk(path) || !lower.endsWith(".mp3")) {
        webserver.send(404, "text/plain", "Not found");
        return;
    }
    if (xSemaphoreTake(sdMutex, pdMS_TO_TICKS(3000)) != pdTRUE) {
        webserver.send(503, "text/plain", "busy");
        return;
    }
    File f = SD.open(path.c_str(), FILE_READ);
    size_t size = f ? f.size() : 0;
    xSemaphoreGive(sdMutex);
    if (!f || f.isDirectory()) {
        if (f) f.close();
        webserver.send(404, "text/plain", "Not found");
        return;
    }
    String base = path.substring(path.lastIndexOf('/') + 1);
    webserver.setContentLength(size);
    webserver.sendHeader("Content-Disposition", "attachment; filename=\"" + base + "\"");
    webserver.send(200, "audio/mpeg", "");
    WiFiClient client = webserver.client();
    uint8_t* buf = (uint8_t*)malloc(2048);
    size_t sent = 0;
    while (buf && sent < size && client.connected()) {
        xSemaphoreTake(sdMutex, portMAX_DELAY);
        int n = f.read(buf, 2048);
        xSemaphoreGive(sdMutex);
        if (n <= 0) break;
        client.write(buf, n);
        sent += n;
        if (audio) audio->loop();
        esp_task_wdt_reset();
    }
    free(buf);
    xSemaphoreTake(sdMutex, portMAX_DELAY);
    f.close();
    xSemaphoreGive(sdMutex);
}

void handleApiFiles() {
    String j = "{\"files\":[";
    bool first = true;
    if (fsReady) {
        File dir = LittleFS.open(MP3_DIR);
        File f = dir ? dir.openNextFile() : File();
        while (f) {
            if (!f.isDirectory()) {
                if (!first) j += ",";
                first = false;
                String n = String(f.name());
                n = n.substring(n.lastIndexOf('/') + 1);
                j += "{\"name\":\"" + jsonEscape(n) + "\",\"size\":" + String((unsigned long)f.size()) + "}";
            }
            f = dir.openNextFile();
        }
    }
    j += "],\"used\":" + String(fsReady ? LittleFS.usedBytes() : 0) + ",\"total\":" + String(fsReady ? LittleFS.totalBytes() : 0) + "}";
    sendJson(200, j);
}

void handleApiDelete() {
    String name = cleanFileName(webserver.arg("file"));
    String path = String(MP3_DIR) + "/" + name;
    if (name.isEmpty() || !LittleFS.exists(path)) return sendErr(404, "Datei nicht gefunden");
    stopIfUsing(name);
    LittleFS.remove(path);
    logLine("[FILES] deleted " + path);
    sendOk("Gel\xC3\xB6scht");
}

File uploadFile;
String uploadError;
bool uploadOnSd = false;
bool uploadOnLang = false;

// Sprachdateien: /lang_<code>.json im Speicher des Geraets; der Code sind zwei bis acht Kleinbuchstaben (zum Beispiel en).
// Language files: /lang_<code>.json in the device storage; the code is two to eight lowercase letters (for example en).

String langCode(String s) {
    s.toLowerCase();
    if (s.endsWith(".json")) s.remove(s.length() - 5);
    if (s.length() < 2 || s.length() > 8) return "";
    for (size_t i = 0; i < s.length(); i++) {
        if (s[i] < 'a' || s[i] > 'z') return "";
    }
    return s;
}

void handleLangJson() {
    String c = langCode(webserver.arg("l"));
    String path = "/lang_" + c + ".json";
    if (c.isEmpty() || !fsReady || !LittleFS.exists(path)) return sendErr(404, "Sprache nicht gefunden");
    File f = LittleFS.open(path, FILE_READ);
    webserver.sendHeader("Cache-Control", "no-cache");
    webserver.streamFile(f, "application/json");
    f.close();
}

void handleApiPins() {
    String pinArg = apiArg("pin");
    if (pinArg.length() > 0) {
        int pin = pinArg.toInt();
        if (pin < 1 || pin > 48 || !pinIdle(pin)) return sendErr(400, "Pin nicht verf\xC3\xBCgbar");
        String type = apiArg("type");
        if (type != "say" && type != "file" && type != "station") type = "";
        String text = apiArg("text");
        text.trim();
        if (text.length() > 300) return sendErr(400, "Text zu lang (max. 300 Zeichen)");
        if (text.isEmpty()) type = "";
        int vol = apiArg("vol").length() > 0 ? constrain(apiArg("vol").toInt(), 0, VOLUME_MAX) : -1;
        pinType[pin] = type;
        pinText[pin] = text;
        pinVol[pin] = vol;
        prefs.putString(("pt" + String(pin)).c_str(), type);
        prefs.putString(("px" + String(pin)).c_str(), text);
        prefs.putInt(("pv" + String(pin)).c_str(), vol);
        logLine("[PIN] GPIO " + String(pin) + " set: " + (type.isEmpty() ? String("no action") : type + " \"" + text + "\""));
        return sendOk("Gespeichert");
    }
    String j = "{\"pins\":[";
    bool first = true;
    for (int pin = 1; pin <= 48; pin++) {
        if (!pinIdle(pin)) continue;
        if (!first) j += ",";
        first = false;
        j += "{\"p\":" + String(pin) + ",\"l\":" + String(digitalRead(pin) == LOW ? 0 : 1) + ",\"t\":\"" + pinType[pin] + "\",\"x\":\"" + jsonEscape(pinText[pin]) + "\",\"v\":" + String(pinVol[pin]) + "}";
    }
    sendJson(200, j + "]}");
}

// Sprachdatei vom Projekt auf GitHub laden (feste Adresse, nur der Sprachcode kommt von aussen) und als /lang_<code>.json ablegen.
// Load a language file from the project on GitHub (fixed address, only the language code comes from outside) and store it as /lang_<code>.json.

#define LANG_GITHUB_BASE "https://raw.githubusercontent.com/holgiw/ESP32-netspeaker/main/lang/"

bool langFetch(const String& code, String& err) {
    if (WiFi.status() != WL_CONNECTED) {
        err = "Keine WLAN-Verbindung";
        return false;
    }
    if (ESP.getFreeHeap() < 70000) {
        err = "Zu wenig freier Speicher (Webradio kurz stoppen)";
        return false;
    }
    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;
    http.useHTTP10(true);
    http.setTimeout(10000);
    http.setConnectTimeout(10000);
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    if (!http.begin(client, String(LANG_GITHUB_BASE) + code + ".json")) {
        err = "HTTP begin failed";
        return false;
    }
    esp_task_wdt_reset();
    int rc = http.GET();
    esp_task_wdt_reset();
    if (rc != HTTP_CODE_OK) {
        err = rc == 404 ? String("Sprachdatei nicht gefunden: ") + code + ".json" : "GitHub HTTP " + String(rc);
        http.end();
        return false;
    }
    LittleFS.remove("/lang_tmp.json");
    File f = LittleFS.open("/lang_tmp.json", FILE_WRITE);
    if (!f) {
        err = "Datei kann nicht angelegt werden";
        http.end();
        return false;
    }
    WiFiClient* s = http.getStreamPtr();
    uint8_t buf[1024];
    size_t total = 0;
    uint8_t first = 0, last = 0;
    unsigned long deadline = millis() + 30000UL;
    bool ok = true;
    while ((http.connected() || s->available()) && millis() < deadline) {
        int n = s->available();
        if (n <= 0) {
            delay(5);
            esp_task_wdt_reset();
            continue;
        }
        if (n > (int)sizeof(buf)) n = sizeof(buf);
        n = s->readBytes(buf, n);
        if (n <= 0) break;
        for (int i = 0; i < n; i++) {
            if (buf[i] > ' ') {
                if (first == 0) first = buf[i];
                last = buf[i];
            }
        }
        total += n;
        if (total > 262144) {
            ok = false;
            err = "Datei zu gross";
            break;
        }
        if (f.write(buf, n) != (size_t)n) {
            ok = false;
            err = "Speicher voll";
            break;
        }
        if (audio) audio->loop();
        esp_task_wdt_reset();
    }
    f.close();
    http.end();
    if (ok && (total < 10 || first != '{' || last != '}')) {
        ok = false;
        err = "Keine gueltige Sprachdatei";
    }
    if (!ok) {
        LittleFS.remove("/lang_tmp.json");
        return false;
    }
    String dest = "/lang_" + code + ".json";
    LittleFS.remove(dest);
    LittleFS.rename("/lang_tmp.json", dest);
    logLine("[LANG] " + code + ".json loaded from GitHub (" + String((unsigned long)total) + " bytes)");
    return true;
}

void handleApiLang() {
    if (webserver.method() == HTTP_POST) {
        String d = langCode(apiArg("delete"));
        if (!d.isEmpty() && fsReady) LittleFS.remove("/lang_" + d + ".json");
        if (apiArg("fetch").length() > 0) {
            String fc = langCode(apiArg("fetch"));
            String err;
            if (fc.isEmpty() || !fsReady) return sendErr(400, "Dateiname wie en.json (zwei bis acht Kleinbuchstaben)");
            if (!langFetch(fc, err)) return sendErr(502, err);
            return sendOk("Geladen");
        }
    }
    String j = "{\"langs\":[";
    bool first = true;
    if (fsReady) {
        File dir = LittleFS.open("/");
        File f = dir ? dir.openNextFile() : File();
        while (f) {
            String n = String(f.name());
            n = n.substring(n.lastIndexOf('/') + 1);
            if (!f.isDirectory() && n.startsWith("lang_") && n.endsWith(".json")) {
                if (!first) j += ",";
                first = false;
                j += "\"" + n.substring(5, n.length() - 5) + "\"";
            }
            f = dir.openNextFile();
        }
    }
    sendJson(200, j + "]}");
}

void handleUploadData() {
    HTTPUpload& up = webserver.upload();
    if (up.status == UPLOAD_FILE_START) {
        uploadError = "";
        uploadOnSd = webserver.arg("target") == "sd";
        uploadOnLang = webserver.arg("target") == "lang";
        if (uploadOnLang) {
            String c = langCode(up.filename);
            if (c.isEmpty()) {
                uploadError = "Dateiname wie en.json (zwei bis acht Kleinbuchstaben)";
            } else {
                uploadFile = LittleFS.open("/lang_" + c + ".json", FILE_WRITE);
                if (!uploadFile) uploadError = "Datei kann nicht angelegt werden";
            }
        } else if (uploadOnSd) {
            String dir = webserver.arg("dir");
            if (dir.isEmpty()) dir = "/";
            String name = sdCleanName(up.filename, true);
            if (!sdReady) {
                uploadError = "SD-Karte nicht verf\xC3\xBCgbar";
            } else if (!sdPathOk(dir)) {
                uploadError = "Ordner nicht erlaubt";
            } else if (name.isEmpty()) {
                uploadError = "Dateiname oder Typ nicht erlaubt";
            } else {
                if (dir.length() > 1 && dir.endsWith("/")) dir.remove(dir.length() - 1);
                String path = (dir == "/" ? String() : dir) + "/" + name;
                sdStopIfUsing(path);
                if (xSemaphoreTake(sdMutex, pdMS_TO_TICKS(3000)) == pdTRUE) {
                    File d = SD.open(dir.c_str());
                    if (d && d.isDirectory()) uploadFile = SD.open(path.c_str(), FILE_WRITE);
                    else uploadError = "Ordner nicht gefunden";
                    if (d) d.close();
                    xSemaphoreGive(sdMutex);
                } else {
                    uploadError = "SD-Karte ist besch\xC3\xA4" "ftigt";
                }
                if (uploadError.isEmpty() && !uploadFile) uploadError = "Datei kann nicht angelegt werden";
            }
        } else {
            String name = cleanFileName(up.filename);
            if (!name.isEmpty()) stopIfUsing(name);
            if (name.isEmpty()) {
                uploadError = "Dateiname oder Typ nicht erlaubt";
            } else {
                uploadFile = LittleFS.open(String(MP3_DIR) + "/" + name, FILE_WRITE);
                if (!uploadFile) uploadError = "Datei kann nicht angelegt werden";
            }
        }
    } else if (up.status == UPLOAD_FILE_WRITE) {
        if (uploadFile) {
            if (uploadOnSd) xSemaphoreTake(sdMutex, portMAX_DELAY);
            size_t written = uploadFile.write(up.buf, up.currentSize);
            if (written != up.currentSize) {
                uploadError = "Speicher voll";
                uploadFile.close();
            }
            if (uploadOnSd) xSemaphoreGive(sdMutex);
        }

        // Ein laufendes Webradio weiter versorgen, solange der Upload die Hauptschleife belegt.
        // Keep feeding a running web radio while the upload occupies the main loop.

        if (audio) audio->loop();
        audioEventPump();
        esp_task_wdt_reset();
    } else if (up.status == UPLOAD_FILE_END) {
        if (uploadFile) {
            if (uploadOnSd) xSemaphoreTake(sdMutex, portMAX_DELAY);
            uploadFile.close();
            if (uploadOnSd) xSemaphoreGive(sdMutex);
        }
        if (uploadError.isEmpty()) logLine("[FILES] uploaded " + up.filename + " (" + String((unsigned long)up.totalSize) + " bytes)");
    } else if (up.status == UPLOAD_FILE_ABORTED) {
        if (uploadFile) {
            if (uploadOnSd) xSemaphoreTake(sdMutex, portMAX_DELAY);
            uploadFile.close();
            if (uploadOnSd) xSemaphoreGive(sdMutex);
        }
        uploadError = "Abgebrochen";
    }
}

void handleUploadDone() {
    if (uploadError.length() > 0) return sendErr(400, uploadError);
    sendOk("Hochgeladen");
}

// Alle WLANs im Umkreis, nach Signalstaerke sortiert, ohne Doppelte; waehrend OTA nie suchen.
// All WiFi networks nearby, sorted by signal strength, without duplicates; never scan during OTA.

void handleApiScan() {
    int n = WiFi.scanNetworks();
    String names[24];
    int rssi[24];
    int count = 0;
    for (int i = 0; i < n && count < 24; i++) {
        String s = WiFi.SSID(i);
        if (s.isEmpty()) continue;
        bool dup = false;
        for (int k = 0; k < count; k++) {
            if (names[k] == s) {
                dup = true;
                if (WiFi.RSSI(i) > rssi[k]) rssi[k] = WiFi.RSSI(i);
            }
        }
        if (dup) continue;
        names[count] = s;
        rssi[count] = WiFi.RSSI(i);
        count++;
    }
    WiFi.scanDelete();
    for (int a = 0; a < count; a++) {
        for (int b = a + 1; b < count; b++) {
            if (rssi[b] > rssi[a]) {
                String t = names[a]; names[a] = names[b]; names[b] = t;
                int r = rssi[a]; rssi[a] = rssi[b]; rssi[b] = r;
            }
        }
    }
    String j = "[";
    for (int i = 0; i < count; i++) j += String(i ? "," : "") + "\"" + jsonEscape(names[i]) + "\"";
    sendJson(200, j + "]");
}

// Eigene NTP-Abfrage, unabhaengig von der eingestellten Systemzeit; Antwort "OK|<UTC-Zeit>" oder "FAILED".
// Own NTP query, independent of the configured system time; answer "OK|<UTC time>" or "FAILED".

void handleTestNtp() {
    String server = webserver.arg("server");
    server.trim();
    webserver.sendHeader("Cache-Control", "no-store");
    IPAddress ip;
    if (server.isEmpty() || WiFi.status() != WL_CONNECTED || !WiFi.hostByName(server.c_str(), ip)) {
        webserver.send(200, "text/plain", "FAILED");
        return;
    }
    WiFiUDP udp;
    uint8_t pkt[48] = {0};
    pkt[0] = 0x1B;
    udp.begin(12300 + (millis() % 1000));
    udp.beginPacket(ip, 123);
    udp.write(pkt, 48);
    udp.endPacket();
    unsigned long start = millis();
    while (millis() - start < 1500) {
        if (udp.parsePacket() >= 48) {
            udp.read(pkt, 48);
            uint32_t secs = ((uint32_t)pkt[40] << 24) | ((uint32_t)pkt[41] << 16) | ((uint32_t)pkt[42] << 8) | pkt[43];
            if (secs < 2208988800UL) break;
            time_t epoch = (time_t)(secs - 2208988800UL);
            struct tm timeinfo;
            gmtime_r(&epoch, &timeinfo);
            char buf[32];
            strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &timeinfo);
            udp.stop();
            webserver.send(200, "text/plain", String("OK|") + buf + " UTC");
            return;
        }
        delay(10);
    }
    udp.stop();
    webserver.send(200, "text/plain", "FAILED");
}

String brokerJson() {
    String groups;
    for (int i = 0; i < 5; i++) groups += String(i > 0 ? "," : "") + "\"" + jsonEscape(cfg.mqttGroup[i]) + "\"";
    return "{\"broker\":\"" + jsonEscape(cfg.mqttServer) + "\",\"port\":" + String(cfg.mqttPort) + ",\"user\":\"" + jsonEscape(cfg.mqttUser) + "\",\"groups\":[" + groups + "],\"connected\":" + (mqttClient.connected() ? "true" : "false") + "}";
}

void handleApiMqtt() {
    if (webserver.method() == HTTP_POST) {
        if (webserver.hasArg("broker")) {
            String b = webserver.arg("broker");
            b.trim();
            if (b.isEmpty() || b.length() > 63) return sendErr(400, "broker invalid");
            if (webserver.hasArg("port")) {
                int p = webserver.arg("port").toInt();
                if (p < 1 || p > 65535) return sendErr(400, "port invalid");
            }
            cfg.mqttServer = b;
            prefs.putString("mq_srv", b);
        }
        if (webserver.hasArg("port")) {
            int p = webserver.arg("port").toInt();
            if (p < 1 || p > 65535) return sendErr(400, "port invalid");
            cfg.mqttPort = p;
            prefs.putInt("mq_port", p);
        }
        if (webserver.hasArg("user")) {
            cfg.mqttUser = webserver.arg("user");
            cfg.mqttUser.trim();
            if (cfg.mqttUser.isEmpty()) cfg.mqttPass = "";
            prefs.putString("mq_usr", cfg.mqttUser);
            prefs.putString("mq_pw", cfg.mqttPass);
        }
        if (webserver.hasArg("pass") && cfg.mqttUser.length() > 0) {
            cfg.mqttPass = webserver.arg("pass");
            prefs.putString("mq_pw", cfg.mqttPass);
        }
        for (int i = 1; i <= 5; i++) {
            if (!webserver.hasArg("group" + String(i))) continue;
            String g = webserver.arg("group" + String(i));
            g.trim();
            cfg.mqttGroup[i - 1] = g;
            prefs.putString(("mq_g" + String(i)).c_str(), g);
        }
        mqttReconnectNow();
        logLine("[MQTT] Broker set via API: " + cfg.mqttServer + ":" + String(cfg.mqttPort));
    }
    sendJson(200, brokerJson());
}

// Dieselben Bedingungen wie topbarHtml(): Erstansicht und Abfrage muessen uebereinstimmen.
// Same conditions as topbarHtml(): first paint and poll must agree.

void handleTopbarStatus() {
    String mq = mqttDotState(), vo = voiceDotState(), au = audioDotState(), ti = timeDotState();
    String j = "{\"mqtt\":\"" + mq + "\",\"mqttPresent\":" + (mqttGatedPresent() ? "true" : "false");
    j += ",\"voice\":\"" + vo + "\",\"voicePresent\":" + (voiceGatedPresent() ? "true" : "false");
    j += ",\"audio\":\"" + au + "\",\"time\":\"" + ti + "\"";
    j += ",\"volValue\":\"" + String(cfg.vol) + "/" + String(VOLUME_MAX) + "\"";
    j += ",\"datetime\":\"" + formatNow() + "\"";
    j += ",\"mqttTitle\":\"" + dotStatusText("MQTT", mq) + "\",\"voiceTitle\":\"" + dotStatusText("VoiceRSS", vo) + "\"";
    j += ",\"audioTitle\":\"" + dotStatusText("Audio", au) + "\",\"timeTitle\":\"" + dotStatusText("Zeit", ti) + "\"";
    j += ",\"version\":\"" + String(FIRMWARE_MARKER + FIRMWARE_MARKER_PREFIX_LEN) + "\"}";
    sendJson(200, j);
}

void handleVersion() {
    webserver.sendHeader("Cache-Control", "no-store");
    webserver.send(200, "text/plain", String(FIRMWARE_MARKER + FIRMWARE_MARKER_PREFIX_LEN));
}

void handleLog() {
    webserver.sendHeader("Cache-Control", "no-store");
    String out;
    if (fsReady) {
        File old = LittleFS.open(LOG_OLD_FILE, FILE_READ);
        if (old) {
            out += old.readString();
            old.close();
        }
        File cur = LittleFS.open(LOG_FILE, FILE_READ);
        if (cur) {
            out += cur.readString();
            cur.close();
        }
    }
    if (!cfg.logEnabled) out = "[Logging ist ausgeschaltet - Schalter oben im Tab Log]\n" + out;
    webserver.send(200, "text/plain; charset=utf-8", out);
}

void handleRestart() {
    webserver.send(200, "text/plain", "restarting");
    logLine("[WEB] restart requested");
    delay(300);
    ESP.restart();
}

void handleFavicon() {
    webserver.sendHeader("Cache-Control", "public, max-age=86400");
    webserver.send(200, "image/svg+xml",
                   "<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 32 32'>"
                   "<circle cx='16' cy='16' r='14' fill='#10151a' stroke='#f5a623' stroke-width='2.5'/>"
                   "<path d='M10 13v6M14 10v12M18 12v8M22 14v4' stroke='#f5a623' stroke-width='2.5' stroke-linecap='round'/></svg>");
}

void setupWeb() {
    webserver.on("/", HTTP_GET, handleRoot);
    webserver.on("/save", HTTP_POST, handleSave);
    webserver.on("/export", HTTP_GET, handleExport);
    webserver.on("/import", HTTP_POST, handleImport);
    webserver.on("/stations.txt", HTTP_GET, handleStationsExport);
    webserver.on("/importstations", HTTP_POST, handleStationsImport);
    webserver.on("/restart", HTTP_POST, handleRestart);
    webserver.on("/log", HTTP_GET, handleLog);
    webserver.on("/favicon.ico", HTTP_GET, handleFavicon);
    webserver.on("/api/say", handleApiSay);
    webserver.on("/api/play", handleApiPlay);
    webserver.on("/api/station", handleApiStation);
    webserver.on("/api/stations", HTTP_GET, handleApiStations);
    webserver.on("/api/stationdelete", handleApiStationDelete);
    webserver.on("/api/url", handleApiUrl);
    webserver.on("/api/file", handleApiFile);
    webserver.on("/api/log", handleApiLog);
    webserver.on("/api/stop", handleApiStop);
    webserver.on("/api/volume", handleApiVolume);
    webserver.on("/api/status", HTTP_GET, handleApiStatus);
    webserver.on("/api/files", HTTP_GET, handleApiFiles);
    webserver.on("/api/sd", HTTP_GET, handleApiSd);
    webserver.on("/api/sdmount", handleApiSdMount);
    webserver.on("/api/sdmkdir", handleApiSdMkdir);
    webserver.on("/api/sddelete", handleApiSdDelete);
    webserver.on("/api/sdrename", handleApiSdRename);
    webserver.on("/api/sdfile", HTTP_GET, handleApiSdFile);
    webserver.on("/api/delete", handleApiDelete);
    webserver.on("/api/scan", HTTP_GET, handleApiScan);
    webserver.on("/api/testNtp", HTTP_GET, handleTestNtp);
    webserver.on("/api/mqtt", handleApiMqtt);
    webserver.on("/api/topbarStatus", HTTP_GET, handleTopbarStatus);
    webserver.on("/api/version", HTTP_GET, handleVersion);
    webserver.on("/upload", HTTP_POST, handleUploadDone, handleUploadData);
    webserver.on("/lang.json", HTTP_GET, handleLangJson);
    webserver.on("/api/lang", handleApiLang);
    webserver.on("/api/pins", handleApiPins);
    webserver.onNotFound([]() { webserver.send(404, "text/plain", "Not found"); });
    webserver.begin();
    logLine("[WEB] server started");
}

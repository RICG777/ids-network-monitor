// HTTP server (E3 — v2f.3). Full write API matching every serial command.
// Jumper-gated: every /api/* returns 403 when the UEXT jumper is open.
// USB serial config is *not* gated.
//
// Endpoints:
//   GET    /                         landing page (stub UI)
//   GET    /api/status               full config + runtime state (JSON)
//   POST   /api/network              partial: {ip?, subnet?, gateway?}
//   POST   /api/verbosity            {level: 0|1|2}
//   POST   /api/relay/{1|2}/triggers {triggers: "..."}
//   POST   /api/relay/{1|2}/mode     {mode: "PULSE"|"LATCH"}
//   POST   /api/relay/{1|2}/duration {seconds: 1..255}
//   POST   /api/relay/{1|2}/reset    {}
//   POST   /api/relay/{1|2}/test     {duration: "quick"|"timed"}
//   POST   /api/ping/config          partial: {enabled?, interval?, threshold?,
//                                              relay?, mode?, duration?}
//   POST   /api/ping/targets         {ip: "..."}
//   DELETE /api/ping/targets?ip=...
//   POST   /api/ping/reset           {}
//   POST   /api/ping/clear           {}
//
// POST handlers require Content-Type: application/json (acts as a light
// CSRF guard since browsers can't set arbitrary content types without
// preflight). DELETE handlers accept no body.

#include "embedded_assets.h"

// --- Response helpers ---

// CORS: Allow any origin so the web app served from localhost:8000 (dev) or
// another LAN host can call this board's API. Jumper gate is the real access
// control; CORS is convenience, not security.
static void addCorsHeaders() {
    server.sendHeader("Access-Control-Allow-Origin",  "*");
    server.sendHeader("Access-Control-Allow-Methods", "GET,POST,DELETE,OPTIONS");
    server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
    server.sendHeader("Access-Control-Max-Age",       "600");
}

void httpError(int code, const char* err, const char* msg) {
    JsonDocument doc;
    doc["error"] = err;
    if (msg && *msg) doc["message"] = msg;
    String out;
    serializeJson(doc, out);
    addCorsHeaders();
    server.send(code, "application/json", out);
}

bool requireJumper() {
    if (!jumperFitted) {
        httpError(403, "jumper-removed",
                  "Web config requires the UEXT jumper (GPIO 16 <-> GPIO 13) fitted.");
        return false;
    }
    return true;
}

bool requireJson() {
    String ct = server.header("Content-Type");
    ct.toLowerCase();
    if (!ct.startsWith("application/json")) {
        httpError(415, "content-type",
                  "Content-Type must be application/json");
        return false;
    }
    return true;
}

bool parseBody(JsonDocument& doc) {
    DeserializationError err = deserializeJson(doc, server.arg("plain"));
    if (err) {
        httpError(400, "invalid-json", err.c_str());
        return false;
    }
    return true;
}

void respondWithStatus() {
    JsonDocument doc;
    buildStatusJson(doc);
    String out;
    serializeJson(doc, out);
    addCorsHeaders();
    server.send(200, "application/json", out);
}

// --- Status doc builder (unchanged from v2f.2) ---

void buildStatusJson(JsonDocument& doc) {
    doc["firmware"] = FIRMWARE_VERSION;
    doc["mac"]      = ETH.macAddress();

    JsonObject net = doc["network"].to<JsonObject>();
    net["ip"]       = ETH.localIP().toString();
    net["subnet"]   = subnet.toString();
    net["gateway"]  = gateway.toString();
    net["eth"]      = ethConnected;

    JsonObject r1 = doc["relay1"].to<JsonObject>();
    r1["triggers"] = r1Trigs;
    r1["mode"]     = relay1Mode == MODE_LATCH ? "LATCH" : "PULSE";
    r1["duration"] = relay1Duration;
    r1["state"]    = relay1IsOn ? "ON" : "OFF";

    JsonObject r2 = doc["relay2"].to<JsonObject>();
    r2["triggers"] = r2Trigs;
    r2["mode"]     = relay2Mode == MODE_LATCH ? "LATCH" : "PULSE";
    r2["duration"] = relay2Duration;
    r2["state"]    = relay2IsOn ? "ON" : "OFF";

    doc["verbosity"] = verbosity;
    doc["jumper"]    = jumperFitted ? "FITTED" : "OPEN";
    doc["heap"]      = (uint32_t)ESP.getFreeHeap();

    JsonObject p = doc["ping"].to<JsonObject>();
    p["enabled"]   = pingEnabled;
    p["interval"]  = pingInterval;
    p["threshold"] = pingThreshold;
    p["relay"]     = pingRelay;
    p["mode"]      = pingMode == PING_MODE_AUTO ? "AUTO"
                   : pingMode == PING_MODE_LATCH ? "LATCH" : "PULSE";
    p["duration"]  = pingDuration;
    p["rstate"]    = pingRelayIsOn ? "ON" : "OFF";
    p["count"]     = pingCount;

    JsonArray targets = p["targets"].to<JsonArray>();
    for (byte i = 0; i < pingCount; i++) {
        if (pingActive[i]) {
            JsonObject t = targets.add<JsonObject>();
            t["ip"]    = pingTargets[i].toString();
            t["state"] = pingFails[i] >= pingThreshold ? "FAIL" : "OK";
            t["fails"] = pingFails[i];
        }
    }
}

// --- Read-only handlers ---

void handleRoot() {
    server.send_P(200, "text/html; charset=utf-8", INDEX_HTML);
}

void handleApiStatus() {
    if (!requireJumper()) return;
    respondWithStatus();
}

// --- Network config ---

void handleApiNetwork() {
    if (!requireJumper() || !requireJson()) return;
    JsonDocument req;
    if (!parseBody(req)) return;

    bool changed = false;
    if (req["ip"].is<const char*>()) {
        const char* v = req["ip"];
        IPAddress nip = parseIP(v);
        if (!isValidIP(nip)) { httpError(400, "invalid-ip", v); return; }
        ip = nip;
        prefs.putString("ip", v);
        changed = true;
        logEvent(VERB_NORMAL, "IP set to %s (HTTP)", v);
    }
    if (req["subnet"].is<const char*>()) {
        const char* v = req["subnet"];
        subnet = parseIP(v);
        prefs.putString("subnet", v);
        changed = true;
    }
    if (req["gateway"].is<const char*>()) {
        const char* v = req["gateway"];
        gateway = parseIP(v);
        prefs.putString("gateway", v);
        changed = true;
    }
    if (changed) {
        if (gateway[0]==0 && gateway[1]==0 && gateway[2]==0 && gateway[3]==0)
            ETH.config(ip, IPAddress(0,0,0,0), subnet);
        else
            ETH.config(ip, gateway, subnet);
        Udp.stop(); Udp.begin(localPort);
    }
    respondWithStatus();
}

// --- Verbosity ---

void handleApiVerbosity() {
    if (!requireJumper() || !requireJson()) return;
    JsonDocument req;
    if (!parseBody(req)) return;
    if (!req["level"].is<int>()) { httpError(400, "missing-level", ""); return; }
    int v = req["level"];
    if (v < 0 || v > 2) { httpError(400, "invalid-level", "must be 0..2"); return; }
    verbosity = v;
    prefs.putUChar("verb", verbosity);
    respondWithStatus();
}

// --- Relay handlers (per-relay small funcs to keep routes explicit) ---

static void applyRelayTriggers(int n, const char* trig) {
    if (n == 1) {
        strncpy(r1Trigs, trig, MAX_TRIG_STR - 1);
        r1Trigs[MAX_TRIG_STR - 1] = '\0';
        prefs.putString("r1trigs", r1Trigs);
    } else {
        strncpy(r2Trigs, trig, MAX_TRIG_STR - 1);
        r2Trigs[MAX_TRIG_STR - 1] = '\0';
        prefs.putString("r2trigs", r2Trigs);
    }
    logEvent(VERB_NORMAL, "Relay %d triggers set (HTTP)", n);
}

static void handleRelayTriggers(int n) {
    if (!requireJumper() || !requireJson()) return;
    JsonDocument req;
    if (!parseBody(req)) return;
    if (!req["triggers"].is<const char*>()) { httpError(400, "missing-triggers", ""); return; }
    const char* trig = req["triggers"];
    int len = strlen(trig);
    if (len < 1 || len >= MAX_TRIG_STR) {
        httpError(400, "invalid-triggers", "length must be 1..119"); return;
    }
    applyRelayTriggers(n, trig);
    respondWithStatus();
}

static void handleRelayMode(int n) {
    if (!requireJumper() || !requireJson()) return;
    JsonDocument req;
    if (!parseBody(req)) return;
    if (!req["mode"].is<const char*>()) { httpError(400, "missing-mode", ""); return; }
    const char* m = req["mode"];
    byte mode;
    if (strcmp(m, "PULSE") == 0) mode = MODE_PULSE;
    else if (strcmp(m, "LATCH") == 0) mode = MODE_LATCH;
    else { httpError(400, "invalid-mode", "PULSE or LATCH"); return; }
    if (n == 1) { relay1Mode = mode; prefs.putUChar("r1mode", mode); }
    else        { relay2Mode = mode; prefs.putUChar("r2mode", mode); }
    logEvent(VERB_NORMAL, "Relay %d mode: %s (HTTP)", n, m);
    respondWithStatus();
}

static void handleRelayDuration(int n) {
    if (!requireJumper() || !requireJson()) return;
    JsonDocument req;
    if (!parseBody(req)) return;
    if (!req["seconds"].is<int>()) { httpError(400, "missing-seconds", ""); return; }
    int s = req["seconds"];
    if (s < 1 || s > 255) { httpError(400, "invalid-seconds", "1..255"); return; }
    if (n == 1) { relay1Duration = s; prefs.putUChar("r1dur", s); }
    else        { relay2Duration = s; prefs.putUChar("r2dur", s); }
    logEvent(VERB_NORMAL, "Relay %d duration: %ds (HTTP)", n, s);
    respondWithStatus();
}

static void handleRelayReset(int n) {
    if (!requireJumper()) return;
    if (n == 1) { relay1IsOn = false; updateRelayOutput(1); }
    else        { relay2IsOn = false; updateRelayOutput(2); }
    logEvent(VERB_NORMAL, "Relay %d reset (HTTP)", n);
    respondWithStatus();
}

// NOTE: blocks the HTTP thread for the test duration (matches serial behaviour
// — board's single-threaded loop can't service other work during TEST either).
static void handleRelayTest(int n) {
    if (!requireJumper() || !requireJson()) return;
    JsonDocument req;
    if (!parseBody(req)) return;
    const char* dur = req["duration"] | "quick";
    int pin = (n == 1) ? relayPin1 : relayPin2;
    byte secs;
    if (strcmp(dur, "quick") == 0) {
        logEvent(VERB_NORMAL, "Relay %d quick test (HTTP)", n);
        digitalWrite(pin, HIGH); delay(TEST_PULSE_MS); digitalWrite(pin, LOW);
    } else if (strcmp(dur, "timed") == 0) {
        secs = (n == 1) ? relay1Duration : relay2Duration;
        logEvent(VERB_NORMAL, "Relay %d timed test %ds (HTTP)", n, secs);
        digitalWrite(pin, HIGH);
        delay((unsigned long)secs * 1000UL);
        digitalWrite(pin, LOW);
    } else {
        httpError(400, "invalid-duration", "'quick' or 'timed'"); return;
    }
    respondWithStatus();
}

// Thin wrappers so server.on() has stable function pointers
void handleR1Triggers() { handleRelayTriggers(1); }
void handleR2Triggers() { handleRelayTriggers(2); }
void handleR1Mode()     { handleRelayMode(1); }
void handleR2Mode()     { handleRelayMode(2); }
void handleR1Duration() { handleRelayDuration(1); }
void handleR2Duration() { handleRelayDuration(2); }
void handleR1Reset()    { handleRelayReset(1); }
void handleR2Reset()    { handleRelayReset(2); }
void handleR1Test()     { handleRelayTest(1); }
void handleR2Test()     { handleRelayTest(2); }

// --- Ping config ---

void handleApiPingConfig() {
    if (!requireJumper() || !requireJson()) return;
    JsonDocument req;
    if (!parseBody(req)) return;

    if (req["enabled"].is<bool>()) {
        pingEnabled = req["enabled"];
        prefs.putUChar("pEn", pingEnabled ? 1 : 0);
        if (pingEnabled) pingLastCycleMs = millis();
    }
    if (req["interval"].is<int>()) {
        int v = req["interval"];
        if (v < 5 || v > 255) { httpError(400, "invalid-interval", "5..255"); return; }
        pingInterval = v; prefs.putUChar("pInt", pingInterval);
    }
    if (req["threshold"].is<int>()) {
        int v = req["threshold"];
        if (v < 1 || v > 255) { httpError(400, "invalid-threshold", "1..255"); return; }
        pingThreshold = v; prefs.putUChar("pThr", pingThreshold);
    }
    if (req["relay"].is<int>()) {
        int v = req["relay"];
        if (v < 1 || v > 2) { httpError(400, "invalid-relay", "1 or 2"); return; }
        byte oldRelay = pingRelay;
        pingRelay = v; prefs.putUChar("pRelay", pingRelay);
        if (pingRelayIsOn && oldRelay != pingRelay) {
            updateRelayOutput(oldRelay); updateRelayOutput(pingRelay);
        }
    }
    if (req["mode"].is<const char*>()) {
        const char* m = req["mode"];
        if (strcmp(m, "AUTO") == 0) pingMode = PING_MODE_AUTO;
        else if (strcmp(m, "LATCH") == 0) pingMode = PING_MODE_LATCH;
        else if (strcmp(m, "PULSE") == 0) pingMode = PING_MODE_PULSE;
        else { httpError(400, "invalid-mode", "AUTO|LATCH|PULSE"); return; }
        prefs.putUChar("pMode", pingMode);
    }
    if (req["duration"].is<int>()) {
        int v = req["duration"];
        if (v < 1 || v > 255) { httpError(400, "invalid-duration", "1..255"); return; }
        pingDuration = v; prefs.putUChar("pDur", pingDuration);
    }
    logEvent(VERB_NORMAL, "Ping config updated (HTTP)");
    respondWithStatus();
}

void handleApiPingTargetAdd() {
    if (!requireJumper() || !requireJson()) return;
    JsonDocument req;
    if (!parseBody(req)) return;
    if (!req["ip"].is<const char*>()) { httpError(400, "missing-ip", ""); return; }
    const char* ipStr = req["ip"];
    IPAddress newIP = parseIP(ipStr);
    if (!isValidIP(newIP)) { httpError(400, "invalid-ip", ipStr); return; }
    for (byte i = 0; i < pingCount; i++) {
        if (pingActive[i] && pingTargets[i] == newIP) {
            httpError(409, "duplicate", ipStr); return;
        }
    }
    if (pingCount >= MAX_PING_TARGETS) {
        httpError(409, "full", "max 20 targets"); return;
    }
    pingTargets[pingCount] = newIP;
    pingActive[pingCount]  = true;
    pingFails[pingCount]   = 0;
    pingOK[pingCount]      = true;
    pingCount++;
    savePingTargets();
    logEvent(VERB_NORMAL, "Ping target added: %s (HTTP)", ipStr);
    respondWithStatus();
}

void handleApiPingTargetDelete() {
    if (!requireJumper()) return;
    if (!server.hasArg("ip")) { httpError(400, "missing-ip", "use ?ip=..."); return; }
    String ipStr = server.arg("ip");
    IPAddress delIP = parseIP(ipStr.c_str());
    for (byte i = 0; i < pingCount; i++) {
        if (pingActive[i] && pingTargets[i] == delIP) {
            for (byte j = i; j < pingCount - 1; j++) {
                pingTargets[j] = pingTargets[j+1];
                pingActive[j]  = pingActive[j+1];
                pingFails[j]   = pingFails[j+1];
                pingOK[j]      = pingOK[j+1];
            }
            pingCount--;
            pingActive[pingCount] = false;
            if (pingCurrentIdx >= pingCount && pingCount > 0) pingCurrentIdx = 0;
            savePingTargets();
            logEvent(VERB_NORMAL, "Ping target removed: %s (HTTP)", ipStr.c_str());
            respondWithStatus();
            return;
        }
    }
    httpError(404, "not-found", ipStr.c_str());
}

void handleApiPingReset() {
    if (!requireJumper()) return;
    pingRelayIsOn = false;
    for (byte i = 0; i < pingCount; i++) pingFails[i] = 0;
    updateRelayOutput(pingRelay);
    logEvent(VERB_NORMAL, "Ping reset (HTTP)");
    respondWithStatus();
}

void handleApiPingClear() {
    if (!requireJumper()) return;
    pingCount = 0;
    for (byte i = 0; i < MAX_PING_TARGETS; i++) {
        pingActive[i] = false;
        pingFails[i]  = 0;
        pingOK[i]     = true;
    }
    pingRelayIsOn = false;
    updateRelayOutput(pingRelay);
    savePingTargets();
    logEvent(VERB_NORMAL, "Ping targets cleared (HTTP)");
    respondWithStatus();
}

// --- 404 / preflight ---

void handleNotFound() {
    // CORS preflight: browsers send OPTIONS before any cross-origin POST with
    // a non-simple content type. Answer 204 + CORS headers unconditionally;
    // authorization happens on the actual request, not the preflight.
    if (server.method() == HTTP_OPTIONS) {
        addCorsHeaders();
        server.send(204);
        return;
    }
    if (String(server.uri()).startsWith("/api/") && !jumperFitted) {
        httpError(403, "jumper-removed", "");
        return;
    }
    httpError(404, "not-found", server.uri().c_str());
}

// --- Setup / tick ---

void setupWebServer() {
    // Collect Content-Type so requireJson() can inspect it.
    const char* hdrs[] = { "Content-Type" };
    server.collectHeaders(hdrs, sizeof(hdrs) / sizeof(hdrs[0]));

    // Reads
    server.on("/",               HTTP_GET,    handleRoot);
    server.on("/api/status",     HTTP_GET,    handleApiStatus);

    // Network + verbosity
    server.on("/api/network",    HTTP_POST,   handleApiNetwork);
    server.on("/api/verbosity",  HTTP_POST,   handleApiVerbosity);

    // Relays 1 & 2
    server.on("/api/relay/1/triggers", HTTP_POST, handleR1Triggers);
    server.on("/api/relay/2/triggers", HTTP_POST, handleR2Triggers);
    server.on("/api/relay/1/mode",     HTTP_POST, handleR1Mode);
    server.on("/api/relay/2/mode",     HTTP_POST, handleR2Mode);
    server.on("/api/relay/1/duration", HTTP_POST, handleR1Duration);
    server.on("/api/relay/2/duration", HTTP_POST, handleR2Duration);
    server.on("/api/relay/1/reset",    HTTP_POST, handleR1Reset);
    server.on("/api/relay/2/reset",    HTTP_POST, handleR2Reset);
    server.on("/api/relay/1/test",     HTTP_POST, handleR1Test);
    server.on("/api/relay/2/test",     HTTP_POST, handleR2Test);

    // Ping
    server.on("/api/ping/config",  HTTP_POST,   handleApiPingConfig);
    server.on("/api/ping/targets", HTTP_POST,   handleApiPingTargetAdd);
    server.on("/api/ping/targets", HTTP_DELETE, handleApiPingTargetDelete);
    server.on("/api/ping/reset",   HTTP_POST,   handleApiPingReset);
    server.on("/api/ping/clear",   HTTP_POST,   handleApiPingClear);

    server.onNotFound(handleNotFound);
    server.begin();
    logEvent(VERB_NORMAL, "Web server listening on port 80");
}

void tickWebServer() {
    server.handleClient();
}

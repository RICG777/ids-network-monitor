// HTTP server (E3 — v2f.2). Serves the stub landing page at / and a JSON
// status document at /api/status. More endpoints (POST/DELETE to match
// every serial command) land in v2f.3.
//
// Jumper gate: every /api/* handler checks jumperFitted. USB serial
// config is *not* gated — physical USB = physical panel access.
//
// Built-in WebServer (synchronous). handleClient() is called first thing
// each loop iteration so HTTP gets serviced ahead of the UDP listener
// and ping engine.

#include "embedded_assets.h"

// Send a consistent JSON error body and HTTP status.
void httpError(int code, const char* err, const char* msg) {
    JsonDocument doc;
    doc["error"] = err;
    if (msg && *msg) doc["message"] = msg;
    String out;
    serializeJson(doc, out);
    server.send(code, "application/json", out);
}

// Gate every /api/* handler — return false if the handler should bail.
bool requireJumper() {
    if (!jumperFitted) {
        httpError(403, "jumper-removed",
                  "Web config requires the UEXT jumper (GPIO 16 <-> GPIO 13) fitted.");
        return false;
    }
    return true;
}

// Populate a JSON document mirroring what STATUS emits over serial.
// Both paths read from the same globals; no divergence.
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

// --- Handlers ---

void handleRoot() {
    // Stub landing page. Client-side JS fetches /api/status and renders it.
    server.send_P(200, "text/html; charset=utf-8", INDEX_HTML);
}

void handleApiStatus() {
    if (!requireJumper()) return;
    JsonDocument doc;
    buildStatusJson(doc);
    String out;
    serializeJson(doc, out);
    server.send(200, "application/json", out);
}

void handleNotFound() {
    // Same gate: if jumper is out we don't even tell you what's there.
    if (String(server.uri()).startsWith("/api/") && !jumperFitted) {
        httpError(403, "jumper-removed", "");
        return;
    }
    httpError(404, "not-found", server.uri().c_str());
}

// --- Setup / tick ---

void setupWebServer() {
    server.on("/",             HTTP_GET, handleRoot);
    server.on("/api/status",   HTTP_GET, handleApiStatus);
    server.onNotFound(handleNotFound);
    server.begin();
    logEvent(VERB_NORMAL, "Web server listening on port 80");
}

void tickWebServer() {
    server.handleClient();
}

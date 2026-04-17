// ICMP ping heartbeat engine — E1 feature
// Up to 20 targets, configurable interval/threshold/relay/mode.
// Shares relay outputs with syslog triggers via updateRelayOutput().

// Forward declarations — needed because esp_ping_handle_t isn't a plain type
// and Arduino's auto-prototype generator skips these.
void pingOnSuccess(esp_ping_handle_t hdl, void *args);
void pingOnTimeout(esp_ping_handle_t hdl, void *args);
void pingOnEnd(esp_ping_handle_t hdl, void *args);

byte findNextActiveTarget(byte startIdx) {
    for (byte i = 0; i < pingCount; i++) {
        byte idx = (startIdx + i) % pingCount;
        if (pingActive[idx]) return idx;
    }
    return 255; // none found
}

void startNextPing() {
    if (pingInProgress || pingCount == 0) return;

    byte idx = findNextActiveTarget(pingCurrentIdx);
    if (idx == 255) return;

    pingCurrentIdx = idx;

    esp_ping_config_t config = ESP_PING_DEFAULT_CONFIG();
    config.target_addr.type = IPADDR_TYPE_V4;
    config.target_addr.u_addr.ip4.addr = (uint32_t)pingTargets[idx];
    config.count = 1;
    config.interval_ms = 1000;
    config.timeout_ms = 2000;
    config.task_stack_size = 4096;

    esp_ping_callbacks_t cbs = {};
    cbs.on_ping_success = pingOnSuccess;
    cbs.on_ping_timeout = pingOnTimeout;
    cbs.on_ping_end = pingOnEnd;
    cbs.cb_args = NULL;

    esp_err_t err = esp_ping_new_session(&config, &cbs, &pingSession);
    if (err != ESP_OK) {
        if (verbosity >= VERB_DEBUG) {
            Serial.print(F("ArdMsg: Ping session error: "));
            Serial.println(err);
        }
        return;
    }

    pingInProgress = true;
    esp_ping_start(pingSession);

    if (verbosity >= VERB_DEBUG) {
        Serial.print(F("ArdMsg: Pinging "));
        Serial.println(pingTargets[idx]);
    }
}

void cleanupPingSession() {
    if (pingSession != NULL) {
        esp_ping_stop(pingSession);
        esp_ping_delete_session(pingSession);
        pingSession = NULL;
    }
    pingInProgress = false;
    // Advance to next target
    pingCurrentIdx = (pingCurrentIdx + 1) % pingCount;
}

bool allTargetsHealthy() {
    for (byte i = 0; i < pingCount; i++) {
        if (pingActive[i] && pingFails[i] >= pingThreshold) return false;
    }
    return true;
}

void pingTriggerRelay() {
    if (!pingRelayIsOn) {
        pingRelayIsOn = true;
        pingRelayOnTime = millis();
        updateRelayOutput(pingRelay);
        logEvent(VERB_QUIET, "Ping FAIL - relay ENERGIZED!");
    }
}

void pingOnSuccess(esp_ping_handle_t hdl, void *args) {
    byte idx = pingCurrentIdx;
    pingFails[idx] = 0;
    pingOK[idx] = true;

    if (verbosity >= VERB_DEBUG) {
        Serial.print(F("ArdMsg: Ping OK: "));
        Serial.println(pingTargets[idx]);
    }

    // AUTO_RESET: clear relay if all targets healthy
    if (pingRelayIsOn && pingMode == PING_MODE_AUTO && allTargetsHealthy()) {
        pingRelayIsOn = false;
        updateRelayOutput(pingRelay);
        logEvent(VERB_NORMAL, "All ping targets OK - relay cleared.");
    }

    cleanupPingSession();
}

void pingOnTimeout(esp_ping_handle_t hdl, void *args) {
    byte idx = pingCurrentIdx;
    if (pingFails[idx] < 255) pingFails[idx]++;
    pingOK[idx] = false;

    logEvent(VERB_NORMAL, "Ping timeout: %s (fails: %d)",
             pingTargets[idx].toString().c_str(), pingFails[idx]);

    if (pingFails[idx] >= pingThreshold) {
        pingTriggerRelay();
    }

    cleanupPingSession();
}

void pingOnEnd(esp_ping_handle_t hdl, void *args) {
    // Safety cleanup if callbacks didn't fire
    if (pingInProgress) {
        cleanupPingSession();
    }
}

// ===================== PING COMMANDS =====================

void handlePingCommand(const char* cmd) {
    if (strncmp(cmd, "ADD:", 4) == 0) {
        IPAddress newIP = parseIP(cmd + 4);
        if (!isValidIP(newIP)) {
            Serial.println(F("ArdERR:PING:invalid IP"));
            return;
        }
        // Check for duplicate
        for (byte i = 0; i < pingCount; i++) {
            if (pingActive[i] && pingTargets[i] == newIP) {
                Serial.println(F("ArdERR:PING:duplicate"));
                return;
            }
        }
        if (pingCount >= MAX_PING_TARGETS) {
            Serial.println(F("ArdERR:PING:full"));
            return;
        }
        pingTargets[pingCount] = newIP;
        pingActive[pingCount] = true;
        pingFails[pingCount] = 0;
        pingOK[pingCount] = true;
        pingCount++;
        savePingTargets();
        Serial.print(F("ArdACK:PING:ADD:"));
        Serial.println(newIP);

    } else if (strncmp(cmd, "DEL:", 4) == 0) {
        IPAddress delIP = parseIP(cmd + 4);
        bool found = false;
        for (byte i = 0; i < pingCount; i++) {
            if (pingActive[i] && pingTargets[i] == delIP) {
                // Compact array
                for (byte j = i; j < pingCount - 1; j++) {
                    pingTargets[j] = pingTargets[j+1];
                    pingActive[j] = pingActive[j+1];
                    pingFails[j] = pingFails[j+1];
                    pingOK[j] = pingOK[j+1];
                }
                pingCount--;
                pingActive[pingCount] = false;
                if (pingCurrentIdx >= pingCount && pingCount > 0)
                    pingCurrentIdx = 0;
                savePingTargets();
                found = true;
                Serial.print(F("ArdACK:PING:DEL:"));
                Serial.println(delIP);
                break;
            }
        }
        if (!found) Serial.println(F("ArdERR:PING:not found"));

    } else if (strncmp(cmd, "LIST", 4) == 0) {
        sendPingStatus();

    } else if (strncmp(cmd, "INT:", 4) == 0) {
        int val = atoi(cmd + 4);
        if (val < 5 || val > 255) { Serial.println(F("ArdERR:PING:INT must be 5-255")); }
        else {
            pingInterval = val;
            prefs.putUChar("pInt", pingInterval);
            Serial.print(F("ArdACK:PING:INT:"));
            Serial.println(pingInterval);
        }

    } else if (strncmp(cmd, "THR:", 4) == 0) {
        int val = atoi(cmd + 4);
        if (val < 1 || val > 255) { Serial.println(F("ArdERR:PING:THR must be 1-255")); }
        else {
            pingThreshold = val;
            prefs.putUChar("pThr", pingThreshold);
            Serial.print(F("ArdACK:PING:THR:"));
            Serial.println(pingThreshold);
        }

    } else if (strncmp(cmd, "RELAY:", 6) == 0) {
        int val = atoi(cmd + 6);
        if (val < 1 || val > 2) { Serial.println(F("ArdERR:PING:RELAY must be 1-2")); }
        else {
            byte oldRelay = pingRelay;
            pingRelay = val;
            prefs.putUChar("pRelay", pingRelay);
            // If relay changed and ping relay was on, update both outputs
            if (pingRelayIsOn && oldRelay != pingRelay) {
                updateRelayOutput(oldRelay);
                updateRelayOutput(pingRelay);
            }
            Serial.print(F("ArdACK:PING:RELAY:"));
            Serial.println(pingRelay);
        }

    } else if (strncmp(cmd, "MODE:", 5) == 0) {
        if (strncmp(cmd + 5, "AUTO", 4) == 0) {
            pingMode = PING_MODE_AUTO;
        } else if (strncmp(cmd + 5, "LATCH", 5) == 0) {
            pingMode = PING_MODE_LATCH;
        } else if (strncmp(cmd + 5, "PULSE", 5) == 0) {
            pingMode = PING_MODE_PULSE;
        } else {
            Serial.println(F("ArdERR:PING:MODE must be AUTO/LATCH/PULSE"));
            return;
        }
        prefs.putUChar("pMode", pingMode);
        Serial.print(F("ArdACK:PING:MODE:"));
        Serial.println(pingMode == PING_MODE_AUTO ? "AUTO" : (pingMode == PING_MODE_LATCH ? "LATCH" : "PULSE"));

    } else if (strncmp(cmd, "DUR:", 4) == 0) {
        int val = atoi(cmd + 4);
        if (val < 1 || val > 255) { Serial.println(F("ArdERR:PING:DUR must be 1-255")); }
        else {
            pingDuration = val;
            prefs.putUChar("pDur", pingDuration);
            Serial.print(F("ArdACK:PING:DUR:"));
            Serial.println(pingDuration);
        }

    } else if (strncmp(cmd, "ON", 2) == 0) {
        pingEnabled = true;
        prefs.putUChar("pEn", 1);
        pingLastCycleMs = millis(); // start fresh
        Serial.println(F("ArdACK:PING:ON"));

    } else if (strncmp(cmd, "OFF", 3) == 0) {
        pingEnabled = false;
        prefs.putUChar("pEn", 0);
        Serial.println(F("ArdACK:PING:OFF"));

    } else if (strncmp(cmd, "RESET", 5) == 0) {
        pingRelayIsOn = false;
        for (byte i = 0; i < pingCount; i++) pingFails[i] = 0;
        updateRelayOutput(pingRelay);
        Serial.println(F("ArdACK:PING:RESET:OK"));

    } else if (strncmp(cmd, "CLR", 3) == 0) {
        pingCount = 0;
        for (byte i = 0; i < MAX_PING_TARGETS; i++) {
            pingActive[i] = false;
            pingFails[i] = 0;
            pingOK[i] = true;
        }
        pingRelayIsOn = false;
        updateRelayOutput(pingRelay);
        savePingTargets();
        Serial.println(F("ArdACK:PING:CLR:OK"));
    }
}

// ===================== PING STATUS =====================

void sendPingStatus() {
    Serial.print(F("ArdSTATUS:PING:EN:"));
    Serial.println(pingEnabled ? 1 : 0);
    Serial.print(F("ArdSTATUS:PING:INT:"));
    Serial.println(pingInterval);
    Serial.print(F("ArdSTATUS:PING:THR:"));
    Serial.println(pingThreshold);
    Serial.print(F("ArdSTATUS:PING:RELAY:"));
    Serial.println(pingRelay);
    Serial.print(F("ArdSTATUS:PING:MODE:"));
    Serial.println(pingMode == PING_MODE_AUTO ? "AUTO" : (pingMode == PING_MODE_LATCH ? "LATCH" : "PULSE"));
    Serial.print(F("ArdSTATUS:PING:DUR:"));
    Serial.println(pingDuration);
    Serial.print(F("ArdSTATUS:PING:CNT:"));
    Serial.println(pingCount);
    for (byte i = 0; i < pingCount; i++) {
        if (pingActive[i]) {
            Serial.print(F("ArdSTATUS:PING:T"));
            Serial.print(i);
            Serial.print(F(":"));
            Serial.print(pingTargets[i]);
            Serial.print(F(":"));
            Serial.print(pingFails[i] >= pingThreshold ? "FAIL" : "OK");
            Serial.print(F(":"));
            Serial.println(pingFails[i]);
        }
    }
    Serial.print(F("ArdSTATUS:PING:RSTATE:"));
    Serial.println(pingRelayIsOn ? "ON" : "OFF");
}

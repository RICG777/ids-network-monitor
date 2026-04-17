// IDS Network Monitor - Syslog Relay Trigger + Ping Heartbeat (ESP32-EVB)
// Version: 2e.2
// Changes from v2e.1:
//   - E1: ICMP ping heartbeat — up to 20 targets, configurable threshold/relay/mode
//   - Relay coexistence: syslog and ping can independently trigger same relay
//   - New PING: serial commands

#include <ETH.h>
#include <NetworkUdp.h>
#include <Preferences.h>
#include <WiFi.h>
#include <esp_bt.h>
#include "ping/ping_sock.h"
#include "lwip/inet.h"

#define FIRMWARE_VERSION "2e.2"
#define TEST_PULSE_MS    3000
#define MODE_PULSE 0
#define MODE_LATCH 1
#define VERB_QUIET  0
#define VERB_NORMAL 1
#define VERB_DEBUG  2
#define DEFAULT_DURATION 10
#define MAX_TRIG_STR     120
#define BUFFER_SIZE      256

// Ping constants
#define MAX_PING_TARGETS  20
#define PING_MODE_AUTO    0
#define PING_MODE_LATCH   1
#define PING_MODE_PULSE   2
#define DEFAULT_PING_INTERVAL  30
#define DEFAULT_PING_THRESHOLD 3

// Relay pins (Olimex ESP32-EVB on-board relays)
const int relayPin1 = 32;
const int relayPin2 = 33;

// Network
NetworkUDP Udp;
unsigned int localPort = 514;
bool ethConnected = false;
IPAddress ip;
IPAddress subnet(255, 255, 255, 0);
IPAddress gateway(0, 0, 0, 0);

// Storage
Preferences prefs;

// Packet buffer
char packetBuffer[BUFFER_SIZE];

// Triggers
char r1Trigs[MAX_TRIG_STR] = "port up";
char r2Trigs[MAX_TRIG_STR] = "port down";

// Syslog relay state
bool relay1IsOn = false;
bool relay2IsOn = false;
unsigned long relay1OnTime = 0;
unsigned long relay2OnTime = 0;
byte relay1Mode = MODE_PULSE;
byte relay2Mode = MODE_PULSE;
byte relay1Duration = DEFAULT_DURATION;
byte relay2Duration = DEFAULT_DURATION;
byte verbosity = VERB_NORMAL;

// Ping targets
IPAddress pingTargets[MAX_PING_TARGETS];
bool      pingActive[MAX_PING_TARGETS];
byte      pingFails[MAX_PING_TARGETS];
bool      pingOK[MAX_PING_TARGETS];
byte      pingCount = 0;

// Ping global settings
bool   pingEnabled   = false;
byte   pingInterval  = DEFAULT_PING_INTERVAL;
byte   pingThreshold = DEFAULT_PING_THRESHOLD;
byte   pingRelay     = 1;
byte   pingMode      = PING_MODE_AUTO;
byte   pingDuration  = DEFAULT_DURATION;

// Ping engine state
byte   pingCurrentIdx  = 0;
bool   pingInProgress  = false;
unsigned long pingLastCycleMs = 0;
bool   pingRelayIsOn   = false;
unsigned long pingRelayOnTime = 0;
esp_ping_handle_t pingSession = NULL;

// ===================== RELAY OUTPUT =====================

void updateRelayOutput(int relay) {
    bool syslogWants = (relay == 1) ? relay1IsOn : relay2IsOn;
    bool pingWants   = pingRelayIsOn && (pingRelay == relay);
    int pin = (relay == 1) ? relayPin1 : relayPin2;
    digitalWrite(pin, (syslogWants || pingWants) ? HIGH : LOW);
}

// ===================== ETHERNET EVENT =====================

void onEthEvent(arduino_event_id_t event) {
    switch (event) {
        case ARDUINO_EVENT_ETH_START:
            if (verbosity >= VERB_DEBUG)
                Serial.println(F("ArdMsg: Ethernet started."));
            break;
        case ARDUINO_EVENT_ETH_CONNECTED:
            if (verbosity >= VERB_NORMAL)
                Serial.println(F("ArdMsg: Ethernet cable connected."));
            break;
        case ARDUINO_EVENT_ETH_GOT_IP:
            ethConnected = true;
            Udp.begin(localPort);
            if (verbosity >= VERB_NORMAL) {
                Serial.print(F("ArdMsg: Board IP: "));
                Serial.println(ETH.localIP());
            }
            break;
        case ARDUINO_EVENT_ETH_DISCONNECTED:
            ethConnected = false;
            if (verbosity >= VERB_NORMAL)
                Serial.println(F("ArdMsg: Ethernet cable disconnected."));
            break;
        default:
            break;
    }
}

// ===================== SETUP =====================

void setup() {
    Serial.begin(115200);
    delay(2000);
    Serial.println(F("ArdMsg: IDS Network Monitor v2e.2 starting..."));

    // Disable Bluetooth (E5 - security hardening)
    btStop();
    esp_bt_controller_disable();
    Serial.println(F("ArdMsg: BLE disabled."));

    // Load settings from NVS
    prefs.begin("ids", false);
    loadSettings();

    // Relay pins
    pinMode(relayPin1, OUTPUT);
    digitalWrite(relayPin1, LOW);
    pinMode(relayPin2, OUTPUT);
    digitalWrite(relayPin2, LOW);

    // Ethernet
    Network.onEvent(onEthEvent);
    ETH.begin();

    // Static IP config
    if (gateway[0] == 0 && gateway[1] == 0 && gateway[2] == 0 && gateway[3] == 0)
        ETH.config(ip, IPAddress(0, 0, 0, 0), subnet);
    else
        ETH.config(ip, gateway, subnet);

    // Disable WiFi after Ethernet is started (shares internal resources)
    WiFi.mode(WIFI_OFF);
    Serial.println(F("ArdMsg: WiFi disabled."));

    if (verbosity >= VERB_NORMAL) {
        Serial.print(F("ArdMsg: R1 triggers: "));
        Serial.println(r1Trigs);
        Serial.print(F("ArdMsg: R2 triggers: "));
        Serial.println(r2Trigs);
        if (pingEnabled && pingCount > 0) {
            Serial.print(F("ArdMsg: Ping monitoring enabled, "));
            Serial.print(pingCount);
            Serial.println(F(" targets."));
        }
    }

    Serial.println(F("ArdMsg: Setup complete. Listening for syslog messages..."));
}

// ===================== NVS HELPERS =====================

void loadSettings() {
    String ipStr = prefs.getString("ip", "192.168.68.142");
    ip = parseIP(ipStr.c_str());
    if (!isValidIP(ip)) ip = IPAddress(192, 168, 68, 142);

    String subStr = prefs.getString("subnet", "255.255.255.0");
    subnet = parseIP(subStr.c_str());

    String gwStr = prefs.getString("gateway", "0.0.0.0");
    gateway = parseIP(gwStr.c_str());

    relay1Mode = clamp(prefs.getUChar("r1mode", MODE_PULSE), 0, 1, MODE_PULSE);
    relay2Mode = clamp(prefs.getUChar("r2mode", MODE_PULSE), 0, 1, MODE_PULSE);
    relay1Duration = prefs.getUChar("r1dur", DEFAULT_DURATION);
    relay2Duration = prefs.getUChar("r2dur", DEFAULT_DURATION);
    if (relay1Duration == 0) relay1Duration = DEFAULT_DURATION;
    if (relay2Duration == 0) relay2Duration = DEFAULT_DURATION;
    verbosity = clamp(prefs.getUChar("verb", VERB_NORMAL), 0, 2, VERB_NORMAL);

    String r1 = prefs.getString("r1trigs", "port up");
    strncpy(r1Trigs, r1.c_str(), MAX_TRIG_STR - 1);
    r1Trigs[MAX_TRIG_STR - 1] = '\0';

    String r2 = prefs.getString("r2trigs", "port down");
    strncpy(r2Trigs, r2.c_str(), MAX_TRIG_STR - 1);
    r2Trigs[MAX_TRIG_STR - 1] = '\0';

    // Ping settings
    pingEnabled   = prefs.getUChar("pEn", 0) == 1;
    pingInterval  = prefs.getUChar("pInt", DEFAULT_PING_INTERVAL);
    if (pingInterval < 5) pingInterval = DEFAULT_PING_INTERVAL;
    pingThreshold = prefs.getUChar("pThr", DEFAULT_PING_THRESHOLD);
    if (pingThreshold < 1) pingThreshold = DEFAULT_PING_THRESHOLD;
    pingRelay     = clamp(prefs.getUChar("pRelay", 1), 1, 2, 1);
    pingMode      = clamp(prefs.getUChar("pMode", PING_MODE_AUTO), 0, 2, PING_MODE_AUTO);
    pingDuration  = prefs.getUChar("pDur", DEFAULT_DURATION);
    if (pingDuration == 0) pingDuration = DEFAULT_DURATION;

    pingCount = prefs.getUChar("pCnt", 0);
    if (pingCount > MAX_PING_TARGETS) pingCount = 0;
    for (byte i = 0; i < MAX_PING_TARGETS; i++) {
        pingActive[i] = false;
        pingFails[i] = 0;
        pingOK[i] = true;
    }
    for (byte i = 0; i < pingCount; i++) {
        char key[5];
        snprintf(key, sizeof(key), "pT%d", i);
        String s = prefs.getString(key, "");
        if (s.length() > 0) {
            pingTargets[i] = parseIP(s.c_str());
            pingActive[i] = isValidIP(pingTargets[i]);
        }
    }
}

void savePingTargets() {
    prefs.putUChar("pCnt", pingCount);
    for (byte i = 0; i < MAX_PING_TARGETS; i++) {
        char key[5];
        snprintf(key, sizeof(key), "pT%d", i);
        if (i < pingCount && pingActive[i]) {
            char buf[16];
            snprintf(buf, sizeof(buf), "%d.%d.%d.%d",
                     pingTargets[i][0], pingTargets[i][1],
                     pingTargets[i][2], pingTargets[i][3]);
            prefs.putString(key, buf);
        } else {
            prefs.remove(key);
        }
    }
}

// ===================== UTILITY =====================

bool isValidIP(IPAddress a) {
    return !(a[0]==0 && a[1]==0 && a[2]==0 && a[3]==0) && !(a[0]==255 && a[1]==255 && a[2]==255 && a[3]==255);
}

byte clamp(byte val, byte lo, byte hi, byte def) {
    return (val >= lo && val <= hi) ? val : def;
}

IPAddress parseIP(const char* str) {
    int p[4];
    sscanf(str, "%d.%d.%d.%d", &p[0], &p[1], &p[2], &p[3]);
    return IPAddress(p[0], p[1], p[2], p[3]);
}

char* strcasestr_local(const char* haystack, const char* needle) {
    if (!*needle) return NULL;
    for (; *haystack; haystack++) {
        const char* h = haystack;
        const char* n = needle;
        while (*h && *n && (tolower((unsigned char)*h) == tolower((unsigned char)*n))) {
            h++; n++;
        }
        if (!*n) return (char*)haystack;
    }
    return NULL;
}

void printMAC() {
    Serial.println(ETH.macAddress());
}

bool matchTriggers(const char* trigStr) {
    char buf[MAX_TRIG_STR];
    strncpy(buf, trigStr, MAX_TRIG_STR - 1);
    buf[MAX_TRIG_STR - 1] = '\0';

    char* token = strtok(buf, ";");
    while (token != NULL) {
        while (*token == ' ') token++;
        int len = strlen(token);
        while (len > 0 && token[len-1] == ' ') token[--len] = '\0';

        if (len > 0 && strcasestr_local(packetBuffer, token)) {
            return true;
        }
        token = strtok(NULL, ";");
    }
    return false;
}

// ===================== PING ENGINE =====================

// Forward declarations
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
        if (verbosity >= VERB_QUIET) {
            Serial.println(F("ArdMsg: Ping FAIL - relay ENERGIZED!"));
        }
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
        if (verbosity >= VERB_NORMAL) {
            Serial.println(F("ArdMsg: All ping targets OK - relay cleared."));
        }
    }

    cleanupPingSession();
}

void pingOnTimeout(esp_ping_handle_t hdl, void *args) {
    byte idx = pingCurrentIdx;
    if (pingFails[idx] < 255) pingFails[idx]++;
    pingOK[idx] = false;

    if (verbosity >= VERB_NORMAL) {
        Serial.print(F("ArdMsg: Ping timeout: "));
        Serial.print(pingTargets[idx]);
        Serial.print(F(" (fails: "));
        Serial.print(pingFails[idx]);
        Serial.println(F(")"));
    }

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

// ===================== STATUS =====================

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

void sendStatus() {
    Serial.print(F("ArdSTATUS:FW:"));
    Serial.println(F(FIRMWARE_VERSION));
    Serial.print(F("ArdSTATUS:MAC:"));
    printMAC();
    Serial.print(F("ArdSTATUS:IP:"));
    Serial.println(ETH.localIP());
    Serial.print(F("ArdSTATUS:SUBNET:"));
    Serial.println(subnet);
    Serial.print(F("ArdSTATUS:GATEWAY:"));
    Serial.println(gateway);
    Serial.print(F("ArdSTATUS:Relay1:"));
    Serial.println(r1Trigs);
    Serial.print(F("ArdSTATUS:Relay2:"));
    Serial.println(r2Trigs);
    Serial.print(F("ArdSTATUS:R1Mode:"));
    Serial.println(relay1Mode == MODE_LATCH ? "LATCH" : "PULSE");
    Serial.print(F("ArdSTATUS:R2Mode:"));
    Serial.println(relay2Mode == MODE_LATCH ? "LATCH" : "PULSE");
    Serial.print(F("ArdSTATUS:R1Duration:"));
    Serial.println(relay1Duration);
    Serial.print(F("ArdSTATUS:R2Duration:"));
    Serial.println(relay2Duration);
    Serial.print(F("ArdSTATUS:R1State:"));
    Serial.println(relay1IsOn ? "ON" : "OFF");
    Serial.print(F("ArdSTATUS:R2State:"));
    Serial.println(relay2IsOn ? "ON" : "OFF");
    Serial.print(F("ArdSTATUS:VERBOSITY:"));
    Serial.println(verbosity);
    sendPingStatus();
}

// ===================== MAIN LOOP =====================

void loop() {
    if (Serial.available()) {
        char message[128];
        int bytesRead = Serial.readBytesUntil('\n', message, sizeof(message) - 1);
        message[bytesRead] = '\0';
        if (bytesRead > 0 && message[bytesRead - 1] == '\r')
            message[bytesRead - 1] = '\0';
        while (Serial.available()) Serial.read();

        if (strncmp(message, "IP:", 3) == 0) {
            ip = parseIP(message + 3);
            prefs.putString("ip", message + 3);
            if (gateway[0]==0 && gateway[1]==0 && gateway[2]==0 && gateway[3]==0)
                ETH.config(ip, IPAddress(0, 0, 0, 0), subnet);
            else
                ETH.config(ip, gateway, subnet);
            Udp.stop();
            Udp.begin(localPort);
            Serial.print(F("ArdACK:IP:"));
            Serial.println(ip);

        } else if (strncmp(message, "SUBNET:", 7) == 0) {
            subnet = parseIP(message + 7);
            prefs.putString("subnet", message + 7);
            Serial.print(F("ArdACK:SUBNET:"));
            Serial.println(subnet);

        } else if (strncmp(message, "GATEWAY:", 8) == 0) {
            gateway = parseIP(message + 8);
            prefs.putString("gateway", message + 8);
            Serial.print(F("ArdACK:GATEWAY:"));
            Serial.println(gateway);

        } else if (strncmp(message, "Relay1:", 7) == 0) {
            const char* trig = message + 7;
            int len = strlen(trig);
            if (len == 0 || len >= MAX_TRIG_STR) {
                Serial.println(F("ArdERR:Relay1:must be 1-119 chars"));
            } else {
                strncpy(r1Trigs, trig, MAX_TRIG_STR - 1);
                r1Trigs[MAX_TRIG_STR - 1] = '\0';
                prefs.putString("r1trigs", r1Trigs);
                Serial.print(F("ArdACK:Relay1:"));
                Serial.println(r1Trigs);
            }

        } else if (strncmp(message, "Relay2:", 7) == 0) {
            const char* trig = message + 7;
            int len = strlen(trig);
            if (len == 0 || len >= MAX_TRIG_STR) {
                Serial.println(F("ArdERR:Relay2:must be 1-119 chars"));
            } else {
                strncpy(r2Trigs, trig, MAX_TRIG_STR - 1);
                r2Trigs[MAX_TRIG_STR - 1] = '\0';
                prefs.putString("r2trigs", r2Trigs);
                Serial.print(F("ArdACK:Relay2:"));
                Serial.println(r2Trigs);
            }

        } else if (strncmp(message, "MODE1:", 6) == 0) {
            relay1Mode = (strncmp(message + 6, "LATCH", 5) == 0) ? MODE_LATCH : MODE_PULSE;
            prefs.putUChar("r1mode", relay1Mode);
            Serial.print(F("ArdACK:MODE1:"));
            Serial.println(relay1Mode == MODE_LATCH ? "LATCH" : "PULSE");

        } else if (strncmp(message, "MODE2:", 6) == 0) {
            relay2Mode = (strncmp(message + 6, "LATCH", 5) == 0) ? MODE_LATCH : MODE_PULSE;
            prefs.putUChar("r2mode", relay2Mode);
            Serial.print(F("ArdACK:MODE2:"));
            Serial.println(relay2Mode == MODE_LATCH ? "LATCH" : "PULSE");

        } else if (strncmp(message, "DURATION1:", 10) == 0) {
            int dur = atoi(message + 10);
            if (dur < 1 || dur > 255) { Serial.println(F("ArdERR:DURATION1:must be 1-255")); }
            else { relay1Duration = dur; prefs.putUChar("r1dur", relay1Duration);
                Serial.print(F("ArdACK:DURATION1:")); Serial.println(relay1Duration); }

        } else if (strncmp(message, "DURATION2:", 10) == 0) {
            int dur = atoi(message + 10);
            if (dur < 1 || dur > 255) { Serial.println(F("ArdERR:DURATION2:must be 1-255")); }
            else { relay2Duration = dur; prefs.putUChar("r2dur", relay2Duration);
                Serial.print(F("ArdACK:DURATION2:")); Serial.println(relay2Duration); }

        } else if (strncmp(message, "VERBOSITY:", 10) == 0) {
            int v = atoi(message + 10);
            if (v < 0 || v > 2) { Serial.println(F("ArdERR:VERBOSITY:must be 0-2")); }
            else { verbosity = v; prefs.putUChar("verb", verbosity);
                Serial.print(F("ArdACK:VERBOSITY:")); Serial.println(verbosity); }

        } else if (strncmp(message, "RESET1", 6) == 0) {
            relay1IsOn = false;
            updateRelayOutput(1);
            Serial.println(F("ArdACK:RESET1:OK"));

        } else if (strncmp(message, "RESET2", 6) == 0) {
            relay2IsOn = false;
            updateRelayOutput(2);
            Serial.println(F("ArdACK:RESET2:OK"));

        } else if (strncmp(message, "STATUS", 6) == 0) {
            sendStatus();

        } else if (strncmp(message, "PING:", 5) == 0) {
            handlePingCommand(message + 5);

        } else if (strncmp(message, "TTEST1", 6) == 0) {
            Serial.print(F("ArdACK:TTEST1:pulsing ")); Serial.print(relay1Duration); Serial.println(F("s"));
            digitalWrite(relayPin1, HIGH); delay((unsigned long)relay1Duration * 1000UL); digitalWrite(relayPin1, LOW);
            Serial.println(F("ArdMsg: Relay 1 timed test complete."));

        } else if (strncmp(message, "TTEST2", 6) == 0) {
            Serial.print(F("ArdACK:TTEST2:pulsing ")); Serial.print(relay2Duration); Serial.println(F("s"));
            digitalWrite(relayPin2, HIGH); delay((unsigned long)relay2Duration * 1000UL); digitalWrite(relayPin2, LOW);
            Serial.println(F("ArdMsg: Relay 2 timed test complete."));

        } else if (strncmp(message, "TEST1", 5) == 0) {
            Serial.println(F("ArdACK:TEST1:pulsing 3s"));
            digitalWrite(relayPin1, HIGH); delay(TEST_PULSE_MS); digitalWrite(relayPin1, LOW);
            Serial.println(F("ArdMsg: Relay 1 test complete."));

        } else if (strncmp(message, "TEST2", 5) == 0) {
            Serial.println(F("ArdACK:TEST2:pulsing 3s"));
            digitalWrite(relayPin2, HIGH); delay(TEST_PULSE_MS); digitalWrite(relayPin2, LOW);
            Serial.println(F("ArdMsg: Relay 2 test complete."));
        }
    }

    // Listen for syslog
    int packetSize = Udp.parsePacket();
    if (packetSize) {
        memset(packetBuffer, 0, BUFFER_SIZE);
        Udp.read(packetBuffer, min(packetSize, BUFFER_SIZE - 1));
        if (verbosity >= VERB_DEBUG) {
            Serial.print(F("ArdMsg: Syslog (")); Serial.print(packetSize);
            Serial.print(F("b): ")); Serial.println(packetBuffer);
        }
    }

    // Syslog trigger matching
    if (!relay1IsOn && matchTriggers(r1Trigs)) {
        relay1IsOn = true;
        relay1OnTime = millis();
        updateRelayOutput(1);
        if (verbosity >= VERB_QUIET) {
            Serial.println(F("ArdMsg: Relay 1 ENERGIZED - trigger matched!"));
        }
        memset(packetBuffer, 0, BUFFER_SIZE);
    }

    if (!relay2IsOn && matchTriggers(r2Trigs)) {
        relay2IsOn = true;
        relay2OnTime = millis();
        updateRelayOutput(2);
        if (verbosity >= VERB_QUIET) {
            Serial.println(F("ArdMsg: Relay 2 ENERGIZED - trigger matched!"));
        }
        memset(packetBuffer, 0, BUFFER_SIZE);
    }

    // Syslog relay timeout — pulse mode only
    if (relay1IsOn && relay1Mode == MODE_PULSE && (millis() - relay1OnTime >= (unsigned long)relay1Duration * 1000UL)) {
        relay1IsOn = false;
        updateRelayOutput(1);
        if (verbosity >= VERB_NORMAL) {
            Serial.print(F("ArdMsg: Relay 1 de-energized (")); Serial.print(relay1Duration); Serial.println(F("s timeout)."));
        }
    }
    if (relay2IsOn && relay2Mode == MODE_PULSE && (millis() - relay2OnTime >= (unsigned long)relay2Duration * 1000UL)) {
        relay2IsOn = false;
        updateRelayOutput(2);
        if (verbosity >= VERB_NORMAL) {
            Serial.print(F("ArdMsg: Relay 2 de-energized (")); Serial.print(relay2Duration); Serial.println(F("s timeout)."));
        }
    }

    // Ping engine
    if (pingEnabled && ethConnected && pingCount > 0) {
        if (!pingInProgress && (millis() - pingLastCycleMs >= (unsigned long)pingInterval * 1000UL)) {
            pingLastCycleMs = millis();
            startNextPing();
        }
    }

    // Ping relay pulse timeout
    if (pingRelayIsOn && pingMode == PING_MODE_PULSE &&
        (millis() - pingRelayOnTime >= (unsigned long)pingDuration * 1000UL)) {
        pingRelayIsOn = false;
        updateRelayOutput(pingRelay);
        if (verbosity >= VERB_NORMAL) {
            Serial.println(F("ArdMsg: Ping relay de-energized (pulse timeout)."));
        }
    }
}

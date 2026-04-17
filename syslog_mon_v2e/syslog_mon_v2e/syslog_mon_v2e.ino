// IDS Network Monitor - Syslog Relay Trigger + Ping Heartbeat (ESP32-EVB)
// Version: 2e.3
// Changes from v2e.2:
//   - Multi-file refactor: serial command dispatcher extracted to serial_cmds.ino,
//     ping engine + commands extracted to ping_engine.ino. No functional change.

#include <ETH.h>
#include <NetworkUdp.h>
#include <Preferences.h>
#include <WiFi.h>
#include <esp_bt.h>
#include "ping/ping_sock.h"
#include "lwip/inet.h"

#define FIRMWARE_VERSION "2e.3"
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
    Serial.println(F("ArdMsg: IDS Network Monitor v2e.3 starting..."));

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

// ===================== STATUS =====================

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

        handleSerialCommand(message);
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

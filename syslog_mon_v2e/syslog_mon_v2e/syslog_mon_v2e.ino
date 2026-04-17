// IDS Network Monitor - Syslog Relay Trigger (ESP32-EVB)
// Version: 2e.1
// Port of v1d.4 (XBoard/ATmega32U4) to Olimex ESP32-EVB-IND
// Changes from v1d.4:
//   - ETH.h (RMII/LAN8720) replaces Ethernet.h (W5100)
//   - Preferences (NVS) replaces EEPROM
//   - Relay pins: GPIO32, GPIO33 (on-board relays)
//   - Serial baud: 115200 (was 9600)
//   - WiFi/BLE disabled on boot
//   - MAC read from hardware (not hardcoded)

#include <ETH.h>
#include <NetworkUdp.h>
#include <Preferences.h>
#include <WiFi.h>
#include <esp_bt.h>

#define FIRMWARE_VERSION "2e.1"
#define TEST_PULSE_MS    3000
#define MODE_PULSE 0
#define MODE_LATCH 1
#define VERB_QUIET  0
#define VERB_NORMAL 1
#define VERB_DEBUG  2
#define DEFAULT_DURATION 10
#define MAX_TRIG_STR     120
#define BUFFER_SIZE      256

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

// Relay state
bool relay1IsOn = false;
bool relay2IsOn = false;
unsigned long relay1OnTime = 0;
unsigned long relay2OnTime = 0;
byte relay1Mode = MODE_PULSE;
byte relay2Mode = MODE_PULSE;
byte relay1Duration = DEFAULT_DURATION;
byte relay2Duration = DEFAULT_DURATION;
byte verbosity = VERB_NORMAL;

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
    Serial.println(F("ArdMsg: IDS Network Monitor v2e.1 starting..."));

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

// Check if packetBuffer matches any semicolon-separated trigger in trigStr
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
            digitalWrite(relayPin1, LOW); relay1IsOn = false;
            Serial.println(F("ArdACK:RESET1:OK"));

        } else if (strncmp(message, "RESET2", 6) == 0) {
            digitalWrite(relayPin2, LOW); relay2IsOn = false;
            Serial.println(F("ArdACK:RESET2:OK"));

        } else if (strncmp(message, "STATUS", 6) == 0) {
            sendStatus();

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

    // Trigger matching — semicolon-separated OR logic
    if (!relay1IsOn && matchTriggers(r1Trigs)) {
        digitalWrite(relayPin1, HIGH);
        relay1IsOn = true;
        relay1OnTime = millis();
        if (verbosity >= VERB_QUIET) {
            Serial.println(F("ArdMsg: Relay 1 ENERGIZED - trigger matched!"));
        }
        memset(packetBuffer, 0, BUFFER_SIZE);
    }

    if (!relay2IsOn && matchTriggers(r2Trigs)) {
        digitalWrite(relayPin2, HIGH);
        relay2IsOn = true;
        relay2OnTime = millis();
        if (verbosity >= VERB_QUIET) {
            Serial.println(F("ArdMsg: Relay 2 ENERGIZED - trigger matched!"));
        }
        memset(packetBuffer, 0, BUFFER_SIZE);
    }

    // Relay timeout — pulse mode only
    if (relay1IsOn && relay1Mode == MODE_PULSE && (millis() - relay1OnTime >= (unsigned long)relay1Duration * 1000UL)) {
        digitalWrite(relayPin1, LOW); relay1IsOn = false;
        if (verbosity >= VERB_NORMAL) {
            Serial.print(F("ArdMsg: Relay 1 de-energized (")); Serial.print(relay1Duration); Serial.println(F("s timeout)."));
        }
    }
    if (relay2IsOn && relay2Mode == MODE_PULSE && (millis() - relay2OnTime >= (unsigned long)relay2Duration * 1000UL)) {
        digitalWrite(relayPin2, LOW); relay2IsOn = false;
        if (verbosity >= VERB_NORMAL) {
            Serial.print(F("ArdMsg: Relay 2 de-energized (")); Serial.print(relay2Duration); Serial.println(F("s timeout)."));
        }
    }
}

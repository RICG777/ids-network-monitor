// IDS Network Monitor - Syslog Relay Trigger
// Version: 1d.4
// Changes from v1d.3:
//   - Triggers now semicolon-separated in a single string per relay (e.g. "FAN_FAILED;SUPPLY_FAILED")
//   - No limit on number of trigger words (limited only by 120-byte EEPROM allocation)
//   - Simplified EEPROM layout: one trigger field per relay instead of three

#include <Ethernet.h>
#include <EthernetUdp.h>
#include <EEPROM.h>

// EEPROM LAYOUT
#define ADDR_IP          0    // 4 bytes
#define ADDR_R1_MODE     4    // 1 byte
#define ADDR_R2_MODE     5
#define ADDR_R1_DURATION 6    // 1 byte
#define ADDR_R2_DURATION 7
#define ADDR_VERBOSITY   8    // 1 byte
#define ADDR_SUBNET      9    // 4 bytes
#define ADDR_GATEWAY     13   // 4 bytes
// 17-19: reserved
#define ADDR_R1_TRIGS    20   // 120 bytes
#define ADDR_R2_TRIGS    140  // 120 bytes
#define ADDR_BOOT_MARKER 260
#define BOOT_MARKER_VAL  126

#define MAX_TRIG_STR     120
#define FIRMWARE_VERSION "1d.4"
#define TEST_PULSE_MS    3000
#define MODE_PULSE 0
#define MODE_LATCH 1
#define VERB_QUIET  0
#define VERB_NORMAL 1
#define VERB_DEBUG  2
#define DEFAULT_DURATION 10

// VARIABLES
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress ip;
IPAddress subnet(255, 255, 255, 0);
IPAddress gateway(0, 0, 0, 0);
EthernetUDP Udp;
unsigned int localPort = 514;
const int relayPin1 = 7;
const int relayPin2 = 8;

#define BUFFER_SIZE 256
char packetBuffer[BUFFER_SIZE];

char r1Trigs[MAX_TRIG_STR] = "port up";
char r2Trigs[MAX_TRIG_STR] = "port down";

bool relay1IsOn = false;
bool relay2IsOn = false;
unsigned long relay1OnTime = 0;
unsigned long relay2OnTime = 0;
byte relay1Mode = MODE_PULSE;
byte relay2Mode = MODE_PULSE;
byte relay1Duration = DEFAULT_DURATION;
byte relay2Duration = DEFAULT_DURATION;
byte verbosity = VERB_NORMAL;

// ===================== SETUP =====================

void setup() {
    Serial.begin(9600);
    delay(5000);
    Serial.println(F("ArdMsg: IDS Network Monitor v1d.4 starting..."));

    if (EEPROM.read(ADDR_BOOT_MARKER) != BOOT_MARKER_VAL) {
        Serial.println(F("ArdMsg: First boot - writing defaults."));
        writeIPField(ADDR_IP, IPAddress(192, 168, 68, 142));
        writeIPField(ADDR_SUBNET, IPAddress(255, 255, 255, 0));
        writeIPField(ADDR_GATEWAY, IPAddress(0, 0, 0, 0));
        EEPROM.write(ADDR_R1_MODE, MODE_PULSE);
        EEPROM.write(ADDR_R2_MODE, MODE_PULSE);
        EEPROM.write(ADDR_R1_DURATION, DEFAULT_DURATION);
        EEPROM.write(ADDR_R2_DURATION, DEFAULT_DURATION);
        EEPROM.write(ADDR_VERBOSITY, VERB_NORMAL);
        writeString(ADDR_R1_TRIGS, "port up", MAX_TRIG_STR);
        writeString(ADDR_R2_TRIGS, "port down", MAX_TRIG_STR);
        EEPROM.write(ADDR_BOOT_MARKER, BOOT_MARKER_VAL);
    }

    ip = readIPField(ADDR_IP);
    if (!isValidIP(ip)) ip = IPAddress(192, 168, 68, 142);
    subnet = readIPField(ADDR_SUBNET);
    gateway = readIPField(ADDR_GATEWAY);

    if (gateway[0] == 0 && gateway[1] == 0 && gateway[2] == 0 && gateway[3] == 0)
        Ethernet.begin(mac, ip);
    else
        Ethernet.begin(mac, ip, gateway, gateway, subnet);

    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
        Serial.println(F("ArdMsg: Ethernet hardware not found. Halting."));
        while (true);
    }
    if (Ethernet.linkStatus() == LinkOFF)
        Serial.println(F("ArdMsg: Ethernet cable is not connected."));

    Udp.begin(localPort);

    relay1Mode = clamp(EEPROM.read(ADDR_R1_MODE), 0, 1, MODE_PULSE);
    relay2Mode = clamp(EEPROM.read(ADDR_R2_MODE), 0, 1, MODE_PULSE);
    relay1Duration = EEPROM.read(ADDR_R1_DURATION);
    relay2Duration = EEPROM.read(ADDR_R2_DURATION);
    if (relay1Duration == 0) relay1Duration = DEFAULT_DURATION;
    if (relay2Duration == 0) relay2Duration = DEFAULT_DURATION;
    verbosity = clamp(EEPROM.read(ADDR_VERBOSITY), 0, 2, VERB_NORMAL);

    readString(ADDR_R1_TRIGS, r1Trigs, MAX_TRIG_STR);
    readString(ADDR_R2_TRIGS, r2Trigs, MAX_TRIG_STR);

    if (verbosity >= VERB_NORMAL) {
        Serial.print(F("ArdMsg: Board IP: "));
        Serial.println(Ethernet.localIP());
        Serial.print(F("ArdMsg: R1 triggers: "));
        Serial.println(r1Trigs);
        Serial.print(F("ArdMsg: R2 triggers: "));
        Serial.println(r2Trigs);
    }

    pinMode(relayPin1, OUTPUT);
    digitalWrite(relayPin1, LOW);
    pinMode(relayPin2, OUTPUT);
    digitalWrite(relayPin2, LOW);

    Serial.println(F("ArdMsg: Setup complete. Listening for syslog messages..."));
}

// ===================== EEPROM HELPERS =====================

void writeIPField(int addr, IPAddress a) {
    for (int i = 0; i < 4; i++) EEPROM.write(addr + i, a[i]);
}

IPAddress readIPField(int addr) {
    return IPAddress(EEPROM.read(addr), EEPROM.read(addr+1), EEPROM.read(addr+2), EEPROM.read(addr+3));
}

bool isValidIP(IPAddress a) {
    return !(a[0]==0 && a[1]==0 && a[2]==0 && a[3]==0) && !(a[0]==255 && a[1]==255 && a[2]==255 && a[3]==255);
}

void readString(int addr, char* dest, int maxLen) {
    int i = 0;
    char ch;
    while ((ch = EEPROM.read(addr + i)) != '\0' && i < maxLen - 1) {
        dest[i++] = ch;
    }
    dest[i] = '\0';
}

void writeString(int addr, const char* str, int maxLen) {
    int len = strlen(str);
    if (len >= maxLen) len = maxLen - 1;
    for (int i = 0; i < len; i++) EEPROM.write(addr + i, str[i]);
    EEPROM.write(addr + len, '\0');
}

byte clamp(byte val, byte lo, byte hi, byte def) {
    return (val >= lo && val <= hi) ? val : def;
}

IPAddress parseIP(const char* str) {
    int p[4];
    sscanf(str, "%d.%d.%d.%d", &p[0], &p[1], &p[2], &p[3]);
    return IPAddress(p[0], p[1], p[2], p[3]);
}

// ===================== UTILITY =====================

char* strcasestr_local(const char* haystack, const char* needle) {
    if (!*needle) return NULL;  // Empty needle should NOT match
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
    for (int i = 0; i < 6; i++) {
        if (mac[i] < 0x10) Serial.print('0');
        Serial.print(mac[i], HEX);
        if (i < 5) Serial.print(':');
    }
    Serial.println();
}

// Check if packetBuffer matches any semicolon-separated trigger in trigStr
// Returns true if any trigger matches
bool matchTriggers(const char* trigStr) {
    char buf[MAX_TRIG_STR];
    strncpy(buf, trigStr, MAX_TRIG_STR - 1);
    buf[MAX_TRIG_STR - 1] = '\0';

    char* token = strtok(buf, ";");
    while (token != NULL) {
        // Trim leading spaces
        while (*token == ' ') token++;
        // Trim trailing spaces
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
    Serial.println(Ethernet.localIP());
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
            writeIPField(ADDR_IP, ip);
            if (gateway[0]==0 && gateway[1]==0 && gateway[2]==0 && gateway[3]==0)
                Ethernet.begin(mac, ip);
            else
                Ethernet.begin(mac, ip, gateway, gateway, subnet);
            Udp.begin(localPort);
            Serial.print(F("ArdACK:IP:"));
            Serial.println(ip);

        } else if (strncmp(message, "SUBNET:", 7) == 0) {
            subnet = parseIP(message + 7);
            writeIPField(ADDR_SUBNET, subnet);
            Serial.print(F("ArdACK:SUBNET:"));
            Serial.println(subnet);

        } else if (strncmp(message, "GATEWAY:", 8) == 0) {
            gateway = parseIP(message + 8);
            writeIPField(ADDR_GATEWAY, gateway);
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
                writeString(ADDR_R1_TRIGS, r1Trigs, MAX_TRIG_STR);
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
                writeString(ADDR_R2_TRIGS, r2Trigs, MAX_TRIG_STR);
                Serial.print(F("ArdACK:Relay2:"));
                Serial.println(r2Trigs);
            }

        } else if (strncmp(message, "MODE1:", 6) == 0) {
            relay1Mode = (strncmp(message + 6, "LATCH", 5) == 0) ? MODE_LATCH : MODE_PULSE;
            EEPROM.write(ADDR_R1_MODE, relay1Mode);
            Serial.print(F("ArdACK:MODE1:"));
            Serial.println(relay1Mode == MODE_LATCH ? "LATCH" : "PULSE");

        } else if (strncmp(message, "MODE2:", 6) == 0) {
            relay2Mode = (strncmp(message + 6, "LATCH", 5) == 0) ? MODE_LATCH : MODE_PULSE;
            EEPROM.write(ADDR_R2_MODE, relay2Mode);
            Serial.print(F("ArdACK:MODE2:"));
            Serial.println(relay2Mode == MODE_LATCH ? "LATCH" : "PULSE");

        } else if (strncmp(message, "DURATION1:", 10) == 0) {
            int dur = atoi(message + 10);
            if (dur < 1 || dur > 255) { Serial.println(F("ArdERR:DURATION1:must be 1-255")); }
            else { relay1Duration = dur; EEPROM.write(ADDR_R1_DURATION, relay1Duration);
                Serial.print(F("ArdACK:DURATION1:")); Serial.println(relay1Duration); }

        } else if (strncmp(message, "DURATION2:", 10) == 0) {
            int dur = atoi(message + 10);
            if (dur < 1 || dur > 255) { Serial.println(F("ArdERR:DURATION2:must be 1-255")); }
            else { relay2Duration = dur; EEPROM.write(ADDR_R2_DURATION, relay2Duration);
                Serial.print(F("ArdACK:DURATION2:")); Serial.println(relay2Duration); }

        } else if (strncmp(message, "VERBOSITY:", 10) == 0) {
            int v = atoi(message + 10);
            if (v < 0 || v > 2) { Serial.println(F("ArdERR:VERBOSITY:must be 0-2")); }
            else { verbosity = v; EEPROM.write(ADDR_VERBOSITY, verbosity);
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

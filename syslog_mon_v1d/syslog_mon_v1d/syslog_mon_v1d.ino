// IDS Network Monitor - Syslog Relay Trigger
// Version: 1d.3
// Changes from v1d.2:
//   - C9:  Multiple triggers per relay (up to 3 each, OR logic)
//   - C11: Subnet mask + gateway configuration, stored in EEPROM
//   - C12: Configurable verbosity (QUIET/NORMAL/DEBUG)

#include <Ethernet.h>
#include <EthernetUdp.h>
#include <EEPROM.h>

// EEPROM LAYOUT (total: 261 bytes of 1024)
#define ADDR_IP          0    // 4 bytes
#define ADDR_R1_MODE     4    // 1 byte (0=pulse, 1=latch)
#define ADDR_R2_MODE     5
#define ADDR_R1_DURATION 6    // 1 byte (seconds)
#define ADDR_R2_DURATION 7
#define ADDR_R1_TCOUNT   8    // 1 byte (active trigger count 1-3)
#define ADDR_R2_TCOUNT   9
#define ADDR_VERBOSITY   10   // 1 byte (0=quiet, 1=normal, 2=debug)
#define ADDR_SUBNET      11   // 4 bytes
#define ADDR_GATEWAY     15   // 4 bytes
// 19: reserved
#define ADDR_R1_TRIG_A   20   // 40 bytes each
#define ADDR_R1_TRIG_B   60
#define ADDR_R1_TRIG_C   100
#define ADDR_R2_TRIG_A   140
#define ADDR_R2_TRIG_B   180
#define ADDR_R2_TRIG_C   220
#define ADDR_BOOT_MARKER 260
#define BOOT_MARKER_VAL  125  // Bumped to force defaults for new layout

#define MAX_TRIG_LEN     40
#define MAX_TRIGS         3
#define FIRMWARE_VERSION "1d.3"
#define TEST_PULSE_MS    3000

// Relay modes
#define MODE_PULSE 0
#define MODE_LATCH 1

// Verbosity levels
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

// 3 triggers per relay
char r1Trigs[MAX_TRIGS][MAX_TRIG_LEN];
char r2Trigs[MAX_TRIGS][MAX_TRIG_LEN];
byte r1TrigCount = 1;
byte r2TrigCount = 1;

bool relay1IsOn = false;
bool relay2IsOn = false;
unsigned long relay1OnTime = 0;
unsigned long relay2OnTime = 0;
byte relay1Mode = MODE_PULSE;
byte relay2Mode = MODE_PULSE;
byte relay1Duration = DEFAULT_DURATION;
byte relay2Duration = DEFAULT_DURATION;
byte verbosity = VERB_NORMAL;

// EEPROM trigger addresses
const int r1TrigAddrs[MAX_TRIGS] = { ADDR_R1_TRIG_A, ADDR_R1_TRIG_B, ADDR_R1_TRIG_C };
const int r2TrigAddrs[MAX_TRIGS] = { ADDR_R2_TRIG_A, ADDR_R2_TRIG_B, ADDR_R2_TRIG_C };

// ===================== SETUP =====================

void setup() {
    Serial.begin(9600);
    delay(5000);
    Serial.println(F("ArdMsg: IDS Network Monitor v1d.3 starting..."));

    // First boot — write all defaults
    if (EEPROM.read(ADDR_BOOT_MARKER) != BOOT_MARKER_VAL) {
        Serial.println(F("ArdMsg: First boot - writing defaults."));
        writeIP(IPAddress(192, 168, 68, 142));
        writeIPField(ADDR_SUBNET, IPAddress(255, 255, 255, 0));
        writeIPField(ADDR_GATEWAY, IPAddress(0, 0, 0, 0));
        EEPROM.write(ADDR_R1_MODE, MODE_PULSE);
        EEPROM.write(ADDR_R2_MODE, MODE_PULSE);
        EEPROM.write(ADDR_R1_DURATION, DEFAULT_DURATION);
        EEPROM.write(ADDR_R2_DURATION, DEFAULT_DURATION);
        EEPROM.write(ADDR_R1_TCOUNT, 1);
        EEPROM.write(ADDR_R2_TCOUNT, 1);
        EEPROM.write(ADDR_VERBOSITY, VERB_NORMAL);
        writeTrigger("port up", ADDR_R1_TRIG_A);
        writeTrigger("", ADDR_R1_TRIG_B);
        writeTrigger("", ADDR_R1_TRIG_C);
        writeTrigger("port down", ADDR_R2_TRIG_A);
        writeTrigger("", ADDR_R2_TRIG_B);
        writeTrigger("", ADDR_R2_TRIG_C);
        EEPROM.write(ADDR_BOOT_MARKER, BOOT_MARKER_VAL);
    }

    // Load IP + network
    ip = readIPField(ADDR_IP);
    if (!isValidIP(ip)) {
        ip = IPAddress(192, 168, 68, 142);
    }
    subnet = readIPField(ADDR_SUBNET);
    gateway = readIPField(ADDR_GATEWAY);

    // Start Ethernet
    if (gateway[0] == 0 && gateway[1] == 0 && gateway[2] == 0 && gateway[3] == 0) {
        Ethernet.begin(mac, ip);  // No gateway
    } else {
        Ethernet.begin(mac, ip, gateway, gateway, subnet);  // DNS = gateway
    }

    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
        Serial.println(F("ArdMsg: Ethernet hardware not found. Halting."));
        while (true);
    }
    if (Ethernet.linkStatus() == LinkOFF) {
        Serial.println(F("ArdMsg: Ethernet cable is not connected."));
    }

    Udp.begin(localPort);

    // Load config
    relay1Mode = EEPROM.read(ADDR_R1_MODE);
    relay2Mode = EEPROM.read(ADDR_R2_MODE);
    relay1Duration = EEPROM.read(ADDR_R1_DURATION);
    relay2Duration = EEPROM.read(ADDR_R2_DURATION);
    r1TrigCount = EEPROM.read(ADDR_R1_TCOUNT);
    r2TrigCount = EEPROM.read(ADDR_R2_TCOUNT);
    verbosity = EEPROM.read(ADDR_VERBOSITY);

    // Clamp values
    if (relay1Mode > 1) relay1Mode = MODE_PULSE;
    if (relay2Mode > 1) relay2Mode = MODE_PULSE;
    if (relay1Duration == 0) relay1Duration = DEFAULT_DURATION;
    if (relay2Duration == 0) relay2Duration = DEFAULT_DURATION;
    if (r1TrigCount < 1 || r1TrigCount > MAX_TRIGS) r1TrigCount = 1;
    if (r2TrigCount < 1 || r2TrigCount > MAX_TRIGS) r2TrigCount = 1;
    if (verbosity > VERB_DEBUG) verbosity = VERB_NORMAL;

    // Load triggers
    for (int i = 0; i < MAX_TRIGS; i++) {
        readTrigger(r1TrigAddrs[i], r1Trigs[i], MAX_TRIG_LEN);
        readTrigger(r2TrigAddrs[i], r2Trigs[i], MAX_TRIG_LEN);
    }

    if (verbosity >= VERB_NORMAL) {
        Serial.print(F("ArdMsg: Board IP: "));
        Serial.println(Ethernet.localIP());
        for (int i = 0; i < r1TrigCount; i++) {
            Serial.print(F("ArdMsg: R1 trigger "));
            Serial.print((char)('A' + i));
            Serial.print(F(": "));
            Serial.println(r1Trigs[i]);
        }
        for (int i = 0; i < r2TrigCount; i++) {
            Serial.print(F("ArdMsg: R2 trigger "));
            Serial.print((char)('A' + i));
            Serial.print(F(": "));
            Serial.println(r2Trigs[i]);
        }
    }

    pinMode(relayPin1, OUTPUT);
    digitalWrite(relayPin1, LOW);
    pinMode(relayPin2, OUTPUT);
    digitalWrite(relayPin2, LOW);

    Serial.println(F("ArdMsg: Setup complete. Listening for syslog messages..."));
}

// ===================== EEPROM HELPERS =====================

void writeIP(IPAddress addr) {
    for (int i = 0; i < 4; i++) EEPROM.write(ADDR_IP + i, addr[i]);
}

void writeIPField(int startAddr, IPAddress addr) {
    for (int i = 0; i < 4; i++) EEPROM.write(startAddr + i, addr[i]);
}

IPAddress readIPField(int startAddr) {
    return IPAddress(EEPROM.read(startAddr), EEPROM.read(startAddr + 1),
                     EEPROM.read(startAddr + 2), EEPROM.read(startAddr + 3));
}

bool isValidIP(IPAddress addr) {
    return !(addr[0] == 0 && addr[1] == 0 && addr[2] == 0 && addr[3] == 0) &&
           !(addr[0] == 255 && addr[1] == 255 && addr[2] == 255 && addr[3] == 255);
}

void readTrigger(int startAddr, char* dest, int maxLen) {
    int index = 0;
    char ch;
    while ((ch = EEPROM.read(startAddr + index)) != '\0' && index < maxLen - 1) {
        dest[index++] = ch;
    }
    dest[index] = '\0';
}

void writeTrigger(const char* msg, int startAddr) {
    int len = strlen(msg);
    if (len >= MAX_TRIG_LEN) len = MAX_TRIG_LEN - 1;
    for (int i = 0; i < len; i++) EEPROM.write(startAddr + i, msg[i]);
    EEPROM.write(startAddr + len, '\0');
}

// ===================== UTILITY =====================

char* strcasestr_local(const char* haystack, const char* needle) {
    if (!*needle) return (char*)haystack;
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

IPAddress parseIP(const char* str) {
    int p[4];
    sscanf(str, "%d.%d.%d.%d", &p[0], &p[1], &p[2], &p[3]);
    return IPAddress(p[0], p[1], p[2], p[3]);
}

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
    // Relay 1 triggers
    Serial.print(F("ArdSTATUS:R1TrigCount:"));
    Serial.println(r1TrigCount);
    for (int i = 0; i < r1TrigCount; i++) {
        Serial.print(F("ArdSTATUS:R1Trig"));
        Serial.print((char)('A' + i));
        Serial.print(':');
        Serial.println(r1Trigs[i]);
    }
    // Relay 2 triggers
    Serial.print(F("ArdSTATUS:R2TrigCount:"));
    Serial.println(r2TrigCount);
    for (int i = 0; i < r2TrigCount; i++) {
        Serial.print(F("ArdSTATUS:R2Trig"));
        Serial.print((char)('A' + i));
        Serial.print(':');
        Serial.println(r2Trigs[i]);
    }
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

// Handle setting a trigger: relay 1 or 2, slot A/B/C
// cmd format: "Relay1A:message", "Relay1B:message", "Relay2C:message"
// Legacy "Relay1:message" maps to slot A
void handleTriggerSet(const char* cmd) {
    // Determine relay and slot
    int relay = 0;
    int slot = 0;
    const char* trigStr = NULL;

    if (strncmp(cmd, "Relay1", 6) == 0) {
        relay = 1;
        if (cmd[6] == ':') { slot = 0; trigStr = cmd + 7; }       // Legacy: Relay1:msg
        else if (cmd[6] == 'A' && cmd[7] == ':') { slot = 0; trigStr = cmd + 8; }
        else if (cmd[6] == 'B' && cmd[7] == ':') { slot = 1; trigStr = cmd + 8; }
        else if (cmd[6] == 'C' && cmd[7] == ':') { slot = 2; trigStr = cmd + 8; }
        else return;
    } else if (strncmp(cmd, "Relay2", 6) == 0) {
        relay = 2;
        if (cmd[6] == ':') { slot = 0; trigStr = cmd + 7; }
        else if (cmd[6] == 'A' && cmd[7] == ':') { slot = 0; trigStr = cmd + 8; }
        else if (cmd[6] == 'B' && cmd[7] == ':') { slot = 1; trigStr = cmd + 8; }
        else if (cmd[6] == 'C' && cmd[7] == ':') { slot = 2; trigStr = cmd + 8; }
        else return;
    } else return;

    int len = strlen(trigStr);
    if (len >= MAX_TRIG_LEN) {
        Serial.print(F("ArdERR:Relay"));
        Serial.print(relay);
        Serial.print((char)('A' + slot));
        Serial.println(F(":trigger must be <40 chars"));
        return;
    }

    // Get the right arrays and EEPROM addresses
    char (*trigs)[MAX_TRIG_LEN] = (relay == 1) ? r1Trigs : r2Trigs;
    const int* addrs = (relay == 1) ? r1TrigAddrs : r2TrigAddrs;
    byte* trigCount = (relay == 1) ? &r1TrigCount : &r2TrigCount;
    int countAddr = (relay == 1) ? ADDR_R1_TCOUNT : ADDR_R2_TCOUNT;

    // Write trigger
    strncpy(trigs[slot], trigStr, MAX_TRIG_LEN - 1);
    trigs[slot][MAX_TRIG_LEN - 1] = '\0';
    writeTrigger(trigs[slot], addrs[slot]);

    // Update trigger count (highest non-empty slot + 1)
    byte newCount = 1;
    for (int i = MAX_TRIGS - 1; i >= 0; i--) {
        if (trigs[i][0] != '\0') { newCount = i + 1; break; }
    }
    *trigCount = newCount;
    EEPROM.write(countAddr, newCount);

    Serial.print(F("ArdACK:Relay"));
    Serial.print(relay);
    Serial.print((char)('A' + slot));
    Serial.print(':');
    Serial.println(trigs[slot]);
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
            writeIP(ip);
            if (gateway[0] == 0 && gateway[1] == 0 && gateway[2] == 0 && gateway[3] == 0)
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

        } else if (strncmp(message, "Relay", 5) == 0) {
            handleTriggerSet(message);

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
            if (dur < 1 || dur > 255) {
                Serial.println(F("ArdERR:DURATION1:must be 1-255"));
            } else {
                relay1Duration = (byte)dur;
                EEPROM.write(ADDR_R1_DURATION, relay1Duration);
                Serial.print(F("ArdACK:DURATION1:"));
                Serial.println(relay1Duration);
            }

        } else if (strncmp(message, "DURATION2:", 10) == 0) {
            int dur = atoi(message + 10);
            if (dur < 1 || dur > 255) {
                Serial.println(F("ArdERR:DURATION2:must be 1-255"));
            } else {
                relay2Duration = (byte)dur;
                EEPROM.write(ADDR_R2_DURATION, relay2Duration);
                Serial.print(F("ArdACK:DURATION2:"));
                Serial.println(relay2Duration);
            }

        } else if (strncmp(message, "VERBOSITY:", 10) == 0) {
            int v = atoi(message + 10);
            if (v < 0 || v > 2) {
                Serial.println(F("ArdERR:VERBOSITY:must be 0-2"));
            } else {
                verbosity = (byte)v;
                EEPROM.write(ADDR_VERBOSITY, verbosity);
                Serial.print(F("ArdACK:VERBOSITY:"));
                Serial.println(verbosity);
            }

        } else if (strncmp(message, "RESET1", 6) == 0) {
            digitalWrite(relayPin1, LOW);
            relay1IsOn = false;
            Serial.println(F("ArdACK:RESET1:OK"));

        } else if (strncmp(message, "RESET2", 6) == 0) {
            digitalWrite(relayPin2, LOW);
            relay2IsOn = false;
            Serial.println(F("ArdACK:RESET2:OK"));

        } else if (strncmp(message, "STATUS", 6) == 0) {
            sendStatus();

        } else if (strncmp(message, "TTEST1", 6) == 0) {
            Serial.print(F("ArdACK:TTEST1:pulsing "));
            Serial.print(relay1Duration);
            Serial.println(F("s"));
            digitalWrite(relayPin1, HIGH);
            delay((unsigned long)relay1Duration * 1000UL);
            digitalWrite(relayPin1, LOW);
            Serial.println(F("ArdMsg: Relay 1 timed test complete."));

        } else if (strncmp(message, "TTEST2", 6) == 0) {
            Serial.print(F("ArdACK:TTEST2:pulsing "));
            Serial.print(relay2Duration);
            Serial.println(F("s"));
            digitalWrite(relayPin2, HIGH);
            delay((unsigned long)relay2Duration * 1000UL);
            digitalWrite(relayPin2, LOW);
            Serial.println(F("ArdMsg: Relay 2 timed test complete."));

        } else if (strncmp(message, "TEST1", 5) == 0) {
            Serial.println(F("ArdACK:TEST1:pulsing 3s"));
            digitalWrite(relayPin1, HIGH);
            delay(TEST_PULSE_MS);
            digitalWrite(relayPin1, LOW);
            Serial.println(F("ArdMsg: Relay 1 test complete."));

        } else if (strncmp(message, "TEST2", 5) == 0) {
            Serial.println(F("ArdACK:TEST2:pulsing 3s"));
            digitalWrite(relayPin2, HIGH);
            delay(TEST_PULSE_MS);
            digitalWrite(relayPin2, LOW);
            Serial.println(F("ArdMsg: Relay 2 test complete."));
        }
    }

    // Listen for syslog packets
    int packetSize = Udp.parsePacket();
    if (packetSize) {
        memset(packetBuffer, 0, BUFFER_SIZE);
        Udp.read(packetBuffer, min(packetSize, BUFFER_SIZE - 1));
        if (verbosity >= VERB_DEBUG) {
            Serial.print(F("ArdMsg: Syslog ("));
            Serial.print(packetSize);
            Serial.print(F("b): "));
            Serial.println(packetBuffer);
        }
    }

    // Trigger matching — check all active triggers per relay (OR logic)
    if (!relay1IsOn) {
        for (int i = 0; i < r1TrigCount; i++) {
            if (r1Trigs[i][0] != '\0' && strcasestr_local(packetBuffer, r1Trigs[i])) {
                digitalWrite(relayPin1, HIGH);
                relay1IsOn = true;
                relay1OnTime = millis();
                if (verbosity >= VERB_QUIET) {
                    Serial.print(F("ArdMsg: Relay 1 ENERGIZED - matched trigger "));
                    Serial.print((char)('A' + i));
                    Serial.print(F(": "));
                    Serial.println(r1Trigs[i]);
                }
                memset(packetBuffer, 0, BUFFER_SIZE);
                break;
            }
        }
    }

    if (!relay2IsOn) {
        for (int i = 0; i < r2TrigCount; i++) {
            if (r2Trigs[i][0] != '\0' && strcasestr_local(packetBuffer, r2Trigs[i])) {
                digitalWrite(relayPin2, HIGH);
                relay2IsOn = true;
                relay2OnTime = millis();
                if (verbosity >= VERB_QUIET) {
                    Serial.print(F("ArdMsg: Relay 2 ENERGIZED - matched trigger "));
                    Serial.print((char)('A' + i));
                    Serial.print(F(": "));
                    Serial.println(r2Trigs[i]);
                }
                memset(packetBuffer, 0, BUFFER_SIZE);
                break;
            }
        }
    }

    // Relay timeout — pulse mode only
    if (relay1IsOn && relay1Mode == MODE_PULSE && (millis() - relay1OnTime >= (unsigned long)relay1Duration * 1000UL)) {
        digitalWrite(relayPin1, LOW);
        relay1IsOn = false;
        if (verbosity >= VERB_NORMAL) {
            Serial.print(F("ArdMsg: Relay 1 de-energized ("));
            Serial.print(relay1Duration);
            Serial.println(F("s timeout)."));
        }
    }

    if (relay2IsOn && relay2Mode == MODE_PULSE && (millis() - relay2OnTime >= (unsigned long)relay2Duration * 1000UL)) {
        digitalWrite(relayPin2, LOW);
        relay2IsOn = false;
        if (verbosity >= VERB_NORMAL) {
            Serial.print(F("ArdMsg: Relay 2 de-energized ("));
            Serial.print(relay2Duration);
            Serial.println(F("s timeout)."));
        }
    }
}

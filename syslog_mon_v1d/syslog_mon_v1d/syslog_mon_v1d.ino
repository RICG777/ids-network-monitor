// IDS Network Monitor - Syslog Relay Trigger
// Version: 1d.2
// Changes from v1d.1:
//   - ADD: configurable relay mode — PULSE (auto-reset) or LATCH (stays on until manual reset)
//   - ADD: configurable pulse duration per relay (1-255 seconds, stored in EEPROM)
//   - ADD: RESET1/RESET2 commands to de-energize latched relays
//   - ADD: MODE1/MODE2 commands to set relay mode
//   - ADD: DURATION1/DURATION2 commands to set pulse duration
//   - ADD: MAC address reported in STATUS response for commissioning/O&M
//   - ADD: relay mode and duration reported in STATUS response

#include <Ethernet.h>
#include <EthernetUdp.h>
#include <EEPROM.h>

// EEPROM STORAGE MAP
//   0-3:     IP address (4 bytes)
//   4:       Relay 1 mode (0=pulse, 1=latch)
//   5:       Relay 2 mode (0=pulse, 1=latch)
//   6:       Relay 1 pulse duration (seconds, 1-255)
//   7:       Relay 2 pulse duration (seconds, 1-255)
//   8-19:    Reserved
//   20-99:   Trigger message 1 (80 bytes)
//   100-109: Reserved
//   110-189: Trigger message 2 (80 bytes)
//   190-199: Reserved
//   200:     First boot marker
//   201+:    Free
#define IP_ADDRESS_START_ADDR 0
#define RELAY1_MODE_ADDR 4
#define RELAY2_MODE_ADDR 5
#define RELAY1_DURATION_ADDR 6
#define RELAY2_DURATION_ADDR 7
#define TRIGGER1_START_ADDR 20
#define TRIGGER2_START_ADDR 110
#define FIRST_BOOT_MARKER_ADDR 200
#define FIRST_BOOT_MARKER_VALUE 124  // Bumped from 123 to force defaults for new fields
#define MAX_TRIGGER_LEN 80
#define FIRMWARE_VERSION "1d.2"
#define TEST_PULSE_MS 3000

// Relay modes
#define MODE_PULSE 0
#define MODE_LATCH 1
#define DEFAULT_DURATION 10

// VARIABLES
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress ip;
EthernetUDP Udp;
unsigned int localPort = 514;
const int relayPin1 = 7;
const int relayPin2 = 8;

#define BUFFER_SIZE 256
char packetBuffer[BUFFER_SIZE];

char triggerMessage1[MAX_TRIGGER_LEN] = "port up";
char triggerMessage2[MAX_TRIGGER_LEN] = "port down";
bool relay1IsOn = false;
bool relay2IsOn = false;
unsigned long relay1OnTime = 0;
unsigned long relay2OnTime = 0;
byte relay1Mode = MODE_PULSE;
byte relay2Mode = MODE_PULSE;
byte relay1Duration = DEFAULT_DURATION;
byte relay2Duration = DEFAULT_DURATION;

void setup() {
    Serial.begin(9600);
    delay(5000);
    Serial.println(F("ArdMsg: IDS Network Monitor v1d.2 starting..."));

    // Load IP from EEPROM
    byte storedIP[4];
    for (int i = 0; i < 4; i++) {
        storedIP[i] = EEPROM.read(IP_ADDRESS_START_ADDR + i);
    }

    if (isValidIP(storedIP)) {
        ip = IPAddress(storedIP[0], storedIP[1], storedIP[2], storedIP[3]);
        Serial.print(F("ArdMsg: IP from EEPROM: "));
        Serial.println(ip);
    } else {
        Serial.println(F("ArdMsg: Invalid IP in EEPROM. Using default 192.168.68.142"));
        ip = IPAddress(192, 168, 68, 142);
        for (int i = 0; i < 4; i++) {
            EEPROM.write(IP_ADDRESS_START_ADDR + i, ip[i]);
        }
    }

    Ethernet.begin(mac, ip);

    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
        Serial.println(F("ArdMsg: Ethernet hardware not found. Halting."));
        while (true);
    }

    if (Ethernet.linkStatus() == LinkOFF) {
        Serial.println(F("ArdMsg: Ethernet cable is not connected."));
    }

    Udp.begin(localPort);

    // First boot — write all defaults
    if (EEPROM.read(FIRST_BOOT_MARKER_ADDR) != FIRST_BOOT_MARKER_VALUE) {
        Serial.println(F("ArdMsg: First boot - writing defaults to EEPROM."));
        writeTriggerMessageToEEPROM("port up", TRIGGER1_START_ADDR);
        writeTriggerMessageToEEPROM("port down", TRIGGER2_START_ADDR);
        EEPROM.write(RELAY1_MODE_ADDR, MODE_PULSE);
        EEPROM.write(RELAY2_MODE_ADDR, MODE_PULSE);
        EEPROM.write(RELAY1_DURATION_ADDR, DEFAULT_DURATION);
        EEPROM.write(RELAY2_DURATION_ADDR, DEFAULT_DURATION);
        EEPROM.write(FIRST_BOOT_MARKER_ADDR, FIRST_BOOT_MARKER_VALUE);
    }

    // Load triggers
    strncpy(triggerMessage1, readTriggerMessageFromEEPROM(TRIGGER1_START_ADDR), sizeof(triggerMessage1) - 1);
    triggerMessage1[sizeof(triggerMessage1) - 1] = '\0';
    strncpy(triggerMessage2, readTriggerMessageFromEEPROM(TRIGGER2_START_ADDR), sizeof(triggerMessage2) - 1);
    triggerMessage2[sizeof(triggerMessage2) - 1] = '\0';

    // Load relay modes and durations
    relay1Mode = EEPROM.read(RELAY1_MODE_ADDR);
    relay2Mode = EEPROM.read(RELAY2_MODE_ADDR);
    relay1Duration = EEPROM.read(RELAY1_DURATION_ADDR);
    relay2Duration = EEPROM.read(RELAY2_DURATION_ADDR);
    if (relay1Mode > 1) relay1Mode = MODE_PULSE;
    if (relay2Mode > 1) relay2Mode = MODE_PULSE;
    if (relay1Duration == 0) relay1Duration = DEFAULT_DURATION;
    if (relay2Duration == 0) relay2Duration = DEFAULT_DURATION;

    Serial.print(F("ArdMsg: R1 trigger: "));
    Serial.println(triggerMessage1);
    Serial.print(F("ArdMsg: R2 trigger: "));
    Serial.println(triggerMessage2);
    Serial.print(F("ArdMsg: R1 mode: "));
    Serial.println(relay1Mode == MODE_LATCH ? "LATCH" : "PULSE");
    Serial.print(F("ArdMsg: R2 mode: "));
    Serial.println(relay2Mode == MODE_LATCH ? "LATCH" : "PULSE");

    pinMode(relayPin1, OUTPUT);
    digitalWrite(relayPin1, LOW);
    pinMode(relayPin2, OUTPUT);
    digitalWrite(relayPin2, LOW);

    Serial.println(F("ArdMsg: Setup complete. Listening for syslog messages..."));
}


// FUNCTIONS
void setNewIPAddress(const char* newIP) {
    int parts[4];
    sscanf(newIP, "%d.%d.%d.%d", &parts[0], &parts[1], &parts[2], &parts[3]);
    ip = IPAddress(parts[0], parts[1], parts[2], parts[3]);
    Ethernet.begin(mac, ip);
    for (int i = 0; i < 4; i++) {
        EEPROM.write(IP_ADDRESS_START_ADDR + i, parts[i]);
    }
}

char* readTriggerMessageFromEEPROM(int startAddr) {
    static char message[MAX_TRIGGER_LEN];
    int addr = startAddr;
    int index = 0;
    char ch;
    while ((ch = EEPROM.read(addr)) != '\0' && index < sizeof(message) - 1) {
        message[index++] = ch;
        addr++;
    }
    message[index] = '\0';
    return message;
}

void writeTriggerMessageToEEPROM(const char* message, int startAddr) {
    int len = strlen(message);
    if (len >= MAX_TRIGGER_LEN) len = MAX_TRIGGER_LEN - 1;
    for (int i = 0; i < len; i++) {
        EEPROM.write(startAddr + i, message[i]);
    }
    EEPROM.write(startAddr + len, '\0');
}

bool isValidIP(byte ip[]) {
    return !(ip[0] == 0 && ip[1] == 0 && ip[2] == 0 && ip[3] == 0) &&
           !(ip[0] == 255 && ip[1] == 255 && ip[2] == 255 && ip[3] == 255);
}

char* strcasestr_local(const char* haystack, const char* needle) {
    if (!*needle) return (char*)haystack;
    for (; *haystack; haystack++) {
        const char* h = haystack;
        const char* n = needle;
        while (*h && *n && (tolower((unsigned char)*h) == tolower((unsigned char)*n))) {
            h++;
            n++;
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

void sendStatus() {
    Serial.print(F("ArdSTATUS:FW:"));
    Serial.println(F(FIRMWARE_VERSION));
    Serial.print(F("ArdSTATUS:MAC:"));
    printMAC();
    Serial.print(F("ArdSTATUS:IP:"));
    Serial.println(Ethernet.localIP());
    Serial.print(F("ArdSTATUS:Relay1:"));
    Serial.println(triggerMessage1);
    Serial.print(F("ArdSTATUS:Relay2:"));
    Serial.println(triggerMessage2);
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
}

void loop() {
    // Handle serial commands from Windows app
    if (Serial.available()) {
        char message[128];
        int bytesRead = Serial.readBytesUntil('\n', message, sizeof(message) - 1);
        message[bytesRead] = '\0';

        if (bytesRead > 0 && message[bytesRead - 1] == '\r') {
            message[bytesRead - 1] = '\0';
        }

        while (Serial.available()) {
            Serial.read();
        }

        if (strncmp(message, "IP:", 3) == 0) {
            setNewIPAddress(message + 3);
            Serial.print(F("ArdACK:IP:"));
            Serial.println(ip);

        } else if (strncmp(message, "Relay1:", 7) == 0) {
            const char* trigger = message + 7;
            int len = strlen(trigger);
            if (len == 0 || len >= MAX_TRIGGER_LEN) {
                Serial.println(F("ArdERR:Relay1:trigger must be 1-79 chars"));
            } else {
                strncpy(triggerMessage1, trigger, sizeof(triggerMessage1) - 1);
                triggerMessage1[sizeof(triggerMessage1) - 1] = '\0';
                writeTriggerMessageToEEPROM(triggerMessage1, TRIGGER1_START_ADDR);
                Serial.print(F("ArdACK:Relay1:"));
                Serial.println(triggerMessage1);
            }

        } else if (strncmp(message, "Relay2:", 7) == 0) {
            const char* trigger = message + 7;
            int len = strlen(trigger);
            if (len == 0 || len >= MAX_TRIGGER_LEN) {
                Serial.println(F("ArdERR:Relay2:trigger must be 1-79 chars"));
            } else {
                strncpy(triggerMessage2, trigger, sizeof(triggerMessage2) - 1);
                triggerMessage2[sizeof(triggerMessage2) - 1] = '\0';
                writeTriggerMessageToEEPROM(triggerMessage2, TRIGGER2_START_ADDR);
                Serial.print(F("ArdACK:Relay2:"));
                Serial.println(triggerMessage2);
            }

        } else if (strncmp(message, "MODE1:", 6) == 0) {
            if (strncmp(message + 6, "LATCH", 5) == 0) {
                relay1Mode = MODE_LATCH;
            } else {
                relay1Mode = MODE_PULSE;
            }
            EEPROM.write(RELAY1_MODE_ADDR, relay1Mode);
            Serial.print(F("ArdACK:MODE1:"));
            Serial.println(relay1Mode == MODE_LATCH ? "LATCH" : "PULSE");

        } else if (strncmp(message, "MODE2:", 6) == 0) {
            if (strncmp(message + 6, "LATCH", 5) == 0) {
                relay2Mode = MODE_LATCH;
            } else {
                relay2Mode = MODE_PULSE;
            }
            EEPROM.write(RELAY2_MODE_ADDR, relay2Mode);
            Serial.print(F("ArdACK:MODE2:"));
            Serial.println(relay2Mode == MODE_LATCH ? "LATCH" : "PULSE");

        } else if (strncmp(message, "DURATION1:", 10) == 0) {
            int dur = atoi(message + 10);
            if (dur < 1 || dur > 255) {
                Serial.println(F("ArdERR:DURATION1:must be 1-255 seconds"));
            } else {
                relay1Duration = (byte)dur;
                EEPROM.write(RELAY1_DURATION_ADDR, relay1Duration);
                Serial.print(F("ArdACK:DURATION1:"));
                Serial.println(relay1Duration);
            }

        } else if (strncmp(message, "DURATION2:", 10) == 0) {
            int dur = atoi(message + 10);
            if (dur < 1 || dur > 255) {
                Serial.println(F("ArdERR:DURATION2:must be 1-255 seconds"));
            } else {
                relay2Duration = (byte)dur;
                EEPROM.write(RELAY2_DURATION_ADDR, relay2Duration);
                Serial.print(F("ArdACK:DURATION2:"));
                Serial.println(relay2Duration);
            }

        } else if (strncmp(message, "RESET1", 6) == 0) {
            digitalWrite(relayPin1, LOW);
            relay1IsOn = false;
            Serial.println(F("ArdACK:RESET1:OK"));
            Serial.println(F("ArdMsg: Relay 1 manually reset."));

        } else if (strncmp(message, "RESET2", 6) == 0) {
            digitalWrite(relayPin2, LOW);
            relay2IsOn = false;
            Serial.println(F("ArdACK:RESET2:OK"));
            Serial.println(F("ArdMsg: Relay 2 manually reset."));

        } else if (strncmp(message, "STATUS", 6) == 0) {
            sendStatus();

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
        Serial.print(F("ArdMsg: Syslog ("));
        Serial.print(packetSize);
        Serial.print(F("b): "));
        Serial.println(packetBuffer);
    }

    // Trigger matching
    if (strcasestr_local(packetBuffer, triggerMessage1) && !relay1IsOn) {
        digitalWrite(relayPin1, HIGH);
        relay1IsOn = true;
        relay1OnTime = millis();
        Serial.println(F("ArdMsg: Relay 1 ENERGIZED - trigger matched!"));
        memset(packetBuffer, 0, BUFFER_SIZE);
    }

    if (strcasestr_local(packetBuffer, triggerMessage2) && !relay2IsOn) {
        digitalWrite(relayPin2, HIGH);
        relay2IsOn = true;
        relay2OnTime = millis();
        Serial.println(F("ArdMsg: Relay 2 ENERGIZED - trigger matched!"));
        memset(packetBuffer, 0, BUFFER_SIZE);
    }

    // Relay timeout — only in PULSE mode
    if (relay1IsOn && relay1Mode == MODE_PULSE && (millis() - relay1OnTime >= (unsigned long)relay1Duration * 1000UL)) {
        digitalWrite(relayPin1, LOW);
        relay1IsOn = false;
        Serial.print(F("ArdMsg: Relay 1 de-energized ("));
        Serial.print(relay1Duration);
        Serial.println(F("s timeout)."));
    }

    if (relay2IsOn && relay2Mode == MODE_PULSE && (millis() - relay2OnTime >= (unsigned long)relay2Duration * 1000UL)) {
        digitalWrite(relayPin2, LOW);
        relay2IsOn = false;
        Serial.print(F("ArdMsg: Relay 2 de-energized ("));
        Serial.print(relay2Duration);
        Serial.println(F("s timeout)."));
    }
}

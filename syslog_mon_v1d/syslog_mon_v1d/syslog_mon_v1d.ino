// IDS Network Monitor - Syslog Relay Trigger
// Version: 1d.1
// Changes from v1d:
//   - FIX: triggerMessage arrays sized to 80 bytes (was sized to initializer, causing truncation)
//   - FIX: packetBuffer reduced 512 -> 256 bytes (frees SRAM, syslog triggers are short)
//   - FIX: String() objects replaced with F() macro + Serial.print() (prevents heap fragmentation)
//   - FIX: trigger length validation before EEPROM write (prevents overflow into adjacent regions)
//   - FIX: null termination enforced after all strncpy calls
//   - ADD: case-insensitive trigger matching (switches may vary case)
//   - ADD: ArdACK/ArdERR responses for config commands (Windows app can verify receipt)
//   - ADD: STATUS command returns current IP, triggers, and relay states

#include <Ethernet.h>
#include <EthernetUdp.h>
#include <EEPROM.h>

// EEPROM STORAGE
#define IP_ADDRESS_START_ADDR 0
#define TRIGGER1_START_ADDR 20
#define TRIGGER2_START_ADDR 110
#define FIRST_BOOT_MARKER_ADDR 200
#define FIRST_BOOT_MARKER_VALUE 123
#define MAX_TRIGGER_LEN 80
#define FIRMWARE_VERSION "1d.1"
#define TEST_PULSE_MS 3000

// VARIABLES SECTION
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress ip;
EthernetUDP Udp;
unsigned int localPort = 514;
const int relayPin1 = 7;
const int relayPin2 = 8;

// Reduced buffer size - 256 is plenty for syslog trigger messages
#define BUFFER_SIZE 256
char packetBuffer[BUFFER_SIZE];

// Trigger messages - sized to match EEPROM allocation (80 bytes max)
char triggerMessage1[MAX_TRIGGER_LEN] = "port up";
char triggerMessage2[MAX_TRIGGER_LEN] = "port down";
bool relay1IsOn = false;
bool relay2IsOn = false;
unsigned long relay1OnTime = 0;
unsigned long relay2OnTime = 0;

void setup() {
    Serial.begin(9600);
    delay(5000);
    Serial.println(F("ArdMsg: IDS Network Monitor v1d starting..."));

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
        Serial.print(F("ArdMsg: IP now set as: "));
        Serial.println(ip);
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
    Serial.println(F("ArdMsg: UDP started on port 514."));

    if (EEPROM.read(FIRST_BOOT_MARKER_ADDR) != FIRST_BOOT_MARKER_VALUE) {
        Serial.println(F("ArdMsg: First boot - writing default triggers to EEPROM."));
        writeTriggerMessageToEEPROM("port up", TRIGGER1_START_ADDR);
        writeTriggerMessageToEEPROM("port down", TRIGGER2_START_ADDR);
        EEPROM.write(FIRST_BOOT_MARKER_ADDR, FIRST_BOOT_MARKER_VALUE);
    }

    strncpy(triggerMessage1, readTriggerMessageFromEEPROM(TRIGGER1_START_ADDR), sizeof(triggerMessage1) - 1);
    triggerMessage1[sizeof(triggerMessage1) - 1] = '\0';
    Serial.print(F("ArdMsg: Relay 1 trigger: "));
    Serial.println(triggerMessage1);

    strncpy(triggerMessage2, readTriggerMessageFromEEPROM(TRIGGER2_START_ADDR), sizeof(triggerMessage2) - 1);
    triggerMessage2[sizeof(triggerMessage2) - 1] = '\0';
    Serial.print(F("ArdMsg: Relay 2 trigger: "));
    Serial.println(triggerMessage2);

    Serial.print(F("ArdMsg: Board IP: "));
    Serial.println(Ethernet.localIP());

    pinMode(relayPin1, OUTPUT);
    digitalWrite(relayPin1, LOW);

    pinMode(relayPin2, OUTPUT);
    digitalWrite(relayPin2, LOW);

    Serial.println(F("ArdMsg: Setup complete. Listening for syslog messages..."));
}


// FUNCTIONS SECTION
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
    char ch;
    int addr = startAddr;
    int index = 0;
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

// Case-insensitive substring search
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

void loop() {
    // Handle serial commands from Windows app
    if (Serial.available()) {
        char message[128];
        int bytesRead = Serial.readBytesUntil('\n', message, sizeof(message) - 1);
        message[bytesRead] = '\0';

        // Strip carriage return
        if (bytesRead > 0 && message[bytesRead - 1] == '\r') {
            message[bytesRead - 1] = '\0';
        }

        // Clear any remaining serial data
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
        } else if (strncmp(message, "STATUS", 6) == 0) {
            Serial.print(F("ArdSTATUS:FW:"));
            Serial.println(F(FIRMWARE_VERSION));
            Serial.print(F("ArdSTATUS:IP:"));
            Serial.println(Ethernet.localIP());
            Serial.print(F("ArdSTATUS:Relay1:"));
            Serial.println(triggerMessage1);
            Serial.print(F("ArdSTATUS:Relay2:"));
            Serial.println(triggerMessage2);
            Serial.print(F("ArdSTATUS:R1State:"));
            Serial.println(relay1IsOn ? "ON" : "OFF");
            Serial.print(F("ArdSTATUS:R2State:"));
            Serial.println(relay2IsOn ? "ON" : "OFF");
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

    // Case-insensitive trigger matching
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

    // Relay timeout (10 seconds)
    if (relay1IsOn && (millis() - relay1OnTime >= 10000)) {
        digitalWrite(relayPin1, LOW);
        relay1IsOn = false;
        Serial.println(F("ArdMsg: Relay 1 de-energized (10s timeout)."));
    }

    if (relay2IsOn && (millis() - relay2OnTime >= 10000)) {
        digitalWrite(relayPin2, LOW);
        relay2IsOn = false;
        Serial.println(F("ArdMsg: Relay 2 de-energized (10s timeout)."));
    }
}

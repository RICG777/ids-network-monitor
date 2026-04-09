#include <Ethernet.h>
#include <EthernetUdp.h>
#include <EEPROM.h>

// EEPROM STORAGE
#define IP_ADDRESS_START_ADDR 0
#define TRIGGER1_START_ADDR 20
#define TRIGGER2_START_ADDR 110
#define FIRST_BOOT_MARKER_ADDR 200
#define FIRST_BOOT_MARKER_VALUE 123

// VARIABLES SECTION
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress ip;
EthernetUDP Udp;
unsigned int localPort = 514;
const int relayPin1 = 7;
const int relayPin2 = 8;

// Reduced buffer size
#define BUFFER_SIZE 512
char packetBuffer[BUFFER_SIZE];

// Using char arrays instead of String for trigger messages
char triggerMessage1[] = "port up";
char triggerMessage2[] = "port down";
bool relay1IsOn = false;
bool relay2IsOn = false;
unsigned long relay1OnTime = 0;
unsigned long relay2OnTime = 0;

void setup() {
    // Only initialize Serial if you need it for debugging during setup.
    Serial.begin(9600);
    delay(5000);  // Give serial port some time to initialize. This can be removed if not using Serial.
    Serial.println("ArdMsg: Initializing Ethernet...");

    byte storedIP[4];
    for (int i = 0; i < 4; i++) {
        storedIP[i] = EEPROM.read(IP_ADDRESS_START_ADDR + i);
    }

    if (isValidIP(storedIP)) {
        ip = IPAddress(storedIP[0], storedIP[1], storedIP[2], storedIP[3]);
        Serial.print("ArdMsg: IP from EEPROM:\n");
        Serial.println(ip);
    } else {
        Serial.println("ArdMsg: Invalid IP in EEPROM. Using default IP 192.168.68.142");
        ip = IPAddress(192, 168, 68, 142);
        for (int i = 0; i < 4; i++) {
            EEPROM.write(IP_ADDRESS_START_ADDR + i, ip[i]);
        }
        Serial.print("ArdMsg: IP now set as: ");
        Serial.println(ip);
    }

    Ethernet.begin(mac, ip);

    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
        Serial.println("ArdMsg: Ethernet shield was not found. Sorry, can't run without hardware.");
        while (true);
    }

    if (Ethernet.linkStatus() == LinkOFF) {
        Serial.println("ArdMsg: Ethernet cable is not connected.");
    }

    Udp.begin(localPort);
    Serial.println("ArdMsg: Start UDP.");

    if (EEPROM.read(FIRST_BOOT_MARKER_ADDR) != FIRST_BOOT_MARKER_VALUE) {
        Serial.println("ArdMsg: Writing first trigger message to EEPROM.");
        writeTriggerMessageToEEPROM("port up", TRIGGER1_START_ADDR);
        writeTriggerMessageToEEPROM("port down", TRIGGER2_START_ADDR);
        EEPROM.write(FIRST_BOOT_MARKER_ADDR, FIRST_BOOT_MARKER_VALUE);
        Serial.println("ArdMsg: 1st Boot Flag Set.");
    }

    strncpy(triggerMessage1, readTriggerMessageFromEEPROM(TRIGGER1_START_ADDR), sizeof(triggerMessage1) - 1);
    Serial.println("ArdMsg: Relay 1 Trigger from EEPROM: " + String(triggerMessage1));
    Serial.println("ArdMsg: Relay1 Trigger Set...");

    strncpy(triggerMessage2, readTriggerMessageFromEEPROM(TRIGGER2_START_ADDR), sizeof(triggerMessage2) - 1);
    Serial.println("ArdMsg: Relay 2 Trigger from EEPROM: " + String(triggerMessage2));
    Serial.println("ArdMsg: Relay2 Trigger Set...");

    Serial.print("ArdMsg: Board IP address: ");
    Serial.println(Ethernet.localIP());

    pinMode(relayPin1, OUTPUT);
    digitalWrite(relayPin1, LOW);

    pinMode(relayPin2, OUTPUT);
    digitalWrite(relayPin2, LOW);

    Serial.println("ArdMsg: Setup complete. Listening for syslog messages...");
}


// FUNCTIONS SECTION
void setNewIPAddress(const char* newIP) {
    int parts[4];
    sscanf(newIP, "%d.%d.%d.%d", &parts[0], &parts[1], &parts[2], &parts[3]);
    ip = IPAddress(parts[0], parts[1], parts[2], parts[3]);
    Ethernet.begin(mac, ip);
    Serial.print("ArdMsg: New IP address set: ");
    Serial.println(ip);
    for (int i = 0; i < 4; i++) {
        EEPROM.write(IP_ADDRESS_START_ADDR + i, parts[i]);
    }
}

char* readTriggerMessageFromEEPROM(int startAddr) {
    static char message[80];  // Assuming a reasonable size for trigger messages
    char ch;
    int addr = startAddr;
    int index = 0;
    while ((ch = EEPROM.read(addr)) != '\0' && index < sizeof(message) - 1) {
        message[index++] = ch;
        addr++;
    }
    message[index] = '\0';
    Serial.print("Reading from EEPROM at address: ");
    Serial.println(startAddr);

    return message;
}

void writeTriggerMessageToEEPROM(const char* message, int startAddr) {
    for (int i = 0; i < strlen(message) && i < 80; i++) {  // Assuming a reasonable size for trigger messages
        EEPROM.write(startAddr + i, message[i]);
    }
    EEPROM.write(startAddr + strlen(message), '\0');
    Serial.print("ArdMsg: Trigger Message to EEPROM Function Complete ");
}

bool isValidIP(byte ip[]) {
    return !(ip[0] == 0 && ip[1] == 0 && ip[2] == 0 && ip[3] == 0) &&
           !(ip[0] == 255 && ip[1] == 255 && ip[2] == 255 && ip[3] == 255);
}

void loop() {
    if (Serial.available()) {
        char message[128];  // Assuming a reasonable size for incoming serial messages
        int bytesRead = Serial.readBytesUntil('\n', message, sizeof(message) - 1); // -1 to leave space for null terminator
        message[bytesRead] = '\0'; // Null-terminate the string

        // Check for carriage return and remove it
        if (bytesRead > 0 && message[bytesRead - 1] == '\r') {
            message[bytesRead - 1] = '\0';
        }

        // Clear the serial buffer
        while (Serial.available()) {
        Serial.read();  // Read and discard the byte
    }

        if (strncmp(message, "IP:", 3) == 0) {
            setNewIPAddress(message + 3);
        } else if (strncmp(message, "Relay1:", 7) == 0) {
            strncpy(triggerMessage1, message + 7, sizeof(triggerMessage1) - 1);
            Serial.println("ArdMsg: Received trigger message for Relay 1: " + String(triggerMessage1));
            for (int i = 0; i < strlen(triggerMessage1); i++) {
                EEPROM.write(TRIGGER1_START_ADDR + i, triggerMessage1[i]);
            }
            EEPROM.write(TRIGGER1_START_ADDR + strlen(triggerMessage1), '\0');
        } else if (strncmp(message, "Relay2:", 7) == 0) {
            strncpy(triggerMessage2, message + 7, sizeof(triggerMessage2) - 1);
            Serial.println("ArdMsg: Received trigger message for Relay 2: " + String(triggerMessage2));
            for (int i = 0; i < strlen(triggerMessage2); i++) {
                EEPROM.write(TRIGGER2_START_ADDR + i, triggerMessage2[i]);
            }
            EEPROM.write(TRIGGER2_START_ADDR + strlen(triggerMessage2), '\0');
        }
    }

    int packetSize = Udp.parsePacket();
    if (packetSize) {
        memset(packetBuffer, 0, BUFFER_SIZE);
        Udp.read(packetBuffer, min(packetSize, BUFFER_SIZE - 1));
        Serial.print("ArdMsg: Received message (length ");
        Serial.print(packetSize);
        Serial.print("): ");
        Serial.println(packetBuffer);
    }

    if (strstr(packetBuffer, triggerMessage1) && !relay1IsOn) {
        digitalWrite(relayPin1, HIGH);
        relay1IsOn = true;
        relay1OnTime = millis();
        Serial.println("Relay 1 energized due to trigger!");
        memset(packetBuffer, 0, BUFFER_SIZE);
    }

    if (strstr(packetBuffer, triggerMessage2) && !relay2IsOn) {
        digitalWrite(relayPin2, HIGH);
        relay2IsOn = true;
        relay2OnTime = millis();
        Serial.println("Relay 2 energized due to trigger!");
        memset(packetBuffer, 0, BUFFER_SIZE);
    }

    if (relay1IsOn && (millis() - relay1OnTime >= 10000)) {
        digitalWrite(relayPin1, LOW);
        relay1IsOn = false;
        Serial.println("Relay 1 de-energized after 10 seconds!");
    }

    if (relay2IsOn && (millis() - relay2OnTime >= 10000)) {
        digitalWrite(relayPin2, LOW);
        relay2IsOn = false;
        Serial.println("Relay 2 de-energized after 10 seconds!");
    }
}

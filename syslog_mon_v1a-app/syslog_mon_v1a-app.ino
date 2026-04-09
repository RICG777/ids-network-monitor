#include <Ethernet.h>
#include <EthernetUdp.h>
#include <EEPROM.h>

//EEPROM STORAGE
#define IP_ADDRESS_START_ADDR 0
#define TRIGGER1_START_ADDR 20  //79 CHARACTER LIMIT
#define TRIGGER2_START_ADDR 100 //49 CHARACTER LIMIT
#define FIRST_BOOT_MARKER_ADDR 150

#define FIRST_BOOT_MARKER_VALUE 123  // Any unique value to indicate first boot

//****VARIABLES SECTION****

// MAC address for the board
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};

// Default IP address for the board
IPAddress ip;

// UDP instance
EthernetUDP Udp;

// UDP port to listen on
unsigned int localPort = 514;

// Relay pin
const int relayPin1 = 7;  //relay 1
const int relayPin2 = 8;  //relay 2

// Buffer size
#define BUFFER_SIZE 1024 
char packetBuffer[BUFFER_SIZE];

// Variables for Relay 1
String triggerMessage1 = "port up";
bool relay1IsOn = false;
unsigned long relay1OnTime = 0;

// Variables for Relay 2
String triggerMessage2 = "port down";
bool relay2IsOn = false;
unsigned long relay2OnTime = 0;



//*****SETUP LOOP*****

void setup() {
  Serial.begin(9600);
  while (!Serial);

  Serial.println("ArdMsg: Initializing Ethernet...");
  
   // Read IP address from EEPROM
  byte storedIP[4];
  for (int i = 0; i < 4; i++) {
      storedIP[i] = EEPROM.read(IP_ADDRESS_START_ADDR + i);
      Serial.println("ArdMsg: Read from EEPROM...");
  }
    
  // Validate the IP from EEPROM
   if (isValidIP(storedIP)) {
        ip = IPAddress(storedIP[0], storedIP[1], storedIP[2], storedIP[3]);
        Serial.print("ArdMsg: IP from EEPROM:\n");
        Serial.println(ip);
   } else {
        // If EEPROM has invalid IP, set the default IP and store it in EEPROM
        Serial.println("ArdMsg: Invalid IP in EEPROM. Using default IP 192.168.68.142");
        ip = IPAddress(192, 168, 68, 142);
        for (int i = 0; i < 4; i++) {
            EEPROM.write(IP_ADDRESS_START_ADDR + i, ip[i]);
        }
        Serial.print("ArdMsg: IP now set as: ");
        Serial.println(ip);
    }

// Read and print the value at FIRST_BOOT_MARKER_ADDR to the serial monitor
  byte bootMarkerValue = EEPROM.read(FIRST_BOOT_MARKER_ADDR);
  Serial.print("ArdMsg: Value at FIRST_BOOT_MARKER_ADDR: ");
  Serial.println(bootMarkerValue);

 Ethernet.begin(mac, ip);

if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("ArdMsg: Ethernet shield was not found. Sorry, can't run without hardware.");
    while (true);
}

if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("ArdMsg: Ethernet cable is not connected.");
    // Handle this situation as needed
}

  Udp.begin(localPort);
  Serial.println("ArdMsg: Start UDP.");  

// Check if it's the first boot
  if (EEPROM.read(FIRST_BOOT_MARKER_ADDR) != FIRST_BOOT_MARKER_VALUE) {
      Serial.println("ArdMsg: Writing first trigger message to EEPROM.");
      //First boot: Write default trigger message and set the marker
      writeTriggerMessageToEEPROM("port up", TRIGGER1_START_ADDR);
      Serial.println("ArdMsg: Writing first trigger message to EEPROM.");
      EEPROM.write(FIRST_BOOT_MARKER_ADDR, FIRST_BOOT_MARKER_VALUE);
      Serial.println("ArdMsg: 1st Boot Flag Set.");
    }

    // Read Relay 1 trigger messages from EEPROM
    Serial.println("ArdMsg: Start Relay 1 Trigger from EEPROM.");
    
    triggerMessage1 = readTriggerMessageFromEEPROM(TRIGGER1_START_ADDR);
    Serial.println("ArdMsg: Relay 1 Trigger from EEPROM: " + triggerMessage1);
    
    Serial.println("ArdMsg: Relay1 Trigger Set...");

    // Read Relay 2 trigger message from EEPROM
    Serial.println("ArdMsg: Start Relay 2 Trigger from EEPROM.");
    
    triggerMessage2 = readTriggerMessageFromEEPROM(TRIGGER2_START_ADDR);
    Serial.println("ArdMsg: Relay 2 Trigger from EEPROM: " + triggerMessage2);
    Serial.println("ArdMsg: Relay2 Trigger Set...");

  Serial.print("ArdMsg: Board IP address: ");
  Serial.println(Ethernet.localIP());

  pinMode(relayPin1, OUTPUT);
  digitalWrite(relayPin1, LOW);

  pinMode(relayPin2, OUTPUT);
  digitalWrite(relayPin2, LOW);

  Serial.println("ArdMsg: Setup complete. Listening for syslog messages...");
}

//****FUNCTIONS SECTION*****

// SET IP ADDRESS
void setNewIPAddress(String newIP) {
    int parts[4];
    sscanf(newIP.c_str(), "%d.%d.%d.%d", &parts[0], &parts[1], &parts[2], &parts[3]);
    ip = IPAddress(parts[0], parts[1], parts[2], parts[3]);
    Ethernet.begin(mac, ip);
    Serial.print("ArdMsg: New IP address set: ");
    Serial.println(ip);

    // Store the IP address in EEPROM
    for (int i = 0; i < 4; i++) {
        EEPROM.write(IP_ADDRESS_START_ADDR + i, parts[i]);
    }
}

//READ TRIGGER MESSAGE FROM EEPROM
String readTriggerMessageFromEEPROM(int startAddr) {
    String message = "";
    char ch;
    int addr = startAddr;
    while ((ch = EEPROM.read(addr)) != '\0') {
        message += ch;
        addr++;
    }
    return message;
}

//WRITE TRIGGER MESSAGE TO EEPROM
void writeTriggerMessageToEEPROM(String message, int startAddr) {
     for (int i = 0; i < message.length(); i++) {
        EEPROM.write(startAddr + i, message[i]);
       }
    EEPROM.write(startAddr + message.length(), '\0');  // Null terminator
    Serial.print("ArdMsg: Trigger Message to EEPROM Function Complete ");
}

//CHECK EEPROM IP ADDRESS IS NOT ALL 0 OR ALL 1
bool isValidIP(byte ip[]) {
    // Check if IP is not 0.0.0.0 or 255.255.255.255
    return !(ip[0] == 0 && ip[1] == 0 && ip[2] == 0 && ip[3] == 0) &&
           !(ip[0] == 255 && ip[1] == 255 && ip[2] == 255 && ip[3] == 255);
}

//****MAIN LOOP - GET IP ADDRESS AND TRIGGER MESSAGE FROM SERIAL CONNECTION****

void loop() {
  
  if (Serial.available()) {
    String message = Serial.readStringUntil('\n');
    if (message.startsWith("IP:")) {
      setNewIPAddress(message.substring(3));  // Extract the IP part and set it
    }
    else if (message.startsWith("Relay1:")) {
    triggerMessage1 = message.substring(7);
    Serial.println("ArdMsg: Received trigger message for Relay 1: " + triggerMessage1);

    // Store the trigger message in EEPROM
    for (int i = 0; i < triggerMessage1.length(); i++) {
        EEPROM.write(TRIGGER1_START_ADDR + i, triggerMessage1[i]);
    }
    EEPROM.write(TRIGGER1_START_ADDR + triggerMessage1.length(), '\0'); // Null terminator
} else if (message.startsWith("Relay2:")) {
    triggerMessage2 = message.substring(7);
    Serial.println("ArdMsg: Received trigger message for Relay 2: " + triggerMessage2);

    // Store the trigger message in EEPROM
    for (int i = 0; i < triggerMessage2.length(); i++) {
        EEPROM.write(TRIGGER2_START_ADDR + i, triggerMessage2[i]);
    }
    EEPROM.write(TRIGGER2_START_ADDR + triggerMessage2.length(), '\0'); // Null terminator
}

  }

//LISTEN FOR SYSLOG/UDP MESSAGES

  int packetSize = Udp.parsePacket();
  if (packetSize) {
    memset(packetBuffer, 0, BUFFER_SIZE);
    Udp.read(packetBuffer, min(packetSize, BUFFER_SIZE - 1));

    Serial.print("ArdMsg: Received message (length ");
    Serial.print(packetSize);
    Serial.print("): ");
    Serial.println(packetBuffer);
  }

//TRIGGER RELAYS

 // Check for trigger message for Relay 1
  if (strstr(packetBuffer, triggerMessage1.c_str()) && !relay1IsOn) {
    digitalWrite(relayPin1, HIGH);
    relay1IsOn = true;
    relay1OnTime = millis();
    Serial.println("Relay 1 energized due to trigger!");
    memset(packetBuffer, 0, BUFFER_SIZE);
  }

  // Check for trigger message for Relay 2
  if (strstr(packetBuffer, triggerMessage2.c_str()) && !relay2IsOn) {
    digitalWrite(relayPin2, HIGH);
    relay2IsOn = true;
    relay2OnTime = millis();
    Serial.println("Relay 2 energized due to trigger!");
    memset(packetBuffer, 0, BUFFER_SIZE);
  }

//TURN OFF RELAYS AFTER 10S

  // De-energize Relay 1 after 10 seconds
  if (relay1IsOn && (millis() - relay1OnTime >= 10000)) {
    digitalWrite(relayPin1, LOW);
    relay1IsOn = false;
    Serial.println("Relay 1 de-energized after 10 seconds!");
  }

  // De-energize Relay 2 after 10 seconds
  if (relay2IsOn && (millis() - relay2OnTime >= 10000)) {
    digitalWrite(relayPin2, LOW);
    relay2IsOn = false;
    Serial.println("Relay 2 de-energized after 10 seconds!");
  }
}

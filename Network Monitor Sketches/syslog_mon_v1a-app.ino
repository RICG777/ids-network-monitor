#include <Ethernet.h>
#include <EthernetUdp.h>

// MAC address for the board
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};

// Default IP address for the board
IPAddress ip(192, 168, 68, 142);

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

void setup() {
  Serial.begin(9600);
  while (!Serial);

  Serial.println("ArdMsg: Initializing Ethernet...");
  
  // Start Ethernet with the default IP
  Ethernet.begin(mac, ip);
  Udp.begin(localPort);

  Serial.print("ArdMsg: Board IP address: ");
  Serial.println(Ethernet.localIP());

  pinMode(relayPin1, OUTPUT);
  digitalWrite(relayPin1, LOW);

  pinMode(relayPin2, OUTPUT);
  digitalWrite(relayPin2, LOW);

  Serial.println("ArdMsg: Setup complete. Listening for syslog messages...");
}

void setNewIPAddress(String newIP) {
  int parts[4];
  sscanf(newIP.c_str(), "%d.%d.%d.%d", &parts[0], &parts[1], &parts[2], &parts[3]);
  ip = IPAddress(parts[0], parts[1], parts[2], parts[3]);
  Ethernet.begin(mac, ip);
  Serial.print("ArdMsg: New IP address set: ");
  Serial.println(ip);
}

//GET IP ADDRESS AND TRIGGER MESSAGE FROM SERIAL CONNECTION

void loop() {
  if (Serial.available()) {
    String message = Serial.readStringUntil('\n');
    if (message.startsWith("IP:")) {
      setNewIPAddress(message.substring(3));  // Extract the IP part and set it
    } else if (message.startsWith("Relay1:")) {
      triggerMessage1 = message.substring(7);
      Serial.println("ArdMsg: Received trigger message for Relay 1: " + triggerMessage1);
    } else if (message.startsWith("Relay2:")) {
      triggerMessage2 = message.substring(7);
      Serial.println("ArdMsg: Received trigger message for Relay 2: " + triggerMessage2);
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

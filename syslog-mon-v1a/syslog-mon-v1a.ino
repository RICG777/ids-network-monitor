#include <Ethernet.h>
#include <EthernetUdp.h>

// MAC address for the board (this can be any address as long as it's unique in your network)
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};

// IP address for the board
IPAddress ip(192, 168, 68, 141);

// UDP instance to let us send and receive packets over UDP
EthernetUDP Udp;

// UDP port to listen on
unsigned int localPort = 514; // Syslog typically uses port 514

// Relay pin
const int relayPin = 7; // Assuming relay 1 is connected to pin 7

// Define the buffer size - we will need to know what is the maximum length of a syslog message
#define BUFFER_SIZE 1024 
char packetBuffer[BUFFER_SIZE];

void setup() {
  // Start serial communication
  Serial.begin(9600);
  while (!Serial); // Wait for serial port to connect (needed for some boards)

  Serial.println("Initializing Ethernet...");
  
  // Start Ethernet and UDP
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // Static IP setup
    Ethernet.begin(mac, ip);
  }
  Udp.begin(localPort);

  // Print the board's IP
  Serial.print("Board IP address: ");
  Serial.println(Ethernet.localIP());

  // Set relay pin as OUTPUT
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW); // Start with relay de-energized

  Serial.println("Setup complete. Listening for syslog messages...");
}

void loop() {
  // Check if there are any incoming UDP packets
  int packetSize = Udp.parsePacket();
  if (packetSize) {
    memset(packetBuffer, 0, BUFFER_SIZE); // Clear the buffer
    Udp.read(packetBuffer, min(packetSize, BUFFER_SIZE - 1)); // Read the message, ensuring we don't exceed the buffer size

    // Print received message to serial monitor
    Serial.print("Received message (length ");
    Serial.print(packetSize);
    Serial.print("): ");
    Serial.println(packetBuffer);

    // Check for "port up" message
    if (strstr(packetBuffer, "port up")) {
      digitalWrite(relayPin, HIGH); // Energize relay
      Serial.println("Relay energized due to 'port up' message.");
    }

    // Check for "port down" message
    if (strstr(packetBuffer, "port down")) {
      digitalWrite(relayPin, LOW); // De-energize relay
      Serial.println("Relay de-energized due to 'port down' message.");
    }
  }
}


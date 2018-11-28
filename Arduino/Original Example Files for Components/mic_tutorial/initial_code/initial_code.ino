#include <MKRNB.h>
#include <Modem.h>
#include <stdio.h>

// Please enter your sensitive data in the Secret tab or arduino_secrets.h
// PIN Number
const char PINNUMBER[]     = "";

unsigned int MICUdpPort = 1234;      // local port to listen for UDP packets

IPAddress MIC_IP(172, 16, 15, 14);

// Initialize the library instance
NBClient client;
GPRS gprs;
NB nbAccess;
NBModem modemTest;
String IMSI = "";
String printOut = "";
byte packetBuffer[512];

// A UDP instance to let us send and receive packets over UDP
NBUDP Udp;

void setup() {
  String response = "";
  response.reserve(100);

  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  // Give Serial time to get ready...
  delay(2000);

  Serial.println("Starting Arduino MKR1500 send to MIC.");

  // Check if modem is ready
  NB_NetworkStatus_t modemStatus;
  for (modemStatus = nbAccess.begin(PINNUMBER); modemStatus != NB_READY; modemStatus = nbAccess.begin(PINNUMBER)) {
    Serial.println("Modem not ready, retrying in 2s...");
    delay(2000);
  }

  // Set radio technology to NB-IoT
  Serial.println("Set radio technology to NB-IoT (7 is for Cat M1 and 8 is for NB-IoT): ");
  MODEM.send("AT+URAT=8");
  MODEM.waitForResponse(100, &response);
  Serial.println(response);

  // Set APN to MIC APN
  Serial.println("Set to mda.ee: ");
  MODEM.send("AT+CGDCONT=1,\"IP\",\"mda.ee\"");
  MODEM.waitForResponse(100, &response);
  Serial.println(response);

  // Set operator to Telenor
  Serial.println("Set operator to Telenor: ");
  MODEM.send("AT+COPS=1,2,\"24201\"");
  MODEM.waitForResponse(100, &response);
  Serial.println(response);

  Serial.println("Try to connect...");
  boolean connected = false;
  while (!connected) {
    if (gprs.attachGPRS() == GPRS_READY) {
      Serial.println("Connected!");
      connected = true;
    } else {
      Serial.println("Not connected");
      delay(1000);
    }
  }

  Serial.println("\nSetup socket for connection to MIC...");
  Udp.begin(MICUdpPort);

  // Seed random number generator with noise from pin 0
  randomSeed(analogRead(0));
}

void loop() {
  int size = 0;
  
  Serial.print("Send packet to MIC: ");
  sendMICUDPpacket(MIC_IP);
  Serial.println("Check if we have received something..");
  size = receiveMICUDPpacket();
  if (size > 0) {
    Serial.println("Received packet...");
    String bufferString = String((char *) packetBuffer);
    Serial.println("Packet data is: <" + bufferString + ">");
  } else {
    Serial.println("No data received...");
  }
  // Wait 30 seconds before Sending again
  Serial.println("Wait 30s before sending again....");
  delay(30000);
}

int receiveMICUDPpacket() {
  int size = 0;
  
  size = Udp.parsePacket();
  // Check if size is larger than 0, if yes we have received something
  if ( size > 0) {
    Serial.println("packet received");
    // We've received a packet, read the data from it
    Udp.read(packetBuffer, size); // read the packet into the buffer
    return(size);
  }
}

unsigned long sendMICUDPpacket(IPAddress& address) {
  String p1, p2, p3, p4, payload = "";
  String comma = ",";
  float hum, tmp, r = 0.0;

  p1 = "Hello";
  hum = 24;
  r = random(0, 9);
  r = r / 10;
  hum = hum + r;
  p2 = hum;
  tmp = 20;
  r = random(0, 9);
  r = r / 10;
  tmp = tmp + r;
  p3 = tmp;

  payload = p1 + comma + p2 + comma + p3;
  Serial.println("payload is: " + payload);
  Udp.beginPacket(address, MICUdpPort);
  Udp.write(payload.c_str(), payload.length());
  Udp.endPacket();
}


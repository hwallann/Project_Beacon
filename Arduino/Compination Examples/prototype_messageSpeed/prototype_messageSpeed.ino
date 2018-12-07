/************************************************************
This file is the initial protoype for the Underwater Rugby Tracking IoT Device.
It uses an accelerometer and a pressure sensor to collect data of a player
and sends this data the HOARDE cloud service of Telenor.
***********************************************************************/

#define SerialPort Serial

/************************************************************
Includes and defines of NB-IoT modem
*************************************************************/
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

/************************************************************
Includes and defines of this specific program
*************************************************************/
#include <ArduinoJson.h>

int Tries = 0;
DynamicJsonBuffer jsonBuffer;
String InitalString =
    "{\"id\":1,\"type\":\"UWR\",\"time\":0,\"data\":{\"sensors\":{\"0\":{\"sensor\":\"Acceloremeter\",\"data\":{}},\"1\":{\"sensor\":\"Pressure\",\"data\":{}}}}}";
JsonObject& root = jsonBuffer.parseObject(InitalString);
int MyId = 1;
unsigned long milliTime = 0;


enum SensorArray { AccelerometerIndex , PressureIndex };

// Setup of microcontroller and device communictions
void setup() 
{
  /************************************************************
  Setup of NB-IoT Modem
  *************************************************************/
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

  /************************************************************
  Setup of Program specific
  *************************************************************/
  milliTime = millis();
}

int MessageSpeedHundreds = 0;
int MessageSpeedTens = 0;

void loop() 
{
  if (MODEM.isConnected())
  {
    if (MessageSpeedHundreds == 900){
      MessageSpeedHundreds += 10;
      MessageSpeedHundreds = MessageSpeedHundreds % 100;
    }
    if (MessageSpeedTens == 0) {
      MessageSpeedHundreds += 100;
      MessageSpeedHundreds = MessageSpeedHundreds % 1000;
    }
    // Set tries count to 0
    Tries = 0;
    // Successfully connected to the network

    // Send message to remote server
    int size = 0;
  
    Serial.print("Send packet to MIC: ");
    sendMICUDPpacket(MIC_IP, MessageSpeedIteration);

    // Wait 15 minutes before sending again
    //delay(60 * 1000);
  } else {
    // Not connected yet. Wait 5 seconds before retrying.
    
    Tries++;
    if (Tries >= 3) {
      Serial.println("Re-establishing Udp Socket");
      Tries = 0;
      Udp.stop();
      delay(10);
      Udp.begin(MICUdpPort);
    }
    
    Serial.println("Connecting...");
    delay(5000);
  }
  int MessageSpeedUp = MessageSpeedHundreds + MessageSpeedTens;
  delay(1000 - MessageSpeedUp);
  
}

unsigned long sendMICUDPpacket(IPAddress& address, unsigned long speed) {
  String* payload = String(speed);
  if (payload == NULL) {
    Serial.println("Failed to allocate memory");
    return;
  }

  Udp.beginPacket(address, MICUdpPort);
  Udp.write(payload.c_str(), payload.length());
  Udp.endPacket();
  free(payload);
}
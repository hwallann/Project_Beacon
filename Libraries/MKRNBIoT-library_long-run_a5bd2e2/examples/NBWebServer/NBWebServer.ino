/*
 NB Web Server

 A simple web server that shows the value of the analog input pins.
 using a MKR NB 1500 board.

 Circuit:
 * MKR NB 1500 board
 * Antenna
 * Analog inputs attached to pins A0 through A5 (optional)

 created 8 Mar 2012
 by Tom Igoe
*/

// libraries
#include <MKRNB.h>

#include "arduino_secrets.h" 
// Please enter your sensitive data in the Secret tab or arduino_secrets.h
// PIN Number
const char PINNUMBER[]     = SECRET_PINNUMBER;

// initialize the library instance
GPRS gprs;
NB nbAccess;     // include a 'true' parameter for debug enabled
NBServer server(80); // port 80 (http default)

// timeout
const unsigned long __TIMEOUT__ = 10 * 1000;

void setup() {
  // initialize serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // connection state
  boolean connected = false;

  // Start module
  // If your SIM has PIN, pass it as a parameter of begin() in quotes
  while (!connected) {
    if ((nbAccess.begin(PINNUMBER) == NB_READY) &&
        (gprs.attachGPRS() == GPRS_READY)) {
      connected = true;
    } else {
      Serial.println("Not connected");
      delay(1000);
    }
  }

  Serial.println("Connected to GPRS network");

  // start server
  server.begin();

  //Get IP.
  IPAddress LocalIP = gprs.getIPAddress();
  Serial.println("Server IP address=");
  Serial.println(LocalIP);
}

void loop() {
  // listen for incoming clients
  NBClient client = server.available();

  if (client) {
    while (client.connected()) {
      if (client.available()) {
        Serial.println("Receiving request!");
        bool sendResponse = false;
        while (int c = client.read()) {
          if (c == -1) {
            break;
          } else if (c == '\n') {
            sendResponse = true;
          }
        }

        // if you've gotten to the end of the line (received a newline
        // character)
        if (sendResponse) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println();
          client.println("<html>");
          // output the value of each analog input pin
          for (int analogChannel = 0; analogChannel < 6; analogChannel++) {
            client.print("analog input ");
            client.print(analogChannel);
            client.print(" is ");
            client.print(analogRead(analogChannel));
            client.println("<br />");
          }
          client.println("</html>");
          //necessary delay
          delay(1000);
          client.stop();
        }
      }
    }
  }
}

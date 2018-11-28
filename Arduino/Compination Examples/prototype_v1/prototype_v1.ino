/************************************************************
This file is the initial protoype for the Underwater Rugby Tracking IoT Device.
It uses an accelerometer and a pressure sensor to collect data of a player
and sends this data the HOARDE cloud service of Telenor.
***********************************************************************/

/************************************************************
Includes and defines of Acceloremeter
*************************************************************/
#include <SparkFunMPU9250-DMP.h>

#define SerialPort Serial

MPU9250_DMP imu;

/************************************************************
Includes and defines of Pressure Sensor
*************************************************************/
#include <Wire.h>
#include "Adafruit_MPRLS.h"

// You dont *need* a reset and EOC pin for most uses, so we set to -1 and don't connect
#define RESET_PIN  -1  // set to any GPIO pin # to hard-reset on begin()
#define EOC_PIN    -1  // set to any GPIO pin to read end-of-conversion by pin
Adafruit_MPRLS mpr = Adafruit_MPRLS(RESET_PIN, EOC_PIN);

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
  Setup of Acceloremeter Part 1 (QUAT)
  *************************************************************/

  // Call imu.begin() to verify communication and initialize
  if (imu.begin() != INV_SUCCESS)
  {
    while (1)
    {
      SerialPort.println("Unable to communicate with MPU-9250");
      SerialPort.println("Check connections, and try again.");
      SerialPort.println();
      delay(5000);
    }
  }
  
  imu.dmpBegin(DMP_FEATURE_6X_LP_QUAT | // Enable 6-axis quat
               DMP_FEATURE_GYRO_CAL, // Use gyro calibration
              10); // Set DMP FIFO rate to 10 Hz
  // DMP_FEATURE_LP_QUAT can also be used. It uses the 
  // accelerometer in low-power mode to estimate quat's.
  // DMP_FEATURE_LP_QUAT and 6X_LP_QUAT are mutually exclusive

  /************************************************************
  Setup of Acceloremeter Part 2 (BASIC)
  *************************************************************/
  
  // Use setSensors to turn on or off MPU-9250 sensors.
  // Any of the following defines can be combined:
  // INV_XYZ_GYRO, INV_XYZ_ACCEL, INV_XYZ_COMPASS,
  // INV_X_GYRO, INV_Y_GYRO, or INV_Z_GYRO
  // Enable all sensors:
  imu.setSensors(INV_XYZ_GYRO | INV_XYZ_ACCEL | INV_XYZ_COMPASS);

  // Use setGyroFSR() and setAccelFSR() to configure the
  // gyroscope and accelerometer full scale ranges.
  // Gyro options are +/- 250, 500, 1000, or 2000 dps
  imu.setGyroFSR(2000); // Set gyro to 2000 dps
  // Accel options are +/- 2, 4, 8, or 16 g
  imu.setAccelFSR(2); // Set accel to +/-2g
  // Note: the MPU-9250's magnetometer FSR is set at 
  // +/- 4912 uT (micro-tesla's)

  // setLPF() can be used to set the digital low-pass filter
  // of the accelerometer and gyroscope.
  // Can be any of the following: 188, 98, 42, 20, 10, 5
  // (values are in Hz).
  imu.setLPF(5); // Set LPF corner frequency to 5Hz

  // The sample rate of the accel/gyro can be set using
  // setSampleRate. Acceptable values range from 4Hz to 1kHz
  imu.setSampleRate(10); // Set sample rate to 10Hz

  // Likewise, the compass (magnetometer) sample rate can be
  // set using the setCompassSampleRate() function.
  // This value can range between: 1-100Hz
  imu.setCompassSampleRate(10); // Set mag rate to 10Hz

  /************************************************************
  Setup of Pressure Sensor
  *************************************************************/
  Serial.println("MPRLS Simple Test");
  if (! mpr.begin()) {
    Serial.println("Failed to communicate with MPRLS sensor, check wiring?");
  }
  Serial.println("Found MPRLS sensor");

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

void loop() 
{

  Serial.println("A");
  // Check for new data in the FIFO
  
  if ( imu.fifoAvailable() )
  {
    Serial.println("Fifo");
    // Use dmpUpdateFifo to update the ax, gx, mx, etc. values
    if ( imu.dmpUpdateFifo() == INV_SUCCESS)
    {
      Serial.println("Could Dump");
      // computeEulerAngles can be used -- after updating the
      // quaternion values -- to estimate roll, pitch, and yaw
      imu.computeEulerAngles();
      //printIMUQuatData();
      updateQuatInformation();
    }
  }
  
  Serial.println("B");
  // dataReady() checks to see if new accel/gyro data
  // is available. It will return a boolean true or false
  // (New magnetometer data cannot be checked, as the library
  //  runs that sensor in single-conversion mode.)
  if ( imu.dataReady() )
  {
    // Call update() to update the imu objects sensor data.
    // You can specify which sensors to update by combining
    // UPDATE_ACCEL, UPDATE_GYRO, UPDATE_COMPASS, and/or
    // UPDATE_TEMPERATURE.
    // (The update function defaults to accel, gyro, compass,
    //  so you don't have to specify these values.)
    imu.update(UPDATE_ACCEL | UPDATE_GYRO | UPDATE_COMPASS);
    //printIMUAccData();
    updateAcclInformation();
  }
  
  Serial.println("C");
  updateMPRLSInformation();
  Serial.println("C done");
  updateJSONTime();
  String output;
  root.printTo(output);
  Serial.println(output);
  
  //printMPRLSData();
  Serial.println("D");

  if (MODEM.isConnected())
  {
    // Set tries count to 0
    Tries = 0;
    // Successfully connected to the network

    // Send message to remote server
    int size = 0;
  
    Serial.print("Send packet to MIC: ");
    sendMICUDPpacket(MIC_IP);
    /*
    if (true == nbiot.sendString(remoteIP, REMOTE_PORT, output)) {
      Serial.println("Successfully sent data");
    } else {
      Serial.println("Failed sending data");
      Udp.stop();
      delay(10);
      Udp.begin(MICUdpPort);
    }
    */

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
  
  Serial.println("E");
  delay(1 * 100);
  
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

void updateJSONTime(void)
{
  milliTime = millis();
  root["time"] = milliTime;
}

void updateAccelerometerInformation(void)
{
  updateAcclInformation();
  updateQuatInformation();
}

void updateAcclInformation(void)
{
  Serial.println("Updating Accelerometer Information");
  String dataString;
  float accelX = imu.calcAccel(imu.ax);
  float accelY = imu.calcAccel(imu.ay);
  float accelZ = imu.calcAccel(imu.az);
  float gyroX = imu.calcGyro(imu.gx);
  float gyroY = imu.calcGyro(imu.gy);
  float gyroZ = imu.calcGyro(imu.gz);
  float magX = imu.calcMag(imu.mx);
  float magY = imu.calcMag(imu.my);
  float magZ = imu.calcMag(imu.mz);


  Serial.println("Collected Info");
  //dataString = "{\"ax\":" + (String)accelX + ",\"ay\":" + (String)accelY + ",\"az\":" + (String)accelZ + ",\"gx\":" + (String)gyroX + ",\"gy\":" + (String)gyroY + ",\"gz\":" + (String)gyroZ + ",\"mx\":" + (String)magX + ",\"my\":" + (String)magY + ",\"mz\":" + (String)magZ + "}";

  //root["data"]["sensors"][(String)AccelerometerIndex]["data"]["accl"] = dataString;
  
  root["data"]["sensors"][(String)AccelerometerIndex]["data"]["ax"] = accelX;
  root["data"]["sensors"][(String)AccelerometerIndex]["data"]["ay"] = accelY;
  root["data"]["sensors"][(String)AccelerometerIndex]["data"]["az"] = accelZ;
  /*
  root["data"]["sensors"][(String)AccelerometerIndex]["data"]["gx"] = gyroX;
  root["data"]["sensors"][(String)AccelerometerIndex]["data"]["gy"] = gyroY;
  root["data"]["sensors"][(String)AccelerometerIndex]["data"]["gz"] = gyroZ;
  root["data"]["sensors"][(String)AccelerometerIndex]["data"]["mx"] = magX;
  root["data"]["sensors"][(String)AccelerometerIndex]["data"]["my"] = magY;
  root["data"]["sensors"][(String)AccelerometerIndex]["data"]["mz"] = magZ;
  */
  Serial.println("Wrote in Info");
}

void updateQuatInformation(void)
{
  Serial.println("Updating Quaternion Information");
  String dataString;
  // After calling dmpUpdateFifo() the ax, gx, mx, etc. values
  // are all updated.
  // Quaternion values are, by default, stored in Q30 long
  // format. calcQuat turns them into a float between -1 and 1
  float q0 = imu.calcQuat(imu.qw);
  float q1 = imu.calcQuat(imu.qx);
  float q2 = imu.calcQuat(imu.qy);
  float q3 = imu.calcQuat(imu.qz);

  dataString = "{\"roll\":" + (String)imu.roll + ",\"pitch\":" + (String)imu.pitch + ",\"yaw\":" + (String)imu.pitch +"}";

  //root["data"]["sensors"][(String)AccelerometerIndex]["data"]["quat"] = dataString;
  
  root["data"]["sensors"][(String)AccelerometerIndex]["data"]["roll"] = imu.roll;
  root["data"]["sensors"][(String)AccelerometerIndex]["data"]["pitch"] = imu.pitch;
  root["data"]["sensors"][(String)AccelerometerIndex]["data"]["yaw"] = imu.yaw;
  
}

void updateMPRLSInformation(void)
{
  Serial.println("Updating Pressure Sensor Information");
  String dataString;
  float pressure_psi = mpr.readPressurePSI();
  float pressure_hPa = pressure_psi * 68.947572932;
  //float pressure_mh2o = pressure_hPa / 98.0638;
  float pressure_mh2o = pressure_psi * 0.703070;
  //dataString = "[" + (String)pressure_hPa + "," + (String)pressure_psi + "]";

  //root["data"]["sensors"][(String)PressureIndex]["data"] = dataString;
  root["data"]["sensors"][(String)PressureIndex]["data"]["hPa"] = pressure_hPa;
  root["data"]["sensors"][(String)PressureIndex]["data"]["PSI"] = pressure_psi;
  root["data"]["sensors"][(String)PressureIndex]["data"]["mH2O"] = pressure_mh2o;
}

void printMPRLSData(void)
{  
  float pressure_psi = mpr.readPressurePSI();
  float pressure_hPa = pressure_psi * 68.947572932;
  float pressure_mh2o = pressure_psi * 0.703070;
  Serial.print("Pressure (hPa): "); Serial.println(pressure_hPa);
  Serial.print("Pressure (PSI): "); Serial.println(pressure_psi);
  Serial.print("Pressure (mH2O, psi): "); Serial.println(pressure_mh2o);

  Serial.println();
}

void printIMUData(void)
{
  printIMUAccData();
  printIMUQuatData();
}

void printIMUAccData(void)
{
  // After calling update() the ax, ay, az, gx, gy, gz, mx,
  // my, mz, time, and/or temerature class variables are all
  // updated. Access them by placing the object. in front:

  // Use the calcAccel, calcGyro, and calcMag functions to
  // convert the raw sensor readings (signed 16-bit values)
  // to their respective units.
  float accelX = imu.calcAccel(imu.ax);
  float accelY = imu.calcAccel(imu.ay);
  float accelZ = imu.calcAccel(imu.az);
  float gyroX = imu.calcGyro(imu.gx);
  float gyroY = imu.calcGyro(imu.gy);
  float gyroZ = imu.calcGyro(imu.gz);
  float magX = imu.calcMag(imu.mx);
  float magY = imu.calcMag(imu.my);
  float magZ = imu.calcMag(imu.mz);
  
  SerialPort.println("Accel: " + String(accelX) + ", " +
              String(accelY) + ", " + String(accelZ) + " g");
  SerialPort.println("Gyro: " + String(gyroX) + ", " +
              String(gyroY) + ", " + String(gyroZ) + " dps");
  SerialPort.println("Mag: " + String(magX) + ", " +
              String(magY) + ", " + String(magZ) + " uT");
  SerialPort.println("Time: " + String(imu.time) + " ms");
  SerialPort.println();
}

void printIMUQuatData(void)
{  
  // After calling dmpUpdateFifo() the ax, gx, mx, etc. values
  // are all updated.
  // Quaternion values are, by default, stored in Q30 long
  // format. calcQuat turns them into a float between -1 and 1
  float q0 = imu.calcQuat(imu.qw);
  float q1 = imu.calcQuat(imu.qx);
  float q2 = imu.calcQuat(imu.qy);
  float q3 = imu.calcQuat(imu.qz);

  SerialPort.println("Q: " + String(q0, 4) + ", " +
                    String(q1, 4) + ", " + String(q2, 4) + 
                    ", " + String(q3, 4));
  SerialPort.println("R/P/Y: " + String(imu.roll) + ", "
            + String(imu.pitch) + ", " + String(imu.yaw));
  SerialPort.println("Time: " + String(imu.time) + " ms");
  SerialPort.println();
}

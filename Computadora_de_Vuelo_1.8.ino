
// Teeney 4.0 + Adafruit Ultimate GPS v3
// Copyright (C) 2021 https://www.roboticboat.uk
// 2f10b6ce-cbe4-42fa-bb40-95c06b133165
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
// These Terms shall be governed and construed in accordance with the laws of
// England and Wales, without regard to its conflict of law provisions.
/*  BMP180    Teensy 3.5

   For Wire,
   SDA       18
   SCL       19
*/

#include <Teensy_BMP180.h>
double baseline;
double bmpValues[2]; //0: Temperature, 1:Pressure
Teensy_BMP180 bmp180(&Wire);


float gpstime;
float gpsdate;
float latitude;
float longitude;
float altitude;
float gpsknots;
float gpstrack;
char latNS, lonEW;
char gpsstatus;
int fixquality;
int numsatelites;

volatile int ptr = 0;
volatile bool flag = true;
volatile char redbuffer[120];
volatile char blubuffer[120];

// Global variables
String inputSerial1 = "";         // a string to hold incoming data
boolean IsReadySerial1 = false;  // whether the string is complete

//para envio
String paquete[15];
int p = 0;
uint32_t timer = millis();
uint32_t timer2 = millis();
int led = 13;

//analog
  int sensorValue;
void setup()
{
  pinMode(led, OUTPUT);
  digitalWrite(led, HIGH);
  // Keep the User informed
  Serial.begin(38400);
  // Do not rush the startup. The GPS module needs to receive its
  // power and sort itself out before you want it running at 10Hz
  delay(1000);

  // Start up the GPS
  Serial1.begin(9600);
  Serial2.begin(115200);

  // Slowly setup the system. If it is rushed the GPS module will
  // still be running at 1Hz.
  delay(1000);

  // Change GPS baud to 38400 bps
  // Need a faster band rate than 9600 to communicate more characters
  Serial1.println("$PMTK251,38400*27");
  delay(1000);

  // Connect to the GPS module at the higher band rate
  Serial1.begin(38400);
  delay(1000);

  // 1 MHz update. Always know how to reset back
  //Serial1.println("$PMTK220,1000*1F");

  // 10MHz update
  Serial1.println("$PMTK220,100*2F"); // 10 Hz update rate
  delay(100);
  SelectSentences();

  delay(100);
  bmp180.begin();
  if (getPressure()) {
    baseline = bmpValues[1];
  }
}

void loop()
{
  /*
    Serial.print("GPS,");
    Serial.print(gpsdate, 0);
    Serial.print(",");
    Serial.print(gpstime, 0);
    Serial.print(",");
    Serial.print(latitude, 8);
    Serial.print(",");
    Serial.print(latNS);
    Serial.print(",");
    Serial.print(longitude, 8);
    Serial.print(",");
    Serial.print(lonEW);
    Serial.print(",");
    Serial.print(altitude);
    Serial.print("m H");
    Serial.print(",");
    Serial.print(fixquality);
    Serial.print(",");
    Serial.print(numsatelites);
    Serial.print(",");
    Serial.print(gpsknots);
    Serial.print("KTs");
    Serial.print(",");
    Serial.print(gpstrack);
    Serial.print(",");
    Serial.println(gpsstatus);
  */
  paquete[0] = String(0);


 // paquete[5] = String(0);
 sensorValue=(analogRead(A0)+analogRead(A0)+analogRead(A0))/3;
  paquete[6] = String(-0.000002*sensorValue*sensorValue+0.013*sensorValue-0.8776);
  paquete[7] = String(latitude, 8);
  paquete[8] = String(longitude, 8);
  paquete[9] = String(altitude);
  paquete[10] = String(numsatelites);
  paquete[13] = String(gpstime);
  paquete[11] = String(33);
  paquete[12] = String(0);
  xBee();
  if (getPressure()) {
    double altVal = bmp180.altitude(bmpValues[1], baseline);
    paquete[4] = String(bmpValues[0]);
    paquete[5] = String(bmpValues[1]);
    paquete[3] = String(altVal);
    /*    Serial.print(altVal);
        Serial.println(" cm");
            Serial.print(bmpValues[0]);
        Serial.print(" C | ");
        Serial.print(bmpValues[1]);
            Serial.print(" mb | ");
    */
  }
}
void xBee() {
  paquete[2] = String(p);
  paquete[1] = String(millis() / 1000);

  // String StrA[15] = {"Z", "T", "P", "H", "C", "K", "V", "L", "O", "A", "S", "E", "G", "M", "R"};
  if (millis() - timer2 > 10) {//aqui van 300 pero cambie a 10
    for (int i = 0; i < 15; i++)
    {

      Serial2.println("<" + String(i) + "," + paquete[i] + ">");
      //Serial.print("<" + String(i) + "," + paquete[i] + ">");
      delay(5);
    }
    timer2 = millis();
    p++;
    Serial.println("Sent");
  }
}
void serialEvent1()
{
  listen();
}

void listen()
{
  while (Serial1.available())
  {
    read(Serial1.read());
  }
}

void read(char nextChar) {

  // Start of a GPS message
  if (nextChar == '$') {
    flag ? redbuffer[ptr] = '\0' : blubuffer[ptr] = '\0';
    ptr = 0;
  }

  // End of a GPS message
  if (nextChar == '\n') {

    if (flag) {
      flag = false;

      // Set termination character of the current buffer
      redbuffer[ptr] = '\0';

      // Process the message if the checksum is correct
      if (CheckSum((char*) redbuffer )) {
        parseString((char*) redbuffer );
      }
    }
    else
    {
      flag = true;

      // Set termination character of the current buffer
      blubuffer[ptr] = '\0';

      // Process the message if the checksum is correct
      if (CheckSum((char*) blubuffer )) {
        parseString((char*) blubuffer );
      }
    }
    ptr = 0;
  }

  // Add a new character
  flag ? redbuffer[ptr] = nextChar : blubuffer[ptr] = nextChar;

  // Check we stay within allocated memory
  if (ptr < 119) ptr++;
}


bool CheckSum(char* msg) {

  // Check the checksum
  //$GPGGA,.........................0000*6A

  // Length of the GPS message
  int len = strlen(msg);

  // Does it contain the checksum, to check
  if (msg[len - 4] == '*')
  {

    // Read the checksum from the message
    int cksum = 16 * Hex2Dec(msg[len - 3]) + Hex2Dec(msg[len - 2]);

    // Loop over message characters
    for (int i = 1; i < len - 4; i++)
    {
      cksum ^= msg[i];
    }

    // The final result should be zero
    if (cksum == 0)
    {
      return true;
    }
  }

  return false;
}


float DegreeToDecimal(float num, byte sign)
{
  // Want to convert DDMM.MMMM to a decimal number DD.DDDDD

  int intpart = (int) num;
  float decpart = num - intpart;

  int degree = (int)(intpart / 100);
  int mins = (int)(intpart % 100);

  if (sign == 'N' || sign == 'E')
  {
    // Return positive degree
    return (degree + (mins + decpart) / 60);
  }

  // Return negative degree
  return -(degree + (mins + decpart) / 60);
}


void parseString(char* msg) {

  messageGGA(msg);
  messageRMC(msg);
}

void messageGGA(char* msg)
{
  // Ensure the checksum is correct before doing this
  // Replace all the commas by end-of-string character '\0'
  // Read the first string
  // Knowing the length of the first string, can jump over to the next string
  // Repeat the process for all the known fields.

  // Do we have a GGA message?
  if (!strstr(msg, "GGA")) return;

  // Length of the GPS message
  int len = strlen(msg);

  // Replace all the commas with end character '\0'
  for (int j = 0; j < len; j++) {
    if (msg[j] == ',' || msg[j] == '*') {
      msg[j] = '\0';
    }
  }

  // Allocate working variables
  int i = 0;

  //$GPGGA

  // GMT time  094728.000
  i += strlen(&msg[i]) + 1;
  gpstime = atof(&msg[i]);

  // Latitude
  i += strlen(&msg[i]) + 1;
  latitude = atof(&msg[i]);

  // North or South (single char)
  i += strlen(&msg[i]) + 1;
  latNS = msg[i];
  if (latNS == '\0') latNS = '.';

  // Longitude
  i += strlen(&msg[i]) + 1;
  longitude = atof(&msg[i]);

  // East or West (single char)
  i += strlen(&msg[i]) + 1;
  lonEW = msg[i];
  if (lonEW == '\0') lonEW = '.';

  // Fix quality (1=GPS)(2=DGPS)
  i += strlen(&msg[i]) + 1;
  fixquality = atof(&msg[i]);

  // Number of satellites being tracked
  i += strlen(&msg[i]) + 1;
  numsatelites = atoi(&msg[i]);

  // Horizontal dilution of position
  i += strlen(&msg[i]) + 1;

  // Altitude
  i += strlen(&msg[i]) + 1;
  altitude = atof(&msg[i]);

  // Height of geoid (mean sea level)
  i += strlen(&msg[i]) + 1;

  // Time in seconds since last DGPS update
  i += strlen(&msg[i]) + 1;

  // DGPS station ID number
  i += strlen(&msg[i]) + 1;

  // Convert from degrees and minutes to degrees in decimals
  latitude = DegreeToDecimal(latitude, latNS);
  longitude = DegreeToDecimal(longitude, lonEW);
}


void messageRMC(char* msg)
{
  // Ensure the checksum is correct before doing this
  // Replace all the commas by end-of-string character '\0'
  // Read the first string
  // Knowing the length of the first string, can jump over to the next string
  // Repeat the process for all the known fields.

  // Do we have a RMC message?
  if (!strstr(msg, "RMC")) return;

  // Length of the GPS message
  int len = strlen(msg);

  // Replace all the commas with end character '\0'
  for (int j = 0; j < len; j++) {
    if (msg[j] == ',' || msg[j] == '*') {
      msg[j] = '\0';
    }
  }

  // Allocate working variables
  int i = 0;

  //$GPRMC

  // GMT time  094728.000
  i += strlen(&msg[i]) + 1;
  gpstime = atof(&msg[i]);

  // Status A=active or V=Void.
  i += strlen(&msg[i]) + 1;
  gpsstatus = msg[i];

  // Latitude
  i += strlen(&msg[i]) + 1;
  latitude = atof(&msg[i]);

  // North or South (single char)
  i += strlen(&msg[i]) + 1;
  latNS = msg[i];
  if (latNS == '\0') latNS = '.';

  // Longitude
  i += strlen(&msg[i]) + 1;
  longitude = atof(&msg[i]);

  // East or West (single char)
  i += strlen(&msg[i]) + 1;
  lonEW = msg[i];
  if (lonEW == '\0') lonEW = '.';

  // // Speed over the ground in knots
  i += strlen(&msg[i]) + 1;
  gpsknots = atof(&msg[i]);

  // Track angle in degrees True North
  i += strlen(&msg[i]) + 1;
  gpstrack = atof(&msg[i]);

  // Date - 31st of March 2018
  i += strlen(&msg[i]) + 1;
  gpsdate = atof(&msg[i]);

  // Magnetic Variation

  // Convert from degrees and minutes to degrees in decimals
  latitude = DegreeToDecimal(latitude, latNS);
  longitude = DegreeToDecimal(longitude, lonEW);
}


// Convert HEX to DEC
int Hex2Dec(char c) {

  if (c >= '0' && c <= '9') {
    return c - '0';
  }
  else if (c >= 'A' && c <= 'F') {
    return (c - 'A') + 10;
  }
  else {
    return 0;
  }
}

void AllSentences() {

  // All sentences
  Serial1.println("$PMTK314,1,1,1,1,1,5,0,0,0,0,0,0,0,0,0,0,0,0,0*2C");
}

void SelectSentences() {

  // Select RMC and GGA sentences
  Serial1.println("$PMTK314,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*28");
}

void SelectGGAonly() {

  // Select GGA sentences
  Serial1.println("$PMTK314,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*29");
}

void setBandRate() {

  // Set 38400 bps
  Serial1.println("$PMTK251,38400*27");
  delay(1000);
}

void Update1MHz() {

  // 1MHz update
  Serial1.println("$PMTK220,1000*1F");
}

void Update10MHz() {

  // 10MHz update
  Serial1.println("$PMTK220,100*2F");
}
boolean getPressure()
{
  if (bmp180.startTemperature() != 0)
  {
    if (bmp180.getTemperature(bmpValues[0]) != 0)
    {
      if (bmp180.startPressure() != 0)
      {
        if (bmp180.getPressure(bmpValues[1],bmpValues[0]) != 0)
        {
          return(1);
        }
      }
    }
  }
  return(0);
}

#include <Adafruit_GPS.h>
#include <SoftwareSerial.h>
#include <Adafruit_BMP085.h>
Adafruit_BMP085 bmp;

SoftwareSerial mySerial(8, 7);
Adafruit_GPS GPS(&mySerial);
#define GPSECHO false
uint32_t timer = millis();
uint32_t timer2 = millis();
int p=0;
SoftwareSerial XBee(2, 3);
int hora=0;
int minutos=0;
int segundos=0;
int mili=0;
String paquete[15];//orden paquetes tinst,paquete,altitud,tatm,presion,Voltaje,lat,long,alt,sats,tiempogps,etapa.
void setup() {
  Serial.begin(115200);
  delay(5000);
  GPS.begin(9600);
  XBee.begin(9600);
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);
  GPS.sendCommand(PGCMD_ANTENNA);
  mySerial.println(PMTK_Q_RELEASE);
  Serial.println("pac");
}

void loop(){
  mySerial.listen();
  GPSFN();
  BMPRead();
  XBEE();
}

void XBEE(){
  paquete[2]=p;
  paquete[1]=millis()/1000;
 // String StrA[15] = {"Z", "T", "P", "H", "C", "K", "V", "L", "O", "A", "S", "E", "G", "M", "R"};
    if (millis() - timer2 > 2000) {
      for(int i = 0; i < 15; i++)
{
  XBee.print("<"+String(i)+","+paquete[i]+">");
  delay(50);
}
  timer2 = millis();
  p++;
}
}
void BMPRead(){

}
void GPSFN(){
  String horam;
  String minutom;
  String segundosm;//variables destructivas
    char c = GPS.read();
  if (GPS.newNMEAreceived()) {
    if (!GPS.parse(GPS.lastNMEA()))
      return; 
  }
  if (millis() - timer > 2000) {
    timer = millis();
    if (GPS.hour < 10) { horam ='0'; }
    paquete[12]= horam + String(GPS.hour,DEC);//imprime horas
    if (GPS.minute < 10) { minutom='0'; }
    paquete[13]=minutom + String(GPS.minute, DEC);//imprime minutos
    if (GPS.seconds < 10) { segundosm='0'; }
    paquete[14]=segundosm + String(GPS.seconds, DEC);//imprime segundos

      if (GPS.fix) {
             paquete[9]=GPS.altitude,1;
    paquete[10]=String((int)GPS.satellites);
          //Serial.print("Satellites: ");Serial.print(paquete[12]); Serial.println((int)GPS.satellites);
        paquete[0]=1;
      paquete[7]=(String(GPS.latitude, 4)+ String(GPS.lat,4));
      paquete[8]=(String(GPS.longitude,4)+String(GPS.lon,4));
      //Serial.print("Speed (knots): "); Serial.println(GPS.speed);

    }
  }
}

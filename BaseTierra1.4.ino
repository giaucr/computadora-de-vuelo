
#include<SoftwareSerial.h>
SoftwareSerial zigbee(11, 12);

const byte numChars = 32;
char receivedChars[numChars];
char tempChars[numChars]; 
char messageFromPC[numChars] = {0};
int integerFromPC = 0;
float floatFromPC = 0.00;
String Valuelist[15]={"Paquetes rcv","Tiempo instantaneo","Numero de paquete","Altitud BMP","TAtmosferica BMP","Presion","Tension de la bateria","Longitud","Latitud","Altitud GPS","Satelites GPS","Etapa de vuelo","GPS Hora","GPS Minutos","GPS Segundos"};
boolean newData = false;
float Floatlist[15];
int RCV=-1;


void setup() {
    Serial.begin(9600);
      Serial3.begin(9600);
    Serial.println("Listo");
}


void loop() {
    recvWithStartEndMarkers();
    if (newData == true) {
        strcpy(tempChars, receivedChars);

        parseData();
        //showParsedData();
            parX();
        newData = false;
    }

}
void parX(){
  int messageFromP=atoi(messageFromPC);
if ((messageFromP >=0) && (messageFromP<= 14)){// no permite que se exceda
    String propiedad;
    propiedad=Valuelist[messageFromP]+": ";
     Floatlist[messageFromP]=floatFromPC;
   // Serial.print(propiedad);
    //Serial.println(Floatlist[messageFromP]);
if (messageFromP==14){RCV++;Floatlist[0]=RCV;handoff();}
}
else{(Serial.println("error de paquete"));}
}

void handoff(){
/*
        for(int n = 0; n < 15; n++)
{
  Serial.print(Valuelist[n]);
        Serial.print(": ");
      Serial.println(Floatlist[n],10);
}
*/
Serial.println(Floatlist[6],10);
}

void recvWithStartEndMarkers() {
    static boolean recvInProgress = false;
    static byte ndx = 0;
    char startMarker = '<';
    char endMarker = '>';
    char rc;

    while (Serial3.available() > 0 && newData == false) {
        rc = Serial3.read();

        if (recvInProgress == true) {
            if (rc != endMarker) {
                receivedChars[ndx] = rc;
                ndx++;
                if (ndx >= numChars) {
                    ndx = numChars - 1;
                }
            }
            else {
                receivedChars[ndx] = '\0';
                recvInProgress = false;
                ndx = 0;
                newData = true;
            }
        }

        else if (rc == startMarker) {
            recvInProgress = true;
        }
    }
}

//============

void parseData() {  

    char * strtokIndx; 

    strtokIndx = strtok(tempChars,",");  
    strcpy(messageFromPC, strtokIndx); 
 
    strtokIndx = strtok(NULL, ",");
    floatFromPC = atof(strtokIndx); 


}

//============

void showParsedData() {
    Serial.print("Message ");
    Serial.println(messageFromPC);
    Serial.print("Integer ");
    Serial.println(floatFromPC,4);

}

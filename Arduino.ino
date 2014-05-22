
#include <SD.h>
#include <Wire.h>
#include "RTClib.h"
 
RTC_DS1307 RTC;

// PIN Konstanten
const int chipSelect = 10;
const int analogPin1 = A1;
const int analogPin2 = A2;
const int SensorPowerPin = 6;
const int rpiIN = 4;
const int Relay = 3;

// Konstanten
const int Grenzwert = 200;
const int anzMessungen = 50;
const unsigned long pause = 300000;

// Variablen
int RelayVariable = 0;
unsigned long RPiKontrolle = 0;
int Sensor1 = 0;
int Sensor2 = 0;

void setup()
{
  pinMode(rpiIN, INPUT);
  pinMode(Relay, OUTPUT);
  pinMode(SensorPowerPin, OUTPUT);
  digitalWrite(SensorPowerPin, LOW);
  digitalWrite(Relay, LOW);
  Serial.begin(9600);
  Wire.begin();
  RTC.begin();
  while (!Serial) {
    ;
  }
  pinMode(10, OUTPUT);
  if (!SD.begin(chipSelect)) {
    return;
  }  
}


void loop()
{
  DateTime now = RTC.now();
  String dataString1 = "";
  int RPiLesen = digitalRead(rpiIN);
    
   int Stunde = now.hour();
   int Minute = now.minute();
   int helper = 0;
   Sensor1 = 0;
   Sensor2 = 0;
   
   digitalWrite(SensorPowerPin, HIGH);
   delay(100);
   for (int i = 0; i < anzMessungen; i++)
   {
     helper = analogRead(analogPin1);
     Sensor1 = Sensor1 + helper;
     delay(50);
     //helper = analogRead(analogPin2);
     //Sensor2 = Sensor2 + helper;
     //delay(50);
   }
   digitalWrite(SensorPowerPin, LOW);
   
   Sensor1 = Sensor1 / anzMessungen;
   //Sensor2 = Sensor2 / anzMessungen;
   helper = 0;
   
   if (Sensor2 < 0 )
   {
     Sensor2 = (-1) * Sensor2;
     helper = helper + 200;
   }
   
   //check ob wir gerade im Gieß-Zeitfenster sind
   if(Stunde > 6 && Stunde < 8 || Stunde > 18 && Stunde < 20)
   {
     //check obs trocken ist
     //if (Sensor1 < Grenzwert && Sensor2 < Grenzwert)
     if (Sensor2 < Grenzwert)
     {
       //gießen; Zeitfenster + trocken
       helper += 1;
       RelayVariable = 1;
     }
     else
     {
       //nicht gießen; Zeitfenster + nicht trocken
       helper += 2;
       RelayVariable = 0;
     }
   }
   else
   {
     //nicht gießen; kein Zeitfenster
     helper += 3;
     RelayVariable = 0;
   }
   
   //zurücksetzen des manuellen-gießen-Zählers um Mitternacht (fast)
   if (Stunde == 23 && Minute == 59)
   {
     RPiKontrolle = 0;
   }
   
   //manuell gießen, aber nur, wenn nicht schon vom Zeitfenster+trocken gegossen wird
   //und max 3*1200 sek pro Tag
   if (RPiKontrolle < 3600000 / pause && RelayVariable == 0 && RPiLesen == 1)
   {
     helper += 100;
     RPiKontrolle += 1;
     RelayVariable = 1;
   }
   
   if (RelayVariable == 1)
    {
    digitalWrite(Relay, HIGH);
    }
   else
    {
     digitalWrite(Relay, LOW);
    }   
   
  //datenstring fürs logfile zusammenstellen
  dataString1 += now.year();
  dataString1 += " ";
  dataString1 += now.month();
  dataString1 += " ";
  dataString1 += now.day();
  dataString1 += " ";
  dataString1 += now.hour();
  dataString1 += " ";
  dataString1 += now.minute();
  dataString1 += " ";
  dataString1 += now.second();
  dataString1 += " ";
  dataString1 += Sensor1;
  dataString1 += " ";
  dataString1 += Sensor2;
  dataString1 += " ";
  dataString1 += RelayVariable*150;
  dataString1 += " ";
  dataString1 += RPiLesen*50;
  dataString1 += " ";
  dataString1 += Grenzwert;
  dataString1 += " ";
  dataString1 += RPiKontrolle*pause/60000;
  dataString1 += " ";
  dataString1 += helper;
  
  //datastring1 Überschriften
  // Jahr Monat Tag Stunde Minute Sekunde Sensor1 Sensor2 gießen RPiIN Grenzwert ManuellGießenZähler Gießgrund
  //
  //Gießgrund:  1 Zeitfenster + trocken
  //            2 Zeitfenster + nicht trocken
  //            3 kein Zeitfenster
  //            +100 manuell gießen
  //            +200 Messwert war negativ
  Serial.println(dataString1);
  
  File dataFile = SD.open("datalog.txt", FILE_WRITE);
  if (dataFile) 
  {
    dataFile.println(dataString1);
    dataFile.close();
  }  
  else {
    Serial.println("error opening datalog.txt");
  } 
  delay(pause);
}










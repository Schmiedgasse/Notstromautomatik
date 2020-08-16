#include <Servo.h>
//#include <notstart.h>

Servo choke;
const int anfpos = 90; //Anfangsposition des Servos
const int endpos = 110; //Endposition des Servos)
int pos = anfpos;
const unsigned long chokezeit = 10000; // Zeit die der Choke offen sein soll, eine Sekunde = 1000
int anlassversuch = 0;
const int LEDPin = 13; // LED
const int SchalterPinAn = 2; // Schalter
const int SchalterFunkAn = 3; // Schalter Funk
int StatusFunk = 1; // Status Funkschalter
const int SchalterPinAus = 4; // Schalter
const int ServoPin = 9; // Hier hinter steckt ein Servo
const int AnlasserPin = 7; // Relais für den Anlasser
const char SpannungsPin = A1; // Spannungssensor Anlasserbatterie
const char VBattPin = A0; // Spannungssensor Versorgungsbatterie
const int IgnitionPin = 10; // Relais Zündung ein


// Wert kann angepasst werden an die Spannung die der Arduino wirklich hat.
// const float arduino5v = 4.88;
// Wie lange soll der Anlasser orgeln
const int anlassertime = 3000; // Zeit die der Anlasser orgeln soll

//Zeitstempel für Parallelverarbeitung
unsigned long chokestart;
unsigned long currentmillis;

// Boolean Werte für Statiänderungen
bool anschalten = false;
bool ausschalten = false;
bool laeuft = false;
bool needchoke = false;
boolean LEDvalue = LOW;


void setup() {

  // Serielle Konsole starten
  Serial.begin(9600);

  // Pins bestimmen
  pinMode(LEDPin, OUTPUT);
  pinMode(SchalterPinAn, INPUT);
  pinMode(SchalterFunkAn, INPUT);
  pinMode(SchalterPinAus, INPUT);
  pinMode(AnlasserPin, OUTPUT);
  pinMode(IgnitionPin, OUTPUT);

  // Zeitstempel erfassen


  // Servo benennen und an Pin 9 (PWM-Pin) anhängen
  choke.attach(ServoPin);

  //Servo auf Anfangszustand zurücksetzen
  Serial.print("Choke auf Anfangsstellung ");
  Serial.print(anfpos);
  Serial.print(" zurücksetzen");
  Serial.println(" ");
  choke.write(anfpos);
  Serial.println("Ready to Go....");


}

void loop() {
  // Variablendeklaration
//  int kaltstart = 0;
//  int VBattVoltage = analogRead(VBattPin);
  // read the input on analog pin 0:
  // Vergleichswerte für AnlasserBatterie
//  int idlespannung = analogRead(SpannungsPin);
//  int limaspannung = 0;
  //int volt;
currentmillis = millis();  

  // Wenn der Anschalter zum ersten Mal gedrückt wird
  if (digitalRead(SchalterPinAn) == 1) {
    anschalten = true;
//   ausschalten = false;
    Serial.println("Es wurde angeschaltet");
  }

  // Wenn der Anschalter zum ersten Mal gedrückt wird
  if (digitalRead(SchalterFunkAn) == 0 && StatusFunk == 0) {
    anschalten = true;
//   ausschalten = false;
    Serial.println("Es wurde ueber Funk angeschaltet");
      Serial.print(digitalRead(SchalterFunkAn));
      Serial.print(StatusFunk);
      Serial.println(" ");
    StatusFunk = 1;
  }

  // Wenn der Anschalter zum ersten Mal gedrückt wird
  if (digitalRead(SchalterFunkAn) == 1 && StatusFunk == 1) {
    ausschalten = true;
//   ausschalten = false;
    Serial.println("Es wurde ueber Funk ausgeschaltet");
      Serial.print(digitalRead(SchalterFunkAn));
      Serial.print(StatusFunk);
      Serial.println(" ");
    StatusFunk = 0;
    }


  // Wenn der Ausschalter gedrückt wird
  if (digitalRead(SchalterPinAus) == 1) {
    ausschalten = true;;
//    anschalten = false;;
    Serial.println("Es wurde ausgeschaltet");
  }

  // Anlassen vom Motor


if (anschalten == true && laeuft == false) {
  if (!digitalRead(LEDPin)) {
      // LED-Ausgabe auf AN
      digitalWrite(LEDPin, HIGH);
      Serial.println("LED geht AN");
      if (laeuft == false) {
        needchoke = true;
        chokestart = millis();
        Serial.println("Choke auf machen");
      }  
//laeuft = true;
ausschalten = false;
anschalten = false;
}
}


// Hier soll der Choke bewegt werden vor dem Start
if (needchoke == true && laeuft == false) {
  
      while (pos < endpos) { // goes from 0 degrees to 90 degrees
        // in steps of 1 degree
        choke.write(pos);              // tell servo to go to position in variable 'pos'
        pos++;
        delay(15);                       // waits 15ms for the servo to reach the position
 //       Serial.println(pos);
      } 
  
}

// Hier soll der Choke wieder geschloßen werden wenn ne gewisse Zeit erreicht ist.

if ( needchoke == true && laeuft == false && currentmillis > (chokestart + chokezeit) ) {

   needchoke = false;
   laeuft = true;
   Serial.println("Choke zu machen");

  
}

// Läuft die Kiste soll der Choke natürlich auch zugehen.

if (needchoke == false && laeuft == true) {
      
      while (pos > anfpos) { // goes from 0 degrees to 90 degrees
        // in steps of 1 degree
        choke.write(pos);              // tell servo to go to position in variable 'pos'
        pos--;
        delay(15);                       // waits 15ms for the servo to reach the position
 //       Serial.println(pos);
      } 
//      needchoke = !needchoke;
//      laeuft = true;
  
}



if (ausschalten == true && laeuft == true) {
  if (digitalRead(LEDPin)) {
      // LED-Ausgabe auf AN
      digitalWrite(LEDPin, LOW);
      Serial.println("LED geht AUS");
  
laeuft = false;
ausschalten = false;
needchoke = false;

}


}

/*
// Nur zum Testen!
Serial.print("Läuft: ");
Serial.print(laeuft);
Serial.println(" ");
Serial.print("Anschalten: ");
Serial.print(anschalten);
Serial.println(" ");
Serial.print("Ausschalten: ");
Serial.print(ausschalten);
Serial.println(" ");
Serial.print("Needchoke: ");
Serial.print(needchoke);
Serial.println(" ");
delay(5000);
*/


/*
    //Langsam den Choke aufmachen. Wir machen den Choke vor jedem Startvorgang auf

 //   if (kaltstart == 0) {
      lastMillis1 = millis();
      Serial.println("Kaltstart, Choke aufmachen");
      while (pos < 90) { // goes from 0 degrees to 90 degrees
        // in steps of 1 degree
        choke.write(pos);              // tell servo to go to position in variable 'pos'
        pos++;
        delay(15);                       // waits 15ms for the servo to reach the position
        //        Serial.println(pos);
 //     }
    }
 //   kaltstart = 1;

    // Startversuch
    Serial.println("Es wird gestartet");
    lastMillis2 = millis();
    digitalWrite(AnlasserPin, HIGH);
    delay(anlassertime);
    digitalWrite(AnlasserPin, LOW);
    // direkt nach dem Startversuch den Choke zurückdrehen
    if (pos > 0) {
        Serial.println("Choke wieder zu");
       while (pos > 0) {
          choke.write(pos);
          pos--;
          delay(15);
        }

    // Um die Schleife abzubrechen wenn Aus gedrückt wird müssen wir natürlich innerhalb der
    // Schleife den Status des Ausschalters überwachen.

    // Wenn der Ausschalter gedrückt wird
    if (digitalRead(SchalterPinAus) == 1) {
      ausschalten = true;
      anschalten = false;
      Serial.println("Es wurde ausgeschaltet");

    }
    // Nach kurzer Wartezeit überprüfen ob der Motor läuft
    delay(5000);
    limaspannung = analogRead(SpannungsPin);
    Serial.print("Limaspannung Sensorwert liegt bei");
    Serial.print(limaspannung);
    Serial.println("");
   // float voltage = ((limaspannung / 5 * arduino5v) / 4.092) ; //Spannungsteiler. Spannung meines Arduino ist bei 4.76 Volt
   // volt = (int)voltage;
   // voltage = (volt / 10.0);
   // // voltage hat den Spannungswert intus
   // Serial.print("Aktuelle Spannung liegt bei");
   // Serial.print(voltage);
   // Serial.print("Volt");
   // Serial.println("");
    // Prüfen anhand der Lima-Spannung ob der Motor läuft.
    if (limaspannung >= (idlespannung + 10)) {
      // sorgt dafür das die Anschalten Schleife verlassen wird
      Serial.println("Spannung höher als vor Start, Motor läuft");
      anschalten = false;
      laeuft = true;
    }
    } else {
      // Falls Spannung nicht größer als vor dem Anlassvorgang und Motor nicht läuft neuer Startversuch
      // Nur 3 mal soll angelassen werden und nicht endlos.
      if (anlassversuch == 3) {
        anschalten = false;
        Serial.println("Abbruch nach 3 erfolglosen Anlassversuchen");
      } else {
        // 10 Sekunden warten vor dem nächsten Startversuch
        Serial.println("Erfolgloser Anlassversuch, Zähler hoch 10 Sekunden warten und nochmal");
        delay(10000);
        anlassversuch++;
      }
    }


  }

  if (laeuft) {
    if (millis() > lastMillis2 + 1000) {
      LEDvalue = !LEDvalue;
      lastMillis2 = millis();
    
    digitalWrite(LEDPin, LEDvalue);
    }



  }


  // Hier wird alles wieder ausgeschaltet. Wenn jemand den Ausschalter betätigt hat.

  while (ausschalten) {
    laeuft = false;
    if (digitalRead(LEDPin)) {
      // LED-Ausgabe auf AUS
      digitalWrite(LEDPin, LOW);
      Serial.println("LED geht AUS");
    }

    //Zündung ausschalten zum Motor ausschalten
    
    digitalWrite(IgnitionPin, LOW);
    Serial.println("Zündung ausgeschaltet");

    // Wenn der Choke nicht auf Null steht, dann werden wir den Choke zurückfahren.

    if (pos > 0) {
      Serial.println("Choke noch offen wird zurückgefahren");
      //    for (pos = 90; pos >= 0; pos -= 1) {
      //      choke.write(pos);
      //      delay(15);
      //    }
      while (pos > 0) {
        choke.write(pos);
        pos--;
        delay(15);
      }
    } else {
      Serial.println("Choke war schon zu, nichts zu tun");
    }




    if (digitalRead(SchalterPinAn) == 1) {
      anschalten = true;
      Serial.println("Es wurde angeschaltet");
    }


    ausschalten = false;
  }

*/

}

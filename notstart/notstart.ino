#include <Servo.h>
//#include <notstart.h>

Servo choke;
int pos = 0;
int anlassversuch = 0;
const int LEDPin = 13;
const int SchalterPinAn = 2;
const int SchalterPinAus = 4;
const int ServoPin = 9;
const int AnlasserPin = 7;
const char SpannungsPin = A1;
const char VBattPin = A0;
// Wert kann angepasst werden an die Spannung die der Arduino wirklich hat.
const float arduino5v = 4.88;

//Zeitstempel für Parallelverarbeitung
unsigned long lastMillis1;
unsigned long lastMillis2;

// Boolean Werte für Statiänderungen
bool anschalten = false;
bool ausschalten = false;
bool laeuft = false;
boolean LEDvalue = LOW;


void setup() {

  // Serielle Konsole starten
  Serial.begin(9600);

  // Pins bestimmen
  pinMode(LEDPin, OUTPUT);
  pinMode(SchalterPinAn, INPUT);
  pinMode(SchalterPinAus, INPUT);
  pinMode(AnlasserPin, OUTPUT);


  // Zeitstempel erfassen
  lastMillis1 = millis();
  lastMillis2 = millis();


  // Servo benennen und an Pin 9 (PWM-Pin) anhängen
  choke.attach(ServoPin);

  //Servo auf Anfangszustand zurücksetzen
  Serial.println("Choke auf Anfangsstellung zurücksetzen");
  choke.write(pos);
  Serial.println("Ready to Go....");


}

void loop() {
  // Variablendeklaration
  int kaltstart = 0;
  int VBattVoltage = analogRead(VBattPin);
  // read the input on analog pin 0:
  int limaspannung = analogRead(SpannungsPin);
  int volt;


  // Wenn der Anschalter zum ersten Mal gedrückt wird
  if (digitalRead(SchalterPinAn) == 1) {
    anschalten = true;
    Serial.println("Es wurde angeschaltet");
  }

  // Wenn der Ausschalter gedrückt wird
  if (digitalRead(SchalterPinAus) == 1) {
    ausschalten = true;;
    Serial.println("Es wurde ausgeschaltet");
  }



  // Anlassen vom Motor

  while (anschalten) {
    // Prüfen ob LED schon an
    if (!digitalRead(LEDPin)) {
      // LED-Ausgabe auf AN
      digitalWrite(LEDPin, HIGH);
      Serial.println("LED geht AN");
    }
    //Langsam den Choke aufmachen

    if (kaltstart == 0) {
      lastMillis1 = millis();
      Serial.println("Kaltstart, Choke aufmachen");
      while (pos < 90) { // goes from 0 degrees to 90 degrees
        // in steps of 1 degree
        choke.write(pos);              // tell servo to go to position in variable 'pos'
        pos++;
        delay(15);                       // waits 15ms for the servo to reach the position
        //        Serial.println(pos);
      }
    }
    kaltstart = 1;

    // Startversuch
    Serial.println("Es wird gestartet");
    lastMillis2 = millis();
    digitalWrite(AnlasserPin, HIGH);
    delay(7000);
    digitalWrite(AnlasserPin, LOW);



    // Um die Schleife abzubrechen wenn Aus gedrückt wird müssen wir natürlich innerhalb der
    // Schleife den Status des Ausschalters überwachen.

    // Wenn der Ausschalter gedrückt wird
    if (digitalRead(SchalterPinAus) == 1) {
      ausschalten = true;
      anschalten = false;
      Serial.println("Es wurde ausgeschaltet");

    }
    // Überprüfen ob der Motor läuft
    limaspannung = analogRead(SpannungsPin);
    Serial.print("Limaspannung Sensorwert liegt bei");
    Serial.print(limaspannung);
    Serial.println("");
    float voltage = ((limaspannung / 5 * arduino5v) / 4.092) ; //Spannungsteiler. Spannung meines Arduino ist bei 4.76 Volt
    volt = (int)voltage;
    voltage = (volt / 10.0);
    // voltage hat den Spannungswert intus
    Serial.print("Aktuelle Spannung liegt bei");
    Serial.print(voltage);
    Serial.print("Volt");
    Serial.println("");
    // Prüfen anhand der Lima-Spannung ob der Motor läuft.
    if (voltage > 12) {
      // sorgt dafür das die Anschalten Schleife verlassen wird
      Serial.println("Limaspannung erreicht, Motor läuft");
      anschalten = false;
      laeuft = true;
      if (pos > 0) {
        Serial.println("Motor läuft, Choke wieder zu");
        // Nachdem der Motor läuft soll der Choke wieder zugemacht werden.

        while (pos > 0) {
          choke.write(pos);
          pos--;
          delay(15);
        }
      }
    } else {
      // Falls Limaspannung kleiner 12 und Motor nicht läuft neuer Startversuch
      // Nur 3 mal soll angelassen werden und nicht endlos.
      if (anlassversuch == 3) {
        anschalten = false;
        Serial.println("Abbruch nach 3 erfolglosen Anlassversuchen");
      } else {
        // 10 Sekunden warten vor dem nächsten Startversuch
        Serial.println("Erfolgloser Anlassversucht, Zähler hoch 10 Sekunden warten und nochmal");
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



}

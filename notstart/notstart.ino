#include <Servo.h>
//#include <notstart.h>

Servo choke;
int pos = 0;
int LEDPin = 13;
int SchalterPinAn = 2;
int SchalterPinAus = 4;
int ServoPin = 9;
int AnlasserPin = 10;
int SpannungsPin = 11;
int VSpannungsPin = 12;

//Zeitstempel für Parallelverarbeitung
unsigned long lastMillis1;
unsigned long lastMillis2;

// Boolean Werte für Statiänderungen
bool anschalten = false;
bool ausschalten = false;
bool laeuft = false;


void setup() {

  // Serielle Konsole starten
  Serial.begin(9600);

  // Pins bestimmen
  pinMode(LEDPin, OUTPUT);
  pinMode(SchalterPinAn, INPUT);
  pinMode(SchalterPinAus, INPUT);
  pinMode(AnlasserPin, OUTPUT);
  pinMode(SpannungsPin, INPUT);


  // Zeitstempel erfassen
  lastMillis1 = millis();
  lastMillis2 = millis();


  // Servo benennen und an Pin 9 (PWM-Pin) anhängen
  choke.attach(ServoPin);

  //Servo auf Anfangszustand zurücksetzen
  Serial.println("Choke auf Anfangsstellung zurücksetzen");
  choke.write(pos);
}

void loop() {
  // Variablendeklaration
  int kaltstart = 0;

  // Schalterzustand mit was initialisieren was normal nicht vorkommt, damit erst dann wenn
  // der Schalter gedrückt wird etwas getan wird.

  // put your main code here, to run repeatedly:
  // Schalterzustand = digitalRead(SchalterPin);
  //  Serial.println(Schalterzustand,DEC);

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



  // Verarbeitunsschleife die endlos laufen soll wenn Status AN ist.

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
    delay(3000);
    digitalWrite(AnlasserPin, LOW);



    // Um die Schleife abzubrechen wenn Aus gedrückt wird müssen wir natürlich innerhalb der
    // Schleife den Status des Ausschalters überwachen.

    // Wenn der Ausschalter gedrückt wird
    if (digitalRead(SchalterPinAus) == 1) {
      ausschalten = true;
      Serial.println("Es wurde ausgeschaltet");

    }

    anschalten = false;
  }



// 15 Sekunden nach Choke auf muss der Choke wieder zugemacht werden.
if (pos > 0) {
    // 5 Sekunden warten ohne das Programm aufzuhalten
    if ((millis() - lastMillis1) >= 15000) {
      Serial.println("Zeitablauf, Choke wieder zu");
      // Nachdem der Motor läuft soll der Choke wieder zugemacht werden.
      // Hier momentan durch eine Zeitverzögerung realisiert, in echt soll D+ oder
      // die Spannungsversorgung gelesen werden.

      while (pos > 0) {
        choke.write(pos);
        pos--;
        delay(15);
      }

      // Zeitschalter zurücksetzen
      lastMillis1 = millis();
    }

}


  // Hier wird alles wieder ausgeschaltet.

  while (ausschalten) {
    if (digitalRead(LEDPin)) {
      // LED-Ausgabe auf AN
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

#include <Servo.h>
//#include <Arduino.h>
//#include <stdio.h>
Servo choke;

// Debug-Kennzeichen
const int Debug = 0; // auf 1 setzen wenn Debug-Informationen gewünscht sind.

// Deklarierung der PINs:
const int LEDPin = 13; // LED
const int ServoPin = 9; // Hier hinter steckt ein Servo
const int AnlasserPin = 2; // Relais für den Anlasser
const int IgnitionPin = 3; // Relais Zündung ein
const int VibrationPin = 8; // Vibrationssensor
const char LM35 = A4; // PIN für Temperatursensor LM35DZ am Motor
const char LM35u = A5; // PIN für Temperatursensor Umgebung
const int switchPin = 10; // Schiebeschalter
const int tasterPin = A3; // Starter manuell

// Deklaration sonstiger wichtiger Werte
const int anfpos = 110; //Anfangsposition des Servos
const int endpos = 50; //Endposition des Servos)
int pos = anfpos;
int modus = 4; // 0=AllesAus, 1=Anschalten, 2 = Läuft, 3=Ausschalten, 4=Start nicht erfolgreich, 5=Manuell
int Schritt = 0;
const int schwellwertchoke = 4; // Grenzwert für Temperatur Choke
int tempmotor = 0; // Temperatur des Motors
int tempumgebung = 0; // Umgebungstemperatur
int versuchohnechoke = 0;
bool chokeON;
int VibrationState;
int senstemperatur;
int senstemp2;
int switchState = 0; // Schiebeschalter Wert
int tasterstatus = 0;

//Zeitstempel für Parallelverarbeitung
unsigned long chokestart;
unsigned long currentMillis;
unsigned long previousMillis = 0;
unsigned long debugMillis;
unsigned long Dauer;
unsigned long chokeMillis = 0;

// Zeiten
const unsigned long chokezeit = 1900; // Zeit die der Choke offen sein soll, eine Sekunde = 1000
unsigned long anlasserzeit = 2000; // Zeit die der Anlasser orgeln soll
const unsigned long anlasserzeitohnechoke = 4000; // Zeit die der Anlasser ohne Choke (2. Versuch) orgeln soll
const unsigned long laufzeit = 5000; // Zeit nachdem der Motor laufen sollte
const unsigned long motorcheckzeit = 10000; // Zeit nachdem überprüft wird ob der Motor läuft

// Programme flow description
int present_cond_vib = 0;
int previous_cond_vib = 0;



void setup() {

  // Serielle Konsole starten
  Serial.begin(9600);

  // Pins bestimmen
  pinMode(LEDPin, OUTPUT);
  pinMode(AnlasserPin, OUTPUT);
  pinMode(IgnitionPin, OUTPUT);
  pinMode(VibrationPin, INPUT);
  pinMode(LM35, INPUT);
  pinMode(LM35u, INPUT);
  pinMode(switchPin, INPUT);
  pinMode(tasterPin, INPUT);

  // Servo benennen und an PWM-Pin anhängen
  choke.attach(ServoPin);

  //Servo auf Anfangszustand zurücksetzen
  Serial.print("Choke auf Anfangsstellung ");
  Serial.print(anfpos);
  Serial.print(" zurücksetzen");
  Serial.println(" ");
  choke.write(anfpos);
  Serial.println("Ready to Go....");
  modus = 0;
}

void loop() {
  currentMillis = millis();

  switchState = digitalRead(switchPin);

  if (switchState == HIGH) {

    previous_cond_vib = present_cond_vib;
    present_cond_vib = digitalRead(VibrationPin);
    if (previous_cond_vib != present_cond_vib) {
      VibrationState = 1;
      //  Serial.print("1");
    }

    if (modus == 0) {
      // Überprüfung ob der Motor bereits warm ist
      senstemperatur = analogRead(LM35);
      tempmotor = map(senstemperatur, 0, 307, 0, 150);

      senstemp2 = analogRead(LM35u);
      tempumgebung = map(senstemp2, 0, 307, 0, 150);

      if (tempmotor <= ( tempumgebung + schwellwertchoke )) {
        Serial.print("Umgebungstemperatur: ");
        Serial.print(tempumgebung);
        Serial.println("°C");
        Serial.print("Temperatur vom Motor: ");
        Serial.print(tempmotor);
        Serial.println("°C");
        Serial.print("und damit kleiner als Mindesttemperatur von ");
        Serial.print((tempumgebung + schwellwertchoke));
        Serial.println("°C, also starten wir mit Choke");
        //Serial.println("");
        Schritt = 0;
      } else {
        Serial.print("Umgebungstemperatur: ");
        Serial.print(tempumgebung);
        Serial.print("°C, Temperatur vom Motor: ");
        Serial.println(tempmotor);
        Serial.print("°C und damit groesser als Mindesttemperatur von ");
        Serial.print((tempumgebung + schwellwertchoke));
        Serial.print("°C also starten wir ohne Choke");
        Serial.println("");
        Schritt = 1;
      }
      digitalWrite(LEDPin, HIGH);
      Serial.println("Es wurde angeschaltet");
      modus = 1;
    }


    if (modus == 1)
    {
      if (currentMillis  - previousMillis > Dauer )
      {
        Schritt++;  // nächster Schritt
        switch (Schritt)
        {
          case 1: // Choke ausfahren
            Serial.println("Choke ausfahren");
            Serial.println(pos);
            Serial.println(endpos);
            while (pos > endpos) { // goes from 0 degrees to 90 degrees
              // in steps of 1 degree
              choke.write(pos);              // tell servo to go to position in variable 'pos'
              pos--;
            }
            Dauer = 1000; // sec warten
            //         chokeMillis = millis();
            chokeON = true;
            break;

          case 2: // Zündung anschalten und Benzinhahn auf
            Serial.println("Zündung an");
            digitalWrite(IgnitionPin, HIGH);
            Dauer = 1000; // Pause zeit 2
            break;

          case 3: // Anlasser betätigen
            Serial.println("Anlasser betätigen");
            if ( chokeON ) {
              chokeMillis = millis(); // Startzeit für den Choke
            }
            digitalWrite(AnlasserPin, HIGH);
            Dauer = anlasserzeit; // Pause zeit 2
            break;

          case 4: // Anlasser stoppen
            Serial.println("Anlasser stoppen");
            digitalWrite(AnlasserPin, LOW);
            Dauer = motorcheckzeit;
            break;

          case 5: // Vibrationsensor zurücksetzen um aktuelle Werte zu lesen
            Serial.println(" Jetzt Vibrationscheck");
            previous_cond_vib = 0;
            VibrationState = 0;

            //       previous_cond_vib = present_cond_vib;
            //       present_cond_vib = digitalRead(VibrationPin);
            //
            //        Serial.print(previous_cond_vib);
            //        Serial.print(" ");
            //        Serial.print(present_cond_vib);
            //       if (previous_cond_vib == present_cond_vib) {
            //        present_cond_vib = digitalRead(VibrationPin);
            //        Serial.println(present_cond_vib);
            //
            //       }


            Dauer = 500;
            break;

          case 6: // Check ob Motor läuft

            Serial.println("Check ob der Motor läuft!");


            if (VibrationState == 1) {
              Serial.println("Motor vibriert und läuft, alles gut!");
              versuchohnechoke = 0;
              modus = 2; // läuft
            } else {
              Serial.println("Sensor1 registriert keine Vibration");
            }
            if ((VibrationState == 0) && (modus == 1))
            {

              if (versuchohnechoke < 3) {
                versuchohnechoke++;
                Serial.println("Probieren wir noch mal ohne Choke zu starten");
                Serial.print("Startversuch: ");
                Serial.print(versuchohnechoke);
                Serial.println("");
                anlasserzeit = anlasserzeitohnechoke;
                Schritt = 2;
                currentMillis = millis();
                previousMillis = 0;
                Dauer = 20000;
              }
              else {
                Serial.println("Mehr als 3 Startversuche machen wir nicht. Fertig.");
                modus = 3; // Da der Funk-Schalter auf An steht muss erst manuell ausgeschaltet werden.
                versuchohnechoke = 0;
                Schritt = 0;
                digitalWrite(LEDPin, LOW);
                Serial.println("LED geht AUS");
                Serial.println("Es wurde ausgeschaltet");
                Dauer = 1;
              }
            }


            //             } // Else: Motor warm
            Dauer = laufzeit;
            break;

        } // Ende Switch 1

        previousMillis = currentMillis; // "Delay" neu starten
      }  //  if (currentMillis  - previousMillis > Dauer )
    } //   if (modus == 1)


    // Der Choke soll unabhängig vom Anlassvorgang zu gehen.
    // Choke zu unabhängig von Anlasserzeit
    if (modus == 1)
    {
      if (chokeON == true)
      {
        if ( chokeMillis != 0 )
        {
          if (  ( chokeMillis + chokezeit) < currentMillis )
            //       if ( currentMillis - chokeMillis > chokezeit )
          {
            Serial.println("Choke zu");
            if (pos < anfpos) {
              while (pos < anfpos) {
                choke.write(pos);
                pos++;
              }
            }
            chokeMillis = 0;
            chokeON = false;
          } // millis-if
        } // millis-if
      } //choke-if
    } // Modus1-if




    if (modus == 3)
    {
      if (currentMillis  - previousMillis > Dauer )
      {
        Schritt++;
        switch (Schritt)
        {
          case 1: // Choke wieder auf Anfang zurückfahren
            Serial.println("Choke zurückfahren wenn ausgefahren");
            if (pos < anfpos) {
              while (pos < anfpos) {
                choke.write(pos);
                pos++;
              }
              chokeMillis = 0;
              chokeON = false;

            }
            Serial.println("Anlasser stoppen");
            digitalWrite(AnlasserPin, LOW);
            Serial.println("Zündung stoppen");
            digitalWrite(IgnitionPin, LOW);
            Serial.println("Startversuche auf 0 zurücksetzen");
            versuchohnechoke = 0;

            Serial.println("Alles auf Anfang gesetzt, FERTIG!");
            Dauer = 1;
            modus = 4; //ausgeschaltet
            break;


          default:
            Schritt = 0;
            Dauer = 1;     // Schrittfolge wieder von vorn
        }
        previousMillis = currentMillis; // "Delay" neu starten
      }  //  if (currentMillis  - previousMillis > Dauer )
    } //   if (modus == 1)

    //debug();





  } else { // Ende Switch-State, ab hier manueller Betrieb

    tasterstatus = digitalRead(tasterPin);

    //Serial.println("Switch steht auf aus");
    if (modus > 0) {

      if (modus != 5) {

        Serial.println("Anlasser stoppen");
        digitalWrite(AnlasserPin, LOW);
        Serial.println("Zündung stoppen");
        digitalWrite(IgnitionPin, LOW);
        Serial.println("Startversuche auf 0 zurücksetzen");
        versuchohnechoke = 0;

        if (chokeON == true) {
          if (pos < anfpos) {
            while (pos < anfpos) {
              choke.write(pos);
              pos++;
            }
            chokeMillis = 0;
            chokeON = false;


          }

        }
        Dauer = 9000;
        previousMillis = currentMillis;
      }
      modus = 5;
      Schritt = 0;

      digitalWrite(LEDPin, LOW);




      if (modus == 5) {
        //Serial.print("currentMillis: ");
        //Serial.println(currentMillis);
        //Serial.print("previousMillis: ");
        //Serial.println(previousMillis);
        if (currentMillis >= (previousMillis + Dauer)) {

          Serial.println("Zündung wieder starten manueller Betrieb");
          digitalWrite(IgnitionPin, HIGH);
          modus = 0;
        }

      }
    } else { // Modus entspricht Null

if (tasterstatus == HIGH) { // Anlasser manuell betätigen
       digitalWrite(AnlasserPin, HIGH);
 
      
 //     Serial.println("Modus Null und Taster gedrueckt");
  
} else { // Anlasser ist nicht betätigt.

       digitalWrite(AnlasserPin, LOW);
 
  
}

    }
  }

} // loop-Ende

//void debug () {
//
//// Debug-Informationen
//if (Debug == 1) {
//
//     if (currentMillis  - debugMillis > 2000 )  {
//      Serial.println("------------------------------------------------------");
//      Serial.println("DEBUG: ");
//      if (LM35u != 00) {
//      Serial.print("Umgebungstemperatur: ");
//      Serial.print(tempumgebung);
//      Serial.println("°C");
//      }
//      if (LM35 != 00) {
//      Serial.print("Temperatur vom Motor: ");
//      Serial.print(tempmotor);
//      Serial.println("°C");
//      }
//      if (VibrationPin != 00) {
//      Serial.print("Vibrationsstatus Sensor 1: ");
//      Serial.println(VibrationState);
//      }
//      if (VibrationPin2 != 00) {
//      Serial.print("Vibrationsstatus Sensor 2: ");
//      Serial.println(VibrationState2);
//      }
//
//      Serial.print("Status Funk: ");
//      Serial.println(StatusFunk);
//      Serial.print("Modus: ");
//      Serial.print(modus);
//      switch (modus)
//        {case 0:
//        Serial.println("  Alles aus");
//        break;
//        case 1:
//        Serial.println("  Anschalten");
//        break;
//        case 2:
//        Serial.println("  Läuft");
//        break;
//        case 3:
//        Serial.println("  Ausschalten");
//        break;
//        default:
//        break;
//        }
//      if (SpannungsPin != 00) {
//        float vout = (analogRead(SpannungsPin) * MAX_VIN) / 1024.0f;
//        float vin = vout / (R2/(R1+R2));
//        Serial.print("Batteriespannung: ");
//        Serial.print(vin,2);
//        Serial.println("");
//      }
//      Serial.println("Choke - Millis, chokezeit, currentMillis: ");
//      Serial.print(chokeMillis);
//      Serial.print(", ");
//      Serial.print(chokezeit);
//      Serial.print(", ");
//      Serial.print(currentMillis);
//      Serial.println("");
//      Serial.println(" == == == == == == == == == == == == == == == == == == == == == == == == == == == ");
//      debugMillis = millis();
//     } //Zeitablauf
//} //Debug-Ende

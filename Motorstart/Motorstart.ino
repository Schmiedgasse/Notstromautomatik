#include <Servo.h>
Servo choke;

// Debug-Kennzeichen
const int Debug = 0; // auf 1 setzen wenn Debug-Informationen gewünscht sind.

// Deklarierung der PINs:
const int LEDPin = 13; // LED
const int SchalterFunkAn = 4; // Schalter Funk
const int ServoPin = 10; // Hier hinter steckt ein Servo
const int AnlasserPin = 3; // Relais für den Anlasser
const int IgnitionPin = 2; // Relais Zündung ein
const int VibrationPin = 5; // Vibrationssensor
// Optional (wenn nicht gebraucht 00 eintragen): 
const char SpannungsPin = A2; // Spannungssensor Anlasserbatterie
//const char VBattPin = 00; // Spannungssensor Versorgungsbatterie
const char MagnetVentil = 00; // Magnetventil als Benzinhahn
float vschwell = 0.5; // Schwellwert für die Spannungsmessung nach Start
const char LM35 = A4; // PIN für Temperatursensor LM35DZ am Motor
const char LM35u = A5; // PIN für Temperatursensor Umgebung
const int VibrationPin2 = 6; // PIN für redundanten Vibrationssensor

// Deklaration sonstiger wichtiger Werte
const int anfpos = 110; //Anfangsposition des Servos
const int endpos = 50; //Endposition des Servos)
int pos = anfpos;
int StatusFunk = 1; // Status Funkschalter
int modus = 0; // 0=AllesAus, 1=Anschalten, 2 = Läuft, 3=Ausschalten
int Schritt = 0;
const int schwellwertchoke = 6; // Grenzwert für Temperatur Choke
const int schwellwertlaeuft = 2; // Grenzwert für Temperatur läuft
int tempmotor = 0; // Temperatur des Motors
int tempumgebung = 0; // Umgebungstemperatur
int versuchohnechoke = 0;
// Für die Spannungsmessung
const float R1 = 30000.0f; // Der Widerstand R1 hat eine größe von 30 kOhm
const float R2 = 7500.0f; //  Der Widerstand R2 hat eine größe von 7,5 kOhm
const float MAX_VIN = 5.0f;
float vstart;
bool chokeON;
int VibrationState;
int VibrationState2;

//Zeitstempel für Parallelverarbeitung
unsigned long chokestart;
unsigned long currentmillis;
unsigned long previousMillis;
unsigned long debugMillis;
unsigned long Dauer;
unsigned long chokeMillis = 0;

// Zeiten
const unsigned long chokezeit = 1900; // Zeit die der Choke offen sein soll, eine Sekunde = 1000
unsigned long anlasserzeit = 2000; // Zeit die der Anlasser orgeln soll
const unsigned long anlasserzeitohnechoke = 4000; // Zeit die der Anlasser ohne Choke (2. Versuch) orgeln soll
const unsigned long laufzeit = 5000; // Zeit nachdem der Motor laufen sollte
const unsigned long motorcheckzeit = 10000; // Zeit nachdem überprüft wird ob der Motor läuft

// Wert kann angepasst werden an die Spannung die der Arduino wirklich hat.
// const float arduino5v = 4.88;


void setup() {

// Serielle Konsole starten
  Serial.begin(9600);

// Pins bestimmen
  pinMode(LEDPin, OUTPUT);
  pinMode(SchalterFunkAn, INPUT_PULLUP);  
  pinMode(AnlasserPin, OUTPUT);
  pinMode(IgnitionPin, OUTPUT);
  if (VibrationPin != 00) {
    pinMode(VibrationPin, INPUT);
  }
  if (VibrationPin2 != 00) {
    pinMode(VibrationPin2, INPUT);
  }
  if (LM35 != 00) {
    pinMode(LM35, INPUT);
  }
  if (LM35u != 00) {
    pinMode(LM35u, INPUT);
  }
  if (SpannungsPin != 00) {
    pinMode(SpannungsPin, INPUT);
  }
  if (MagnetVentil != 00) {
    pinMode(MagnetVentil, OUTPUT);
  }

// Servo benennen und an PWM-Pin anhängen
  choke.attach(ServoPin);

//Servo auf Anfangszustand zurücksetzen
  Serial.print("Choke auf Anfangsstellung ");
  Serial.print(anfpos);
  Serial.print(" zurücksetzen");
  Serial.println(" ");
  choke.write(anfpos);
  Serial.println("Ready to Go....");

}

void loop()
{
unsigned long currentMillis = millis();

//Deklarierung von veränderlichen Werten
//if (VibrationPin != 00) {
   VibrationState = digitalRead(VibrationPin);
//  }

//if (VibrationPin2 != 00) {
   VibrationState2 = digitalRead(VibrationPin2);
//  }
  
if (SpannungsPin != 00) {
  float vout = (analogRead(SpannungsPin) * MAX_VIN) / 1024.0f;
  float vin = vout / (R2/(R1+R2)); 
}

if (LM35 != 00) {
int senstemperatur=analogRead(LM35);
tempmotor=map(senstemperatur, 0, 307, 0, 150);
}

if (LM35u != 00) {
int senstemp2=analogRead(LM35u);
tempumgebung=map(senstemp2, 0, 307, 0, 150); 
}



 // Wenn per Funk-Schalter gedrückt wird
  if (digitalRead(SchalterFunkAn) == 0 && StatusFunk == 0) {
    modus = 1;
    currentMillis = millis();
    previousMillis = 0;


    if (SpannungsPin != 00) {
      float vout = (analogRead(SpannungsPin) * MAX_VIN) / 1024.0f;
      vstart = vout / (R2/(R1+R2));
      Serial.print("Spannung zu Start betraegt: ");
      Serial.println(vstart,2); 
    }


    // Überprüfung ob der Motor bereits warm ist   
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
    Serial.println("Es wurde ueber Funk angeschaltet");
//    Serial.print(digitalRead(SchalterFunkAn));
//    Serial.print(StatusFunk);
//    Serial.println(" ");
    StatusFunk = 1;
  }

// Wenn der Funk-Ausschalter gedrückt wird
  if (digitalRead(SchalterFunkAn) == 1 && StatusFunk == 1) {
    Dauer = 0; 
    Schritt = 0;
    modus = 3;
    digitalWrite(LEDPin, LOW);
    Serial.println("LED geht AUS");
    Serial.println("Es wurde ueber Funk ausgeschaltet");
//    Serial.print(digitalRead(SchalterFunkAn));
//    Serial.print(StatusFunk);
//    Serial.println(" ");
    StatusFunk = 0;
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
         if (MagnetVentil != 00) {
             Serial.println("Magnetventil Benzinhahn auf");
             digitalWrite(MagnetVentil, HIGH);
          }

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

       case 5:
         Dauer = 1;
         break;

      case 6: // Check ob Motor läuft
        Serial.println("Check ob der Motor läuft!");
        Serial.print("Vibration: ");
        Serial.println(VibrationState);
         
        if(VibrationState == 1)
          {
          Serial.println("Motor vibriert und läuft, alles gut!");
          versuchohnechoke = 0;
          modus = 2; // läuft
          } else {
           Serial.println("Sensor1 registriert keine Vibration");
           if (VibrationState2 == 1) {
            Serial.println("Redundanter Vibrationssensor vibriert, Motor läuft, alles gut!");
            versuchohnechoke = 0;
            modus = 2; // läuft 
           } else {
            Serial.println("Sensor2 registriert keine Vibration");
           }
          }
        if ((SpannungsPin != 00) && (modus == 1)) {
            Serial.println("Spannungs-Check: ");
              float vout = (analogRead(SpannungsPin) * MAX_VIN) / 1024.0f;
              float vin = vout / (R2/(R1+R2));
              if (vin > ( vstart + vschwell) ) {
                 Serial.print("Spannung der Starterbatterie jetzt ");
                 Serial.print(vin,2);
                 Serial.print(" und damit ueber dem Schwellwert ");
                 Serial.print( (vstart + vschwell) );
                 Serial.print(", Motor läuft");
                 Serial.println("");
                 modus = 2; //läuft
              } else {
                Serial.print("Spannung der Starterbatterie jetzt ");
                Serial.print(vin,2);
                Serial.print(" und damit NICHT ueber dem Schwellwert von ");
                Serial.print( (vstart + vschwell) );
                Serial.print(", Motor läuft NICHT");
                Serial.println("");
                 
              }
          }
          if ((VibrationState == 0) && (modus == 1))
          {
            Serial.println("Motor vibriert nicht und scheint nicht zu laufen. Mal Temperatur checken");
            //Schaun wir mal, vielleicht ist der Motor ja warm und läuft schon
             if (tempmotor > (tempumgebung + schwellwertlaeuft))
             {
             Serial.print("Motortemperatur: ");
             Serial.print(tempmotor);
             Serial.print(" und damit größer als der Schwellwert von ");
             Serial.print((tempumgebung + schwellwertlaeuft));
             Serial.print(" Grad, Motor läuft");
             Serial.println("");
             versuchohnechoke = 0;
             modus = 2; //läuft
             } else
             {
             Serial.print("Motortemperatur: ");
             Serial.print(tempmotor);
             Serial.print(" und damit kleiner als der Schwellwert von ");
             Serial.print((tempumgebung + schwellwertlaeuft));
             Serial.print(" Grad, Motor läuft NICHT");
             Serial.println("");

             if (versuchohnechoke < 3) {
                 versuchohnechoke++;
                 Serial.println("Probieren wir noch mal ohne Choke zu starten");
                 Serial.print("Startversuch: ");
                 Serial.print(versuchohnechoke);
                 Serial.println("");
                 anlasserzeit=anlasserzeitohnechoke;
                 Schritt=2;
                 currentMillis = millis();
                 previousMillis = 0;    
                 Dauer=20000;
             }
             else {
                 Serial.println("Mehr als 3 Startversuche machen wir nicht. Fertig.");
                 modus = 3; // Da der Funk-Schalter auf An steht muss erst manuell ausgeschaltet werden.
                 versuchohnechoke = 0;
                 Schritt = 0;
                 digitalWrite(LEDPin, LOW);
                 Serial.println("LED geht AUS");
                 Serial.println("Es wurde ausgeschaltet");
                 Dauer=1;
         }
        }

             
             } // Else: Motor warm
        Dauer=laufzeit;
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
         if (MagnetVentil != 00) {
             Serial.println("Magnetventil Benzinhahn zu");
             digitalWrite(MagnetVentil, LOW);
          }
         Serial.println("Startversuche auf 0 zurücksetzen");
         versuchohnechoke = 0;
 
//         Dauer = 1000; // sec warten
//         break;
//
//        case 2: // Endprozedur alles auf Anfang
//        StatusFunk = 0;
         Serial.println("Alles auf Anfang gesetzt, FERTIG!");
         Dauer=1;
         modus = 0; //ausgeschaltet
         break;


       default:
         Schritt = 0;
         Dauer = 1;     // Schrittfolge wieder von vorn
       }
       previousMillis = currentMillis; // "Delay" neu starten
     }  //  if (currentMillis  - previousMillis > Dauer )  
 } //   if (modus == 1)

// Debug-Informationen
if (Debug == 1) {

     if (currentMillis  - debugMillis > 2000 )  {
      Serial.println("------------------------------------------------------");
      Serial.println("DEBUG:");
      if (LM35u != 00) {
      Serial.print("Umgebungstemperatur: ");
      Serial.print(tempumgebung);
      Serial.println("°C");
      }
      if (LM35 != 00) {
      Serial.print("Temperatur vom Motor: ");
      Serial.print(tempmotor);
      Serial.println("°C");
      }
      if (VibrationPin != 00) {
      Serial.print("Vibrationsstatus Sensor 1: ");
      Serial.println(VibrationState); 
      }
      if (VibrationPin2 != 00) {
      Serial.print("Vibrationsstatus Sensor 2: ");
      Serial.println(VibrationState2); 
      }

      Serial.print("Status Funk: ");
      Serial.println(StatusFunk);
      Serial.print("Modus: ");
      Serial.print(modus);
      switch (modus)
        {case 0:
        Serial.println("  Alles aus");
        break;
        case 1:
        Serial.println("  Anschalten");
        break;
        case 2:
        Serial.println("  Läuft");
        break;
        case 3:
        Serial.println("  Ausschalten");
        break;
        default:
        break;
        }
      if (SpannungsPin != 00) {
        float vout = (analogRead(SpannungsPin) * MAX_VIN) / 1024.0f;
        float vin = vout / (R2/(R1+R2)); 
        Serial.print("Batteriespannung: ");
        Serial.print(vin,2);
        Serial.println("");
      }
      Serial.println("Choke-Millis, chokezeit, currentMillis:");
      Serial.print(chokeMillis);
      Serial.print(",");
      Serial.print(chokezeit);
      Serial.print(",");
      Serial.print(currentMillis);
      Serial.println("");
      Serial.println("======================================================");
      debugMillis = millis();
     } //Zeitablauf
} //Debug-Ende

} // loop-Ende

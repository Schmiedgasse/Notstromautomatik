#include <Servo.h>
//#include <notstart.h>

Servo choke;

// Deklarierung der PINs:
const int LEDPin = 13; // LED
const int SchalterFunkAn = 4; // Schalter Funk
const int ServoPin = 10; // Hier hinter steckt ein Servo
const int AnlasserPin = 3; // Relais für den Anlasser
//const char SpannungsPin = A2; // Spannungssensor Anlasserbatterie
//const char VBattPin = A0; // Spannungssensor Versorgungsbatterie
const char LM35 = A4; // PIN für Temperatursensor LM35DZ am Motor
const char LM35u = A5; // PIN für Temperatursensor Umgebung
const int IgnitionPin = 2; // Relais Zündung ein
const int VibrationPin = 5; // Vibrationssensor

// Deklaration sonstiger wichtiger Werte
const int anfpos = 110; //Anfangsposition des Servos
const int endpos = 50; //Endposition des Servos)
int pos = anfpos;
int StatusFunk = 1; // Status Funkschalter
int modus = 0;
int einaus = 0;
int Schritt = 0;
const int schwellwertchoke = 8; // Grenzwert für Temperatur Choke
const int schwellwertlaeuft = 4; // Grenzwert für Temperatur läuft
int tempmotor = 0; // Temperatur des Motors
int tempumgebung = 0; // Umgebungstemperatur
int versuchohnechoke = 0;


// Zeiten
const unsigned long chokezeit = 200; // Zeit die der Choke offen sein soll, eine Sekunde = 1000
unsigned long anlasserzeit = 2000; // Zeit die der Anlasser orgeln soll
const unsigned long anlasserzeitohnechoke = 4000; // Zeit die der Anlasser ohne Choke (2. Versuch) orgeln soll
const unsigned long laufzeit = 5000; // Zeit nachdem der Motor laufen sollte
const unsigned long motorcheckzeit = 10000; // Zeit nachdem der Motor auf Vibrationen getestet wird.

// Wert kann angepasst werden an die Spannung die der Arduino wirklich hat.
// const float arduino5v = 4.88;

//Zeitstempel für Parallelverarbeitung
unsigned long chokestart;
unsigned long currentmillis;
unsigned long previousMillis;
unsigned long Dauer;

void setup() {

// Serielle Konsole starten
  Serial.begin(9600);

// Pins bestimmen
  pinMode(LEDPin, OUTPUT);
  pinMode(SchalterFunkAn, INPUT_PULLUP);  
  pinMode(AnlasserPin, OUTPUT);
  pinMode(IgnitionPin, OUTPUT);
  pinMode(VibrationPin, INPUT);
  pinMode(LM35, INPUT);
  pinMode(LM35u, INPUT);

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
/*    Serial.println("DEBUG:");
    Serial.print("Status Funk: ");
    Serial.println(StatusFunk);
    Serial.print("einaus: ");
    Serial.println(einaus);
    Serial.print("Modus: ");
    Serial.println(modus);
//    Serial.print("Vibration: ");
//    Serial.println(vibration);    
*/  
unsigned long currentMillis = millis();

//Deklarierung von veränderlichen Werten
int VibrationState = digitalRead(VibrationPin);

int senstemperatur=analogRead(LM35);
tempmotor=map(senstemperatur, 0, 307, 0, 150);

int senstemp2=analogRead(LM35u);
tempumgebung=map(senstemp2, 0, 307, 0, 150); 

//Serial.print("Temperatur vom Motor: ");
//Serial.print(temperatur);
//Serial.print("°C");
//Serial.println("");

 // Wenn per Funk-Schalter gedrückt wird
  if (digitalRead(SchalterFunkAn) == 0 && StatusFunk == 0) {
    einaus = 1;
    modus = 1;
    currentMillis = millis();
    previousMillis = 0;

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
    einaus = 0;
    Dauer = 0; 
    Schritt = 0;
    modus = 1;
    digitalWrite(LEDPin, LOW);
    Serial.println("LED geht AUS");
    Serial.println("Es wurde ueber Funk ausgeschaltet");
//    Serial.print(digitalRead(SchalterFunkAn));
//    Serial.print(StatusFunk);
//    Serial.println(" ");
    StatusFunk = 0;
    }

if (modus == 1 && einaus == 1)
 {
     if (currentMillis  - previousMillis > Dauer )  
     {
       Schritt++;  // nächster Schritt
       switch (Schritt)
       {
       case 1: // Choke ausfahren
         Serial.println("Choke ausfahren");
         while (pos > endpos) { // goes from 0 degrees to 90 degrees
             // in steps of 1 degree
             choke.write(pos);              // tell servo to go to position in variable 'pos'
             pos--;
         }
         Dauer = 1000; // sec warten
         break;

       case 2: // Zündung anschalten
         Serial.println("Zündung an");
            digitalWrite(IgnitionPin, HIGH);
         Dauer = 1000; // Pause zeit 2
         break;
       
       case 3: // Anlasser betätigen
         Serial.println("Anlasser betätigen");
            digitalWrite(AnlasserPin, HIGH);
         Dauer = anlasserzeit; // Pause zeit 2
         break;


       case 4: // Anlasser stoppen
         Serial.println("Anlasser stoppen");
         digitalWrite(AnlasserPin, LOW);
         Dauer = chokezeit;
         break;

       case 5: // Choke zu

         Serial.println("Choke zu wenn offen");
         if (pos < anfpos) {
            while (pos < anfpos) {
                choke.write(pos);
                pos++;
            }
         }

         Dauer=motorcheckzeit;
         break;

       case 6: // Check ob Motor läuft
         Serial.print("Vibration: ");
         Serial.println(VibrationState);

        if(VibrationState == HIGH)
          {
          Serial.println("Motor vibriert, alles gut!");
          versuchohnechoke = 0;
          modus = 0;  
          }
          else
          {
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
             modus = 0;
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
                 einaus = 0;
                 versuchohnechoke = 0;
                 Schritt = 0;
                 digitalWrite(LEDPin, LOW);
                 Serial.println("LED geht AUS");
                 Serial.println("Es wurde ausgeschaltet");
                 Dauer=1;
         }
        }

             
             }
//          }



         
//         if(VibrationState == LOW)
            
           
        Dauer=laufzeit;
        break;

       default:
         Schritt = 0;
         Dauer = 1;     // Schrittfolge wieder von vorn
       }
       previousMillis = currentMillis; // "Delay" neu starten
     }  //  if (currentMillis  - previousMillis > Dauer )  
 } //   if (modus == 1)


if (modus == 1 && einaus == 0)
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
         }
         Serial.println("Anlasser stoppen");
            digitalWrite(AnlasserPin, LOW);
         Serial.println("Zündung stoppen");
            digitalWrite(IgnitionPin, LOW);
         Serial.println("Startversuche auf 0 zurücksetzen");
         versuchohnechoke = 0;
 
//         Dauer = 1000; // sec warten
//         break;
//
//        case 2: // Endprozedur alles auf Anfang
//        StatusFunk = 0;
         Serial.println("Alles auf Anfang gesetzt, FERTIG!");
         Dauer=1;
         modus = 0;
         break;


       default:
         Schritt = 0;
         Dauer = 1;     // Schrittfolge wieder von vorn
       }
       previousMillis = currentMillis; // "Delay" neu starten
     }  //  if (currentMillis  - previousMillis > Dauer )  
 } //   if (modus == 1)

} // loop-Ende

#include <Servo.h>
//#include <notstart.h>

Servo choke;
const int anfpos = 110; //Anfangsposition des Servos
const int endpos = 50; //Endposition des Servos)
int pos = anfpos;
const unsigned long chokezeit = 200; // Zeit die der Choke offen sein soll, eine Sekunde = 1000
int anlassversuch = 0;
const int LEDPin = 13; // LED
const int SchalterPinAn = 2; // Schalter
const int SchalterPinAus = 4; // Schalter
const int SchalterFunkAn = 3; // Schalter Funk
int StatusFunk = 1; // Status Funkschalter
const int ServoPin = 9; // Hier hinter steckt ein Servo
const int AnlasserPin = 5; // Relais für den Anlasser
const char SpannungsPin = A1; // Spannungssensor Anlasserbatterie
const char VBattPin = A0; // Spannungssensor Versorgungsbatterie
const int IgnitionPin = 10; // Relais Zündung ein
int modus = 0;
int einaus = 0;
int endlageValue = 0;
int Schritt = 0;


// Wert kann angepasst werden an die Spannung die der Arduino wirklich hat.
// const float arduino5v = 4.88;
// Wie lange soll der Anlasser orgeln
const int anlasserzeit = 2000; // Zeit die der Anlasser orgeln soll

//Zeitstempel für Parallelverarbeitung
unsigned long chokestart;
unsigned long currentmillis;
unsigned long previousMillis;
unsigned long Dauer;

// Boolean Werte für Statiänderungen
//bool anschalten = false;
//bool ausschalten = false;
//bool laeuft = false;
//bool needchoke = false;
//boolean LEDvalue = LOW;


void setup() {

  // Serielle Konsole starten
  Serial.begin(9600);

  // Pins bestimmen
  pinMode(LEDPin, OUTPUT);
  pinMode(SchalterPinAn, INPUT);
  pinMode(SchalterPinAus, INPUT);
  pinMode(SchalterFunkAn, INPUT_PULLUP);  
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

void loop()
{
unsigned long currentMillis = millis();


 // Wenn der Funk-Anschalter zum ersten Mal gedrückt wird
  if (digitalRead(SchalterFunkAn) == 0 && StatusFunk == 0) {
    einaus = 1;
    modus = 1;
    endlageValue = 0;
    currentMillis = millis();
    previousMillis = 0;    
    Schritt = 0;
    digitalWrite(LEDPin, HIGH);
    Serial.println("Es wurde ueber Funk angeschaltet");
    Serial.print(digitalRead(SchalterFunkAn));
    Serial.print(StatusFunk);
    Serial.println(" ");
    StatusFunk = 1;
  }

  // Wenn der Funk-Ausschalter zum ersten Mal gedrückt wird
  if (digitalRead(SchalterFunkAn) == 1 && StatusFunk == 1) {
    einaus = 0;
//    currentMillis = millis();
//    previousMillis = currentMillis;
    Dauer = 0; 
    Schritt = 0;
    endlageValue = 0;
    digitalWrite(LEDPin, LOW);
    Serial.println("LED geht AUS");
    Serial.println("Es wurde ueber Funk ausgeschaltet");
    Serial.print(digitalRead(SchalterFunkAn));
    Serial.print(StatusFunk);
    Serial.println(" ");
    StatusFunk = 0;
    }



  if (digitalRead(SchalterPinAn) == 1) {
    einaus = 1;
    modus = 1;
    endlageValue = 0;
    currentMillis = millis();
    previousMillis = 0;    
    Schritt = 0;
    digitalWrite(LEDPin, HIGH);
    Serial.println("LED geht AN");
    Serial.println("Es wurde angeschaltet");
  }

  // Wenn der Ausschalter gedrückt wird
  if (digitalRead(SchalterPinAus) == 1) {
    einaus = 0;
//    currentMillis = millis();
//    previousMillis = currentMillis;
    Dauer = 0; 
    Schritt = 0;
    endlageValue = 0;
    digitalWrite(LEDPin, LOW);
    Serial.println("LED geht AUS");
    Serial.println("Es wurde ausgeschaltet");
/*    Serial.println(currentMillis);
    Serial.println(previousMillis);
    Serial.println(Dauer);
    Serial.println(modus);
    Serial.println(einaus);
    Serial.println(endlageValue);
*/  }
  
 
if (modus == 1 && einaus == 1)
 {
   if (endlageValue==0)
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
         Dauer = 1000;
         break;

       case 5: // Motor läuft
         Serial.println("Motor läuft, Choke zu");
         if (pos < anfpos) {
            while (pos < anfpos) {
                choke.write(pos);
                pos++;
            }
         }

         endlageValue = 1;
         Dauer=chokezeit;
         break;

       default:
         Schritt = 0;
         Dauer = 1;     // Schrittfolge wieder von vorn
       }
       previousMillis = currentMillis; // "Delay" neu starten
     }  //  if (currentMillis  - previousMillis > Dauer )  
   }   // if (endlageValue==0)
 } //   if (modus == 1)


if (modus == 1 && einaus == 0)
 {
   if (endlageValue==0)
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
 
         Dauer = 1000; // sec warten
         break;

        case 2: // Endprozedur alles auf Anfang
        Serial.println("Alles auf Anfang, Endprozedur");
         endlageValue = 1;
         modus = 0;
         Dauer=1;
         break;


       default:
         Schritt = 0;
         Dauer = 1;     // Schrittfolge wieder von vorn
       }
       previousMillis = currentMillis; // "Delay" neu starten
     }  //  if (currentMillis  - previousMillis > Dauer )  
   }   // if (endlageValue==0)
 } //   if (modus == 1)



















} // loop-Ende

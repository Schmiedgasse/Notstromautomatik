#define SENSOR A2
const int LEDPin = 13; // LED
const char LM35 = A4; // PIN für Temperatursensor LM35DZ am Motor
const char LM35u = A5; // PIN für Temperatursensor Umgebung
int Vibration = 8; // PIN für Vibrationssensor
int tempmotor = 0; // Temperatur des Motors
int tempumgebung = 0; // Umgebungstemperatur
const float R1 = 30000.0f; // Der Widerstand R1 hat eine größe von 30 kOhm
const float R2 = 7500.0f; //  Der Widerstand R2 hat eine größe von 7,5 kOhm
const float MAX_VIN = 5.0f;
const int switchPin = 10; // Schiebeschalter
const int tasterPin = A3; // Starter manuell
int switchstate = 0; // Schiebeschalter Wert
int tasterstatus = 0; // Taster Status



void setup() {
    Serial.begin(9600);
    pinMode(LEDPin,OUTPUT);
    pinMode(SENSOR, INPUT);
    pinMode(LM35, INPUT);
    pinMode(LM35u, INPUT);
    pinMode(Vibration, INPUT);
    pinMode(switchPin, INPUT);
    pinMode(tasterPin, INPUT);
}
 
void loop() {
    int sensorState = digitalRead(Vibration);
    Serial.print("Vibration: ");
    Serial.println(sensorState);
    if(sensorState == HIGH)
    {
        digitalWrite(LEDPin,HIGH);
    }
    else
    {
        digitalWrite(LEDPin,LOW);
    }
    
    int senstemperatur=analogRead(LM35);
    tempmotor=map(senstemperatur, 0, 307, 0, 150);
    Serial.print("Temperatur vom Motor: ");
    Serial.print(tempmotor);
    Serial.println("");

    int senstemp2=analogRead(LM35u);
    tempumgebung=map(senstemp2, 0, 307, 0, 150); 
    Serial.print("Umgebungstemperatur: ");
    Serial.print(tempumgebung);
    Serial.println("");

switchstate = digitalRead(switchPin);

if (switchstate == HIGH) {
   Serial.print("Schiebeschalter auf An");
} else {
  Serial.print("Schiebeschalter auf AUS");
}
    Serial.println("");

tasterstatus = digitalRead(tasterPin);

if (tasterstatus == HIGH) {
  Serial.print("Taster auf An");
} else {
  Serial.print("Taster auf AUS");
}
        Serial.println("");
    
//  float vout = (analogRead(SENSOR) * MAX_VIN) / 1024.0f;
//  float vin = vout / (R2/(R1+R2)); 
//   
//  Serial.print("Eingangsspannung ");
//  Serial.print(vin,2);
//  Serial.println("V");
//  Serial.println("-------------------------------------------------");
    


    delay(1000);

}

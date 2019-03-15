
void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
}

void loop() {
  // read the input on analog pin 0:
  int sensorValue = analogRead(A1);
  int volt;
  // print out the value you read:
  Serial.println(sensorValue);

  float voltage = (sensorValue / 5 * 4.85) / 4.092 ; //Spannungsteiler. Spannung meines Arduino ist bei 4.76 Volt
  volt=(int)voltage;
  voltage=((volt%100)/10.0);
  Serial.print("Die gemessene Spannung beträgt: ");
  Serial.print(voltage);
  Serial.print("V");
  Serial.println("");
  
  delay(1000);        // delay in between reads for stability


}

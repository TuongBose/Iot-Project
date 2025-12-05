
#define LIGHT 7 // define pint 7 for sensor
#define RELAY 4 // define pin 4 as for relay

void setup() {
  // Light LDR Sensor 
  Serial.begin(9600);// setup Serial Monitor to display information
  pinMode(LIGHT, INPUT_PULLUP);// define pin as Input  sensor
  pinMode(RELAY, OUTPUT);// define pin as OUTPUT for relay
}

void loop() {
  // Light LDR Sensor 
  int L =digitalRead(LIGHT);// read the sensor 
  
    if(L == 0){
    Serial.println(" light is OFF");
    digitalWrite(RELAY,HIGH);// turn the relay ON
  }else{

     Serial.println("  === light is ON");
     digitalWrite(RELAY,LOW);// turn the relay OFF
  }
  delay(500);
 // Light LDR Sensor 
}
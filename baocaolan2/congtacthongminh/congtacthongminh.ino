#include <ArduinoIoTCloud.h>
#include <Arduino_ConnectionHandler.h>
#include <ESP8266Wifi.h>

const char DEVICE_LOGIN_NAME[]  = "dc30aff4-8707-4436-8277-2906c5634137"; //Enter DEVICE ID

const char SSID[]               = "IP15 promax Haivo";    //Enter WiFi SSID (name)
const char PASS[]               = "0963414050";    //Enter WiFi password
const char DEVICE_KEY[]         = "3WGqjE6IXhkY!jjxgQ@X4QS6K";    //Enter Secret device password (Secret Key)


// define the GPIO connected with Relays and switches
#define RelayPin1 5  //D1
#define RelayPin2 4  //D2

#define SwitchPin1 14 //D5
#define SwitchPin2 12 //D6

#define SwitchPin3 13  //D7
#define SwitchPin4 3   //RX

#define wifiLed   16   //D0


int toggleState_1 = 0; //Define integer to remember the toggle state for relay 1
int toggleState_2 = 0; //Define integer to remember the toggle state for relay 2
int toggleState_3 = 0; //Define integer to remember the toggle state for relay 3
int toggleState_4 = 0; //Define integer to remember the toggle state for relay 4

void onSwitch1Change();
void onSwitch2Change();
void onSwitch3Change();
void onSwitch4Change();

CloudSwitch switch1;
CloudSwitch switch2;
CloudSwitch switch3;
CloudSwitch switch4;

void initProperties(){

  ArduinoCloud.setBoardId(DEVICE_LOGIN_NAME);
  ArduinoCloud.setSecretDeviceKey(DEVICE_KEY);

  ArduinoCloud.addProperty(switch1, READWRITE, ON_CHANGE, onSwitch1Change);
  ArduinoCloud.addProperty(switch2, READWRITE, ON_CHANGE, onSwitch2Change);

}

WiFiConnectionHandler ArduinoIoTPreferredConnection(SSID, PASS);

const int debounceDelay = 200; // Debounce delay in milliseconds
unsigned long lastDebounceTime1 = 0;
unsigned long lastDebounceTime2 = 0;
unsigned long lastDebounceTime3 = 0;
unsigned long lastDebounceTime4 = 0;

// Track the previous state of each switch to handle edge detection
bool previousSwitchState1 = HIGH;
bool previousSwitchState2 = HIGH;
bool previousSwitchState3 = HIGH;
bool previousSwitchState4 = HIGH;

void manual_control() {
  unsigned long currentMillis = millis();

  // Manual Switch Control
  // Check each switch pin and toggle the corresponding relay if the switch is pressed (LOW state)
  
  // Check if Switch 1 is pressed
  if (digitalRead(SwitchPin1) == LOW && previousSwitchState1 == HIGH) {
    if (currentMillis - lastDebounceTime1 >= debounceDelay) {
      toggleState_1 = !toggleState_1;
      digitalWrite(RelayPin1, toggleState_1);
      switch1 = toggleState_1; // Toggle Relay 1
      lastDebounceTime1 = currentMillis; // Update the last debounce time
    }
  }
  previousSwitchState1 = digitalRead(SwitchPin1);

  // Check if Switch 2 is pressed
  if (digitalRead(SwitchPin2) == LOW && previousSwitchState2 == HIGH) {
    if (currentMillis - lastDebounceTime2 >= debounceDelay) {
      toggleState_2 = !toggleState_2;
      digitalWrite(RelayPin2, toggleState_2);
      switch2 = toggleState_2; // Toggle Relay 2
      lastDebounceTime2 = currentMillis; // Update the last debounce time
    }
  }
  previousSwitchState2 = digitalRead(SwitchPin2);
}


void setup() {
  // Initialize serial and wait for port to open:
  Serial.begin(9600);
  // This delay gives the chance to wait for a Serial Monitor without blocking if none is found
  delay(1500);

  // Defined in thingProperties.h
  initProperties();

  // Connect to Arduino IoT Cloud
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);

  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();

  pinMode(RelayPin1, OUTPUT);
  pinMode(RelayPin2, OUTPUT);

  pinMode(wifiLed, OUTPUT);

  pinMode(SwitchPin1, INPUT_PULLUP);
  pinMode(SwitchPin2, INPUT_PULLUP);

  //During Starting all Relays should TURN OFF
  digitalWrite(RelayPin1, HIGH);
  digitalWrite(RelayPin2, HIGH);

  digitalWrite(wifiLed, HIGH);  //Turn OFF WiFi LED
}

void loop() {
  ArduinoCloud.update();
  
  manual_control(); //Control relays manually

  if (WiFi.status() != WL_CONNECTED)
  {
    digitalWrite(wifiLed, HIGH); //Turn OFF WiFi LED
    //Serial.println("NO WIFI");
  }
  else{
    digitalWrite(wifiLed, LOW); //Turn ON WiFi LED
    //Serial.println("YES WIFI");
  }
}

void onSwitch1Change() {
  if (switch1 == 1)
  {
    digitalWrite(RelayPin1, HIGH);
    Serial.println("Device1 ON");
    toggleState_1 = 1;
  }
  else
  {
    digitalWrite(RelayPin1, LOW);
    Serial.println("Device1 OFF");
    toggleState_1 = 0;
  }
}

void onSwitch2Change() {
  if (switch2 == 1)
  {
    digitalWrite(RelayPin2, HIGH);
    Serial.println("Device2 ON");
    toggleState_2 = 1;
  }
  else
  {
    digitalWrite(RelayPin2, LOW);
    Serial.println("Device2 OFF");
    toggleState_2 = 0;
  }
}
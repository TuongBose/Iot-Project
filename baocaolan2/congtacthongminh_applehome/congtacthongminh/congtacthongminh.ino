#include <Arduino.h>
#include "wifi_info.h"
#include "ESPButton.h"
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#define LOG_D(fmt, ...)   printf_P(PSTR(fmt "\n") , ##__VA_ARGS__);
//khai báo nút nhấn
#define BUTTON_01 14//D5
#define BUTTON_02 12//D6
#define BUTTON_03 0//D3
#define BUTTON_04 2//D4
//khai báo Relay
#define LIGHT_01 5//D1
#define LIGHT_02 4//D2
#define LIGHT_03 13//D7
#define LIGHT_04 15//D8
String _ssid1,_pass1;
//////////////////////////////////////
//Thay đổi IP tĩnh ở đây
IPAddress local_IP(192, 168, 1, 139);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 8, 8);   
IPAddress secondaryDNS(8, 8, 4, 4); 

void setup() {
  Serial.begin(115200);
  WiFi.disconnect(); 
  EEPROM.begin(512);
  delay(1000);
  init_lights_and_buttons();
  Serial.println("Reading EEPROM user");
  for (int i = 400; i < 432; ++i)//0
  {
    delay(10);
    _ssid1 += char(EEPROM.read(i));
  }
  Serial.println();
  Serial.print("User: ");
  Serial.println(_ssid1);
  
  Serial.println("Reading EEPROM pass");
  for (int i = 432; i < 496; ++i)
  {
    delay(10);
    _pass1 += char(EEPROM.read(i));
  }
  Serial.print("PASS: ");
  Serial.println(_pass1);
  /////////////////////////////////////////////////////////////
   //Đặt IP tĩnh thì enable lên
  //WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS);
  /////////////////////////////////////////////////////////////
  WiFi.begin(_ssid1.c_str(), _pass1.c_str());
  for(int i=0;i<=10;i++)
  {
   delay(1000);
   if ((WiFi.status() == WL_CONNECTED))
  {
     Serial.println(WiFi.localIP());
     Serial.println(WiFi.gatewayIP()); 
     i=10;
  }
  }
  Serial.println("Đã khởi động lại:");
  my_homekit_setup();
  //wifi_connect(); 
}

void loop() {
  my_homekit_loop();
  ESPButton.loop();
}
// access your HomeKit characteristics defined in my_accessory.c
extern "C" homekit_server_config_t config;

extern "C" homekit_characteristic_t cha_switch_on1;
extern "C" homekit_characteristic_t cha_switch_on2;



void cha_switch_on1_setter(const homekit_value_t value) {
  cha_switch_on1.value.bool_value = value.bool_value;  
  LOG_D("Switch1: %s", value.bool_value ? "ON" : "OFF");
  digitalWrite(LIGHT_01, value.bool_value);
}

void cha_switch_on2_setter(const homekit_value_t value) {
  cha_switch_on2.value.bool_value = value.bool_value;  
  LOG_D("Switch2: %s", value.bool_value ? "ON" : "OFF");
  digitalWrite(LIGHT_02, value.bool_value);
}

void button_pressed(uint8_t id, ESPButtonEvent event) {
  LOG_D("Button %d Event: %s", id, ESPButton.getButtonEventDescription(event));

  if (event == ESPBUTTONEVENT_SINGLECLICK) {
    invert_light(id);
  } else if (event == ESPBUTTONEVENT_LONGCLICK) {
    
    wifi_setup_ap();
  }
}

void invert_light(int button) {
  switch (button) {
    case 1:
      cha_switch_on1.value.bool_value = !cha_switch_on1.value.bool_value;
      digitalWrite(LIGHT_01, cha_switch_on1.value.bool_value);
      homekit_characteristic_notify(&cha_switch_on1, cha_switch_on1.value);
      break;
    case 2:
      cha_switch_on2.value.bool_value = !cha_switch_on2.value.bool_value;
      digitalWrite(LIGHT_02, cha_switch_on2.value.bool_value);
      homekit_characteristic_notify(&cha_switch_on2, cha_switch_on2.value);
      break;
  }
}

void init_lights_and_buttons() {
  pinMode(LIGHT_01, OUTPUT);
  pinMode(LIGHT_02, OUTPUT);
  digitalWrite(LIGHT_01, LOW);
  digitalWrite(LIGHT_02, LOW);

  pinMode(BUTTON_01, INPUT_PULLUP);
  pinMode(BUTTON_02, INPUT_PULLUP);
  ESPButton.add(1, BUTTON_01, LOW, false, true);
  ESPButton.add(2, BUTTON_02, LOW, false, true);

  ESPButton.setCallback(button_pressed);
  ESPButton.begin();
}

extern "C" void identify_switch_1(homekit_value_t _value) {
  invert_light(1);
  printf("identify switch 1\n");
}

extern "C" void identify_switch_2(homekit_value_t _value) {
  invert_light(2);
  printf("identify switch 2\n");
}

extern "C" void identify_accessory(homekit_value_t _value) {
  invert_light(1);
  invert_light(2);
  printf("identify accessory\n");
}

void my_homekit_setup() {
 
  cha_switch_on1.setter = cha_switch_on1_setter;
  cha_switch_on2.setter = cha_switch_on2_setter;
  arduino_homekit_setup(&config);
}

static uint32_t next_heap_millis = 0;
void my_homekit_loop() {
  arduino_homekit_loop();
  const uint32_t t = millis();
  if (t > next_heap_millis) {
    // show heap info every 5 seconds
    next_heap_millis = t + 5 * 1000;
    LOG_D("Free heap: %d, HomeKit clients: %d",
          ESP.getFreeHeap(), arduino_homekit_connected_clients_count());
  }
}

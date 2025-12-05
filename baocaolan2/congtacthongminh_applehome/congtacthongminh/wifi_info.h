#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include "WiFiManager.h"




extern "C" char accessoryManufacturer[];
extern "C" char accessorySerialNumber[];

void wifi_setup_ap() {
  WiFiManager wifiManager;
  wifiManager.setConfigPortalTimeout(60);
  char* apName;
  apName = (char*)malloc(strlen(accessoryManufacturer) + strlen(accessorySerialNumber) + 2);
  strcpy(apName, accessoryManufacturer);
  strcat(apName, "-");
  strcat(apName, accessorySerialNumber);
  wifiManager.startConfigPortal(apName);
  free(apName);
  Serial.println("Switched to AP");
}

void wifi_connect() {
  WiFi.persistent(true);
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);

}

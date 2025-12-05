#include <homekit/homekit.h>
#include <homekit/characteristics.h>

char accessoryName[] = "LMT";
char accessoryManufacturer[] = "LMT";
char accessorySerialNumber[] = "LMT";
char accessoryModel[] = "ESP8266";
char accessoryFirmwareRevision[] = "1.0";
char accessoryPassword[] = "210-92-004";

extern void identify_accessory(homekit_value_t);
extern void identify_switch_1(homekit_value_t);
extern void identify_switch_2(homekit_value_t);

homekit_characteristic_t cha_switch_on1 = HOMEKIT_CHARACTERISTIC_(ON, false);
homekit_characteristic_t cha_switch_on2 = HOMEKIT_CHARACTERISTIC_(ON, false);

homekit_accessory_t *accessories[] = {
  HOMEKIT_ACCESSORY(.id = 1, .category = homekit_accessory_category_switch, .services = (homekit_service_t *[]){ HOMEKIT_SERVICE(ACCESSORY_INFORMATION, .characteristics = (homekit_characteristic_t *[]){ HOMEKIT_CHARACTERISTIC(NAME, accessoryName), HOMEKIT_CHARACTERISTIC(MANUFACTURER, accessoryManufacturer), HOMEKIT_CHARACTERISTIC(SERIAL_NUMBER, accessorySerialNumber), HOMEKIT_CHARACTERISTIC(MODEL, accessoryModel), HOMEKIT_CHARACTERISTIC(FIRMWARE_REVISION, accessoryFirmwareRevision), HOMEKIT_CHARACTERISTIC(IDENTIFY, identify_accessory), NULL }), NULL }),

  HOMEKIT_ACCESSORY(.id = 2, .category = homekit_accessory_category_switch, .services = (homekit_service_t *[]){ HOMEKIT_SERVICE(ACCESSORY_INFORMATION, .characteristics = (homekit_characteristic_t *[]){ HOMEKIT_CHARACTERISTIC(NAME, "Switch 1"), HOMEKIT_CHARACTERISTIC(IDENTIFY, identify_switch_1), NULL }), HOMEKIT_SERVICE(SWITCH, .primary = true, .characteristics = (homekit_characteristic_t *[]){ &cha_switch_on1, HOMEKIT_CHARACTERISTIC(NAME, "Switch 1"), NULL }), NULL }),

  HOMEKIT_ACCESSORY(.id = 3, .category = homekit_accessory_category_switch, .services = (homekit_service_t *[]){ HOMEKIT_SERVICE(ACCESSORY_INFORMATION, .characteristics = (homekit_characteristic_t *[]){ HOMEKIT_CHARACTERISTIC(NAME, "Switch 2"), HOMEKIT_CHARACTERISTIC(IDENTIFY, identify_switch_2), NULL }), HOMEKIT_SERVICE(SWITCH, .primary = true, .characteristics = (homekit_characteristic_t *[]){ &cha_switch_on2, HOMEKIT_CHARACTERISTIC(NAME, "Switch 2"), NULL }), NULL }),

  NULL
};

homekit_server_config_t config = {
  .accessories = accessories,
  .password = accessoryPassword
};

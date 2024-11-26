#include <stdio.h>
#include "../DEVICES/devices.hpp"
#include "esp_log.h"

class AppManager
{
public:
  AppManager();
  virtual ~AppManager();
  static void message_callback(const char *topic, const char *message);
  void setup();
  void application(const byte *uidByte, size_t size);

private:
  static bool register_rfid;
  MQTTClient mqtt_client; // Membro da classe para o objeto MQTTClient
  WIFI wifi;
};
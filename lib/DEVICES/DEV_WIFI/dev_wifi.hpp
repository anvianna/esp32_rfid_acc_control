#ifndef WIFI_H
#define WIFI_H

#include "esp_wifi.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "freertos/event_groups.h"

class WIFI
{
public:
  WIFI(const char *ssid, const char *password);
  void start();

private:
  const char *ssid;
  const char *password;
  static void event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
  void wifi_init_sta();

  static EventGroupHandle_t s_wifi_event_group;
  static const char *TAG;
  static const int WIFI_CONNECTED_BIT;
};

#endif // WIFI_H

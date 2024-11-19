#ifndef DEV_WIFI_HPP
#define DEV_WIFI_HPP

#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <string.h>
#include <freertos/event_groups.h>

class WIFI
{
public:
  WIFI(const char *ssid, const char *password);
  void start();

private:
  const char *ssid;
  const char *password;
  static const char *TAG;
  static const int WIFI_CONNECTED_BIT;
  static EventGroupHandle_t s_wifi_event_group;

  static void event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
  void wifi_init_sta();
};

#endif // DEV_WIFI_HPP

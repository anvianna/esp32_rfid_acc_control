#ifndef DEV_WIFI_HPP
#define DEV_WIFI_HPP

#include "esp_wifi.h"
#include "esp_event.h"

class WiFiConnection {
public:
    void connect(const char* ssid, const char* password);
};

#endif // DEV_WIFI_HPP

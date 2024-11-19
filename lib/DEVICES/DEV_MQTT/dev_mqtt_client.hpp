#ifndef MQTT_CLIENT_HPP
#define MQTT_CLIENT_HPP

#include "mqtt_client.h"
#include "esp_log.h"
#include <functional>

class MQTTClient
{
public:
  using MessageCallback = std::function<void(const char *topic, const char *message)>;

  MQTTClient(const char *uri);
  void start();
  void publish(const char *topic, const char *data);
  void subscribe(const char *topic);
  void setMessageCallback(MessageCallback callback);

private:
  static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);
  const char *uri;
  esp_mqtt_client_handle_t client;

  MessageCallback messageCallback; // Callback for messages
};

#endif // MQTT_CLIENT_HPP
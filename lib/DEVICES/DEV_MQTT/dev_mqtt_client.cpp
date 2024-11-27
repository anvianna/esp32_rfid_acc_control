#include "dev_mqtt_client.hpp"

// Define the logging tag for this module
static const char *TAG = "MQTTClient";

char topic[128];   // Suponha um tamanho máximo razoável para o tópico
char message[512]; // Suponha um tamanho máximo razoável para a mensagem

// Constructor for the MQTTClient class, initializes the client with the given URI
MQTTClient::MQTTClient(const char *uri) : uri(uri), messageCallback(nullptr) {}

// Start the MQTT client
void MQTTClient::start()
{
  esp_mqtt_client_config_t mqtt_cfg = {}; // Initialize the MQTT client configuration structure
  mqtt_cfg.broker.address.uri = uri;      // Set the broker URI

  client = esp_mqtt_client_init(&mqtt_cfg);                                         // Initialize the MQTT client
  esp_mqtt_client_register_event(client, MQTT_EVENT_ANY, mqtt_event_handler, this); // Register the event handler
  esp_mqtt_client_start(client);                                                    // Start the MQTT client
}

// Publish a message to the specified topic
void MQTTClient::publish(const char *topic, const char *data)
{
  esp_mqtt_client_publish(client, topic, data, 0, 1, 0);
}

// Subscribe to the specified topic
void MQTTClient::subscribe(const char *topic)
{
  esp_mqtt_client_subscribe(client, topic, 0);
}

void MQTTClient::setMessageCallback(MessageCallback callback)
{
  messageCallback = callback; // Configura o callback
}

// Event handler for MQTT events
void MQTTClient::mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
  esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;
  MQTTClient *mqttClient = (MQTTClient *)handler_args;

  // Handle different types of MQTT events
  switch (event_id)
  {
  case MQTT_EVENT_CONNECTED:
    ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
    mqttClient->publish("lock/test", "Hello from ESP32!"); // Use the instance to call non-static methods
    mqttClient->subscribe("lock/access/confirmation");
    mqttClient->subscribe("lock/register/start");
    break;
  case MQTT_EVENT_DISCONNECTED:
    ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
    break;
  case MQTT_EVENT_SUBSCRIBED:
    ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
    break;
  case MQTT_EVENT_UNSUBSCRIBED:
    ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
    break;
  case MQTT_EVENT_PUBLISHED:
    ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
    break;
  case MQTT_EVENT_DATA:
    ESP_LOGI(TAG, "MQTT_EVENT_DATA");
    strncpy(topic, event->topic, event->topic_len);
    topic[event->topic_len] = '\0';

    strncpy(message, event->data, event->data_len);
    message[event->data_len] = '\0';

    ESP_LOGI(TAG, "Message received on topic: %s, data: %s", topic, message);

    // Triggers callback
    if (mqttClient->messageCallback)
    {
      mqttClient->messageCallback(topic, message);
    }
    break;
  case MQTT_EVENT_ERROR:
    ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
    break;
  default:
    ESP_LOGI(TAG, "Other event id:%d", event->event_id);
    break;
  }
}
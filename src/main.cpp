#include "devices.hpp"
#include <iostream>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "cJSON.h"

extern "C"
{

  WIFI wifi("Vrumvrum", "jayjayojatinho");
  MQTTClient mqttClient("mqtt://192.168.75.237:1883");
  void confirmationHandler(const char *topic, const char *message)
  {
    if (std::string(topic) == "lock/access/confirmation")
    {
      printf("Confirmation received: %s\n", message);
    }
  }

  void mqtt_task(void *pvParameters)
  {
    wifi.start();
    vTaskDelay(pdMS_TO_TICKS(2000)); // Aguarda Wi-Fi conectar
    mqttClient.start();

    mqttClient.setMessageCallback(confirmationHandler);
    mqttClient.start();
    mqttClient.subscribe("lock/access/confirmation");

    // Create the JSON object for the message
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "rfid", "1234569");
    char *json_str = cJSON_Print(root);
    mqttClient.publish("lock/access", json_str);

    // Clean up JSON object and string
    cJSON_Delete(root);
    free(json_str);

    while (true)
    {
      vTaskDelay(pdMS_TO_TICKS(1000)); // Mantém a tarefa viva
    }
  }

  void app_main()
  {
    xTaskCreate(&mqtt_task, "mqtt_task", 4096, nullptr, 5, nullptr);
  }

} // extern "C"

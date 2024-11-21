// mosquitto -v -c testconfig.txt / ativar mosquitto com o ip da maquina
#include "app.hpp"
#include "DEV_MQTT/dev_mqtt_client.hpp"
#include "DEV_WIFI/dev_wifi.hpp"
#include "esp_log.h"
#include "esp_system.h"
#include "cJSON.h"
#include <iostream>

WIFI wifi("Vrumvrum", "jayjayojatinho");                 // WiFi SSID and password
MQTTClient mqttClient("mqtt://test.mosquitto.org:1883"); // MQTT broker URI

// Timezone offset in seconds (3 hours for Brazil, GMT -3)
const long timezoneOffset = -3 * 3600;

/**
 * @brief Constructor for the AppManager class.
 */
AppManager::AppManager()
{
    initialize();
}

AppManager::~AppManager() {}

/**
 * @brief Initialize the application components.
 */
void AppManager::initialize()
{
    // Start WiFi and MQTT services
    wifi.start();
    mqttClient.start();
    vTaskDelay(pdMS_TO_TICKS(10000));
    mqttClient.setMessageCallback(confirmationHandler);
    mqttClient.subscribe("lock/access/confirmation");
}

/**
 * @brief Main application logic.
 */
void AppManager::application()
{
    // Create the JSON object for the message
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "rfid", "1234569");
    char *json_str = cJSON_Print(root);
    mqttClient.publish("lock/access", json_str);

    // Clean up JSON object and string
    cJSON_Delete(root);
    free(json_str);
}

void AppManager::confirmationHandler(const char *topic, const char *message)
{
    if (std::string(topic) == "lock/access/confirmation")
    {
        printf("Confirmation received: %s\n", message);
    }
}

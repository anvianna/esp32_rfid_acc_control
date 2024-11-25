#include "app.hpp"
#include "cJSON.h"
#include <string>

#define WIFI_SSID "Vrumvrum"
#define WIFI_PASS "jayjayojatinho"
#define MQTT_BROKER_URL "mqtt://test.mosquitto.org"

// Timezone offset in seconds (3 hours for Brazil, GMT -3)
const long timezoneOffset = -3 * 3600;

/**
 * @brief Constructor for the AppManager class.
 */
AppManager::AppManager() : mqtt_client(MQTT_BROKER_URL), wifi(WIFI_SSID, WIFI_PASS) {
	setup();
}

AppManager::~AppManager() {}

void AppManager::message_callback(const char *topic, const char *message)
{
    if (std::string(topic) == "lock/access/confirmation")
    {
		//logica quando o rfid for liberado aqui. Colocar ação do motor e leds...
        printf("Confirmation received: %s\n", message);
    }
}

void AppManager::setup() {
    wifi.start();

	mqtt_client.setMessageCallback(message_callback);
	mqtt_client.start();
	mqtt_client.subscribe("lock/topic/test");
	mqtt_client.publish("lock/topic/test", "Hello from ESP32!");
	mqtt_client.subscribe("lock/access/confirmation");
}

/**
 * @brief Main application logic.
 */
void AppManager::application()
{

	//Quando houver uma leitura de rfid fazer o processo a seguir:
	// Create the JSON object for the message
	cJSON *root = cJSON_CreateObject();
	//trocar o dado 1234569 pela rfid lida
	cJSON_AddStringToObject(root, "rfid", "1234569");
	char *json_str = cJSON_Print(root);
	//publish msg
	mqtt_client.publish("lock/access", json_str);
	// Clean up JSON object and string
	cJSON_Delete(root);
	free(json_str);
}


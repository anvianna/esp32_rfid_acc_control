#include "app.hpp"
#include "cJSON.h"
#include <string>

#define WIFI_SSID "Vrumvrum"
#define WIFI_PASS "jayjayojatinho"
#define MQTT_BROKER_URL "mqtt://test.mosquitto.org"

const char *registeringName;
bool register_rfid;

// Timezone offset in seconds (3 hours for Brazil, GMT -3)
const long timezoneOffset = -3 * 3600;

/**
 * @brief Constructor for the AppManager class.
 */
AppManager::AppManager() : mqtt_client(MQTT_BROKER_URL), wifi(WIFI_SSID, WIFI_PASS)
{
	register_rfid = false;
	setup();
}

AppManager::~AppManager() {}

void AppManager::message_callback(const char *topic, const char *message)
{
	if (std::string(topic) == "lock/access/confirmation")
	{
		if (std::string(message) == "1")
		{
			// logica quando o rfid for liberado aqui. Colocar ação do motor e leds...
		}
		if (std::string(message) == "0")
		{
			// 0 para access denied! Piscar led vermelha.
		}
	}
	if (std::string(topic) == "lock/register/start")
	{
		register_rfid = true;
		registeringName = message; // O nome é enviado na mensagem
	}
	printf("Topic: %s, Message: %s\n", topic, message);
}

void AppManager::setup()
{
	wifi.start();

	mqtt_client.setMessageCallback(message_callback);
	mqtt_client.start();
	mqtt_client.publish("lock/test", "Hello from ESP32!");
	mqtt_client.subscribe("lock/access/confirmation");
	mqtt_client.subscribe("lock/register/start");
}

/**
 * @brief Main application logic.
 */
void AppManager::application(const byte *uidByte, size_t size)
{
	// Convert UID bytes to a string representation
	char rfid_str[21]; // Each byte represented as 2 hex digits + null terminator (10 * 2 + 1)
	for (size_t i = 0; i < size; ++i)
	{
		sprintf(&rfid_str[i * 2], "%02X", uidByte[i]);
	}

	// Create the JSON object for the message
	cJSON *root = cJSON_CreateObject();
	cJSON_AddStringToObject(root, "rfid", rfid_str);
	char *json_str = cJSON_Print(root);

	if (register_rfid)
	{
		cJSON_AddStringToObject(root, "name", registeringName);
		char *json_str = cJSON_Print(root);
		// Publish the JSON message to the register topic
		mqtt_client.publish("lock/register/completed", json_str);
		register_rfid = false;
		// desativa blink led registro
	}
	else
	{
		char *json_str = cJSON_Print(root);
		// Publish the JSON message to access topic
		mqtt_client.publish("lock/access", json_str);
	}

	// Clean up JSON object and string
	cJSON_Delete(root);
	free(json_str);
}

#include "app.hpp"
#include "cJSON.h"
#include <string>

#define WIFI_SSID "Vrumvrum"
#define WIFI_PASS "jayjayojatinho"
#define MQTT_BROKER_URL "mqtt://test.mosquitto.org"
#define SERVO_PIN 4
static AppManager *appManagerInstance = nullptr;

bool register_rfid = false;					// Inicializar como false
char registeringName[100] = {'\0'}; // Inicializar como uma string vazia

// Timezone offset in seconds (3 hours for Brazil, GMT -3)
const long timezoneOffset = -3 * 3600;

/**
 * @brief Constructor for the AppManager class.
 */
AppManager::AppManager() : mqtt_client(MQTT_BROKER_URL), servo(SERVO_PIN), ledBlue(PA2), ledRed(PA26), ledYellow(PA27)
{
	DrvGPIO resetRFID(PA17);
	resetRFID.write(false);
	resetRFID.write(true);
	register_rfid = false;
	ledBlue.write(false);
	ledRed.write(false);
	ledYellow.write(false);
	setup();
	// Armazenando a instância do AppManager
	appManagerInstance = this;
}

AppManager::~AppManager() {}

void AppManager::message_callback(const char *topic, const char *message)
{
	if (std::string(topic) == "lock/access/confirmation")
	{
		if (std::string(message) == "1")
		{
			// LED TOGGLE 3x (pisca 3 vezes)
			for (int i = 0; i < 3; ++i)
			{
				appManagerInstance->ledBlue.toggle(); // Acende o LED
				vTaskDelay(pdMS_TO_TICKS(500));				// Espera 500ms
				appManagerInstance->ledBlue.toggle(); // Apaga o LED
				vTaskDelay(pdMS_TO_TICKS(500));				// Espera 500ms
			}
			appManagerInstance->servo.clockWise(0, 90, 90, 1000);
			ledc_get_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
			appManagerInstance->servo.anticlockWise(90, 0, 90, 1000);
			ledc_get_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
			// logica quando o rfid for liberado aqui. Colocar ação do motor e leds...
		}
		if (std::string(message) == "0")
		{
			// LED TOGGLE 3x (pisca 3 vezes)
			for (int i = 0; i < 3; ++i)
			{
				appManagerInstance->ledRed.toggle(); // Acende o LED
				vTaskDelay(pdMS_TO_TICKS(500));			 // Espera 500ms
				appManagerInstance->ledRed.toggle(); // Apaga o LED
				vTaskDelay(pdMS_TO_TICKS(500));			 // Espera 500ms
			}
			// 0 para access denied! Piscar led vermelha.
		}
	}
	if (std::string(topic) == "lock/register/start")
	{
		// LED TOGGLE 3x (pisca 3 vezes)
		for (int i = 0; i < 3; ++i)
		{
			appManagerInstance->ledYellow.toggle(); // Acende o LED
			vTaskDelay(pdMS_TO_TICKS(500));					// Espera 500ms
			appManagerInstance->ledYellow.toggle(); // Apaga o LED
			vTaskDelay(pdMS_TO_TICKS(500));					// Espera 500ms
		}
		register_rfid = true;
		strncpy(registeringName, message, sizeof(registeringName) - 1);
		registeringName[sizeof(registeringName) - 1] = '\0'; // Garantir que termina em '\0'
	}
	printf("Topic: %s, Message: %s\n", topic, message);
}

void AppManager::setup()
{
	wifi_config.mode = DRV_WIFI_MODE_STA;
	wifi_config.ssid_sta = const_cast<char *>(WIFI_SSID);
	wifi_config.pass_sta = const_cast<char *>(WIFI_PASS);
	DRV_WIFI_Start(&wifi_config);

	while (DRV_WIFI_connected() != 2)
		vTaskDelay(1);

	mqtt_client.setMessageCallback(message_callback);
	mqtt_client.start();

	servo.init();
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

	if (register_rfid)
	{
		cJSON_AddStringToObject(root, "name", registeringName);
		char *json_str = cJSON_Print(root);
		// Publish the JSON message to the register topic
		mqtt_client.publish("lock/register/completed", json_str);
		register_rfid = false;
		free(json_str);
	}
	else
	{
		char *json_str = cJSON_Print(root);
		// Publish the JSON message to access topic
		mqtt_client.publish("lock/access", json_str);
		printf("Publicando no tópico: %s com mensagem: %s\n", (register_rfid ? "lock/register/completed" : "lock/access"), json_str);

		free(json_str);
	}

	// Clean up JSON object and string
	cJSON_Delete(root);
}

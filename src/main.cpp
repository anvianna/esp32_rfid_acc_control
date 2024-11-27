#include "drivers.hpp"
#include "devices.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/spi_master.h"
#include "esp_timer.h"

// Ricarti
#include "main.hpp"
#include "app.hpp"

// GPIO PINOUT TO SPI
#define PIN_CLK drvIoPin_t::DRV_IO_PIN_18
#define PIN_MISO drvIoPin_t::DRV_IO_PIN_19
#define PIN_MOSI drvIoPin_t::DRV_IO_PIN_23
#define PIN_SS drvIoPin_t::DRV_IO_PIN_5
#define PIN_RST drvIoPin_t::DRV_IO_PIN_17

spi_config_t config =
    {
        .PIN_NUM_MOSI = (int)PIN_MOSI,
        .PIN_NUM_MISO = (int)PIN_MISO,
        .PIN_NUM_CLK = (int)PIN_CLK,
        .CLOCK_SPEED_HZ = MFRC522_SPICLOCK,
        .MODE = driverSPImode_t::DRV_SPI_CLK_0_PHA_0,
        .SPI_host_number = driverSPIPeripheral_t::DRV_SPI_PERIPHERAL_2};

// spi_config_t config;
//  Spi handling
DrvSPI spi_instance(&config, drvIoPort_t::DRV_IO_PORT_A, PIN_SS, false);
// Pin for reseting MFRC522
DrvGPIO RFID_reset(drvIoPort_t::DRV_IO_PORT_A, PIN_RST);
// Object construction
MFRC522 rfid_reader(&spi_instance, &RFID_reset);

uint8_t LastUidProcessed[10];
bool refresh_UID = true;

TaskHandle_t rfid_task_handle = nullptr;
TaskHandle_t MQTT_task_handle = nullptr;
QueueHandle_t UidQueue;

void rfidTask(void *pvParameter)
{
    printf("Inicializando RFID...\n");
    rfid_reader.PCD_Init();
    if (!rfid_reader.PCD_PerformSelfTest())
    {
        printf("Erro no teste do MFRC522!\n");
        vTaskDelay(portMAX_DELAY); // Bloqueia se o RFID não for inicializado
    }

    MFRC522::Uid lastUid = {};
    int64_t lastCheckTime = esp_timer_get_time();
    for (;;)
    {
        if ((esp_timer_get_time() - lastCheckTime) > 2000000)
        {
            refresh_UID = true;
            lastCheckTime = esp_timer_get_time();
        }

        if (rfid_reader.PICC_IsNewCardPresent() && rfid_reader.PICC_ReadCardSerial())
        {
            if (refresh_UID || !(lastUid == rfid_reader.uid))
            {
                lastUid = rfid_reader.uid;
                refresh_UID = false;
                if (xQueueSend(UidQueue, &rfid_reader.uid, pdMS_TO_TICKS(100)) != pdPASS)
                {
                    printf("Erro ao enviar UID para a fila.\n");
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(100)); // Reduz consumo de CPU
    }
}

void AppManagerTask(void *pvParameters)
{
    AppManager appManager;
    MFRC522::Uid receivedUid;
    for (;;)
    {
        if (xQueueReceive(UidQueue, &receivedUid, pdMS_TO_TICKS(1000)) == pdPASS)
        {
            appManager.application(receivedUid.uidByte, receivedUid.size);
            printf("UID enviado via MQTT.\n");
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void app_main()
{
    printf("Iniciando aplicação...\n");
    UidQueue = xQueueCreate(5, sizeof(MFRC522::Uid));
    if (UidQueue == NULL)
    {
        printf("Falha ao criar fila de UIDs.\n");
        return;
    }

    xTaskCreate(rfidTask, "rfidTask", 4096, NULL, 5, &rfid_task_handle);
    xTaskCreate(AppManagerTask, "AppManagerTask", 8192, NULL, 10, &MQTT_task_handle);
    for (;;)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

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

#if 1
spi_config_t config = 
    {
    .PIN_NUM_MOSI = (int)PIN_MOSI,
    .PIN_NUM_MISO = (int)PIN_MISO,
    .PIN_NUM_CLK =  (int)PIN_CLK,
    .CLOCK_SPEED_HZ = MFRC522_SPICLOCK,
    .MODE = driverSPImode_t::DRV_SPI_CLK_0_PHA_0,
    .SPI_host_number = driverSPIPeripheral_t::DRV_SPI_PERIPHERAL_2
    };

//spi_config_t config;    
// Spi handling
DrvSPI spi_instance(&config,drvIoPort_t::DRV_IO_PORT_A,PIN_SS,false);
// Pin for reseting MFRC522
DrvGPIO RFID_reset(drvIoPort_t::DRV_IO_PORT_A,PIN_RST);
// Object construction
MFRC522 rfid_reader(&spi_instance,&RFID_reset);

uint8_t LastUidProcessed[10];
bool refresh_UID = true;

TaskHandle_t rfid_task_handle = nullptr;
TaskHandle_t MQTT_task_handle = nullptr;
QueueHandle_t UidQueue;

void rfidTask(void *pvParameter)
{
    printf("Teste22\n");
    //vTaskDelay(pdMS_TO_TICKS(5000));
    MFRC522::Uid lastUId;
    rfid_reader.PCD_Init();
    rfid_reader.PCD_DumpVersionToSerial();
    //printf("testing MFRC522\n");
    // if(0)//!rfid_reader.PCD_PerformSelfTest())
    // {
    //     printf("*Test Failed - 2");
    //     fflush(stdout);
    //     while (1)
    //     {
    //         vTaskDelay(1);
    //     }
        
    // }
    printf("Scan PICC to see UID, SAK, type, and data blocks...\n");
    int64_t now = esp_timer_get_time();
    for(;;)
    {
        if(now+2e6 <= esp_timer_get_time())
        {
            printf("...\n");
            refresh_UID = true;
            now = esp_timer_get_time();
        }
        vTaskDelay(1);
        if(rfid_reader.PICC_IsNewCardPresent())
        {
            printf("CARD IS PRESENT\n");
            if(rfid_reader.PICC_ReadCardSerial())
            {   
                rfid_reader.PICC_DumpDetailsToSerial(&(rfid_reader.uid));
                printf("\n**********\n");
                if(!refresh_UID && (lastUId == rfid_reader.uid))
                {
                    refresh_UID = false;
                    printf("Same RFID\n");
                    vTaskDelay(pdMS_TO_TICKS(1000));
                    continue;
                }
                lastUId = rfid_reader.uid;
                if(!(xQueueSend(UidQueue,&rfid_reader.uid,pdMS_TO_TICKS(100)) == pdPASS))
                {
                    printf("Error QueueSend");
                }
            }
            else continue;
        }
    }
}
#endif

void AppManagerTask(void *pvParameters)
{
  AppManager appManager;
  MFRC522::Uid MyUid;
  for(;;)
  {
    // Check if rfID sent Data
    if(xQueueReceive(UidQueue,&MyUid,pdMS_TO_TICKS(10)) == pdPASS)
    {
        // Send throug MQTT
        appManager.application();
    }
    vTaskDelay(3000 / portTICK_PERIOD_MS);
  }
}



extern "C"{
void app_main()
{
    printf("Teste1\n");
    UidQueue = xQueueCreate(5,sizeof(MFRC522::uid));
    //Setting RFID reading
    xTaskCreate(rfidTask,"rfidTask",4096,NULL,1,&rfid_task_handle);
    xTaskCreate(AppManagerTask, "AppManagerTask", 4096, NULL, 1, &MQTT_task_handle);
    vTaskStartScheduler();
    for(;;)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
} // extern "C"






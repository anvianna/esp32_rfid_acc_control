#include "drivers.hpp"
#include "devices.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"
#include "esp_timer.h"

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
bool new_Uid = false;

void rfidTask(void *pvParameter)
{
    printf("Teste22\n");
    //vTaskDelay(pdMS_TO_TICKS(5000));
    rfid_reader.PCD_Init();
    rfid_reader.PCD_DumpVersionToSerial();
    //printf("testing MFRC522\n");
    if(0)//!rfid_reader.PCD_PerformSelfTest())
    {
        printf("*Test Failed - 2");
        fflush(stdout);
        while (1)
        {
            vTaskDelay(1);
        }
        
    }
    printf("Scan PICC to see UID, SAK, type, and data blocks...\n");
    //uint8_t LastUid[10];
    int64_t now = esp_timer_get_time();
    for(;;)
    {
        //vTaskDelay(pdMS_TO_TICKS(20));
        if(now+2e6 <= esp_timer_get_time())
        {
            printf("...\n");
            now = esp_timer_get_time();
        //vTaskDelay(pdMS_TO_TICKS(10));
        //if(!rfid_reader.PICC_IsNewCardPresent() || !rfid_reader.PICC_ReadCardSerial())
        //{
        //    continue;
        //}
        }
        vTaskDelay(1);
        // if(!rfid_reader.PICC_IsNewCardPresent() || !rfid_reader.PICC_ReadCardSerial())
        // {
        //     //taskYIELD();
        //     //
        //     continue;
        // }
        if(rfid_reader.PICC_IsNewCardPresent())
        {
            printf("CARD IS PRESEEEEEEEEEEEEEEEEEEENT\n");
            if(rfid_reader.PICC_ReadCardSerial())
            {        
                rfid_reader.PICC_DumpDetailsToSerial(&(rfid_reader.uid));
                printf("\n**********\n");
            }
            else continue;
        }
    }
}
#endif

TaskHandle_t rfid_task_handle = nullptr;


extern "C"{
void app_main()
{
    printf("Teste1\n");

    //Setting RFID reading
    xTaskCreatePinnedToCore(rfidTask,"rfidTask",1024*10,NULL,1,&rfid_task_handle,0);

    for(;;)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
} // extern "C"
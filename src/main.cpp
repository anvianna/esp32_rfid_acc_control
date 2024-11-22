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
DrvGPIO RFID_reset(drvIoPort_t::DRV_IO_PORT_A,drvIoPin_t::DRV_IO_PIN_12);
// Object construction
MFRC522 rfid_reader(&spi_instance,&RFID_reset);

uint8_t LastUidProcessed[10];
bool new_Uid = false;

void rfidTask(void *pvParameter)
{
    printf("Teste22\n");
    //vTaskDelay(pdMS_TO_TICKS(5000));
    rfid_reader.PCD_Init();
    printf("Teste2\n");
    uint8_t LastUid[10];
    int64_t now = esp_timer_get_time();
    for(;;)
    {
        //vTaskDelay(pdMS_TO_TICKS(500));
        if(now+1000000 >= esp_timer_get_time())
        {
            printf("now: %lli\n",now);
            now = esp_timer_get_time();
        //vTaskDelay(pdMS_TO_TICKS(10));
        //if(!rfid_reader.PICC_IsNewCardPresent() || !rfid_reader.PICC_ReadCardSerial())
        //{
        //    continue;
        //}
        if(rfid_reader.PICC_IsNewCardPresent())
        {
            printf("card present");
            if(rfid_reader.PICC_ReadCardSerial())
            {
                printf("New_card detected:");
                new_Uid = true;
                for(uint8_t i =0; i < rfid_reader.uid.size; i++)
                {
                    LastUid[i] = rfid_reader.uid.uidByte[i];
                    printf("%X",LastUid[i]);
                    if(new_Uid && (LastUid[i] != LastUidProcessed[i])){
                        new_Uid = false;
                    }
                }
                printf("\n\r");
                if(new_Uid){
                    memcpy(LastUidProcessed,LastUid,sizeof(LastUid));
                }
                rfid_reader.PICC_HaltA();
                rfid_reader.PCD_StopCrypto1();
            }
            continue;
        }
        }
        
    }
}

TaskHandle_t rfid_task_handle = nullptr;

extern "C"{
void app_main()
{
    printf("Teste1\n");
    for(uint8_t i = 0; i < 10; i++)
    {
        LastUidProcessed[i] = 0;
    }

    //Setting RFID reading
    xTaskCreate(rfidTask,"rfidTask",1024*10,NULL,1,&rfid_task_handle);

    for(;;)
    {
        vTaskDelay(1);
    }
}
} // extern "C"
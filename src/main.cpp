#include "drivers.hpp"
#include "devices.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"

// PINOUT TO SPI
#define PIN_CLK 35
#define PIN_MISO 38
#define PIN_MOSI 36
#define PIN_SS 34

spi_config_t config = 
    {
        
        .PIN_NUM_MOSI = PIN_MOSI,
        .PIN_NUM_MISO = PIN_MISO,
        .PIN_NUM_CLK = PIN_CLK,
        .CLOCK_SPEED_HZ = MFRC522_SPICLOCK,
        .MODE = driverSPImode_t::DRV_SPI_CLK_0_PHA_0,
        .SPI_host_number = driverSPIPeripheral_t::DRV_SPI_PERIPHERAL_1
    };
    
// Spi handling
DrvSPI spi_instance(&config,drvIoPort_t::DRV_IO_PORT_A,drvIoPin_t(PIN_SS),false);
// Pin for reseting MFRC522
DrvGPIO RFID_reset(drvIoPort_t::DRV_IO_PORT_A,drvIoPin_t::DRV_IO_PIN_30);
// Object construction
MFRC522 rfid_reader(&spi_instance,&RFID_reset);

uint8_t LastUidProcessed[10];
bool new_Uid = false;

void rfidTask(void *pvParameter)
{
    uint8_t LastUid[10];
    if(!rfid_reader.PICC_IsNewCardPresent() || !rfid_reader.PICC_ReadCardSerial())
    {
        return;
    }
    new_Uid = true;
    for(uint8_t i =0; i < rfid_reader.uid.size; i++)
    {
        LastUid[i] = rfid_reader.uid.uidByte[i];
        if(new_Uid && (LastUid[i] != LastUidProcessed[i])){
            new_Uid = false;
        }
    }
    if(new_Uid){
        memcpy(LastUidProcessed,LastUid,sizeof(LastUid));
    }
    rfid_reader.PICC_HaltA();
    rfid_reader.PCD_StopCrypto1();
}

TaskHandle_t rfid_task_handle = nullptr;

extern "C"{
void app_main()
{
    
    for(uint8_t i = 0; i < 10; i++)
    {
        LastUidProcessed[i] = 0;
    }
    //Setting RFID reading
    rfid_reader.PCD_Init();

    xTaskCreate(rfidTask,"rfidTask",1024,NULL,1,&rfid_task_handle);
}
} // extern "C"
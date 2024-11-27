/**
 * @file dev_rfid.hpp
 * @author André Vianna (andre.v@fieb.org.br)
 * @brief 
 * @version 0.1
 * @date 2024-11-12
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#ifdef  DEV_RFID_HPP
#define DEV_RFID_HPP

#include "drivers.hpp"

class DevRFID
{
private:
  /* data */
  void * driver;
public:
  DevRFID(DrvSPI &spi);
  DevRFID(DrvUART &UART);
  ~DevRFID();
};

DevRFID::DevRFID(DrvSPI &spi)
{
}

DevRFID::DevRFID(DrvUART &UART)
{
}

DevRFID::~DevRFID()
{
}



#endif // DEV_RFID_HPP
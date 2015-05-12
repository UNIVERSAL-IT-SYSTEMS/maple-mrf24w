
#ifndef MRF24W_H
#define	MRF24W_H

#include <stdint.h>
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_dma.h"
#include "stm32f4xx_hal_spi.h"
#include "mrf24w_g2100.h"

void (*Mrf24wProcessEvent)(uint8_t event, uint16_t eventInfo, uint8_t* extraInfo);

void Mrf24w_init(SPI_TypeDef *spi, uint16_t resetPin, uint16_t intPin);
void mrf24w_begin();
void mrf24w_end();
void mrf24w_loop();
void mrf24w_connect();
void mrf24w_scan(uint8_t cpid);

void mrf24w_setLocalIp(uint8_t localIpAddr[]);
void mrf24w_setGatewayIp(uint8_t gatewayIpAddr[]);
void mrf24w_setSubnetMask(uint8_t subnetMask[]);
void mrf24w_setSSID(const char* ssid);
void mrf24w_setSecurityPassphrase(const char* securityPassphrase);
void mrf24w_setSecurityType(uint8_t securityType);
void mrf24w_setWirelessMode(uint8_t wirelessMode);

void mrf24w_setProcessEventFn(void * processEventFn);

uint8_t m_csPin;
uint8_t m_intPin;
//Mrf24wProcessEvent m_processEventFn;
void * m_processEventFn;
char m_ssid[32];
uint8_t m_securityPassphrase[64];
uint8_t m_securityPassphraseLen;
uint8_t m_securityType;
uint8_t m_wirelessMode;

void mrf24w_wf_processEvent(uint8_t event, uint16_t eventInfo, uint8_t* extraInfo);

void mrf24w_processEvent(uint8_t event, uint16_t eventInfo, uint8_t* extraInfo);


#endif	/* MRF24W_H */


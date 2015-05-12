
#include "mrf24w.h"
#include "mrf24w_g2100.h"
#include "mrf24w_stack.h"
#include <string.h>
#include <stdio.h>

#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_dma.h"
#include "stm32f4xx_hal_spi.h"
#include "stm32f4xx_hal_gpio.h"

#include "pico_defines.h"
#include "pico_config.h"
#include "pico_device.h"

void *g_mrf24wInstance = NULL;
uint8_t stack_local_ip[4] = {192, 168, 0, 99};
uint8_t stack_gateway_ip[4] = {192, 168, 0, 1};
uint8_t stack_subnet_mask[4] = {255, 255, 255, 0};

//SPI_HandleTypeDef* wf_spi;
SPI_TypeDef* wf_spi;
GPIO_TypeDef* wf_cs_port;
uint8_t wf_cs_bit;

void wf_processEvent(uint8_t event, uint16_t eventInfo, uint8_t* extraInfo) {
    ASSERT(g_mrf24wInstance);
    mrf24w_processEvent(event, eventInfo, extraInfo);
}

struct pico_device * pico_eth_create(char *name, uint8_t *mac)
{
    struct pico_device * mrf24wg = PICO_ZALLOC(sizeof(struct pico_device));
    if (!mrf24wg)
        return NULL;

    /*          SPI     RESET       INTERRUPT   */
    Mrf24w_init(SPI1, GPIO_PIN_2, GPIO_PIN_0);

    return mrf24wg;
}

void Mrf24w_init(SPI_TypeDef *spi, uint16_t resetPin, uint16_t intPin)
//: m_csPin(csPin), m_intPin(intPin), m_processEventFn(NULL)
{
    //XXX: TODO
    //const stm32_pin_info *csPinInfo = &PIN_MAP[csPin];
    //XXX: TODO: set instance?
    //g_mrf24wInstance = this;

    m_securityType = WF_SECURITY_OPEN;
    m_wirelessMode = WF_INFRASTRUCTURE;

    wf_spi = spi; //.c_dev();
    // TODO: set port
    //wf_cs_port = csPinInfo->gpio_device;
    //wf_cs_bit = csPin; //csPinInfo->gpio_bit;
}

void mrf24w_begin() {
    // XXX TODO
    //pinMode(m_intPin, INPUT_PULLUP);
    //attachInterrupt(m_intPin, wf_isr, FALLING);

    // XXX TODO
    //pinMode(m_csPin, OUTPUT);
    //digitalWrite(m_csPin, HIGH);

    printf("wf_init");
    //  wf_hook_on_connected = Mrf24w_wf_hook_on_connected;
    //  wf_hook_on_connected_user_data = this;
    wf_init();

    stack_init();

    printf("begin end");
}

void mrf24w_end() {

}

void mrf24w_setProcessEventFn(void * processEventFn)
{
    m_processEventFn = processEventFn;
}

void mrf24w_scan(uint8_t cpid) {
    wf_scan(cpid);
}

void mrf24w_connect() {
    uint8_t connectionProfileId;
    uint8_t channelList[] = {};

    connectionProfileId = wf_cpCreate();
    ASSERT(connectionProfileId != 0xff);
    wf_cpSetSsid(connectionProfileId, (uint8_t*) m_ssid, strlen(m_ssid));
    wf_cpSetNetworkType(connectionProfileId, m_wirelessMode);
    wf_caSetScanType(WF_ACTIVE_SCAN);
    wf_caSetChannelList(channelList, sizeof (channelList));
    wf_caSetListRetryCount(10);
    wf_caSetBeaconTimeout(40);
    wf_cpSetSecurity(connectionProfileId, m_securityType, 0, m_securityPassphrase, m_securityPassphraseLen);
    wf_cmConnect(connectionProfileId);
}

void mrf24w_loop() {
    if (wf_connected) {
        stack_loop();
    }
    wf_macProcess();
}

void mrf24w_setLocalIp(uint8_t localIpAddr[]) {
    memcpy(stack_local_ip, localIpAddr, 4);
}

void mrf24w_setGatewayIp(uint8_t gatewayIpAddr[]) {
    memcpy(stack_gateway_ip, gatewayIpAddr, 4);
}

void mrf24w_setSubnetMask(uint8_t subnetMask[]) {
    memcpy(stack_subnet_mask, subnetMask, 4);
}

void mrf24w_setSSID(const char* ssid) {
    strcpy(m_ssid, ssid);
}

void mrf24w_setSecurityPassphrase(const char* securityPassphrase) {
    strcpy((char*) m_securityPassphrase, securityPassphrase);
    m_securityPassphraseLen = strlen(securityPassphrase);
}

void mrf24w_setSecurityType(uint8_t securityType) {
    m_securityType = securityType;
}

void mrf24w_setWirelessMode(uint8_t wirelessMode) {
    m_wirelessMode = wirelessMode;
}

void mrf24w_processEvent(uint8_t event, uint16_t eventInfo, uint8_t* extraInfo) {
    wf_setFuncState(WF_PROCESS_EVENT_FUNC, WF_ENTERING_FUNCTION);

    printf("wf_processEvent: %s, %s\n", event, eventInfo);

    switch (event) {
        case WF_EVENT_CONNECTION_SUCCESSFUL:
            printf("WF_EVENT_CONNECTION_SUCCESSFUL\n");
            break;

        case WF_EVENT_CONNECTION_FAILED:
            printf("WF_EVENT_CONNECTION_FAILED: %s\n", eventInfo);
            break;

        case WF_EVENT_CONNECTION_TEMPORARILY_LOST:
            printf("WF_EVENT_CONNECTION_TEMPORARILY_LOST: %s\n", eventInfo);
            break;

        case WF_EVENT_CONNECTION_PERMANENTLY_LOST:
            printf("WF_EVENT_CONNECTION_PERMANENTLY_LOST: %s\n", eventInfo);
            break;

        case WF_EVENT_CONNECTION_REESTABLISHED:
            printf("WF_EVENT_CONNECTION_REESTABLISHED\n");
            break;

        case WF_EVENT_SCAN_RESULTS_READY:
            {
                uint16_t i;
                tWFScanResult scanResult;
                printf("WF_EVENT_SCAN_RESULTS_READY: %s\n", eventInfo);
                for (i = 0; i < eventInfo; i++) {
                    char ssid[20];
                    wf_scanGetResult(i, &scanResult);
                    strncpy(ssid, (const char*) scanResult.ssid, scanResult.ssidLen);
                    ssid[scanResult.ssidLen] = '\0';
                    printf("%s\n", ssid);
                }
                break;
            }

        case WF_EVENT_RX_PACKET_RECEIVED:
            printf("WF_EVENT_RX_PACKET_RECEIVED: %s\n", eventInfo);
            break;

        case WF_EVENT_INVALID_WPS_PIN:
            printf("WF_EVENT_INVALID_WPS_PIN\n");
            break;

        default:
            printf("UNKNOWN Event: %s\n", eventInfo);
            break;
    }

    if (m_processEventFn) {
        // TODO: make function pointer!
        //m_processEventFn(event, eventInfo, extraInfo);
    }

    wf_setFuncState(WF_PROCESS_EVENT_FUNC, WF_LEAVING_FUNCTION);
}

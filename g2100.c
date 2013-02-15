#include <string.h>
#include "uip/uip.h"
#include "g2100.h"
#include "spi.h"

#include "libmaple.h"
#include "nvic.h"
#include "delay.h"

#include <libmaple/usart.h>

#define BOOL  uint8_t
#define FALSE 0
#define TRUE  1

void usart_puthex4(uint8_t value) {
  char ch;
  value = value & 0xf;
  if (value > 10) {
    ch = (value - 10) + 'a';
  } else {
    ch = value + '0';
  }
  usart_putc(USART1, ch);
}

void usart_puthex8(uint8_t value) {
  usart_putc(USART1, ' ');
  usart_putc(USART1, '0');
  usart_putc(USART1, 'x');
  usart_puthex4(value >> 4);
  usart_puthex4(value >> 0);
  usart_putc(USART1, ' ');
}

void usart_puthex16(uint16_t value) {
  usart_putc(USART1, ' ');
  usart_putc(USART1, '0');
  usart_putc(USART1, 'x');
  usart_puthex4(value >> 12);
  usart_puthex4(value >> 8);
  usart_puthex4(value >> 4);
  usart_puthex4(value >> 0);
  usart_putc(USART1, ' ');
}

#define MRF24WB0M_DEVICE  (1)
#define MRF24WG0M_DEVICE  (2)

#define WF_READ_REGISTER_MASK          ((uint8_t)(0x40))
#define WF_WRITE_REGISTER_MASK         ((uint8_t)(0x00))

#define WF_MAC_ADDRESS_LENGTH            (6)

/*------------------------------------------------------------------------------*/
/* These are error codes returned in the result field of a management response. */
/*------------------------------------------------------------------------------*/
#define WF_SUCCESS                                              ((uint16_t)1)
#define WF_ERROR_INVALID_SUBTYPE                                ((uint16_t)2)
#define WF_ERROR_OPERATION_CANCELLED                            ((uint16_t)3)
#define WF_ERROR_FRAME_END_OF_LINE_OCCURRED                     ((uint16_t)4)
#define WF_ERROR_FRAME_RETRY_LIMIT_EXCEEDED                     ((uint16_t)5)
#define WF_ERROR_EXPECTED_BSS_VALUE_NOT_IN_FRAME                ((uint16_t)6)
#define WF_ERROR_FRAME_SIZE_EXCEEDS_BUFFER_SIZE                 ((uint16_t)7)
#define WF_ERROR_FRAME_ENCRYPT_FAILED                           ((uint16_t)8)
#define WF_ERROR_INVALID_PARAM                                  ((uint16_t)9)
#define WF_ERROR_AUTH_REQ_ISSUED_WHILE_IN_AUTH_STATE            ((uint16_t)10)
#define WF_ERROR_ASSOC_REQ_ISSUED_WHILE_IN_ASSOC_STATE          ((uint16_t)11)
#define WF_ERROR_INSUFFICIENT_RESOURCES                         ((uint16_t)12)
#define WF_ERROR_TIMEOUT_OCCURRED                               ((uint16_t)13)
#define WF_ERROR_BAD_EXCHANGE_ENCOUNTERED_IN_FRAME_RECEPTION    ((uint16_t)14)
#define WF_ERROR_AUTH_REQUEST_REFUSED                           ((uint16_t)15)
#define WF_ERROR_ASSOCIATION_REQUEST_REFUSED                    ((uint16_t)16)
#define WF_ERROR_PRIOR_MGMT_REQUEST_IN_PROGRESS                 ((uint16_t)17)
#define WF_ERROR_NOT_IN_JOINED_STATE                            ((uint16_t)18)
#define WF_ERROR_NOT_IN_ASSOCIATED_STATE                        ((uint16_t)19)
#define WF_ERROR_NOT_IN_AUTHENTICATED_STATE                     ((uint16_t)20)
#define WF_ERROR_SUPPLICANT_FAILED                              ((uint16_t)21)
#define WF_ERROR_UNSUPPORTED_FEATURE                            ((uint16_t)22)
#define WF_ERROR_REQUEST_OUT_OF_SYNC                            ((uint16_t)23)
#define WF_ERROR_CP_INVALID_ELEMENT_TYPE                        ((uint16_t)24)
#define WF_ERROR_CP_INVALID_PROFILE_ID                          ((uint16_t)25)
#define WF_ERROR_CP_INVALID_DATA_LENGTH                         ((uint16_t)26)
#define WF_ERROR_CP_INVALID_SSID_LENGTH                         ((uint16_t)27)
#define WF_ERROR_CP_INVALID_SECURITY_TYPE                       ((uint16_t)28)
#define WF_ERROR_CP_INVALID_SECURITY_KEY_LENGTH                 ((uint16_t)29)
#define WF_ERROR_CP_INVALID_WEP_KEY_ID                          ((uint16_t)30)
#define WF_ERROR_CP_INVALID_NETWORK_TYPE                        ((uint16_t)31)
#define WF_ERROR_CP_INVALID_ADHOC_MODE                          ((uint16_t)32)
#define WF_ERROR_CP_INVALID_SCAN_TYPE                           ((uint16_t)33)
#define WF_ERROR_CP_INVALID_CP_LIST                             ((uint16_t)34)
#define WF_ERROR_CP_INVALID_CHANNEL_LIST_LENGTH                 ((uint16_t)35)  
#define WF_ERROR_NOT_CONNECTED                                  ((uint16_t)36)
#define WF_ERROR_ALREADY_CONNECTING                             ((uint16_t)37)
#define WF_ERROR_DISCONNECT_FAILED                              ((uint16_t)38)
#define WF_ERROR_NO_STORED_BSS_DESCRIPTOR                       ((uint16_t)39)
#define WF_ERROR_INVALID_MAX_POWER                              ((uint16_t)40)
#define WF_ERROR_CONNECTION_TERMINATED                          ((uint16_t)41)
#define WF_ERROR_HOST_SCAN_NOT_ALLOWED                          ((uint16_t)42)
#define WF_ERROR_INVALID_WPS_PIN                                ((uint16_t)44)

/*--------------------------------*/
/* MRF24W 8-bit Host Registers    */
/*--------------------------------*/
#define WF_HOST_INTR_REG               ((uint8_t)(0x01))  /**< 8-bit register containing 1st level interrupt bits. */
#define WF_HOST_MASK_REG               ((uint8_t)(0x02))  /**< 8-bit register containing 1st level interrupt mask. */

/*---------------------------------*/
/* MRF24W 16-bit Host Registers    */
/*---------------------------------*/
#define WF_HOST_RAW0_CTRL1_REG         ((uint8_t)(0x26))
#define WF_HOST_RAW0_STATUS_REG        ((uint8_t)(0x28))
#define WF_HOST_RAW1_CTRL1_REG         ((uint8_t)(0x2a))
#define WF_HOST_INTR2_REG              ((uint8_t)(0x2d)) /**< 16-bit register containing 2nd level interrupt bits */
#define WF_HOST_INTR2_MASK_REG         ((uint8_t)(0x2e))
#define WF_HOST_WFIFO_BCNT0_REG        ((uint8_t)(0x2f)) /**< 16-bit register containing available write size for fifo 0 (data) (LS 12 bits contain the length) */
#define WF_HOST_WFIFO_BCNT1_REG        ((uint8_t)(0x31)) /**< 16-bit register containing available write size for fifo 1 (mgmt) (LS 12 bits contain the length) */
#define WF_HOST_RFIFO_BCNT0_REG        ((uint8_t)(0x33)) /**< 16-bit register containing number of bytes in read fifo 0 (data rx) (LS 12 bits contain the length) */
#define WF_HOST_RESET_REG              ((uint8_t)(0x3c))
#define WF_HOST_RESET_MASK             ((uint16_t)(0x0001))                                                       
#define WF_PSPOLL_H_REG                ((uint8_t)(0x3d)) /**< 16-bit register used to control low power mode                      */
#define WF_INDEX_ADDR_REG              ((uint8_t)(0x3e)) /**< 16-bit register to move the data window                             */
#define WF_INDEX_DATA_REG              ((uint8_t)(0x3f)) /**< 16-bit register to read or write address-indexed register           */

/*----------------------------------------------------------------------------------------*/
/* MRF24W registers accessed via the WF_INDEX_ADDR_REG and WF_INDEX_DATA_REG registers    */
/*----------------------------------------------------------------------------------------*/
#define WF_HW_STATUS_REG               ((uint8_t)(0x2a)) /**< 16-bit read only register providing hardware status bits */
#define WF_CONFIG_CTRL0_REG            ((uint8_t)(0x2e)) /**< 16-bit register used to initiate Hard reset              */
#define WF_LOW_POWER_STATUS_REG        ((uint8_t)(0x3e)) /**< 16-bit register read to determine when low power is done */

/* This bit mask is used in the HW_STATUS_REG to determine */
/* when the MRF24W has completed its hardware reset.       */
/*  0 : MRF24W is in reset                                 */
/*  1 : MRF24W is not in reset                             */
#define WF_HW_STATUS_NOT_IN_RESET_MASK ((uint16_t)(0x1000)) 

/* Definitions represent individual interrupt bits for the 8-bit host interrupt registers */
/*  WF_HOST_INTR_REG and WF_HOST_MASK_REG                                                 */
#define WF_HOST_INT_MASK_INT2               ((uint8_t)(0x01))
#define WF_HOST_INT_MASK_FIFO_1_THRESHOLD   ((uint8_t)(0x80))
#define WF_HOST_INT_MASK_FIFO_0_THRESHOLD   ((uint8_t)(0x40))
#define WF_HOST_INT_MASK_RAW_1_INT_0        ((uint8_t)(0x04))
#define WF_HOST_INT_MASK_RAW_0_INT_0        ((uint8_t)(0x02))
#define WF_HOST_INT_MASK_ALL_INT            ((uint8_t)(0xff))

/** Bit mask for all interrupts in the level 2 16-bit interrupt register */
#define WF_HOST_2_INT_MASK_ALL_INT          ((uint16_t)(0xffff))

/* these definitions are used in calls to enable and
 * disable interrupt bits. */
#define WF_INT_DISABLE            ((uint8_t)0)
#define WF_INT_ENABLE             ((uint8_t)1)

#define WF_LOW_POWER_MODE_ON      (1)
#define WF_LOW_POWER_MODE_OFF     (0)

#define RAW_ID_0                        (0)
#define RAW_ID_1                        (1)

// RAW0 used for Rx, RAW1 used for Tx
#define RAW_RX_ID                       RAW_ID_0
#define RAW_TX_ID                       RAW_ID_1
#define RAW_INVALID_ID                  (0xff)

/* RAW Window states */
#define WF_RAW_UNMOUNTED            (0)
#define WF_SCRATCH_MOUNTED          (1)
#define WF_RAW_DATA_MOUNTED         (2)
#define WF_RAW_MGMT_MOUNTED         (3)

// Source/Destination objects on the MRF24W
#define RAW_MAC                         (0x00)   /* Cmd processor (aka MRF24W MAC)                 */
#define RAW_MGMT_POOL                   (0x10)   /* For 802.11 Management packets                  */
#define RAW_DATA_POOL                   (0x20)   /* Data Memory pool used for tx and rx operations */
#define RAW_SCRATCH_POOL                (0x30)   /* Scratch object                                 */
#define RAW_STACK_MEM                   (0x40)   /* single level stack to save state of RAW        */
#define RAW_COPY                        (0x70)   /* RAW to RAW copy                                */

/* 8-bit RAW registers */
#define RAW_0_DATA_REG                  (0x20)
#define RAW_1_DATA_REG                  (0x21)

/* 16 bit RAW registers */
#define RAW_0_CTRL_0_REG                (0x25)
#define RAW_0_CTRL_1_REG                (0x26)
#define RAW_0_INDEX_REG                 (0x27)
#define RAW_0_STATUS_REG                (0x28)
#define RAW_1_CTRL_0_REG                (0x29)
#define RAW_1_CTRL_1_REG                (0x2a)
#define RAW_1_INDEX_REG                 (0x2b)
#define RAW_1_STATUS_REG                (0x2c)

// RAW register masks
#define WF_RAW_STATUS_REG_ERROR_MASK    ((uint16_t)(0x0002))
#define WF_RAW_STATUS_REG_BUSY_MASK     ((uint16_t)(0x0001))

#define ENC_RD_PTR_ID                   (0)
#define ENC_WT_PTR_ID                   (1)

// Memory addresses
#define RESERVED_CRYPTO_MEMORY          (128ul)
#define RAMSIZE                         (24 * 1024ul)
#define TXSTART                         (0x0000ul)
#define RXSTART                         ((TXSTART + 1518ul + TCP_ETH_RAM_SIZE + RESERVED_HTTP_MEMORY + RESERVED_SSL_MEMORY + RESERVED_CRYPTO_MEMORY + 1ul) & 0xFFFE)
#define	RXSTOP                          (RAMSIZE - 1ul)
#define RXSIZE                          (RXSTOP - RXSTART + 1ul)
#define BASE_TX_ADDR                    (TXSTART)
#define BASE_SCRATCH_ADDR               (BASE_TX_ADDR + 1518ul)
#define BASE_HTTPB_ADDR                 (BASE_SCRATCH_ADDR + TCP_ETH_RAM_SIZE)
#define BASE_SSLB_ADDR                  (BASE_HTTPB_ADDR + RESERVED_HTTP_MEMORY)
#define BASE_CRYPTOB_ADDR               (BASE_SSLB_ADDR + RESERVED_SSL_MEMORY)

#define ENABLE_MRF24WB0M                (1)

#define MSG_PARAM_START_DATA_INDEX          (6)
#define MULTICAST_ADDRESS                   (6)
#define ADDRESS_FILTER_DEACTIVATE           (0)

extern void wf_assertionFailed(uint16_t lineNumber);
#define WF_ASSERT(expr)              \
  do {                               \
    if (!(expr)) {                   \
      wf_assertionFailed(__LINE__);  \
    }                                \
  } while (0)

/* tWFParam - Names (ID's) of WF MAC configurable parameters. */
typedef enum {
  PARAM_MAC_ADDRESS                   = 1,       /**< the device MAC address (6 bytes)                                                                                  */
  PARAM_REGIONAL_DOMAIN               = 2,       /**< the device Regional Domain (1 byte)                                                                               */
  PARAM_RTS_THRESHOLD                 = 3,       /**< the RTS byte threshold 256 - 2347 (2 bytes)                                                                       */
  PARAM_LONG_FRAME_RETRY_LIMIT        = 4,       /**< the long Frame Retry limit  (1 byte)                                                                              */ 
  PARAM_SHORT_FRAME_RETRY_LIMIT       = 5,       /**< the short Frame Retry limit (1 byte)                                                                              */
  PARAM_TX_LIFETIME_TU                = 6,       /**< the Tx Request lifetime in TU's 0 - 4194303 (4 bytes)                                                             */
  PARAM_RX_LIFETIME_TU                = 7,       /**< the Rx Frame lifetime in TU's 0 - 4194303 (4 bytes)                                                               */
  PARAM_SUPPLICANT_ON_OFF             = 8,       /**< boolean 1 = on 0 = off (1 byte)                                                                                   */
  PARAM_CONFIRM_DATA_TX_REQ           = 9,       /**< boolean 1 = on 0 = off (1 byte)                                                                                   */
  PARAM_MASTER_STATE                  = 10,      /**< master state of the MAC using enumerated values (1 byte)                                                          */
  PARAM_HOST_ALERT_BITS               = 11,      /**< a bit field which enables/disables various asynchronous indications from the MAC to the host (2 bytes)            */
  PARAM_NUM_MISSED_BEACONS            = 12,      /**< number of consecutive beacons MAC can miss before it considers the network lost (1 byte)                          */        
  PARAM_DIFS_AND_EIFS                 = 13,      /**< delay intervals in usec DIFS and EIFS ( 2 * 2 bytes)                                                              */
  PARAM_TX_POWER                      = 14,      /**< max and min boundaries for Tx power (2 * 2 bytes)                                                                 */
  PARAM_DEFAULT_DEST_MAC_ADDR         = 15,      /**< stores a persistant destination MAC address for small Tx Requests (6 bytes)                                       */
  PARAM_WPA_INFO_ELEMENT              = 16,      /**< stores a WPA info element (IE) in 802.11 IE format. Used in Assoc Request and Supplicant exchange (3 - 258 bytes) */
  PARAM_RSN_INFO_ELEMENT              = 17,      /**< stores a RSN info element (IE) in 802.11 IE format. Used in Assoc Request and Supplicant exchange (3 - 258 bytes) */
  PARAM_ON_OFF_RADIO                  = 18,      /**< bool to force a radio state change 1 = on 0 = off (1 byte)                                                        */
  PARAM_COMPARE_ADDRESS               = 19,      /**< A MAC address used to filter received frames (sizeof(tAddressFilterInput) = 8 bytes)                              */
  PARAM_SUBTYPE_FILTER                = 20,      /**< Bitfield used to filter received frames based on type and sub-type (sizeof(tAddressFilterInput) = 4 bytes)        */
  PARAM_ACK_CONTROL                   = 21,      /**< Bitfield used to control the type of frames that cause ACK responses (sizeof(tAckControlInput) = 4 bytes)         */
  PARAM_STAT_COUNTERS                 = 22,      /**< Complete set of statistics counters that are maintained by the MAC                                                */
  PARAM_TX_THROTTLE_TABLE             = 23,      /**< Custom Tx Rate throttle table to be used to control tx Rate                                                       */
  PARAM_TX_THROTTLE_TABLE_ON_OFF      = 24,      /**< A boolean to enable/disable use of the throttle Table and a tx rate to use if the throttle table is disabled      */
  PARAM_TX_CONTENTION_ARRAY           = 25,      /**< Custom Retry contention ladder used for backoff calculation prior to a Tx attempt                                 */
  PARAM_SYSTEM_VERSION                = 26,      /**< 2 byte representation of a version number for the ROM and Patch                                                   */
  PARAM_STATUE_INFO                   = 27,      /**< MAC State information                                                                                             */
  PARAM_SECURITY_CONTROL              = 28,      /**< 2 byte data structure to enable/disable encryption                                                                */
  PARAM_FACTORY_SET_TX_MAX_POWER      = 29,      /**< gets the factory-set tx max power level                                                                           */
  PARAM_MRF24WB0M                     = 30,      /**< a set enables MRF24WB0M Mode, a get gets the version                                                              */ 
  PARAM_BROADCAST_PROBE_RESPONSE      = 31,      /**< Allows broadcast probe response in Adhoc mode                                                                     */   
  PARAM_AGGRESSIVE_PS                 = 32,      /**< Allows to turn off RF power quicker                                                                               */ 
  PARAM_CONNECT_CONTEXT               = 33,      /**< gets current connection status                                                                                    */ 
  PARAM_DEFERRED_POWERSAVE            = 34,      /**< delay power start after dhcp done                                                                                 */
  PARAM_LINK_DOWN_THRESHOLD           = 35       /**< sets link down threshold */
} tWFParam;

/* SPI Tx Message Types */
#define WF_DATA_REQUEST_TYPE            ((uint8_t)1)
#define WF_MGMT_REQUEST_TYPE            ((uint8_t)2)

/* SPI Rx Message Types */
#define WF_DATA_TX_CONFIRM_TYPE         ((uint8_t)1)
#define WF_MGMT_CONFIRM_TYPE            ((uint8_t)2)
#define WF_DATA_RX_INDICATE_TYPE        ((uint8_t)3)
#define WF_MGMT_INDICATE_TYPE           ((uint8_t)4)

#define DO_NOT_FREE_MGMT_BUFFER         (0)
#define FREE_MGMT_BUFFER                (1)

#define WF_MAX_TX_MGMT_MSG_SIZE         (128)

/*----------------------------------------------*/
/* Management Message Request/Response Subtypes */
/*----------------------------------------------*/
typedef enum
{
  /* Misc subtypes */
  WF_SCAN_SUBTYPE                             = 1,
  WF_JOIN_SUBTYPE                             = 2, 
  WF_AUTH_SUBTYPE                             = 3, 
  WF_ASSOC_SUBTYPE                            = 4, 
  WF_DISCONNECT_SUBTYPE                       = 5, 
  WF_DISASOCC_SUBTYPE                         = 6,
  WF_SET_POWER_MODE_SUBTYPE                   = 7,
  WF_SET_PM_KEY_SUBTYPE                       = 8,
  WF_SET_WEP_MAP_SUBTYPE                      = 9,
  WF_SET_WEP_KEY_SUBTYPE                      = 10,
  WF_SET_TEMP_KEY_SUBTYPE                     = 11,
  WF_CALC_PSK_KEY_SUBTYPE                     = 12,
  WF_SET_WEP_KEY_ID_SUBTYPE                   = 13, 
  WF_CONFIG_KEY_SPACE_SUBTYPE                 = 14,
  WF_SET_PARAM_SUBTYPE                        = 15,
  WF_GET_PARAM_SUBTYPE                        = 16,
  WF_ADHOC_CONNECT_SUBTYPE                    = 17,
  WF_ADHOC_START_SUBTYPE                      = 18,

  /* Connection Profile Message Subtypes */
  WF_CP_CREATE_PROFILE_SUBTYPE                = 21,
  WF_CP_DELETE_PROFILE_SUBTYPE                = 22,
  WF_CP_GET_ID_LIST_SUBTYPE                   = 23,
  WF_CP_SET_ELEMENT_SUBTYPE                   = 24,
  WF_CP_GET_ELEMENT_SUBTYPE                   = 25,

  /* Connection Algorithm Message Subtypes */
  WF_CA_SET_ELEMENT_SUBTYPE                   = 26,
  WF_CA_GET_ELEMENT_SUBTYPE                   = 27,

  /* Connnection Manager Message Subtypes */
  WF_CM_CONNECT_SUBYTPE                       = 28,
  WF_CM_DISCONNECT_SUBYTPE                    = 29,           
  WF_CM_GET_CONNECTION_STATUS_SUBYTPE         = 30,

  WF_SCAN_START_SUBTYPE                       = 31,
  WF_SCAN_GET_RESULTS_SUBTYPE                 = 32,

  WF_CM_INFO_SUBTYPE                          = 33,

  WF_SCAN_FOR_IE_SUBTYPE                      = 34,  /* not yet supported */
  WF_SCAN_IE_GET_RESULTS_SUBTYPE              = 35,  /* not yet supported */

  WF_CM_GET_CONNECTION_STATISTICS_SUBYTPE     = 36,  /* not yet supported so moved here for now */
  WF_NUM_REQUEST_SUBTYPES    
} tMgmtMsgSubtypes;

/*----------------------------------------------------------------------------------------*/
/* Structs                                                                                */
/*----------------------------------------------------------------------------------------*/

/** tComContext - Used by the COM layer to manage State information */
typedef struct
{
    volatile uint8_t rawInterrupt;
    BOOL             waitingForRawMoveCompleteInterrupt;
} tRawMoveState;

typedef struct tWFDeviceInfoStruct
{
    uint8_t deviceType;    /**< MRF24WB0M_DEVICE_TYPE  */
    uint8_t romVersion;    /**< ROM version number     */
    uint8_t patchVersion;  /**< Patch version number   */
} tWFDeviceInfo;

/** 
 * This structure describes the format of the first four bytes of all
 * mgmt response messages received from the MRF24W
 */
typedef struct mgmtRxHdrStruct {
  uint8_t type; /* always 0x02                  */
  uint8_t subtype; /* mgmt msg subtype             */
  uint8_t result; /* 1 if success, else failure   */
  uint8_t macState; /* not used                     */

} tMgmtMsgRxHdr;

/*----------------------------------------------------------------------------------------*/
/* Global variables                                                                       */
/*----------------------------------------------------------------------------------------*/
extern spi_dev*  wf_spi;
extern gpio_dev* wf_cs_port;
extern uint8_t   wf_cs_bit;

static uint8_t   g_buf[3];
static uint8_t   g_hostIntSaved = 0;
static BOOL      g_psPollActive = FALSE;     
static BOOL      g_wasDiscarded;
static uint16_t  g_sizeofScratchMemory = 0;
static uint8_t   g_encPtrRAWId[2]; /**< indexed by ENC_RD_PTR_ID (0) and ENC_WT_PTR_ID (1).  Values can be: RAW_RX_ID, RAW_TX_ID, BACKE_TCB_ADDR, RAW_INVALID_ID */
static uint16_t  g_encIndex[2]; /**< index 0 stores current ENC read index, index 1 stores current ENC write index */
static uint16_t  g_rxBufferSize;
static uint16_t  g_txPacketLength;
static BOOL      g_txBufferFlushed;
extern BOOL      g_hostRAWDataPacketReceived;
static BOOL      g_rawWindowReady[2]; /**< for Tx and Rx, TRUE = ready for use, FALSE = not ready for use */
static uint8_t   g_rawWindowState[2]; /**< see RAW Window states above                                    */
static BOOL      g_rxIndexSetBeyondBuffer; /**< TODO debug -- remove after test */
static tRawMoveState g_rawMoveState;
static BOOL      g_rfModuleVer1209orLater;
static volatile BOOL g_mgmtConfirmMsgReceived = FALSE;
static BOOL      g_restoreRxData = FALSE;
static uint8_t   g_waitingForMgmtResponse = FALSE;
static BOOL      g_mgmtRxInProgress = FALSE;
static BOOL      g_mgmtReadMsgReady; /**< TRUE if rx mgmt msg to process, else FALSE              */
static BOOL      g_mgmtAppWaiting = FALSE;
static volatile BOOL g_exIntNeedsServicing; /**< TRUE if external interrupt needs processing, else FALSE */

/*----------------------------------------------------------------------------------------*/
/* Function declarations                                                                  */
/*----------------------------------------------------------------------------------------*/

/**
 * Performs the necessary SPI operations to cause the MRF24W to reset.
 * This function also implements a delay so that it will not return until
 * the WiFi device is ready to receive messages again.  The delay time will
 * vary depending on the amount of code that must be loaded from serial
 * flash.
 */
void wf_chipReset();

void wf_libInitialize();

/**
 * /brief Enables or disables Tx data confirmation management messages.
 * 
 * Enables or disables the MRF24W Tx data confirm mgmt message.  Data
   confirms should always be disabled.
 * 
 * @param state WF_DISABLED or WF_ENABLED
 */
void wf_setTxDataConfirm(uint8_t state);

/**
 * Writes WF 16-bit register
 * @param regId ID of 16-bit register being written to
 * @param value value to write
 */
void wf_write16BitWFRegister(uint8_t regId, uint16_t value);

/**
 * Reads WF 16-bit register
 * @param regId ID of 16-bit register being read
 * @return value read from register
 */
uint16_t wf_read16BitWFRegister(uint8_t regId);

/**
 * Writes WF 8-bit register
 * @param regId ID of 8-bit register being written to
 * @param value value to write
 */
void wf_write8BitWFRegister(uint8_t regId, uint8_t value);

/**
 * Reads WF 8-bit register
 * @param regId ID of 8-bit register being read
 * @return value read from register
 */
uint8_t wf_read8BitWFRegister(uint8_t regId);

/**
 * Initializes the 16-bit Host Interrupt register on the MRF24W with the
 * specified mask value either setting or clearing the mask register
 * as determined by the input parameter state. 
 * @param hostIntMaskRegMask The bit mask to be modified
 * @param state One of WF_INT_DISABLE, WF_INT_ENABLE where Disable implies clearing the bits and enable sets the bits.
 */
void wf_hostInterrupt2RegInit(uint16_t hostIntMaskRegMask, uint8_t state);

/**
 * Initializes the 8-bit Host Interrupt register on the MRF24W with the
 * specified mask value either setting or clearing the mask register
 * as determined by the input parameter state.  The process requires
 * 2 spi operations which are performed in a blocking fashion.  The
 * function does not return until both spi operations have completed.
 * @param hostIntrMaskRegMask The bit mask to be modified.
 * @param state one of WF_EXINT_DISABLE, WF_EXINT_ENABLE where Disable implies clearing the bits and enable sets the bits.
 */
void wf_hostInterruptRegInit(uint8_t hostIntrMaskRegMask, uint8_t state);

/**
 * \brief Driver function to configure PS Poll mode.
 * 
 * This function is only used by the driver, not the application.  This
 * function, other than at initialization, is only used when the application
 * has enabled PS-Poll mode.  This function is used to temporarily deactivate 
 * PS-Poll mode when there is mgmt or data message tx/rx and then, when message 
 * activity has ceased, to again activate PS-Poll mode. 
 * @param action Can be either:
 *                 - WF_LOW_POWER_MODE_ON
 *                 - WF_LOW_POWER_MODE_OFF
 */
void wf_wfConfigureLowPowerMode(uint8_t action);

void wf_hardwareInit();

/**
 * /brief If PS-Poll is active or the MRF24W is asleep, ensure that it is woken up.
 * 
 * Called by the WiFi driver when it needs to transmit or receive a data or 
 *  mgmt message. If the application has enabled PS-Poll mode and the WiFi 
 *  driver has activated PS-Poll mode then this function will deactivate PS-Poll
 *  mode and wake up the MRF24W.
 */
void wf_ensureWFisAwake();

void wf_rawInit();

/**
 * Mounts Scratch using the specified RAW window.
 * @param rawId desired RAW window to mount Scratch to.
 * @return Number of bytes mounted
 */
uint16_t wf_scratchMount(uint8_t rawId);

/**
 * Unmounts Scratch from the specified RAW window.
 * @param rawId RAW window ID that scratch had been mounted to.
 */
void wf_scratchUnmount(uint8_t rawId);

/**
 * Performs a RAW move operation between a RAW engine and a MRF24W object
 * @param rawId RAW ID
 * @param srcDest MRF24W object that will either source or destination of move
 * @param rawIsDestination TRUE if RAW engine is the destination, FALSE if its the source
 * @param size number of bytes to overlay (not always applicable)
 * @return Number of bytes that were overlayed (not always applicable)
 */
uint16_t wf_rawMove(uint16_t rawId, uint16_t srcDest, BOOL rawIsDestination, uint16_t size);

/**
 * Pushes a RAW window onto the 1-level deep RAW stack.  The RAW window state is preserved
 *    and is restored when PopRawWindow() is called
 * 
 * (1) The RAW architecture supports a 1-level deep stack.  Each time this function is called
*                   any state that had been previously saved is lost.
 * 
 * @param rawId RAW window ID that is being pushed
 */
void wf_pushRawWindow(uint8_t rawId);

/**
 * determines if a RAW Tx buf is ready for a management msg, and if so, creates the RAW tx buffer.
 * @return Returns TRUE if successful, else FALSE.
 */
BOOL wf_isTxMgmtReady(void);

BOOL wf_isMgmtTxBufAvailable(void);

#define wf_setRawWindowState(rawId, state)    g_rawWindowState[rawId] = state         
#define wf_getRawWindowState(rawId)           g_rawWindowState[rawId]

/**
 * Waits for a RAW move to complete.
 * @param rawId RAW ID
 * @return Number of bytes that were overlayed (not always applicable)
 */
uint16_t wf_waitForRawMoveComplete(uint8_t rawId);

void spi_transfer(volatile uint8_t* buf, uint16_t len, uint8_t toggle_cs);

void wf_writeWFArray(uint8_t regId, uint8_t* p_Buf, uint16_t length);

void wf_readWFArray(uint8_t regId, uint8_t *p_Buf, uint16_t length);

/**
 * /brief Sends a SetParam Mgmt request to MRF24W and waits for response.
 * 
 *   Index Set Param Request
 *   ----- -----------------
 *   0     type            (always 0x02 signifying a mgmt request)
 *   1     subtype         (always 0x10 signifying a Set Param Msg)
 *   2     param ID [msb]  (MS byte of parameter ID being requested, e.g. 
 *                          PARAM_SYSTEM_VERSION)
 *   3     param ID [lsb]  (LS byte of parameter ID being requested. e.g. 
 *                          PARAM_SYSTEM_VERSION)
 *   4     payload[0]      first byte of param data
 *   N     payload[n]      Nth byte of payload data
 *           
 *   Index  Set Param Response
 *   ------ ------------------
 *   0      type           (always 0x02 signifying a mgmt response)
 *   1      subtype        (always 0x10 signifying a Param Response Msg
 *   2      result         (1 if successful -- any other value indicates failure
 *   3      mac state      (not used)
 * @param paramType Parameter type associated with the SetParam msg.
 * @param p_paramData pointer to parameter data
 * @param paramDataLength Number of bytes pointed to by p_paramData
 */
void wf_sendSetParamMsg(uint8_t paramType, uint8_t* p_paramData, uint8_t paramDataLength);

/**
 * /brief Sends a GetParam Mgmt request to MRF24W and waits for response.
 * 
 * After response is received the param data is read from message and written 
 *   to p_paramData.  It is up to the caller to fix up endianness.
 *    
 *   Index Get Param Request
 *   ----- -----------------
 *   0     type            (always 0x02 signifying a mgmt request)
 *   1     subtype         (always 0x10 signifying a Get Param Msg)
 *   2     param ID [msb]  (MS byte of parameter ID being requested, e.g. 
 *                          PARAM_SYSTEM_VERSION)
 *   3     param ID [lsb]  (LS byte of parameter ID being requested, e.g. 
 *                          PARAM_SYSTEM_VERSION)
 *          
 *   Index  Get Param Response
 *   ------ ------------------
 *   0      type           (always 0x02 signifying a mgmt response)
 *   1      subtype        (always 0x10 signifying a Param Response Msg
 *   2      result         (1 if successful -- any other value indicates failure
 *   3      mac state      (not used)
 *   4      data length    Length of response data starting at index 6 (in bytes)
 *   5      not used         
 *   6      Data[0]        first byte of returned parameter data
 *   N      Data[N]        Nth byte of param data
 * 
 * @param paramType
 * @param p_paramData
 * @param paramDataLength
 */
void wf_sendGetParamMsg(uint8_t paramType, uint8_t* p_paramData, uint8_t paramDataLength);

/**
 * Called after sending a mgmt request.  This function waits for a mgmt
 * response.  The caller can optionally request the the management 
 * response be freed immediately (by this function) or not freed.  If not
 * freed the caller is responsible to free the response buffer.
 * @param expectedSubtype The expected subtype of the mgmt response
 * @param freeAction FREE_MGMT_BUFFER or DO_NOT_FREE_MGMT_BUFFER
 */
void wf_waitForMgmtResponse(uint8_t expectedSubtype, uint8_t freeAction);

/**
 * This function is called from WFProcess.  It does the following:
 *             1) checks for and processes MRF24W external interrupt events
 *             2) checks for and processes received management messages from the MRF24W
 *             3) maintains the PS-Poll state (if applicable)
 */
void wf_process(void);

/**
 * Processes EXINT from MRF24W.  Called by WFProcess().
 */
void wf_processInterruptServiceResult(void);

/**
 * Waits for the mgmt response message and validates it by:
 *  1) checking the result field
 *  2) verifying that the received subtype matches the execpted subtype
 *
 * In addition, this function reads the desired number of data bytes from 
 * the mgmt response, copies them to p_data, and then frees the mgmt buffer.
 * 
 * @param expectedSubtype management message subtype that we are expecting
 * @param numDataBytes Number of data bytes from mgmt response to write to
 *                      p_data.  Data always starts at index 4 of mgmt response
 * @param startIndex if TRUE, then no data will be read and the mgmt buffer will not
 *                    be freed.  If FALSE, the data will be read and the mgmt buffer
 *                    will be freed.
 * @param p_data pointer where any desired management data bytes will be written
 */
void wf_waitForMgmtResponseAndReadData(uint8_t expectedSubtype, uint8_t numDataBytes, uint8_t startIndex, uint8_t* p_data);

/**
 * Must be called to configure the MRF24WB0M for operations.
 */
void wf_enableMRF24WB0MMode(void);

/**
 * Retrieves WF device information
 * @param p_deviceInfo Pointer where device info will be written
 */
void wf_getDeviceInfo(tWFDeviceInfo* p_deviceInfo);

/**
 * Sends a management message
 * @param p_header pointer to mgmt message header data
 * @param headerLength number of bytes in the header will be written
 * @param p_data pointer to mgmt message data
 * @param dataLength number of byte of data
 */
void wf_sendMgmtMsg(uint8_t* p_header, uint8_t headerLength, uint8_t* p_data, uint8_t dataLength);

/**
 * Called form main loop to support 802.11 operations
 */
void wf_macProcess(void);

void wf_sendRAWManagementFrame(uint16_t bufLen);

/**
 * if a mgmt msg mounted in RAW window then message handled by MRF24W.
 * If a data message mounted in RAW window then will be transmitted to 802.11 network
 * @param len
 */
void wf_rawSendTxBuffer(uint16_t len);

void wf_freeMgmtTx(void);

/**
 * Reads the specified number of bytes from a mounted RAW window from the specified starting index
 * @param rawId RAW window ID being read from
 * @param startIndex start index within RAW window to read from
 * @param length number of bytes to read from the RAW window
 * @param p_dest pointer to Host buffer where read data is copied
 */
void wf_rawRead(uint8_t rawId, uint16_t startIndex, uint16_t length, uint8_t* p_dest);

/**
 * Reads bytes from the RAW engine
 * @param rawId RAW ID
 * @param pBuffer Buffer to read bytes into
 * @param length number of bytes to read
 */
void wf_rawGetByte(uint16_t rawId, uint8_t* pBuffer, uint16_t length);

/**
 * mounts the most recent Rx message.  Could be a management or data message
 * @return 
 */
uint16_t wf_rawMountRxBuffer(void);

void wf_deallocateMgmtRxBuffer(void);

void wf_deallocateDataRxBuffer(void);

void wf_deallocateDataTxBuffer(void);

BOOL wf_allocateMgmtTxBuffer(uint16_t bytesNeeded);

/**
 * /brief Pops a RAW window state from the 1-level deep RAW stack.  The RAW window state that was 
 *               mounted prior to this call is lost.
 * 
 * (1) The RAW architecture supports a 1-level deep stack.  When this fucntion is called the 
*                   RAW window state that had been mounted is lost.  If trying to pop a non-existent RAW
*                   window state (no push has taken place), the the returned byte count is 0.
 * 
 * @param rawId RAW window ID that is being popped
 * @return byte count of the RAW window state that was saved and is now restored.  In other words, the
 *               size, in bytes, of the RAW window when it was first created.
 *               of the o
 */
uint16_t wf_popRawWindow(uint8_t rawId);

/**
 * Sets the RAW index for the specified RAW engine.  If attempt to set RAW
 *         index outside boundaries of RAW window this function will time out.
 * @param rawId RAW ID
 * @param index desired index
 * @return True is success, false if timed out, which means attempted to set
 *          raw index past end of raw window.  Not a problem as long as no read
 *          or write occurs.
 *
 */
BOOL wf_rawSetIndex(uint16_t rawId, uint8_t index);

BOOL wf_rawGetMgmtRxBuffer(uint16_t* p_numBytes);

void wf_processMgmtRxMsg(void);

/***************************************************************************************************/


void wf_init() {
  tWFDeviceInfo deviceInfo;

  wf_hardwareInit();
  wf_rawInit();

  wf_enableMRF24WB0MMode();
  wf_getDeviceInfo(&deviceInfo);

  // if MRF24WB   
#if !defined(MRF24WG)
  WF_ASSERT(deviceInfo.romVersion == 0x12);
  WF_ASSERT(deviceInfo.patchVersion >= 0x02);
  if (deviceInfo.romVersion == 0x12 && deviceInfo.patchVersion >= 0x09) {
    g_rfModuleVer1209orLater = TRUE;
  }
#else // must be a MRF24WG
  WF_ASSERT(deviceInfo.romVersion == 0x30 || deviceInfo.romVersion == 0x31);
#endif

  /* send init messages to MRF24W */
  wf_libInitialize();

#if defined(WF_CONSOLE)
  WFConsoleInit();
#if defined(WF_CONSOLE_DEMO)
  IperfAppInit();
#endif
#endif
}

void wf_libInitialize() {
  wf_setTxDataConfirm(WF_DISABLED);
}

void wf_setTxDataConfirm(uint8_t state) {
  wf_sendSetParamMsg(PARAM_CONFIRM_DATA_TX_REQ, &state, 1);
}   

void wf_getMacAddress(uint8_t* p_mac) {
  wf_sendGetParamMsg(PARAM_MAC_ADDRESS, p_mac, WF_MAC_ADDRESS_LENGTH);
}

void wf_getDeviceInfo(tWFDeviceInfo *p_deviceInfo) {
  uint8_t msgData[2];

  wf_sendGetParamMsg(PARAM_SYSTEM_VERSION, msgData, sizeof (msgData));

  p_deviceInfo->deviceType = MRF24WB0M_DEVICE;
  p_deviceInfo->romVersion = msgData[0];
  p_deviceInfo->patchVersion = msgData[1];
}

void wf_enableMRF24WB0MMode(void) {
  uint8_t buf[1] = {ENABLE_MRF24WB0M};
  wf_sendSetParamMsg(PARAM_MRF24WB0M, buf, sizeof (buf));
}

void wf_sendSetParamMsg(uint8_t paramType, uint8_t* p_paramData, uint8_t paramDataLength) {
  uint8_t hdr[4];

  hdr[0] = WF_MGMT_REQUEST_TYPE;
  hdr[1] = WF_SET_PARAM_SUBTYPE;
  hdr[2] = 0x00; /* MS 8 bits of param Id, always 0 */
  hdr[3] = paramType; /* LS 8 bits of param ID           */

  wf_sendMgmtMsg(hdr, sizeof (hdr), p_paramData, paramDataLength); 

  /* wait for MRF24W management response; free response because not needed */
  wf_waitForMgmtResponse(WF_SET_PARAM_SUBTYPE, FREE_MGMT_BUFFER);
}

void wf_sendGetParamMsg(uint8_t paramType, uint8_t* p_paramData, uint8_t paramDataLength) {
  uint8_t hdr[4];

  hdr[0] = WF_MGMT_REQUEST_TYPE;
  hdr[1] = WF_GET_PARAM_SUBTYPE;
  hdr[2] = 0x00; /* MS 8 bits of param Id, always 0 */
  hdr[3] = paramType; /* LS 8 bits of param ID           */

  wf_sendMgmtMsg(hdr, sizeof (hdr), NULL, 0);

  wf_waitForMgmtResponseAndReadData(WF_GET_PARAM_SUBTYPE, paramDataLength, MSG_PARAM_START_DATA_INDEX, p_paramData); 
}

void wf_waitForMgmtResponseAndReadData(uint8_t expectedSubtype, uint8_t numDataBytes, uint8_t startIndex, uint8_t* p_data) {
  tMgmtMsgRxHdr hdr; /* management msg header struct */

  wf_waitForMgmtResponse(expectedSubtype, DO_NOT_FREE_MGMT_BUFFER);

  /* if made it here then received a management message */
  wf_rawRead(RAW_RX_ID, 0, (uint16_t) (sizeof (tMgmtMsgRxHdr)), (uint8_t *) & hdr);

  /* check header result and subtype fields */
  WF_ASSERT(hdr.result == WF_SUCCESS || hdr.result == WF_ERROR_NO_STORED_BSS_DESCRIPTOR);
  WF_ASSERT(hdr.subtype == expectedSubtype);

  /* if caller wants to read data from this mgmt response */
  if (numDataBytes > 0) {
    wf_rawRead(RAW_RX_ID, startIndex, numDataBytes, p_data);
  }

  /* free the mgmt buffer */
  wf_deallocateMgmtRxBuffer();

  /* if there was a mounted data packet prior to the mgmt tx/rx transaction, then restore it */
  if (g_restoreRxData == TRUE) {
    g_restoreRxData = FALSE;
    wf_popRawWindow(RAW_RX_ID);
    wf_setRawWindowState(RAW_RX_ID, WF_RAW_DATA_MOUNTED);
  }
}

uint16_t wf_popRawWindow(uint8_t rawId) {
  uint16_t byteCount;
  byteCount = wf_rawMove(rawId, RAW_STACK_MEM, TRUE, 0);
  return byteCount;
}

void wf_deallocateMgmtRxBuffer(void) {
  /* Unmount (release) mgmt packet now that we are done with it */
  wf_rawMove(RAW_RX_ID, RAW_MGMT_POOL, FALSE, 0);
  wf_setRawRxMgmtInProgress(FALSE);
  g_waitingForMgmtResponse = FALSE;
}

BOOL wf_allocateMgmtTxBuffer(uint16_t bytesNeeded) {
  uint16_t bufAvail;
  uint16_t byteCount;

  /* get total bytes available for MGMT tx memory pool */
  bufAvail = wf_read16BitWFRegister(WF_HOST_WFIFO_BCNT1_REG) & 0x0fff; /* LS 12 bits contain length */

  /* if enough bytes available to allocate */
  if (bufAvail >= bytesNeeded) {
    /* allocate and create the new Tx buffer (mgmt or data) */
    byteCount = wf_rawMove(RAW_TX_ID, RAW_MGMT_POOL, TRUE, bytesNeeded);
    WF_ASSERT(byteCount != 0);
  }/* else not enough bytes available at this time to satisfy request */
  else {
    return FALSE;
  }

  g_rawWindowReady[RAW_TX_ID] = TRUE;
  wf_setRawWindowState(RAW_TX_ID, WF_RAW_MGMT_MOUNTED);

  return TRUE;
}    

void wf_setRawRxMgmtInProgress(BOOL action) {
  if (action == FALSE) {
    //        RawWindowReady[RAW_RX_ID] = TRUE;
    wf_setRawWindowState(RAW_RX_ID, WF_RAW_UNMOUNTED);
  }
  g_mgmtRxInProgress = action;
}

void wf_waitForMgmtResponse(uint8_t expectedSubtype, uint8_t freeAction) {
  tMgmtMsgRxHdr hdr;

  g_waitingForMgmtResponse = TRUE;

  /* Wait until mgmt response is received */
  while (g_mgmtConfirmMsgReceived == FALSE) {
    wf_process();

    /* if received a data packet while waiting for mgmt packet */
    if (g_hostRAWDataPacketReceived) {
      // We can't let the StackTask processs data messages that come in while waiting for mgmt 
      // response because the application might send another mgmt message, which is illegal until the response
      // is received for the first mgmt msg.  And, we can't prevent the race condition where a data message 
      // comes in before a mgmt response is received.  Thus, the only solution is to throw away a data message
      // that comes in while waiting for a mgmt response.  This should happen very infrequently.  If using TCP then the 
      // stack takes care of retries.  If using UDP, the application has to deal with occasional data messages not being
      // received.  Also, applications typically do not send a lot of management messages after connected.

      // throw away the data rx 
      wf_rawMountRxBuffer();
      wf_deallocateDataRxBuffer();
      g_hostRAWDataPacketReceived = FALSE;

      /* ensure interrupts enabled */
      // TODO: WF_EintEnable();
    }
  }

  /* set this back to FALSE so the next mgmt send won't think he has a response before one is received */
  g_mgmtConfirmMsgReceived = FALSE;

  /* if the caller wants to delete the response immediately (doesn't need any data from it */
  if (freeAction == FREE_MGMT_BUFFER) {
    /* read and verify result before freeing up buffer to ensure our message send was successful */
    wf_rawRead(RAW_RX_ID, 0, (uint16_t) (sizeof (tMgmtMsgRxHdr)), (uint8_t *) & hdr);

    /* mgmt response subtype had better match subtype we were expecting */
    WF_ASSERT(hdr.subtype == expectedSubtype);

    if (hdr.result == WF_ERROR_DISCONNECT_FAILED
            || hdr.result == WF_ERROR_NOT_CONNECTED) {
#if defined(STACK_USE_UART)
      putrsUART("Disconnect failed. Disconnect is allowed only when module is in connected state\r\n");
#endif
    } else if (hdr.result == WF_ERROR_NO_STORED_BSS_DESCRIPTOR) {
#if defined(STACK_USE_UART)
      putrsUART("No stored scan results\r\n");
#endif
    } else {
      WF_ASSERT(hdr.result == WF_SUCCESS);
    }

    /* free mgmt buffer */
    wf_deallocateMgmtRxBuffer();

    /* if there was a mounted data packet prior to the mgmt tx/rx transaction, then restore it */
    if (g_restoreRxData == TRUE) {
      g_restoreRxData = FALSE;
      wf_popRawWindow(RAW_RX_ID);
      wf_setRawWindowState(RAW_RX_ID, WF_RAW_DATA_MOUNTED);
    }
  }
}

void wf_process(void) {
  uint16_t len;

  if (g_exIntNeedsServicing == TRUE) { /* if there is a MRF24W External interrupt (EINT) to process */
    g_exIntNeedsServicing = FALSE;
    wf_processInterruptServiceResult();
  } else if (g_mgmtReadMsgReady == TRUE) { /* else if there is management msg to read */
    //-----------------------------
    // process management read
    //-----------------------------
    // if the Raw Rx buffer is available, or only has the scratch mounted, then mount it so
    // we can process received Mgmt message.  Otherwise, stay in this state and keep checking
    // until we can mount the Raw Rx buffer and get the management message.  Once the Raw Rx
    // is acquired, rx data packets are held off until we finish processing mgmt message.
    if (wf_rawGetMgmtRxBuffer(&len)) {
      // handle received managment message
      g_mgmtReadMsgReady = FALSE;
      wf_processMgmtRxMsg();

      // reenable interrupts
      // TODO: wf_EintEnable();
    }
  }
}

void wf_processInterruptServiceResult(void) {
  uint8_t hostIntRegValue;
  uint8_t hostIntMaskRegValue;
  uint8_t hostInt;

  /* read hostInt register to determine cause of interrupt */
  hostIntRegValue = wf_read8BitWFRegister(WF_HOST_INTR_REG);

  // OR in the saved interrupts during the time when we were waiting for raw complete, set by WFEintHandler()
  hostIntRegValue |= g_hostIntSaved;

  // done with the saved interrupts, clear variable
  g_hostIntSaved = 0;

  hostIntMaskRegValue = wf_read8BitWFRegister(WF_HOST_MASK_REG);

  // AND the two registers together to determine which active, enabled interrupt has occurred
  hostInt = hostIntRegValue & hostIntMaskRegValue;

  // if received a level 2 interrupt (should not happen!)
  if ((hostInt & WF_HOST_INT_MASK_INT2) == WF_HOST_INT_MASK_INT2) {
    /* read the 16 bit interrupt register */
    /* CURRENTLY unhandled interrupt */
    WF_ASSERT(FALSE);
    // TODO: WF_EintEnable();
  }    // else if got a FIFO 1 Threshold interrupt (Management Fifo)
  else if ((hostInt & WF_HOST_INT_MASK_FIFO_1_THRESHOLD) == WF_HOST_INT_MASK_FIFO_1_THRESHOLD) {
    /* clear this interrupt */
    wf_write8BitWFRegister(WF_HOST_INTR_REG, WF_HOST_INT_MASK_FIFO_1_THRESHOLD);
    // notify MAC state machine that management message needs to be processed
    g_mgmtReadMsgReady = TRUE;
  }    // else if got a FIFO 0 Threshold Interrupt (Data Fifo)
  else if ((hostInt & WF_HOST_INT_MASK_FIFO_0_THRESHOLD) == WF_HOST_INT_MASK_FIFO_0_THRESHOLD) {
    /* clear this interrupt */
    wf_write8BitWFRegister(WF_HOST_INTR_REG, WF_HOST_INT_MASK_FIFO_0_THRESHOLD);

    g_hostRAWDataPacketReceived = TRUE; /* this global flag is used in MACGetHeader() to determine a received data packet */
  }    // else got a Host interrupt that we don't handle
  else if (hostInt) {
    /* unhandled interrupt */
    /* clear this interrupt */
    wf_write8BitWFRegister(WF_HOST_INTR_REG, hostInt);
    // TODO: WF_EintEnable();
  }    // we got a spurious interrupt (no bits set in register)
  else {
    /* spurious interrupt */
    // TODO: WF_EintEnable();
  }
}

void wf_processMgmtRxMsg(void) {
  uint8_t msgType;

  /* read first byte from Mgmt Rx message (msg type) */
  wf_rawRead(RAW_RX_ID, 0, 1, &msgType);

  /* if not a management response or management indicate then fatal error */
  WF_ASSERT((msgType == WF_MGMT_CONFIRM_TYPE) || (msgType == WF_MGMT_INDICATE_TYPE));

  if (msgType == WF_MGMT_CONFIRM_TYPE) {
    /* signal that a mgmt response has been received */
    wf_signalMgmtConfirmReceivedEvent();
  } else /* must be WF_MGMT_INDICATE_TYPE */ {
    /* handle the mgmt indicate */
    wf_processMgmtIndicateMsg();
  }
}

BOOL wf_rawGetMgmtRxBuffer(uint16_t* p_numBytes) {
  BOOL res = TRUE;
  // UINT16  numBytes;
  *p_numBytes = 0;

  // if Raw Rx is not currently mounted, or the Scratch is mounted
  if (wf_getRawWindowState(RAW_RX_ID) == WF_RAW_DATA_MOUNTED) {
    // save whatever was mounted to Raw Rx
    wf_pushRawWindow(RAW_RX_ID);
  }

  // mount the mgmt pool rx data, returns number of bytes in mgmt msg.  Index
  // defaults to 0.
  *p_numBytes = wf_rawMountRxBuffer();

  /* Should never receive a mgmt msg with 0 bytes */
  WF_ASSERT(*p_numBytes > 0);

  // set flag so we do not try to mount an incoming data packet until after the rx Mgmt msg
  // has been handled.
  wf_setRawRxMgmtInProgress(TRUE);

  return res;
}

void wf_deallocateDataRxBuffer(void) {
  wf_rawMove(RAW_RX_ID, RAW_DATA_POOL, FALSE, 0);
}    

uint16_t wf_rawMountRxBuffer(void)
{
    uint16_t length;
    
    length = wf_rawMove(RAW_RX_ID, RAW_MAC, TRUE, 0);
    
    g_rawWindowReady[RAW_RX_ID] = TRUE;
    wf_setRawWindowState(RAW_RX_ID, WF_RAW_DATA_MOUNTED);
    
    return length;
} 

void wf_rawRead(uint8_t rawId, uint16_t startIndex, uint16_t length, uint8_t* p_dest) {
  wf_rawSetIndex(rawId, startIndex);
  wf_rawGetByte(rawId, p_dest, length);
}

void wf_rawGetByte(uint16_t rawId, uint8_t* pBuffer, uint16_t length) {
  uint8_t regId;
#if defined(OUTPUT_RAW_TX_RX)
  uint16_t i;
#endif

  /* if reading a data message do following check */
  if (!g_waitingForMgmtResponse) {
    // if RAW index previously set out of range and caller is trying to do illegal read
    if ((rawId == RAW_RX_ID) && g_rxIndexSetBeyondBuffer && (wf_getRawWindowState(RAW_RX_ID) == WF_RAW_DATA_MOUNTED)) {
      WF_ASSERT(FALSE); /* attempting to read past end of RAW buffer */
    }
  }

  regId = (rawId == RAW_ID_0) ? RAW_0_DATA_REG : RAW_1_DATA_REG;
  wf_readWFArray(regId, pBuffer, length);

#if defined(OUTPUT_RAW_TX_RX)
  for (i = 0; i < length; ++i) {
    char buf[16];
    sprintf(buf, "R: %#x\r\n", pBuffer[i]);
    putsUART(buf);
  }
#endif
}

void wf_rawSetByte(uint16_t rawId, uint8_t* pBuffer, uint16_t length) {
  uint8_t regId;
#if defined(OUTPUT_RAW_TX_RX)
  uint16_t i;
#endif    


  /* if previously set index past legal range and now trying to write to RAW engine */
  if ((rawId == 0) && g_rxIndexSetBeyondBuffer && (wf_getRawWindowState(RAW_TX_ID) == WF_RAW_DATA_MOUNTED)) {
    //WF_ASSERT(FALSE);  /* attempting to write past end of RAW window */
  }

  /* write RAW data to chip */
  regId = (rawId == RAW_ID_0) ? RAW_0_DATA_REG : RAW_1_DATA_REG;
  wf_writeWFArray(regId, pBuffer, length);

#if defined(OUTPUT_RAW_TX_RX)
  for (i = 0; i < length; ++i) {
    char buf[16];
    sprintf(buf, "T: %#x\r\n", pBuffer[i]);
    putsUART(buf);
  }
#endif
}

BOOL wf_rawSetIndex(uint16_t rawId, uint8_t index) {
  uint8_t regId;
  uint16_t regValue;
  uint32_t startTickCount;
  uint32_t maxAllowedTicks;

  // set the RAW index
  regId = (rawId == RAW_ID_0) ? RAW_0_INDEX_REG : RAW_1_INDEX_REG;
  wf_write16BitWFRegister(regId, index);

  startTickCount = millis();
  maxAllowedTicks = 5 * 1000; /* 5ms */

  regId = (rawId == RAW_ID_0) ? RAW_0_STATUS_REG : RAW_1_STATUS_REG;

  while (1) {
    regValue = wf_read16BitWFRegister(regId);
    if ((regValue & WF_RAW_STATUS_REG_BUSY_MASK) == 0) {
      return TRUE;
    }

    /* if timed out then trying to set index past end of raw window, which is OK so long as the app */
    /* doesn't try to access it                                                                     */
    if (millis() - startTickCount >= maxAllowedTicks) {
      return FALSE; /* timed out waiting for Raw set index to complete */
    }
  }
}

void wf_sendMgmtMsg(uint8_t* p_header, uint8_t headerLength, uint8_t* p_data, uint8_t dataLength) {
  uint32_t startTime;
  uint32_t maxAllowedTime;

  /* cannot send management messages while in WF_ProcessEvent() */
  WF_ASSERT(!wf_sInWFProcessEvent());

  wf_ensureWFisAwake();

  /* if a Rx Data packet is mounted that has not yet been processed */
  if (wf_getRawWindowState(RAW_RX_ID) == WF_RAW_DATA_MOUNTED) {
    /* save it, so after mgmt response received it can be restored */
    wf_pushRawWindow(RAW_RX_ID);
    g_restoreRxData = TRUE;
  }

  /* mounts a tx mgmt buffer on the MRF24W when data tx is done */
  maxAllowedTime = 5 * 1000; /* 5 ms timeout */
  startTime = millis();
  while (!wf_isTxMgmtReady()) {
    wf_macProcess();

    /* DEBUG -- REMOVE AFTER FIGURE OUT WHY TIMING OUT (RARELY HAPPENS) */
    if (millis() - startTime >= maxAllowedTime) {
      /* force flags so WFisTxMgmtReady will return TRUE */
      wf_setRawWindowState(RAW_TX_ID, WF_RAW_UNMOUNTED);
      g_rawWindowReady[RAW_TX_ID] = FALSE;
    }
  }

  /* write out management header */
  wf_rawSetByte(RAW_TX_ID, p_header, headerLength);

  /* write out data (if any) */
  if (dataLength > 0) {
    wf_rawSetByte(RAW_TX_ID, p_data, dataLength);
  }

  /* send mgmt msg to MRF24W */
  wf_sendRAWManagementFrame(headerLength + dataLength);
}

void wf_sendRAWManagementFrame(uint16_t bufLen) {
  /* Instruct WF chip to transmit the packet data in the raw window */
  wf_rawSendTxBuffer(bufLen);

  /* let tx buffer be reused (for either data or management tx) */
  wf_freeMgmtTx();
}

void wf_rawSendTxBuffer(uint16_t len) {
  wf_rawMove(RAW_TX_ID, RAW_MAC, FALSE, len);
  g_rawWindowReady[RAW_TX_ID] = FALSE;
  wf_setRawWindowState(RAW_TX_ID, WF_RAW_UNMOUNTED);
}

void wf_freeMgmtTx(void) {
  // This flag needs to clear so data path can proceed.
  wf_setRawRxMgmtInProgress(FALSE);
}

void wf_macProcess(void) {
  // Let 802.11 processes have a chance to run
  wf_process();

#if defined( WF_CONSOLE_IFCFGUTIL )
  if (WF_hibernate.wakeup_notice && WF_hibernate.state == WF_HB_WAIT_WAKEUP) {
    DelayMs(200);

    WF_hibernate.state = WF_HB_NO_SLEEP;
    StackInit();
#if defined(WF_CONSOLE) && !defined(STACK_USE_EZ_CONFIG)
    IperfAppInit();
#endif

    WF_Connect();
  }
#endif


#if defined(WF_CONSOLE_DEMO)
  IperfAppCall();
#endif

#if !defined (WF_EASY_CONFIG_DEMO)
#if defined(WF_CONSOLE_IFCFGUTIL) 
wait_console_input:
#endif

#if defined(WF_CONSOLE)
  WFConsoleProcess();
#if defined( WF_CONSOLE_IFCFGUTIL )
  if (WF_hibernate.state == WF_HB_NO_SLEEP)
    IperfAppCall();
#elif !defined(STACK_USE_EZ_CONFIG)
  IperfAppCall();
#endif
  WFConsoleProcessEpilogue();
#endif

#if defined( WF_CONSOLE_IFCFGUTIL )
  if (WF_hibernate.state != WF_HB_NO_SLEEP) {
    if (WF_hibernate.state == WF_HB_ENTER_SLEEP) {
      SetLogicalConnectionState(FALSE);
#if defined(WF_USE_POWER_SAVE_FUNCTIONS)
      WF_HibernateEnable();
#endif
      WF_hibernate.state = WF_HB_WAIT_WAKEUP;
    }
    if (WF_hibernate.wakeup_notice) {
      //continue;
    }
    else {
      goto wait_console_input;
    }
  }
#endif             
#endif /* !defined (WF_EASY_CONFIG_DEMO) */

  /* SG. Deadlock avoidance code when two applications contend for the one tx pipe                              */
  /* ApplicationA is a data plane application, and applicationB is a control plane application                  */
  /* In this scenario, the data plane application could be the WiFi manager, and the control plane application  */
  /* a sockets application.  If the sockets application keeps doing a IsUDPPutReady() and never follows with    */
  /* a UDPFlush, then the link manager will be locked out.   This would be catescrophic if an AP connection     */
  /* goes down, then the link manager could never re-establish connection.  Why?  The link manager is a control */
  /* plane application, which does mgmt request/confirms.                                                       */
  /*                                                                                              */
  /* Sequence of events:                                                                          */
  /* T0: ApplicationA will issue a call like UDPIsPutReady(), which results in a AllocateDataTxBuffer */
  /* T1: ApplicationB attempts a mgmt request with IsTxMbmtReady() call.  The call fails.         */
  /* T3: Stack process runs and does not deallocate the tx pipe from the data plane application.  */
  /* T4: ApplicationB attempts N+1th time, and fails.                                             */
  if (g_mgmtAppWaiting) {
    if (wf_getRawWindowState(RAW_TX_ID) == WF_RAW_DATA_MOUNTED) {
      /* deallocate the RAW on MRF24W - return memory to pool */
      wf_deallocateDataTxBuffer();

      if (g_encPtrRAWId[ENC_RD_PTR_ID] == RAW_RX_ID) {
        g_encPtrRAWId[ENC_RD_PTR_ID] = RAW_INVALID_ID;
      }

      if (g_encPtrRAWId[ENC_WT_PTR_ID] == RAW_TX_ID) {
        g_encPtrRAWId[ENC_WT_PTR_ID] = RAW_INVALID_ID;
      }
    }
      /* This else is important in that it gives the main loop one iteration for the mgmt application to get it's timeslice  */
      /* Otherwise, a data plane task could snatch away the tx pipe again, especially if it's placed before                  */
      /* the link manager in the main()'s while(1) blk.  This code is functionally coupled with isRawRxMgmtInProgress()    */
      /* as it will keep the dataplane application locked out for 1 iteration, until this else is executed on N+2 iteration    */
    else {
      g_mgmtAppWaiting = FALSE;
    }
  }

#if defined(STACK_CLIENT_MODE) && defined(USE_GRATUITOUS_ARP)
  //following is the workaround algorithm for the 11Mbps broadcast bugfix
  WFPeriodicGratuitousArp();
#endif     
}

void wf_deallocateDataTxBuffer(void) {
  wf_rawMove(RAW_TX_ID, RAW_DATA_POOL, FALSE, 0);
  g_rawWindowReady[RAW_TX_ID] = FALSE;
  wf_setRawWindowState(RAW_TX_ID, WF_RAW_UNMOUNTED);
}   

BOOL wf_isTxMgmtReady(void) {
  BOOL res = TRUE;

  if (wf_isMgmtTxBufAvailable()) {
    // create and mount tx buffer to hold RAW Mgmt packet
    if (wf_allocateMgmtTxBuffer(WF_MAX_TX_MGMT_MSG_SIZE)) {
      res = TRUE;

      /* Bug. This flag must be set otherwise the data path does not know */
      /* that the tx pipe has been mounted for mgmt operation.  SG */
      wf_setRawRxMgmtInProgress(TRUE);
    } else {
      res = FALSE;
    }
  }    // else Tx RAW not available for Mgmt packet
  else {
    res = FALSE;

    /* See comment in MACProcess */
    g_mgmtAppWaiting = TRUE;
  }

  return res;
}

BOOL wf_isMgmtTxBufAvailable(void) {
  // if the Tx RAW buf is not being used for a data packet or scratch, then it
  // is available for a Mgmt packet.
  if ((g_rawWindowReady[RAW_TX_ID] == FALSE)
          && ((wf_getRawWindowState(RAW_TX_ID) == WF_RAW_UNMOUNTED) || (wf_getRawWindowState(RAW_TX_ID) == WF_SCRATCH_MOUNTED))) {
    wf_setRawWindowState(RAW_TX_ID, WF_RAW_UNMOUNTED);
    return TRUE;
  } else {
    return FALSE;
  }
}

void wf_pushRawWindow(uint8_t rawId) {
  wf_rawMove(rawId, RAW_STACK_MEM, FALSE, 0);
}

void wf_ensureWFisAwake() {
#if defined(WF_USE_POWER_SAVE_FUNCTIONS)
  /* if the application desires the MRF24W to be in PS-Poll mode (PS-Poll with DTIM enabled or disabled */
  if ((g_powerSaveState == WF_PS_PS_POLL_DTIM_ENABLED) || (g_powerSaveState == WF_PS_PS_POLL_DTIM_DISABLED)) {
    /* if the WF driver has activated PS-Poll */
    if (g_psPollActive == TRUE) {
      /* wake up MRF24W */
      WFConfigureLowPowerMode(WF_LOW_POWER_MODE_OFF);
    }

    // will need to put device back into PS-Poll sleep mode
    SetSleepNeeded();
  }

#endif
}  

void wf_hardwareInit() {
  wf_chipReset();

  /* disable the interrupts gated by the 16-bit host int register */
  wf_hostInterrupt2RegInit(WF_HOST_2_INT_MASK_ALL_INT, WF_INT_DISABLE);

  /* disable the interrupts gated the by main 8-bit host int register */
  wf_hostInterruptRegInit(WF_HOST_INT_MASK_ALL_INT, WF_INT_DISABLE);

  /* enable the following MRF24W interrupts */
  wf_hostInterruptRegInit(
          WF_HOST_INT_MASK_FIFO_1_THRESHOLD /* Mgmt Rx Msg interrupt        */
          | WF_HOST_INT_MASK_FIFO_0_THRESHOLD /* Data Rx Msg interrupt        */
          | WF_HOST_INT_MASK_RAW_0_INT_0 /* RAW0 Move Complete interrupt */
          | WF_HOST_INT_MASK_RAW_1_INT_0, /* RAW1 Move Complete interrupt */
          WF_INT_ENABLE);

  /* Disable PS-Poll mode */
  wf_wfConfigureLowPowerMode(WF_LOW_POWER_MODE_OFF);
}

void wf_rawInit() {
  // By default, Scratch is mounted to RAW 1 after reset.  In order to mount it on RAW0
    // we need to first unmount it from RAW 1.
    wf_scratchUnmount(RAW_TX_ID);

    // mount scratch temporarily to see how many bytes it has, then unmount it
    g_sizeofScratchMemory = wf_scratchMount(RAW_RX_ID);  /* put back in if need to know size of scratch */
    wf_scratchUnmount(RAW_RX_ID);

    g_rawWindowReady[RAW_RX_ID] = FALSE;

    g_encPtrRAWId[ENC_RD_PTR_ID] = RAW_RX_ID;
    g_encIndex[ENC_RD_PTR_ID] = BASE_SCRATCH_ADDR; //BASE_TCB_ADDR;

    // don't mount tx raw at init because it's needed for raw mgmt messages
    g_rawWindowReady[RAW_TX_ID] = FALSE;
    wf_setRawWindowState(RAW_TX_ID, WF_RAW_UNMOUNTED);

    g_encPtrRAWId[ENC_WT_PTR_ID] = RAW_INVALID_ID;
    g_encIndex[ENC_WT_PTR_ID] = BASE_TX_ADDR;                     // set tx encode ptr (index) to start of tx buf + 4 bytes

    g_wasDiscarded    = TRUE;                                     // set state such that last rx packet was discarded
    g_rxBufferSize    = 0;                                        // current rx buffer length (none) is 0 bytes
    g_txPacketLength  = 0;                                        // current tx packet length (none) is 0 bytes
    g_txBufferFlushed = TRUE;                                     // tx buffer is flushed

    // from ENC MAC init
    // encWrPtr is left pointing to BASE_TX_ADDR
    // encRdPtr is not initialized... we leave it pointing to BASE_TCB_ADDR

    g_rxIndexSetBeyondBuffer = FALSE;
}

void wf_isr() {
 // TODO
}

void wf_scan() {
 // TODO
}

void wf_connect(){
 // TODO
}

uint8_t* wf_get_mac() {
  return NULL; // TODO
}

void wf_chipReset() {
  uint16_t value;

  delay_us(1000);

  // clear the power bit to disable low power mode on the MRF24W
  wf_write16BitWFRegister(WF_PSPOLL_H_REG, 0x0000);

  // Set HOST_RESET bit in register to put device in reset
  wf_write16BitWFRegister(WF_HOST_RESET_REG, wf_read16BitWFRegister(WF_HOST_RESET_REG) | WF_HOST_RESET_MASK);

  // Clear HOST_RESET bit in register to take device out of reset
  wf_write16BitWFRegister(WF_HOST_RESET_REG, wf_read16BitWFRegister(WF_HOST_RESET_REG) & ~WF_HOST_RESET_MASK);

  // after reset is started poll register to determine when HW reset has completed
  do {
    delay_us(5000000);
    wf_write16BitWFRegister(WF_INDEX_ADDR_REG, WF_HW_STATUS_REG);
    value = wf_read16BitWFRegister(WF_INDEX_DATA_REG);
  } while ((value & WF_HW_STATUS_NOT_IN_RESET_MASK) == 0);

  do {
    delay_us(5000000);
    value = wf_read16BitWFRegister(WF_HOST_WFIFO_BCNT0_REG);
  } while (value == 0);
}

uint16_t wf_scratchMount(uint8_t rawId) {
  uint16_t byteCount;
    
  byteCount = wf_rawMove(rawId, RAW_SCRATCH_POOL, TRUE, 0);
  if (byteCount == 0) {
    /* work-around, somehow the scratch was already mounted to the other raw window */
    rawId = !rawId;
    //   WF_ASSERT(byteCount > 0);  /* scratch mount should always return value > 0 */
  }    


  wf_setRawWindowState(rawId, WF_SCRATCH_MOUNTED);
  return byteCount;
}

void wf_scratchUnmount(uint8_t rawId) {
  wf_rawMove(rawId, RAW_SCRATCH_POOL, FALSE, 0);
  if (rawId == RAW_RX_ID) {
    wf_setRawWindowState(RAW_RX_ID, WF_RAW_UNMOUNTED);
  } else {
    wf_setRawWindowState(RAW_TX_ID, WF_RAW_UNMOUNTED);        
  } 
}

uint16_t wf_rawMove(uint16_t rawId, uint16_t srcDest, BOOL rawIsDestination, uint16_t size) {
  uint16_t byteCount;
  uint8_t regId;
  uint8_t regValue8;
  uint16_t ctrlVal = 0;

  if (rawIsDestination)
  {
      ctrlVal |= 0x8000;
  }

  /* fix later, simply need to ensure that size is 12 bits are less */
  ctrlVal |= (srcDest << 8);              /* defines are already shifted by 4 bits */
  ctrlVal |= ((size >> 8) & 0x0f) << 8;   /* MS 4 bits of size (bits 11:8)         */
  ctrlVal |= (size & 0x00ff);             /* LS 8 bits of size (bits 7:0)          */

  /* Clear the interrupt bit in the register */
  regValue8 = (rawId == RAW_ID_0) ? WF_HOST_INT_MASK_RAW_0_INT_0 : WF_HOST_INT_MASK_RAW_1_INT_0;
  wf_write8BitWFRegister(WF_HOST_INTR_REG, regValue8);

  /* write update control value to register to control register */
  regId = (rawId == RAW_ID_0) ? RAW_0_CTRL_0_REG : RAW_1_CTRL_0_REG;
  wf_write16BitWFRegister(regId, ctrlVal);

  // Wait for the RAW move operation to complete, and read back the number of bytes, if any, that were overlayed
  byteCount = wf_waitForRawMoveComplete(rawId);

  return byteCount;
}

uint16_t wf_waitForRawMoveComplete(uint8_t rawId) {
  uint8_t rawIntMask;
  uint16_t byteCount;
  uint8_t regId;
  BOOL intDisabled;
  #if defined(WF_DEBUG)
    uint32_t startTickCount;
    uint32_t maxAllowedTicks;
  #endif

  /* create mask to check against for Raw Move complete interrupt for either RAW0 or RAW1 */
  rawIntMask = (rawId == RAW_ID_0) ? WF_HOST_INT_MASK_RAW_0_INT_0 : WF_HOST_INT_MASK_RAW_1_INT_0;

  /* 
  These variables are shared with the ISR so need to be careful when setting them.
  the WFEintHandler() is the isr that will touch these variables but will only touch
  them if RawMoveState.waitingForRawMoveCompleteInterrupt is set to TRUE.
  RawMoveState.waitingForRawMoveCompleteInterrupt is only set TRUE here and only here.
  so as long as we set RawMoveState.rawInterrupt first and then set RawMoveState.waitingForRawMoveCompleteInterrupt 
  to TRUE, we are guranteed that the ISR won't touch RawMoveState.rawInterrupt and 
  RawMoveState.waitingForRawMoveCompleteInterrupt. 
  */
  g_rawMoveState.rawInterrupt  = 0;  
  g_rawMoveState.waitingForRawMoveCompleteInterrupt = TRUE;

  // save state of external interrupt here
  // TODO: intDisabled = WF_EintIsDisabled();
  
  // if external interrupt is disabled, enable it because we need it for the while(1) loop to exit
  // TODO: if(intDisabled) {
  // TODO:   WF_EintEnable();
  // TODO: } else if(WF_EintIsPending()) {
  // TODO:   WF_EintEnable();
  // TODO: }

  #if defined(WF_DEBUG)
    // Before we enter the while loop, get the tick timer count and save it
    maxAllowedTicks = TICKS_PER_SECOND / 2;  /* 500 ms timeout */
    startTickCount = (UINT32)TickGet();
  #endif
  while (1)
  {
    /* if received an external interrupt that signalled the RAW Move */
    /* completed then break out of this loop                         */
    if(g_rawMoveState.rawInterrupt & rawIntMask) {
      break;
    }

    #if defined(WF_DEBUG)
      /* If timed out waiting for RAW Move complete than lock up */
      if (TickGet() - startTickCount >= maxAllowedTicks) {
          WF_ASSERT(FALSE);
      }
    #endif
  } /* end while */

  /* if interrupt was enabled by us here, we should disable it now that we're finished */
  // TODO: if(intDisabled) {
  // TODO:   WF_EintDisable();
  // TODO: }

  /* read the byte count and return it */
  regId = (rawId == RAW_ID_0) ? WF_HOST_RAW0_CTRL1_REG : WF_HOST_RAW1_CTRL1_REG;
  byteCount = wf_read16BitWFRegister(regId); 

  return ( byteCount );
}

void wf_write16BitWFRegister(uint8_t regId, uint16_t value) {
  g_buf[0] = regId | WF_WRITE_REGISTER_MASK;
  g_buf[1] = (uint8_t) (value >> 8); /* MS byte being written     */
  g_buf[2] = (uint8_t) (value & 0x00ff); /* LS byte being written     */
  spi_transfer(g_buf, 3, 1);
}

uint16_t wf_read16BitWFRegister(uint8_t regId) {
  g_buf[0] = regId | WF_READ_REGISTER_MASK;
  g_buf[1] = 0x00;
  g_buf[2] = 0x00;
  spi_transfer(g_buf, 3, 1);
  return (((uint16_t) g_buf[1]) << 8) | ((uint16_t) (g_buf[2]));
}

void wf_write8BitWFRegister(uint8_t regId, uint8_t value) {
  g_buf[0] = regId | WF_WRITE_REGISTER_MASK;
  g_buf[1] = value;
  spi_transfer(g_buf, 2, 1);
}

uint8_t wf_read8BitWFRegister(uint8_t regId) {
  g_buf[0] = regId | WF_READ_REGISTER_MASK;
  g_buf[1] = 0x00;
  spi_transfer(g_buf, 2, 1);
  return g_buf[1];
}

void spi_transfer(volatile uint8_t* buf, uint16_t len, uint8_t toggle_cs) {
  uint16_t i;

  gpio_write_bit(wf_cs_port, wf_cs_bit, 0);

  for (i = 0; i < len; i++) {
    while (!spi_is_tx_empty(wf_spi));
    spi_tx(wf_spi, (const void *) &buf[i], 1);
    while (!spi_is_rx_nonempty(wf_spi));
    buf[i] = spi_rx_reg(wf_spi);
  }

  if (toggle_cs) {
    gpio_write_bit(wf_cs_port, wf_cs_bit, 1);
  }

  return;
}

void wf_writeWFArray(uint8_t regId, uint8_t* p_Buf, uint16_t length) {
  /* output command byte */
  g_buf[0] = regId;
  spi_transfer(g_buf, 1, 0);

  /* write data array */
  spi_transfer(p_Buf, length, 1);
}

void wf_readWFArray(uint8_t regId, uint8_t *p_Buf, uint16_t length) {
  /* output command byte */
  g_buf[0] = regId | WF_READ_REGISTER_MASK;
  spi_transfer(g_buf, 1, 0);

  /* read data array */
  spi_transfer(p_Buf, length, 1);
}

void wf_hostInterrupt2RegInit(uint16_t hostIntMaskRegMask, uint8_t state) {
  uint16_t int2MaskValue;

  /* Host Int Register is a status register where each bit indicates a specific event  */
  /* has occurred. In addition, writing a 1 to a bit location in this register clears  */
  /* the event.                                                                        */

  /* Host Int Mask Register is used to enable those events activated in Host Int Register */
  /* to cause an interrupt to the host                                                    */

  /* read current state of int2 mask reg */
  int2MaskValue = wf_read16BitWFRegister(WF_HOST_INTR2_MASK_REG);

  /* if caller is disabling a set of interrupts */
  if (state == WF_INT_DISABLE) {
    /* zero out that set of interrupts in the interrupt mask copy */
    int2MaskValue &= ~hostIntMaskRegMask;
  }/* else caller is enabling a set of interrupts */
  else {
    /* set to 1 that set of interrupts in the interrupt mask copy */
    int2MaskValue |= hostIntMaskRegMask;
  }

  /* write out new interrupt mask value */
  wf_write16BitWFRegister(WF_HOST_INTR2_MASK_REG, int2MaskValue);

  /* ensure that pending interrupts from those updated interrupts are cleared */
  wf_write16BitWFRegister(WF_HOST_INTR2_REG, hostIntMaskRegMask);
}

void wf_hostInterruptRegInit(uint8_t hostIntrMaskRegMask, uint8_t state) {
  uint8_t hostIntMaskValue;

  /* Host Int Register is a status register where each bit indicates a specific event  */
  /* has occurred. In addition, writing a 1 to a bit location in this register clears  */
  /* the event.                                                                        */

  /* Host Int Mask Register is used to enable those events activated in Host Int Register */
  /* to cause an interrupt to the host                                                    */

  /* read current state of Host Interrupt Mask register */
  hostIntMaskValue = wf_read8BitWFRegister(WF_HOST_MASK_REG);

  /* if caller is disabling a set of interrupts */
  if (state == WF_INT_DISABLE) {
    /* zero out that set of interrupts in the interrupt mask copy */
    hostIntMaskValue = (hostIntMaskValue & ~hostIntrMaskRegMask);
  }/* else caller is enabling a set of interrupts */
  else {
    /* set to 1 that set of interrupts in the interrupt mask copy */
    hostIntMaskValue = (hostIntMaskValue & ~hostIntrMaskRegMask) | hostIntrMaskRegMask;
  }

  /* write out new interrupt mask value */
  wf_write8BitWFRegister(WF_HOST_MASK_REG, hostIntMaskValue);

  /* ensure that pending interrupts from those updated interrupts are cleared */
  wf_write8BitWFRegister(WF_HOST_INTR_REG, hostIntrMaskRegMask);
}

#define REG_ENABLE_LOW_POWER_MASK   ((uint16_t)(0x01))
#define REG_DISABLE_LOW_POWER_MASK  ((uint16_t)(0x00))

void wf_wfConfigureLowPowerMode(uint8_t action) {
  uint16_t lowPowerStatusRegValue;

  if (action == WF_LOW_POWER_MODE_ON) { /* if activating PS-Poll mode on MRF24W */
    //putrsUART("Enable PS\r\n");
    wf_write16BitWFRegister(WF_PSPOLL_H_REG, REG_ENABLE_LOW_POWER_MASK);
    g_psPollActive = TRUE;
  } else /* action == WF_LOW_POWER_MODE_OFF */ { /* else deactivating PS-Poll mode on MRF24W (taking it out of low-power mode and waking it up) */
    //putrsUART("Disable PS\r\n");
    wf_write16BitWFRegister(WF_PSPOLL_H_REG, REG_DISABLE_LOW_POWER_MASK);
    g_psPollActive = FALSE;

    /* poll the response bit that indicates when the MRF24W has come out of low power mode */
    do {
#if defined(MRF24WG)
      /* set the index register to the register we wish to read */
      wf_write16BitWFRegister(WF_INDEX_ADDR_REG, WF_SCRATCHPAD_1_REG);
#else /* must be a MRF24WB */
      /* set the index register to the register we wish to read */
      wf_write16BitWFRegister(WF_INDEX_ADDR_REG, WF_LOW_POWER_STATUS_REG);
#endif
      lowPowerStatusRegValue = wf_read16BitWFRegister(WF_INDEX_DATA_REG);

    } while (lowPowerStatusRegValue & REG_ENABLE_LOW_POWER_MASK);
  }
}

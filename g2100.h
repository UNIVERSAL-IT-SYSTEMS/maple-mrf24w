
#ifndef G2100_H_
#define G2100_H_

#include <stdint.h>
#include <gpio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define WF_DISABLED  (0)
#define WF_ENABLED   (1)

  /*-------------------------------------------------------*/
  /* Security Type defines                                 */
  /* Used in WF_CPSet/GetSecurityType WF_CPSet/GetElements */
  /*-------------------------------------------------------*/
#define WF_SECURITY_OPEN                         (0)
#define WF_SECURITY_WEP_40                       (1)
#define WF_SECURITY_WEP_104                      (2)
#define WF_SECURITY_WPA_WITH_KEY                 (3)
#define WF_SECURITY_WPA_WITH_PASS_PHRASE         (4)
#define WF_SECURITY_WPA2_WITH_KEY                (5)
#define WF_SECURITY_WPA2_WITH_PASS_PHRASE        (6)
#define WF_SECURITY_WPA_AUTO_WITH_KEY            (7)
#define WF_SECURITY_WPA_AUTO_WITH_PASS_PHRASE    (8)
#define WF_SECURITY_WPS_PUSH_BUTTON              (9)
#define WF_SECURITY_WPS_PIN                      (10)
#define WF_SECURITY_EAP                          (11)    /* currently not supported */


  /* Wep key types */
#define WF_SECURITY_WEP_SHAREDKEY  (0)
#define WF_SECURITY_WEP_OPENKEY    (1)

  /*---------------------------------------------------------------------*/
  /* Network Type defines                                                */
  /* Used in WF_CPSet/GetNetworkType, WF_CPSetElements, WF_CPGetElements */
  /*---------------------------------------------------------------------*/
#define WF_INFRASTRUCTURE 1
#define WF_ADHOC          2
#define WF_P2P            3     
#define WF_SOFT_AP        4     


  void wf_init();
  void wf_isr();
  void wf_scan();
  void wf_connect();
  void wf_getMacAddress(uint8_t* buf);

  //#define ZG_WIRELESS_MODE_INFRA	1
  //#define ZG_WIRELESS_MODE_ADHOC	2
  //
  //#define u8 uint8_t
  //#define U8 uint8_t
  //#define u16 uint16_t
  //#define U16 uint16_t
  //
  //  //Host to Zero G long
  //#define HTOZGL(a) (	 ((a & 0x000000ff)<<24) 
  //					|((a & 0x0000ff00)<<8)  
  //					|((a & 0x00ff0000)>>8) 
  //					|((a & 0xff000000)>>24)	)
  //#define ZGTOHL(a) HTOZGL(a)
  //
  //  // Host to Zero G short
  //#define HSTOZGS(a) (u16)(((a)<<8) | ((a)>>8))
  //#define ZGSTOHS(a) HSTOZGS(a)
  //  //#define HTONS(a) HSTOZGS(a)
  //
  //#define ZG_INTERRUPT_PIN	0	// Pin on Arduino
  //
  //  // Command values which appear in ZG_PREAMBLE_CMD_IDX for each SPI message
  //#define ZG_CMD_FIFO_ACCESS    (0x80)
  //#define ZG_CMD_WT_FIFO_DATA   (ZG_CMD_FIFO_ACCESS | 0x20)
  //#define ZG_CMD_WT_FIFO_MGMT   (ZG_CMD_FIFO_ACCESS | 0x30)
  //#define ZG_CMD_RD_FIFO        (ZG_CMD_FIFO_ACCESS | 0x00)
  //#define ZG_CMD_WT_FIFO_DONE   (ZG_CMD_FIFO_ACCESS | 0x40)
  //#define ZG_CMD_RD_FIFO_DONE   (ZG_CMD_FIFO_ACCESS | 0x50)
  //#define ZG_CMD_WT_REG         (0x00)
  //#define ZG_CMD_RD_REG         (0x40)
  //
  //  // Type values which appear in ZG_PREAMBLE_TYPE_IDX for each SPI message
  //#define ZG_MAC_TYPE_TXDATA_REQ  1
  //#define ZG_MAC_TYPE_MGMT_REQ    2
  //
  //#define ZG_MAC_TYPE_TXDATA_CONFIRM	((u8)1)
  //#define ZG_MAC_TYPE_MGMT_CONFIRM	((u8)2)
  //#define ZG_MAC_TYPE_RXDATA_INDICATE	((u8)3)
  //#define ZG_MAC_TYPE_MGMT_INDICATE	((u8)4)
  //
  //  // Subtype values which appear in ZG_PREAMBLE_SUBTYPE_IDX for each SPI message
  //  // Subtype for ZG_MAC_TYPE_TXDATA_REQ and ZG_MAC_TYPE_TXDATA_CONFIRM
  //#define ZG_MAC_SUBTYPE_TXDATA_REQ_STD			((u8)1)
  //
  //  // Subtype for ZG_MAC_TYPE_MGMT_REQ and ZG_MAC_TYPE_MGMT_CONFIRM
  //#define ZG_MAC_SUBTYPE_MGMT_REQ_PMK_KEY        0x08
  //#define ZG_MAC_SUBTYPE_MGMT_REQ_WEP_KEY        0x0a
  //#define ZG_MAC_SUBTYPE_MGMT_REQ_CALC_PSK       0x0c
  //#define ZG_MAC_SUBTYPE_MGMT_REQ_SET_PARAM      0x0f
  //#define ZG_MAC_SUBTYPE_MGMT_REQ_GET_PARAM      0x10
  //#define ZG_MAC_SUBTYPE_MGMT_REQ_ADHOC_START    0x12
  //#define ZG_MAC_SUBTYPE_MGMT_REQ_CONNECT        0x13
  //#define ZG_MAC_SUBTYPE_MGMT_REQ_CONNECT_MANAGE 0x14
  //#define ZG_MAX_SUBTYPE_MGMT_REQ_SCAN_START     0x1f
  //
  //  // Subtype for ZG_MAC_TYPE_RXDATA_INDICATE
  //#define ZG_MAC_SUBTYPE_RXDATA_IND_STD          1
  //
  //  // Subtype for ZG_MAC_TYPE_MGMT_INDICATE
  //#define ZG_MAC_SUBTYPE_MGMT_IND_DISASSOC       1
  //#define ZG_MAC_SUBTYPE_MGMT_IND_DEAUTH         2
  //#define ZG_MAC_SUBTYPE_MGMT_IND_CONN_STATUS    4
  //
  //  // Parameter IDs for ZG_MAC_SUBTYPE_MGMT_REQ_SET_PARAM
  //#define ZG_PARAM_MAC_ADDRESS			(1)
  //
  //  // MAC result code
  //
  //  enum {
  //    ZG_RESULT_SUCCESS = 0x01,
  //    ZG_RESULT_INVALID_SUBTYPE = 0x02,
  //    ZG_RESULT_CANCELLED = 0x03,
  //    ZG_RESULT_FRAME_EOL = 0x04,
  //    ZG_RESULT_FRAME_RETRY_LIMIT = 0x05,
  //    ZG_RESULT_FRAME_NO_BSS = 0x06,
  //    ZG_RESULT_FRAME_TOO_BIG = 0x07,
  //    ZG_RESULT_FRAME_ENCRYPT_FAILURE = 0x08,
  //    ZG_RESULT_INVALID_PARAMS = 0x09,
  //    ZG_RESULT_ALREADY_AUTH = 0x0a,
  //    ZG_RESULT_ALREADY_ASSOC = 0x0b,
  //    ZG_RESULT_INSUFFICIENT_RSRCS = 0x0c,
  //    ZG_RESULT_TIMEOUT = 0x0d,
  //    ZG_RESULT_BAD_EXCHANGE = 0x0e, // frame exchange problem with peer (AP or STA)
  //    ZG_RESULT_AUTH_REFUSED = 0x0f, // authenticating node refused our request
  //    ZG_RESULT_ASSOC_REFUSED = 0x10, // associating node refused our request
  //    ZG_RESULT_REQ_IN_PROGRESS = 0x11, // only one mlme request at a time allowed
  //    ZG_RESULT_NOT_JOINED = 0x12, // operation requires that device be joined with target
  //    ZG_RESULT_NOT_ASSOC = 0x13, // operation requires that device be associated with target
  //    ZG_RESULT_NOT_AUTH = 0x14, // operation requires that device be authenticated with target
  //    ZG_RESULT_SUPPLICANT_FAILED = 0x15,
  //    ZG_RESULT_UNSUPPORTED_FEATURE = 0x16,
  //    ZG_RESULT_REQUEST_OUT_OF_SYNC = 0x17 // Returned when a request is recognized but invalid given the current state of the MAC
  //  };
  //
  //#define ZG_WRITE_REGISTER_MASK 0x00
  //#define ZG_READ_REGISTER_MASK  0x40
  //
  //#define ZG_SCAN_ALL ((uint8_t)(0xff))
  //
  //
  //#define ZG_INTR_REG_LEN             (1)
  //#define ZG_INTR_MASK_REG_LEN        (1)
  //#define ZG_SYS_INFO_DATA_REG_LEN    (1)
  //#define ZG_SYS_INFO_IDX_REG_LEN     (2)
  //#define ZG_INTR2_REG_LEN            (2)
  //#define ZG_INTR2_MASK_REG_LEN       (2)
  //#define ZG_BYTE_COUNT_REG_LEN       (2)
  //#define ZG_BYTE_COUNT_FIFO0_REG_LEN (2)
  //#define ZG_BYTE_COUNT_FIFO1_REG_LEN (2)
  //#define ZG_PWR_CTRL_REG_LEN         (2)
  //#define ZG_INDEX_ADDR_REG_LEN       (2)
  //#define ZG_INDEX_DATA_REG_LEN       (2)
  //
  //  // registers accessed via the ZG_INDEX_ADDR_REG and ZG_INDEX_DATA_REG registers
  //#define ZG_HW_STATUS_REG        (0x2a) // 16-bit read only register providing HW status bits
  //#define ZG_CONFIG_CTRL0_REG     (0x2e) // 16-bit register used to initiate hard reset
  //#define ZG_LOW_PWR_STATUS_REG   (0x3e) // 16-bit register read to determine when device
  //  // out of sleep state
  //
  //  /**
  //   * This bit mask is used in the ZG_HW_STATUS_REG to determine
  //   * when the MRF24W has completed its hardware reset.       
  //   *  0 : MRF24W is in reset                                 
  //   *  1 : MRF24W is not in reset                             
  //   */
  //#define WF_HW_STATUS_NOT_IN_RESET_MASK ((uint16_t)(0x1000)) 
  //
  //#define ZG_ENABLE_LOW_PWR_MASK  (0x01) // used by the Host to enable/disable sleep state
  //  // indicates to G2100 that the Host has completed
  //  // transactions and the device can go into sleep
  //  // state if possible
  //
  //  // states for interrupt state machine
  //#define ZG_INTR_ST_RD_INTR_REG	(1)
  //#define ZG_INTR_ST_WT_INTR_REG	(2)
  //#define ZG_INTR_ST_RD_CTRL_REG	(3)
  //
  //  // interrupt state
  //#define ZG_INTR_DISABLE		((u8)0)
  //#define ZG_INTR_ENABLE		((u8)1)
  //
  //  // mask values for ZG_INTR_REG and ZG_INTR2_REG
  //#define	ZG_INTR_MASK_FIFO1       (0x80)
  //#define ZG_INTR_MASK_FIFO0       (0x40)
  //#define ZG_INTR_MASK_RAW_1_INT_0 (0x04)
  //#define ZG_INTR_MASK_RAW_0_INT_0 (0x02)
  //#define ZG_INTR_MASK_ALL         (0xff)
  //#define ZG_INTR2_MASK_ALL        (0xffff)
  //
  //  // Buffer size
  //#define ZG_BUFFER_SIZE		450
  //
  //  // Types of networks
  //#define ZG_BSS_INFRA		(1)    // infrastructure only
  //#define ZG_BSS_ADHOC		(2)    // Ad-hoc only (ibss)
  //
  //  // Max characters in network SSID
  //#define ZG_MAX_SSID_LENGTH		32
  //
  //  // Security keys
  //#define ZG_MAX_ENCRYPTION_KEYS 		4
  //#define ZG_MAX_ENCRYPTION_KEY_SIZE	13
  //#define ZG_MAX_WPA_PASSPHRASE_LEN	64
  //#define ZG_MAX_PMK_LEN				32
  //
  //#define ZG_SECURITY_TYPE_NONE	0
  //#define ZG_SECURITY_TYPE_WEP	1
  //#define ZG_SECURITY_TYPE_WPA	2
  //#define ZG_SECURITY_TYPE_WPA2	3
  //
  //#define ZG_LOW_POWER_MODE_ON  (1)
  //#define ZG_LOW_POWER_MODE_OFF (0)
  //
  //#define ZG_REG_ENABLE_LOW_POWER_MASK   0x01
  //#define ZG_REG_DISABLE_LOW_POWER_MASK  0x00
  //
  //#define ZG_CONNECT_MANAGE_ENABLE  0x01
  //#define ZG_CONNECT_MANAGE_DISABLE 0x00
  //
  //  /**
  //   * enable start and stop indication messages from G2100 during reconnection
  //   */
  //#define ZG_CONNECT_MANAGE_START_STOP_MSG         0x10
  //
  //  /**
  //   * start reconnection on receiving a deauthentication message from the AP
  //   */
  //#define ZG_CONNECT_MANAGE_RECONNECT_DEAUTH       0x02
  //
  //  /**
  //   * start reconnection when the missed beacon count exceeds the threshold. 
  //   * uses default value of 100 missed beacons if not set during initialization
  //   */
  //#define ZG_CONNECT_MANAGE_RECONNECT_BEACON_COUNT 0x01
  //
  //#pragma pack(push, 1)
  //
  //  typedef struct {
  //    u8 slot; /* slot index */
  //    u8 keyLen;
  //    u8 defID; /* the default wep key id */
  //    u8 ssidLen; /* num valid bytes in ssid */
  //    u8 ssid[ZG_MAX_SSID_LENGTH]; /* ssid of network */
  //    u8 key[ZG_MAX_ENCRYPTION_KEYS][ZG_MAX_ENCRYPTION_KEY_SIZE]; /* wep key data for 4 default keys */
  //  } zg_wep_key_req_t;
  //
  //#define ZG_WEP_KEY_REQ_SIZE		(4 + ZG_MAX_SSID_LENGTH + ZG_MAX_ENCRYPTION_KEYS*ZG_MAX_ENCRYPTION_KEY_SIZE)
  //
  //  typedef struct {
  //    u8 configBits;
  //    u8 phraseLen; /* number of valid bytes in passphrase */
  //    u8 ssidLen; /* number of valid bytes in ssid */
  //    u8 reserved; /* alignment byte */
  //    u8 ssid[ZG_MAX_SSID_LENGTH]; /* the string of characters representing the ssid */
  //    u8 passPhrase[ZG_MAX_WPA_PASSPHRASE_LEN]; /* the string of characters representing the passphrase */
  //  } zg_psk_calc_req_t;
  //
  //#define ZG_PSK_CALC_REQ_SIZE	(4 + ZG_MAX_SSID_LENGTH + ZG_MAX_WPA_PASSPHRASE_LEN) /* 100 bytes */
  //
  //  typedef struct {
  //    u8 result; /* indicating success or other */
  //    u8 macState; /* current State of the on-chip MAC */
  //    u8 keyReturned; /* 1 if psk contains key data, 0 otherwise */
  //    u8 reserved; /* pad byte */
  //    u8 psk[ZG_MAX_PMK_LEN]; /* the psk bytes */
  //  } zg_psk_calc_cnf_t;
  //
  //  typedef struct {
  //    u8 slot;
  //    u8 ssidLen;
  //    u8 ssid[ZG_MAX_SSID_LENGTH];
  //    u8 keyData[ZG_MAX_PMK_LEN];
  //  } zg_pmk_key_req_t;
  //
  //#define ZG_PMK_KEY_REQ_SIZE		(2 + ZG_MAX_SSID_LENGTH + ZG_MAX_PMK_LEN)
  //
  //  typedef struct {
  //    u16 rssi; /* the value of the G1000 RSSI when the data frame was received */
  //    u8 dstAddr[6]; /* MAC Address to which the data frame was directed. */
  //    u8 srcAddr[6]; /* MAC Address of the Station that sent the Data frame. */
  //    u16 arrivalTime_th; /* the value of the 32-bit G1000 system clock when the frame arrived */
  //    u16 arrivalTime_bh;
  //    u16 dataLen; /* the length in bytes of the payload which immediately follows this data structure */
  //  } zg_rx_data_ind_t;
  //
  //  typedef struct {
  //    uint8_t secType; /* security type : 0 - none; 1 - wep; 2 - wpa; 3 - wpa2; 0xff - best available */
  //    uint8_t ssidLen; /* num valid bytes in ssid */
  //    uint8_t ssid[ZG_MAX_SSID_LENGTH]; /* the ssid of the target */
  //    uint16_t sleepDuration; /* power save sleep duration in units of 100 milliseconds */
  //    uint8_t modeBss; /* 1 - infra; 2 - adhoc */
  //    uint8_t reserved;
  //  } zg_connect_req_t;
  //
  //  typedef struct {
  //    uint8_t enable;
  //    uint8_t retryCount;
  //    uint8_t flags;
  //    uint8_t unknown;
  //  } zg_connect_manage_t;
  //
  //#pragma pack(pop)
  //
  //  void zg_init();
  //  void zg_reset();
  //  void zg_scan();
  //  void zg_connect();
  //  void zg_chip_reset();
  //  void zg_config_low_power_mode(uint8_t action);
  //  void zg_isr();
  //  void zg_drv_process();
  //  void zg_send(U8* buf, U16 len);
  //  void zg_recv(U8* buf, U16* len);
  //  U16 zg_get_rx_status();
  //  void zg_clear_rx_status();
  //  void zg_set_tx_status(U8 status);
  //  U8 zg_get_conn_state();
  //  void zg_set_buf(U8* buf, U16 buf_len);
  //  U8* zg_get_mac();
  //  void zg_set_ssid(U8* ssid, U8 ssid_len);
  //  void zg_set_sec(U8 sec_type, U8* sec_key, U8 sec_key_len);
  //
  //  typedef void(*zg_hook_on_connected_fn)(void* userData, uint8_t connected);
  //  extern zg_hook_on_connected_fn zg_hook_on_connected;
  //  extern void* zg_hook_on_connected_user_data;

#ifdef __cplusplus
}
#endif

#endif /* G2100_H_ */

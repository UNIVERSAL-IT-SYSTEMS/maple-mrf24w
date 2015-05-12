
#ifndef MRF24W_G2100_H_
#define MRF24W_G2100_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// TODO: move?? 
void usart_putc(void* dev, uint8_t ch);

struct uip_eth_addr {
  uint8_t addr[6];
};

struct uip_eth_hdr {
  struct uip_eth_addr dest;
  struct uip_eth_addr src;
  uint16_t type;
};





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

  /*----------------------------------------------------------------------------*/
  /* Events that can be invoked in WF_ProcessEvent().  Note that the            */
  /* connection events are optional, all other events the app must be notified. */
  /*----------------------------------------------------------------------------*/

#define WF_EVENT_CONNECTION_SUCCESSFUL           (1)   /**< Connection attempt to network successful            */
#define WF_EVENT_CONNECTION_FAILED               (2)   /**< Connection attempt failed                           */


#define WF_EVENT_CONNECTION_TEMPORARILY_LOST     (3)   /**< Connection lost; MRF24W attempting to reconnect     */
#define WF_EVENT_CONNECTION_PERMANENTLY_LOST     (4)   /**< Connection lost; MRF24W no longer trying to connect */  
#define WF_EVENT_CONNECTION_REESTABLISHED        (5)

#define WF_EVENT_FLASH_UPDATE_SUCCESSFUL         (6)   /**< Update to FLASH successful                          */
#define WF_EVENT_FLASH_UPDATE_FAILED             (7)   /**< Update to FLASH failed                              */



#define WF_EVENT_KEY_CALCULATION_REQUEST         (8)   /**< Key calculation is required                         */

#define WF_EVENT_SCAN_RESULTS_READY              (9)   /**< scan results are ready                              */ 
#define WF_EVENT_IE_RESULTS_READY                (10)  /**< IE data ready                                       */


#define WF_EVENT_RX_PACKET_RECEIVED              (11)  /**< Rx data packet has been received by MRF24W          */
#define WF_EVENT_INVALID_WPS_PIN                 (12)  /**< Invalid WPS pin was entered                         */

  /* this block of defines is used to check illegal reentry when in WF API functions */
#define WF_ENTERING_FUNCTION    (1)
#define WF_LEAVING_FUNCTION     (0)

  /* bit masks for functions that need to be tracked when they are called */
#define WF_PROCESS_EVENT_FUNC   ((uint8_t)0x01)

  /*---------------------------------------------------------------------*/
  /* Network Type defines                                                */
  /* Used in WF_CPSet/GetNetworkType, WF_CPSetElements, WF_CPGetElements */
  /*---------------------------------------------------------------------*/
#define WF_INFRASTRUCTURE 1
#define WF_ADHOC          2
#define WF_P2P            3     
#define WF_SOFT_AP        4     

#define WF_SCAN_ALL ((uint8_t)0xff)

#define WF_MAX_SSID_LENGTH              (32)
#define WF_BSSID_LENGTH                 (6)
#define WF_MAX_NUM_RATES                (8)

  /*----------------------------------------------------*/
  /* Scan type defines                                  */
  /* Used in WF_CASet/GetScanType, WF_CASet/GetElements */
  /*----------------------------------------------------*/
#define WF_ACTIVE_SCAN   (1)
#define WF_PASSIVE_SCAN  (2)

#define MAC_IP      	(0x00u)
#define MAC_ARP     	(0x06u)
#define MAC_UNKNOWN 	(0xFFu)

#define RESERVED_HTTP_MEMORY 0ul
#define RESERVED_SSL_MEMORY  0ul
#define MAX_PACKET_SIZE      (1514ul)

  // Memory addresses
#define RAMSIZE           (14170ul - 8192ul - RESERVED_HTTP_MEMORY - RESERVED_SSL_MEMORY)
#define TXSTART           (RAMSIZE - (4ul + MAX_PACKET_SIZE + 4ul))
#define RXSTART           (0ul)
#define RXSTOP            ((TXSTART - 2ul) | 0x0001ul)
#define RXSIZE            (RXSTOP - RXSTART + 1ul)
#define BASE_TX_ADDR      (TXSTART + 4ul)
#define BASE_SCRATCH_ADDR (BASE_TX_ADDR + (MAX_PACKET_SIZE + 4ul))
#define BASE_HTTPB_ADDR   (BASE_SCRATCH_ADDR)
#define BASE_SSLB_ADDR    (BASE_HTTPB_ADDR + RESERVED_HTTP_MEMORY)
#define BASE_TCB_ADDR     (BASE_SSLB_ADDR + RESERVED_SSL_MEMORY)

  /**
   * Scan Results
   */
  typedef struct {
    uint8_t bssid[WF_BSSID_LENGTH]; // Network BSSID value
    uint8_t ssid[WF_MAX_SSID_LENGTH]; // Network SSID value

    /**
      Access point configuration
      <table>
        Bit 7       Bit 6       Bit 5       Bit 4       Bit 3       Bit 2       Bit 1       Bit 0
        -----       -----       -----       -----       -----       -----       -----       -----
        WPA2        WPA         Preamble    Privacy     Reserved    Reserved    Reserved    IE
      </table>
      
      <table>
      IE        1 if AP broadcasting one or more Information Elements, else 0
      Privacy   0 : AP is open (no security)
                 1: AP using security,  if neither WPA and WPA2 set then security is WEP.
      Preamble  0: AP transmitting with short preamble
                 1: AP transmitting with long preamble
      WPA       Only valid if Privacy is 1.
                 0: AP does not support WPA
                 1: AP supports WPA
      WPA2      Only valid if Privacy is 1.
                 0: AP does not support WPA2
                 1: AP supports WPA2
      </table>
     */
    uint8_t apConfig;
    uint8_t reserved;
    uint16_t beaconPeriod; // Network beacon interval          
    uint16_t atimWindow; // Only valid if bssType = WF_INFRASTRUCTURE

    /**
      List of Network basic rates.  Each rate has the following format:
      
      Bit 7
     * 0 � rate is not part of the basic rates set
     * 1 � rate is part of the basic rates set

      Bits 6:0 
      Multiple of 500kbps giving the supported rate.  For example, a value of 2 
      (2 * 500kbps) indicates that 1mbps is a supported rate.  A value of 4 in 
      this field indicates a 2mbps rate (4 * 500kbps).
     */
    uint8_t basicRateSet[WF_MAX_NUM_RATES];
    uint8_t rssi; // Signal strength of received frame beacon or probe response
    uint8_t numRates; // Number of valid rates in basicRates
    uint8_t DtimPeriod; // Part of TIM element
    uint8_t bssType; // WF_INFRASTRUCTURE or WF_ADHOC
    uint8_t channel; // Channel number
    uint8_t ssidLen; // Number of valid characters in ssid

  } tWFScanResult;

  /**
   * Structure to contain a MAC address
   */
  typedef struct __attribute__((__packed__)) {
    uint8_t v[6];
  }
  MAC_ADDR;

  extern uint8_t wf_connected;

  extern void wf_processEvent(uint8_t event, uint16_t eventInfo, uint8_t* extraInfo);

  void wf_init();
  void wf_isr();

  /**
   * Returns the ECON1.TXRTS bit
   * 
   * @return TRUE: If no Ethernet transmission is in progress
   *         FALSE: If a previous transmission was started, and it has
   *                 not completed yet.  While FALSE, the data in the
   *                 transmit buffer and the TXST/TXND pointers must not
   *                 be changed.
   */
  uint8_t wf_macIsTxReady(void);

  /**
   * SPI write pointer is updated.  All calls to
   *   MACPut() and MACPutArray() will use this new value.
   * 
   * @param address Address to seek to
   * @return Old EWRPT location
   */
  uint32_t wf_macSetWritePtr(uint32_t address);

  /**
   * Because of the dataLen parameter, it is probably
   *                  advantagous to call this function immediately before
   *                  transmitting a packet rather than initially when the
   *                  packet is first created.  The order in which the packet
   *                  is constructed (header first or data first) is not
   *                  important.
   * 
   * @param remote Pointer to memory which contains the destination
   *                           MAC address (6 bytes)
   * @param type The constant ETHER_ARP or ETHER_IP, defining which
   *                        value to write into the Ethernet header's type field.
   * @param dataLen Length of the Ethernet data payload
   */
  void wf_macPutHeader(MAC_ADDR *remote, uint8_t type, uint16_t dataLen);

  /**
   * MACPutArray writes several sequential bytes to the
   *                  MRF24W RAM.  It performs faster than multiple MACPut()
   *                  calls.  EWRPT is incremented by len.
   * 
   * @param val Pointer to source of bytes to copy.
   * @param len Number of bytes to write to the data buffer.
   */
  void wf_macPutArray(uint8_t *val, uint16_t len);

  /**
   * MACFlush causes the current TX packet to be sent out on
   *                  the Ethernet medium.  The hardware MAC will take control
   *                  and handle CRC generation, collision retransmission and
   *                  other details.
   * 
   * After transmission completes (MACIsTxReady() returns TRUE),
   *                  the packet can be modified and transmitted again by calling
   *                  MACFlush() again.  Until MACPutHeader() or MACPut() is
   *                  called (in the TX data area), the data in the TX buffer
   *                  will not be corrupted.
   */
  void wf_macFlush(void);

  /**
   * /brief Commands the MRF24W to start a scan operation.  This will generate the WF_EVENT_SCAN_RESULTS_READY event.
   * 
   * Directs the MRF24W to initiate a scan operation utilizing the input 
   * Connection Profile ID.  The Host Application will be notified that the scan 
   * results are ready when it receives the WF_EVENT_SCAN_RESULTS_READY event.  
   * The eventInfo field for this event will contain the number of scan results.  
   * Once the scan results are ready they can be retrieved with 
   * WF_ScanGetResult().
   * 
   * Scan results are retained on the MRF24W until:
   * 1.  Calling WF_Scan() again (after scan results returned from previous call).
   * 2.  MRF24W reset.
   * 
   * @param cpid Connection Profile to use.
   *             If the CpId is valid then the values from that Connection Profile 
   *             will be used for filtering scan results.  If the CpId is set to 
   *             WF_SCAN_ALL (0xFF) then a default filter will be used.
   * 
   *             Valid CpId
   *             * If CP has a defined SSID only scan results with that SSID are 
   *                retained.  
   *             * If CP does not have a defined SSID then all scanned SSID�s will be 
   *                retained
   *             * Only scan results from Infrastructure or AdHoc networks are 
   *                retained, depending on the value of networkType in the Connection Profile
   *             * The channel list that is scanned will be determined from 
   *                channelList in the Connection Algorithm (which must be defined 
   *                before calling this function).
   * 
   *             CpId is equal to WF_SCAN_ALL
   *             * All scan results are retained (both Infrastructure and Ad Hoc 
   *                networks).
   *             * All channels within the MRF24W�s regional domain will be 
   *                scanned.
   *             * No Connection Profiles need to be defined before calling this 
   *                function.
   *             * The Connection Algorithm does not need to be defined before 
   *                calling this function.
   */
  uint16_t wf_scan(uint8_t cpid);

  /**
   * /brief Read scan results back from MRF24W.
   * 
   * After a scan has completed this function is used to read one or more of the 
   *     scan results from the MRF24W.  The scan results will be written 
   *     contiguously starting at p_scanResults (see tWFScanResult structure for 
   *     format of scan result).    
   * @param listIndex Index (0-based list) of the scan entry to retrieve
   * @param p_scanResult Pointer to location to store the scan result structure
   */
  void wf_scanGetResult(uint8_t listIndex, tWFScanResult* p_scanResult);

  /**
   * Called by WF_ProcessEvent() to be able to detect if there is an attempt 
   *           to send a management message while processing the event (not allowed).
   * @param funcMask bit mask indicating the calling function
   * @param state WF_ENTERING_FUNCTION or WF_LEAVING_FUNCTION
   */
  void wf_setFuncState(uint8_t funcMask, uint8_t state);

  void wf_getMacAddress(uint8_t* buf);

  /**
   * 
   * @param remote Location to store the Source MAC address of the received frame.
   * @param type Location of a BYTE to store the constant
   *                         MAC_UNKNOWN, ETHER_IP, or ETHER_ARP, representing
   *                         the contents of the Ethernet type field.
   * @return If a packet was waiting in the RX buffer.  The
   *          remote, and type values are updated and the size is returned.
   *         If a packet was not pending.  remote and type are not changed and 0 is returned.
   */
  uint16_t wf_macGetHeader(MAC_ADDR* remote, uint8_t* type);

  /**
   * Burst reads several sequential bytes from the data buffer
   *                  and places them into local memory.  With SPI burst support,
   *                  it performs much faster than multiple MACGet() calls.
   *                  ERDPT is incremented after each byte, following the same
   *                  rules as MACGet().
   * 
   * @param val Pointer to storage location
   * @param len Number of bytes to read from the data buffer
   * @return Byte(s) of data read from the data buffer.
   */
  uint16_t wf_macGetArray(uint8_t *val, uint16_t len);

  /**
   * Marks the last received packet (obtained using
   *                  MACGetHeader())as being processed and frees the buffer
   *                  memory associated with it
   * 
   * It is safe to call this function multiple times between
   *                  MACGetHeader() calls.  Extra packets won't be thrown away
   *                  until MACGetHeader() makes it available.
   */
  void wf_macDiscardRx(void);

  /**
   * Called form main loop to support 802.11 operations
   */
  void wf_macProcess(void);

  /**
   * /brief Creates a Connection Profile on the MRF24W.
   * 
   * Requests the MRF24W to create a Connection Profile (CP), assign it an ID, 
   *     and set all the elements to default values.  The ID returned by this function
   *     is used in other connection profile functions.  A maximum of 2 Connection 
   *     Profiles can exist on the MRF24W.
   * @return 
   */
  uint8_t wf_cpCreate();

  /**
   * /brief Sets the SSID for the specified Connection Profile ID.
   * 
   * Sets the SSID and SSID Length elements in the Connection Profile.  Note that
   *     an Access Point can have either a visible or hidden SSID.  If an Access Point
   *     uses a hidden SSID then an active scan must be used (see scanType field in the 
   *     Connection Algorithm).
   * @param CpId Connection Profile ID
   * @param p_ssid Pointer to the SSID string
   * @param ssidLength Number of bytes in the SSID
   */
  void wf_cpSetSsid(uint8_t CpId, uint8_t *p_ssid, uint8_t ssidLength);

  /**
   * /brief Sets the network for the specified Connection Profile ID.
   * 
   * Sets the Network Type element a Connection Profile.  Allowable values are:
   * WF_INFRASTRUCTURE
   * WF_ADHOC
   * 
   * @param CpId Connection Profile ID
   * @param networkType Type of network to create (infrastructure or adhoc)
   */
  void wf_cpSetNetworkType(uint8_t CpId, uint8_t networkType);

  /**
   * /brief Sets the channel list.
   * 
   * Sets the Channel List used by the Connection Algorithm.
   * 
   * @param p_channelList Pointer to channel list.
   * @param numChannels Number of channels in p_channelList.  If set to 0, the
   *                      MRF24W will use all valid channels for the current 
   *                      regional domain.
   */
  void wf_caSetChannelList(uint8_t *p_channelList, uint8_t numChannels);

  /**
   * /brief Sets the list retry count
   * 
   * Number of times to cycle through Connection Profile List before giving up on 
   *     the connection attempt.  Since lists are not yet supported, this function 
   *     actually sets the number of times the Connection Manager will try to connect
   *     with the current Connection Profile before giving up.
   * 
   * @param listRetryCount 0 to 254 or WF_RETRY_FOREVER (255)
   */
  void wf_caSetListRetryCount(uint8_t listRetryCount);

  /**
   * /brief Sets the beacon timeout value.
   * 
   * Sets the Beacon Timeout used by the Connection Algorithm.
   * 
   *     <table>
   *         Value   Description
   *         -----   -----------
   *         0       No monitoring of the beacon timeout condition.  The host will
   *                  not be notified of this event.
   *         1-255   Number of beacons missed before disconnect event occurs and 
   *                  beaconTimeoutAction occurs.  If enabled, host will receive
   *                  an event message indicating connection temporarily or 
   *                  permanently lost, and if retrying, a connection successful
   *                  event.
   *     </table>
   * 
   * @param beaconTimeout Number of beacons that can be missed before the action in 
   *                     beaconTimeoutAction is taken.
   */
  void wf_caSetBeaconTimeout(uint8_t beaconTimeout);

  /**
   * /brief Sets the security for the specified Connection Profile.
   * 
   * Configures security for a Connection Profile.
   * 
   *     <table>
   *     Security                                Key         Length
   *     --------                                ---         ------
   *     WF_SECURITY_OPEN                        N/A         N/A
   *     WF_SECURITY_WEP_40                      hex         4, 5 byte keys
   *     WF_SECURITY_WEP_104                     hex         4, 13 byte keys
   *     WF_SECURITY_WPA_WITH_KEY                hex         32 bytes
   *     WF_SECURITY_WPA_WITH_PASS_PHRASE        ascii       8-63 ascii characters
   *     WF_SECURITY_WPA2_WITH_KEY               hex         32 bytes
   *     WF_SECURITY_WPA2_WITH_PASS_PHRASE       ascii       8-63 ascii characters
   *     WF_SECURITY_WPA_AUTO_WITH_KEY           hex         32 bytes
   *     WF_SECURITY_WPA_AUTO_WITH_PASS_PHRASE   ascii       8-63 ascii characters
   *     </table>
   * 
   * @param CpId Connection Profile ID
   * @param securityType Value corresponding to the security type desired.
   * @param wepKeyIndex 0 thru 3 (only used if security type is WF_SECURITY_WEP_40 or WF_SECURITY_WEP_104)
   * @param p_securityKey Binary key or passphrase (not used if security is WF_SECURITY_OPEN)
   * @param securityKeyLength Number of bytes in p_securityKey (not used if security is WF_SECURITY_OPEN)
   */
  void wf_cpSetSecurity(uint8_t CpId, uint8_t securityType, uint8_t wepKeyIndex, uint8_t *p_securityKey, uint8_t securityKeyLength);

  /**
   * /brief Sets the Connection Algorith scan type
   * 
   * Configures the Connection Algorithm for the desired scan type.
   * 
   * Active scanning causes the MRF24W to send probe requests.  Passive
   *     scanning implies the MRF24W only listens for beacons.
   *     Default is WF_ACTIVE_SCAN.
   * 
   * @param scanType Desired scan type.  Either WF_ACTIVE_SCAN or WF_PASSIVE_SCAN.
   */
  void wf_caSetScanType(uint8_t scanType);

  /**
   * /brief Commands the MRF24W to start a connection.
   * 
   * Directs the Connection Manager to scan for and connect to a WiFi network.
   *     This function does not wait until the connection attempt is successful, but 
   *     returns immediately.  See WF_ProcessEvent for events that can occur as a 
   *     result of a connection attempt being successful or not.
   * 
   *     Note that if the Connection Profile being used has WPA or WPA2 security
   *     enabled and is using a passphrase, the connection manager will first 
   *     calculate the PSK key, and then start the connection process.  The key 
   *     calculation can take up to 30 seconds.
   * 
   * @param CpId If this value is equal to an existing Connection Profile�s ID than 
   *             only that Connection Profile will be used to attempt a connection to 
   *             a WiFi network.  
   *             If this value is set to WF_CM_CONNECT_USING_LIST then the 
   *             connectionProfileList will be used to connect, starting with the 
   *             first Connection Profile in the list.
   */
  void wf_cmConnect(uint8_t CpId);

#ifdef __cplusplus
}
#endif

#endif /* MRF24W_G2100_H_ */

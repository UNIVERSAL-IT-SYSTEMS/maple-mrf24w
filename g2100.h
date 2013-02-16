
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

  extern void wf_processEvent(uint8_t event, uint16_t eventInfo, uint8_t* extraInfo);

  void wf_init();
  void wf_isr();

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

  void wf_connect();
  void wf_getMacAddress(uint8_t* buf);

#ifdef __cplusplus
}
#endif

#endif /* G2100_H_ */

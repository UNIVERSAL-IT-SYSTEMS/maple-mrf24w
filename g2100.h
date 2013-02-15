
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
  
#ifdef __cplusplus
}
#endif

#endif /* G2100_H_ */

//*****************************************************************************
//
// Copyright (C) 2014 Texas Instruments Incorporated - http://www.ti.com/ 
// 
// 
//  Redistribution and use in source and binary forms, with or without 
//  modification, are permitted provided that the following conditions 
//  are met:
//
//    Redistributions of source code must retain the above copyright 
//    notice, this list of conditions and the following disclaimer.
//
//    Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the 
//    documentation and/or other materials provided with the   
//    distribution.
//
//    Neither the name of Texas Instruments Incorporated nor the names of
//    its contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
//  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
//  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
//  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
//  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
//  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
//  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
//  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
//  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
//  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
//  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//*****************************************************************************



//*****************************************************************************
//
// Application Name     - TCP Socket
// Application Overview - This particular application illustrates how this
//                        device can be used as a client or server for TCP
//                        communication.
// Application Details  -
// http://processors.wiki.ti.com/index.php/CC32xx_TCP_Socket_Application
// or
// docs\examples\CC32xx_TCP_Socket_Application.pdf
//
//*****************************************************************************


//****************************************************************************
//
//! \addtogroup tcp_socket
//! @{
//
//****************************************************************************

// Standard includes
#include <stdlib.h>
#include <string.h>

// simplelink includes 
#include "simplelink.h"
#include "wlan.h"

// driverlib includes 
#include "hw_ints.h"
#include "hw_types.h"
#include "hw_memmap.h"
#include "hw_common_reg.h"
#include "rom.h"
#include "rom_map.h"
#include "interrupt.h"
#include "prcm.h"
#include "uart.h"
#include "utils.h"

#include "gpio.h"
#include "pin.h"

// common interface includes 
#include "udma_if.h"
#include "common.h"
#ifndef NOTERM
#include "uart_if.h"
#endif

#include "pinmux.h"

#include "led.h"
#include "systick_if.h"

#include "wifi_cfg.h"
#include "stdio.h"

#include "ucfg.h"
#include "at.h"
#include "zxbee-inf.h"
#include "sensor.h"

#define APPLICATION_NAME        "serial wifi"
#define APPLICATION_VERSION     "1.1.0"

#define DEBUG 0
#if DEBUG
#define DebugMsg        DBG_PRINT 
#else
#define DebugMsg(...)
#endif

#define  EXT_HEAD_LEN           18

// Application specific status/error codes
typedef enum{
    // Choosing -0x7D0 to avoid overlap w/ host-driver's error codes
    UCP_CLIENT_FAILED  = -0x7D0,
    UCP_SERVER_FAILED  = UCP_CLIENT_FAILED  - 1,
    DEVICE_NOT_IN_STATION_MODE = UCP_SERVER_FAILED - 1,

    STATUS_CODE_MAX = -0xBB8
}e_AppStatusCodes;

/*
 * iSockID：连接到zhiyun网关的套接字
 */
static int  iSockID = -1;
//接收缓存
static char SockBuf[1400];
//发送缓存
static char msgTx[1400];

//****************************************************************************
//                      LOCAL FUNCTION PROTOTYPES
//****************************************************************************
void sensorLinkOn(void);

int BsdTcpServer(unsigned short usPort);
static long WlanConnect();
static void DisplayBanner();
static void BoardInit();
static void InitializeAppVariables();
static long ConfigureSimpleLinkToDefaultState();

//*****************************************************************************
//                 GLOBAL VARIABLES -- Start
//*****************************************************************************
unsigned long  g_ulStatus = 0;//SimpleLink Status
unsigned long  g_ulGatewayIP = 0; //Network Gateway IP address
unsigned char  g_ucConnectionSSID[SSID_LEN_MAX+1]; //Connection SSID
unsigned char  g_ucConnectionBSSID[BSSID_LEN_MAX]; //Connection BSSID

//unsigned long  g_ulDestinationIp = IP_ADDR;
//unsigned int   g_uiPortNum = PORT_NUM;

unsigned char  g_ucConnectionStatus = 0;
unsigned char  g_ucSimplelinkstarted = 0;
unsigned long  g_ulIpAddr = 0;

#if defined(ccs) || defined (gcc)
extern void (* const g_pfnVectors[])(void);
#endif
#if defined(ewarm)
extern uVectorEntry __vector_table;
#endif

static unsigned long led2_tm = 0;//led2开启时间

//*****************************************************************************
//                 GLOBAL VARIABLES -- End
//*****************************************************************************


//*****************************************************************************
// SimpleLink Asynchronous Event Handlers -- Start
//*****************************************************************************


//*****************************************************************************
//
//! \brief The Function Handles WLAN Events
//!
//! \param[in]  pWlanEvent - Pointer to WLAN Event Info
//!
//! \return None
//!
//*****************************************************************************
void SimpleLinkWlanEventHandler(SlWlanEvent_t *pWlanEvent)
{
    if(!pWlanEvent)
    {
        return;
    }

    switch(pWlanEvent->Event)
    {
        case SL_WLAN_CONNECT_EVENT:
        {
            SET_STATUS_BIT(g_ulStatus, STATUS_BIT_CONNECTION);
            LEDOn(1);
            //
            // Information about the connected AP (like name, MAC etc) will be
            // available in 'slWlanConnectAsyncResponse_t'-Applications
            // can use it if required
            //
            //  slWlanConnectAsyncResponse_t *pEventData = NULL;
            // pEventData = &pWlanEvent->EventData.STAandP2PModeWlanConnected;
            //

            // Copy new connection SSID and BSSID to global parameters
            memcpy(g_ucConnectionSSID,pWlanEvent->EventData.
                   STAandP2PModeWlanConnected.ssid_name,
                   pWlanEvent->EventData.STAandP2PModeWlanConnected.ssid_len);
            memcpy(g_ucConnectionBSSID,
                   pWlanEvent->EventData.STAandP2PModeWlanConnected.bssid,
                   SL_BSSID_LENGTH);
        }
        break;

        case SL_WLAN_DISCONNECT_EVENT:
        {
            LEDOff(1);
            slWlanConnectAsyncResponse_t*  pEventData = NULL;

            CLR_STATUS_BIT(g_ulStatus, STATUS_BIT_CONNECTION);
            CLR_STATUS_BIT(g_ulStatus, STATUS_BIT_IP_AQUIRED);

            pEventData = &pWlanEvent->EventData.STAandP2PModeDisconnected;

            // If the user has initiated 'Disconnect' request,
            //'reason_code' is SL_USER_INITIATED_DISCONNECTION
            if(SL_USER_INITIATED_DISCONNECTION == pEventData->reason_code)
            {
                DebugMsg("[WLAN EVENT]Device disconnected from the AP: %s,"
                "BSSID: %x:%x:%x:%x:%x:%x on application's request \n\r",
                           g_ucConnectionSSID,g_ucConnectionBSSID[0],
                           g_ucConnectionBSSID[1],g_ucConnectionBSSID[2],
                           g_ucConnectionBSSID[3],g_ucConnectionBSSID[4],
                           g_ucConnectionBSSID[5]);
            }
            else
            {
                DebugMsg("[WLAN ERROR]Device disconnected from the AP AP: %s,"
                            "BSSID: %x:%x:%x:%x:%x:%x on an ERROR..!! \n\r",
                           g_ucConnectionSSID,g_ucConnectionBSSID[0],
                           g_ucConnectionBSSID[1],g_ucConnectionBSSID[2],
                           g_ucConnectionBSSID[3],g_ucConnectionBSSID[4],
                           g_ucConnectionBSSID[5]);
            }
            memset(g_ucConnectionSSID,0,sizeof(g_ucConnectionSSID));
            memset(g_ucConnectionBSSID,0,sizeof(g_ucConnectionBSSID));
            
        }
        break;

        default:
        {
            DebugMsg("[WLAN EVENT] Unexpected event [0x%x]\n\r",
                       pWlanEvent->Event);
        }
        break;
    }
}
//*****************************************************************************
//
//! \brief This function handles network events such as IP acquisition, IP
//!           leased, IP released etc.
//!
//! \param[in]  pNetAppEvent - Pointer to NetApp Event Info
//!
//! \return None
//!
//*****************************************************************************
void SimpleLinkNetAppEventHandler(SlNetAppEvent_t *pNetAppEvent)
{
    if(!pNetAppEvent)
    {
        return;
    }

    switch(pNetAppEvent->Event)
    {
        case SL_NETAPP_IPV4_IPACQUIRED_EVENT:
        {
            SlIpV4AcquiredAsync_t *pEventData = NULL;

            SET_STATUS_BIT(g_ulStatus, STATUS_BIT_IP_AQUIRED);

            //Ip Acquired Event Data
            pEventData = &pNetAppEvent->EventData.ipAcquiredV4;
            g_ulIpAddr = pEventData->ip;

            //Gateway IP address
            g_ulGatewayIP = pEventData->gateway;

            DebugMsg("[NETAPP EVENT] IP Acquired: IP=%d.%d.%d.%d , "
                        "Gateway=%d.%d.%d.%d\n\r",
                            SL_IPV4_BYTE(g_ulIpAddr,3),
                            SL_IPV4_BYTE(g_ulIpAddr,2),
                            SL_IPV4_BYTE(g_ulIpAddr,1),
                            SL_IPV4_BYTE(g_ulIpAddr,0),
                            SL_IPV4_BYTE(g_ulGatewayIP,3),
                            SL_IPV4_BYTE(g_ulGatewayIP,2),
                            SL_IPV4_BYTE(g_ulGatewayIP,1),
                            SL_IPV4_BYTE(g_ulGatewayIP,0));
        }
        break;

        default:
        {
            DebugMsg("[NETAPP EVENT] Unexpected event [0x%x] \n\r",
                       pNetAppEvent->Event);
        }
        break;
    }
}
//*****************************************************************************
//
//! \brief This function handles HTTP server events
//!
//! \param[in]  pServerEvent - Contains the relevant event information
//! \param[in]    pServerResponse - Should be filled by the user with the
//!                                      relevant response information
//!
//! \return None
//!
//****************************************************************************
void SimpleLinkHttpServerCallback(SlHttpServerEvent_t *pHttpEvent,
                                  SlHttpServerResponse_t *pHttpResponse)
{
    // Unused in this application
}
//*****************************************************************************
//
//! \brief This function handles General Events
//!
//! \param[in]     pDevEvent - Pointer to General Event Info
//!
//! \return None
//!
//*****************************************************************************
void SimpleLinkGeneralEventHandler(SlDeviceEvent_t *pDevEvent)
{
    if(!pDevEvent)
    {
        return;
    }

    //
    // Most of the general errors are not FATAL are are to be handled
    // appropriately by the application
    //
    DebugMsg("[GENERAL EVENT] - ID=[%d] Sender=[%d]\n\n",
               pDevEvent->EventData.deviceEvent.status,
               pDevEvent->EventData.deviceEvent.sender);
}
//*****************************************************************************
//
//! This function handles socket events indication
//!
//! \param[in]      pSock - Pointer to Socket Event Info
//!
//! \return None
//!
//*****************************************************************************
void SimpleLinkSockEventHandler(SlSockEvent_t *pSock)
{
    if(!pSock)
    {
        return;
    }

    //
    // This application doesn't work w/ socket - Events are not expected
    //
    switch( pSock->Event )
    {
        case SL_SOCKET_TX_FAILED_EVENT:
            switch( pSock->EventData.status )
            {
                case SL_ECLOSE:
                    DebugMsg("[SOCK ERROR] - close socket (%d) operation "
                    "failed to transmit all queued packets\n\n",
                           pSock->EventData.sd);
                    break;
                default:
                    DebugMsg("[SOCK ERROR] - TX FAILED : socket %d , reason"
                        "(%d) \n\n",
                           pSock->EventData.sd, pSock->EventData.status);
            }
            break;

        default:
            DebugMsg("[SOCK EVENT] - Unexpected Event [%x0x]\n\n",pSock->Event);
    }
}
//*****************************************************************************
// SimpleLink Asynchronous Event Handlers -- End
//*****************************************************************************



//*****************************************************************************
//
//! This function initializes the application variables
//!
//! \param[in]    None
//!
//! \return None
//!
//*****************************************************************************
static void InitializeAppVariables()
{
    g_ulStatus = 0;
    g_ulGatewayIP = 0;
    memset(g_ucConnectionSSID,0,sizeof(g_ucConnectionSSID));
    memset(g_ucConnectionBSSID,0,sizeof(g_ucConnectionBSSID));
    //g_ulDestinationIp = IP_ADDR;
    //g_uiPortNum = PORT_NUM;
   // g_ulPacketCount = TCP_PACKET_COUNT;
}
//*****************************************************************************
//! \brief This function puts the device in its default state. It:
//!           - Set the mode to STATION
//!           - Configures connection policy to Auto and AutoSmartConfig
//!           - Deletes all the stored profiles
//!           - Enables DHCP
//!           - Disables Scan policy
//!           - Sets Tx power to maximum
//!           - Sets power policy to normal
//!           - Unregister mDNS services
//!           - Remove all filters
//!
//! \param   none
//! \return  On success, zero is returned. On error, negative is returned
//*****************************************************************************
static long ConfigureSimpleLinkToDefaultState()
{
    SlVersionFull   ver = {0};
    _WlanRxFilterOperationCommandBuff_t  RxFilterIdMask = {0};

    unsigned char ucVal = 1;
    unsigned char ucConfigOpt = 0;
    unsigned char ucConfigLen = 0;
    unsigned char ucPower = 0;

    long lRetVal = -1;
    long lMode = -1;

    lMode = sl_Start(0, 0, 0);
    ASSERT_ON_ERROR(lMode);

    // If the device is not in station-mode, try configuring it in station-mode 
    if (ROLE_STA != lMode)
    {
        if (ROLE_AP == lMode)
        {
            // If the device is in AP mode, we need to wait for this event 
            // before doing anything 
            while(!IS_IP_ACQUIRED(g_ulStatus))
            {
#ifndef SL_PLATFORM_MULTI_THREADED
              _SlNonOsMainLoopTask(); 
#endif
            }
        }

        // Switch to STA role and restart 
        lRetVal = sl_WlanSetMode(ROLE_STA);
        ASSERT_ON_ERROR(lRetVal);

        lRetVal = sl_Stop(0xFF);
        ASSERT_ON_ERROR(lRetVal);

        lRetVal = sl_Start(0, 0, 0);
        ASSERT_ON_ERROR(lRetVal);

        // Check if the device is in station again 
        if (ROLE_STA != lRetVal)
        {
            // We don't want to proceed if the device is not coming up in STA-mode 
            return DEVICE_NOT_IN_STATION_MODE;
        }
    }
    
    // Get the device's version-information
    ucConfigOpt = SL_DEVICE_GENERAL_VERSION;
    ucConfigLen = sizeof(ver);
    lRetVal = sl_DevGet(SL_DEVICE_GENERAL_CONFIGURATION, &ucConfigOpt, 
                                &ucConfigLen, (unsigned char *)(&ver));
    ASSERT_ON_ERROR(lRetVal);
  
    DebugMsg("Host Driver Version: %s\n\r",SL_DRIVER_VERSION);
    DebugMsg("Build Version %d.%d.%d.%d.31.%d.%d.%d.%d.%d.%d.%d.%d\n\r",

    ver.NwpVersion[0],ver.NwpVersion[1],ver.NwpVersion[2],ver.NwpVersion[3],
    ver.ChipFwAndPhyVersion.FwVersion[0],ver.ChipFwAndPhyVersion.FwVersion[1],
    ver.ChipFwAndPhyVersion.FwVersion[2],ver.ChipFwAndPhyVersion.FwVersion[3],
    ver.ChipFwAndPhyVersion.PhyVersion[0],ver.ChipFwAndPhyVersion.PhyVersion[1],
    ver.ChipFwAndPhyVersion.PhyVersion[2],ver.ChipFwAndPhyVersion.PhyVersion[3]);
 
    // Set connection policy to Auto + SmartConfig 
    //      (Device's default connection policy)
    lRetVal = sl_WlanPolicySet(SL_POLICY_CONNECTION, 
                                SL_CONNECTION_POLICY(1, 0, 0, 0, 1), NULL, 0);
    ASSERT_ON_ERROR(lRetVal);

    // Remove all profiles
    lRetVal = sl_WlanProfileDel(0xFF);
    ASSERT_ON_ERROR(lRetVal);

    

    //
    // Device in station-mode. Disconnect previous connection if any
    // The function returns 0 if 'Disconnected done', negative number if already
    // disconnected Wait for 'disconnection' event if 0 is returned, Ignore 
    // other return-codes
    //
    lRetVal = sl_WlanDisconnect();
    if(0 == lRetVal)
    {
        // Wait
        while(IS_CONNECTED(g_ulStatus))
        {
#ifndef SL_PLATFORM_MULTI_THREADED
              _SlNonOsMainLoopTask(); 
#endif
        }
    }

    // Enable DHCP client
    lRetVal = sl_NetCfgSet(SL_IPV4_STA_P2P_CL_DHCP_ENABLE,1,1,&ucVal);
    ASSERT_ON_ERROR(lRetVal);

    // Disable scan
    ucConfigOpt = SL_SCAN_POLICY(0);
    lRetVal = sl_WlanPolicySet(SL_POLICY_SCAN , ucConfigOpt, NULL, 0);
    ASSERT_ON_ERROR(lRetVal);

    // Set Tx power level for station mode
    // Number between 0-15, as dB offset from max power - 0 will set max power
    ucPower = 0;
    lRetVal = sl_WlanSet(SL_WLAN_CFG_GENERAL_PARAM_ID, 
            WLAN_GENERAL_PARAM_OPT_STA_TX_POWER, 1, (unsigned char *)&ucPower);
    ASSERT_ON_ERROR(lRetVal);

    // Set PM policy to normal
    lRetVal = sl_WlanPolicySet(SL_POLICY_PM , SL_NORMAL_POLICY, NULL, 0);
    ASSERT_ON_ERROR(lRetVal);

    // Unregister mDNS services
    lRetVal = sl_NetAppMDNSUnRegisterService(0, 0);
    ASSERT_ON_ERROR(lRetVal);

    // Remove  all 64 filters (8*8)
    memset(RxFilterIdMask.FilterIdMask, 0xFF, 8);
    lRetVal = sl_WlanRxFilterSet(SL_REMOVE_RX_FILTER, (_u8 *)&RxFilterIdMask,
                       sizeof(_WlanRxFilterOperationCommandBuff_t));
    ASSERT_ON_ERROR(lRetVal);

    lRetVal = sl_Stop(SL_STOP_TIMEOUT);
    ASSERT_ON_ERROR(lRetVal);

    InitializeAppVariables();
    
    return lRetVal; // Success
}
//****************************************************************************
//
//!  \brief Connecting to a WLAN Accesspoint
//!
//!   This function connects to the required AP (SSID_NAME) with Security
//!   parameters specified in te form of macros at the top of this file
//!
//!   \param[in]              None
//!
//!   \return     Status value
//!
//!   \warning    If the WLAN connection fails or we don't aquire an IP
//!            address, It will be stuck in this function forever.
//
//****************************************************************************
static long WlanConnect()
{
    DebugMsg("Connecting to AP......\n\r");
 
    SlSecParams_t secParams = {0};
    long lRetVal = 0;

    secParams.Key = (signed char*)&cfg.skey[0];
    secParams.KeyLen = strlen(cfg.skey);
    secParams.Type = cfg.stype;

    lRetVal = sl_WlanConnect((signed char*)&cfg.ssid[0], strlen(cfg.ssid), 0, &secParams, 0);
    ASSERT_ON_ERROR(lRetVal);

    return SUCCESS;

}
/*********************************************************************************************
* 名称：SocketInit
* 功能：初始化与远程服务器通信的套接字
* 参数：
* 返回：
* 修改：
* 注释：
*********************************************************************************************/
int SocketInit(void)
{
    SlSockAddrIn_t  LocalAddr;
    //int             iSockID;
    int             iStatus;
   
    DebugMsg(" open socket ...\r\n");
    // creating a UDP socket
    iSockID = sl_Socket(SL_AF_INET,SL_SOCK_DGRAM, 0);
    if( iSockID < 0 )
    {
        // error
        ASSERT_ON_ERROR(UCP_CLIENT_FAILED);
    }

    LocalAddr.sin_family = SL_AF_INET;
    LocalAddr.sin_port = sl_Htons(LO_PORT/*cfg.lport*/);
    LocalAddr.sin_addr.s_addr = 0;              //填充本地地址

    iStatus = sl_Bind(iSockID, (SlSockAddr_t *)&LocalAddr,  sizeof(SlSockAddrIn_t));  //将地址绑定到刚刚创建的UDP socket
    if( iStatus < 0 )                           //如果绑定失败           
    {
        // error
       sl_Close(iSockID);
       ASSERT_ON_ERROR(UCP_CLIENT_FAILED);
    }
    
    SlSockNonblocking_t enableOption;
    enableOption.NonblockingEnabled = 1;
    
    iStatus = sl_SetSockOpt(iSockID,SL_SOL_SOCKET,SL_SO_NONBLOCKING, (_u8 *)&enableOption,sizeof(enableOption));//设置socket选项
    if( iStatus < 0 )       //如果设置失败
    {
       // error
       sl_Close(iSockID);
       ASSERT_ON_ERROR(UCP_CLIENT_FAILED);
    } 
    
#ifndef CC3200_Serial
    sensorLinkOn();
#else
    AT_ReportedLinkStatus();
#endif
    return 0;
}
/*********************************************************************************************
* 名称：SocketCheck
* 功能：检查与服务器连接的套接字是否有数据需要接收。
* 参数：
* 返回：
* 修改：
* 注释：
*********************************************************************************************/
int SocketCheck(void)
{
   
   _i16 AddrSize = sizeof(SlSockAddrIn_t);
   SlSockAddrIn_t Addr;
      
    int   iStatus = sl_RecvFrom(iSockID, SockBuf, sizeof SockBuf, 0, (SlSockAddr_t *)&Addr, (SlSocklen_t*)&AddrSize);  //接收UDP数据
      if (iStatus > 0) {   //接收到UDP数据
        /*
        iStatus = sl_SendTo(iSockID, Buf, iStatus, 0,
                                (SlSockAddr_t *)&sAddr, iAddrSize);*/
        DebugMsg("socket recv datat....\r\n");
        
        return iStatus;
      } 
      else if (iStatus == 0 || iStatus == SL_EAGAIN) {  
        //如果没有收到UDP数据
      } else {        //发生错误
        DebugMsg("socket error ...close\r\n");
          sl_Close(iSockID);
          ASSERT_ON_ERROR(iStatus);
          iSockID = -1;
      } 
      return 0;
}
/*********************************************************************************************
* 名称：SocketSendMessage
* 功能：发送数据到远程服务器
* 参数：
* 返回：
* 修改：
* 注释：
*********************************************************************************************/
int SocketSendMessage(char *buf, int len)
{
  SlSockAddrIn_t  sAddr;
  int  iAddrSize;
  int ret;
  
  if (iSockID < 0) return -1;
  LEDOn(2);
  //filling the UDP server socket address
#if DEBUG
  DebugMsg("<<< ");
  for (int i=0; i<len; i++) {
    DebugMsg("%02X ", buf[i]);
  }
  DebugMsg("\r\n");
#endif
    
  sAddr.sin_family = SL_AF_INET;
  sAddr.sin_port = sl_Htons((unsigned short)cfg.sport);
  sAddr.sin_addr.s_addr = sl_Htonl(cfg.sip[0]<<24 | cfg.sip[1]<<16 | cfg.sip[2]<<8 | cfg.sip[3]);

  iAddrSize = sizeof(SlSockAddrIn_t);
  
  ret = sl_SendTo(iSockID, buf, len, 0,
                 (SlSockAddr_t *)&sAddr, iAddrSize);
  //UTUtilsDelay( 30000);                                      //延时30ms
  LEDOff(2);
  return ret;
}
/*********************************************************************************************
* 名称：sendMessage
* 功能：发送数据
* 参数：
* 返回：
* 修改：
* 注释：
*********************************************************************************************/
int sendMessage(char *buf, int len)
{
  led2_tm = UTUtilsGetSysTime();
  LEDOn(2);
  for (int i=0; i<len; i++) {
    msgTx[EXT_HEAD_LEN+i] = buf[i];
  }
  SocketSendMessage(msgTx, len+EXT_HEAD_LEN);
  return len;
}
/*********************************************************************************************
* 名称：SocketRecvMessage
* 功能：socket接收到数据调用
* 参数：
* 返回：
* 修改：
* 注释：
*********************************************************************************************/
int SocketRecvMessage(char *buf, int len)
{
  led2_tm = UTUtilsGetSysTime();
  LEDOn(2);
              
  if (len < EXT_HEAD_LEN+2) return 0;
  
  buf = &buf[EXT_HEAD_LEN]; 
  len -= EXT_HEAD_LEN;
  
  at_notify_data(buf, len);
  ZXBeeInfRecv(buf, len);
  return 1;
}
/*********************************************************************************************
* 名称：macInit
* 功能：初始化mac地址，获取发送节点信息
* 参数：
* 返回：
* 修改：
* 注释：
*********************************************************************************************/
void macInit(void)
{
   char macAddressVal[SL_MAC_ADDR_LEN];
  _u8 macAddressLen = SL_MAC_ADDR_LEN;
  
  sl_NetCfgGet(SL_MAC_ADDRESS_GET,NULL,&macAddressLen,(_u8 *)macAddressVal);
  
  sprintf(msgTx, "%02X:%02X:%02X:%02X:%02X:%02X=",macAddressVal[0],
          macAddressVal[1],macAddressVal[2],macAddressVal[3],
          macAddressVal[4],macAddressVal[5]);
}
/*********************************************************************************************
* 名称：getOurIpAddr
* 功能：获取本机ip地址
* 参数：
* 返回：ip地址
* 修改：
* 注释：
*********************************************************************************************/
char * getOurIpAddr(void)
{
  static char p[20];
  sprintf(p, "%u.%u.%u.%u", SL_IPV4_BYTE(g_ulIpAddr,3),SL_IPV4_BYTE(g_ulIpAddr,2),
          SL_IPV4_BYTE(g_ulIpAddr,1),SL_IPV4_BYTE(g_ulIpAddr,0));
  return p;
}
//*****************************************************************************
//
//! Application startup display on UART
//!
//! \param  none
//!
//! \return none
//!
//*****************************************************************************
static void DisplayBanner(char * AppName)
{
    DebugMsg("\n\n\n\r");
    DebugMsg("\t\t *************************************************\n\r");
    DebugMsg("\t\t      CC3200 %s Application       \n\r", AppName);
    DebugMsg("\t\t *************************************************\n\r");
    DebugMsg("\n\n\n\r");
}
//*****************************************************************************
//
//! Board Initialization & Configuration
//!
//! \param  None
//!
//! \return None
//
//*****************************************************************************
static void
BoardInit(void)
{
/* In case of TI-RTOS vector table is initialize by OS itself */
#ifndef USE_TIRTOS
  //
  // Set vector table base
  //
#if defined(ccs) || defined (gcc)
    MAP_IntVTableBaseSet((unsigned long)&g_pfnVectors[0]);
#endif
#if defined(ewarm)
    MAP_IntVTableBaseSet((unsigned long)&__vector_table);
#endif
#endif
    //
    // Enable Processor
    //
    MAP_IntMasterEnable();
    MAP_IntEnable(FAULT_SYSTICK);

    PRCMCC3200MCUInit();
}
//****************************************************************************
//                            MAIN FUNCTION
//****************************************************************************
void main()
{
    long lRetVal = -1;

    //
    // Board Initialization
    //
    BoardInit();    //板载初始化

    SysTickInit();  //初始化systick定时器
    //
    // Initialize the uDMA
    //
    UDMAInit();     //初始化DMA控制

    //
    // Configure the pinmux settings for the peripherals exercised
    //
    PinMuxConfig();  //引脚复用配置

    LEDInit();       //LED初始化
    //
    // Configuring UART
    //
    InitTerm();      //配置串口
   
    DisplayBanner(APPLICATION_NAME);    //打印应用名称
   
   
    InitializeAppVariables();           //初始化应用
     
 
    // Following function configure the device to default state by cleaning
    // the persistent settings stored in NVMEM (viz. connection profiles &
    // policies, power policy etc)
    //
    // Applications may choose to skip this step if the developer is sure
    // that the device is in its default state at start of applicaton
    //
    // Note that all profiles and persistent settings that were done on the
    // device will be lost
    //
    lRetVal = ConfigureSimpleLinkToDefaultState(); //配置simpleLink为默认状态(station)

    if(lRetVal < 0)
    {
      if (DEVICE_NOT_IN_STATION_MODE == lRetVal)   
          DebugMsg("Failed to configure the device in its default state \n\r");
      LEDOn(3);
      LOOP_FOREVER();     //无限循环
    }  
    
    

    // Asumption is that the device is configured in station mode already
    // and it is in its default state
    //
    lRetVal = sl_Start(0, 0, 0);  //启动simplelink设备
    if (lRetVal < 0)              //如果启动失败
    {
        LEDOn(3);
        DebugMsg("Failed to start the device \n\r");
        LOOP_FOREVER();          //无线循环
    }
    

    ucfg_read();
    DebugMsg("cfg ssid: %s\r\n", cfg.ssid);
    DebugMsg("cfg skey: %s\r\n", cfg.skey);
    DebugMsg("cfg stype: %d\r\n", cfg.stype);
    DebugMsg("cfg sip: %u.%u.%u.%u\r\n", cfg.sip[0],cfg.sip[1],cfg.sip[2],cfg.sip[3]);
    DebugMsg("cfg sport: %u\r\n", cfg.sport);
    //DebugMsg("cfg lport: %u\r\n", cfg.lport);
    
    int mode = 1; //0 trans, 1 at mode
    int at_ret = 0;
  
    macInit();
    
    ATCommandInit();
    
    ZXBeeInfInit();
#ifndef CC3200_Serial
    sensorInit();
#endif
    
    while (1) { 
      if (led2_tm != 0) {
        
        if ((((long)UTUtilsGetSysTime() - (long)led2_tm) > 50)) {
          led2_tm = 0;
          LEDOff(2);
        }
      }
      
      if ((IS_CONNECTED(g_ulStatus)) && (IS_IP_ACQUIRED(g_ulStatus))) {
          LEDOn(1);
          
          //DebugMsg("check at ret %d\r\n",at_ret);
          
          if (iSockID < 0) {
            SocketInit();
       
          } else if (at_ret <= 0){
            int ret = SocketCheck();
            if (ret > 0) {
              
              SocketRecvMessage(SockBuf, ret);
            }
          }
      } else {
         static char s = 0;
         static unsigned long led_tm = 0, conn_tm = 0;
         unsigned long t = UTUtilsGetSysTime();
         if (t - led_tm > 200) {
           if (s) LEDOff(1);
           else LEDOn(1);
           s = !s;
           led_tm = t;
         }
         if (conn_tm == 0 || t - conn_tm > 60*1000) {
           InitializeAppVariables();
           if (strlen(cfg.ssid) > 0) {
             WlanConnect();    //连接到WiFi设备
           }
           conn_tm = t;
         }
      }
      
      if (mode == 1) {
         at_ret = at_check();
      }
      
#ifndef CC3200_Serial
        sensorLoop();
#endif
      
      _SlNonOsMainLoopTask();
    }
}
//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************

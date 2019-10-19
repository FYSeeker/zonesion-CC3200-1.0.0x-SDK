// Simplelink includes
#include "simplelink.h"
#include "ucfg.h"
#include "common.h"
#include "wifi_cfg.h"

#define VERSION         1

#define USER_FILE_NAME "serial_wifi.cfg"

#define DEBUG   0
#if DEBUG
#define Debug(...)   Report(__VA_ARGS__)
#else
#define Debug(...)   
#endif
cfg_t cfg;

static void set_default(void)
{
         memset(&cfg, 0, sizeof cfg);
        cfg.ver = VERSION;
        cfg.sip[3] = GW_IP&0xff;
        cfg.sip[2] = (GW_IP >> 8)&0xff;
        cfg.sip[1] = (GW_IP >> 16)&0xff;
        cfg.sip[0] = (GW_IP >> 24)&0xff;
        strcpy(cfg.ssid, Z_SSID_NAME);
        strcpy(cfg.skey, Z_SECURITY_KEY);
        cfg.stype = Z_SECURITY_TYPE;
        cfg.lport = LO_PORT;
        cfg.sport = GW_PORT;
        ucfg_save();
}
/*
 * 函数名：ucfg_init
 * 功能： 用户配置文件初始化
 */
void ucfg_init(void)
{
  unsigned long ulToken;
  long lFileHandle;
  
    long lRetVal = -1;
    //
    //  create a user file
    //
    lRetVal = sl_FsOpen((unsigned char *)USER_FILE_NAME,
                FS_MODE_OPEN_CREATE(65536, \
                          _FS_FILE_OPEN_FLAG_COMMIT|_FS_FILE_PUBLIC_WRITE),
                        &ulToken,
                        &lFileHandle);
    if(lRetVal < 0)
    {
        //
        // File may already be created
        //
        lRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
        //ASSERT_ON_ERROR(lRetVal);
        Debug("create file error\r\n");
    }
    else
    {
      Debug("create file...\r\n");
        
        //
        // close the user file
        //
        lRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
        if (SL_RET_CODE_OK != lRetVal)
        {
            //ASSERT_ON_ERROR(lRetVal);
           Debug(" close file error\r\n");
        }
        
    }    
}

void ucfg_save(void)
{
  unsigned long ulToken;
  long lFileHandle;
  
    //
    //  open a user file for writing
    //
   int lRetVal = sl_FsOpen((unsigned char *)USER_FILE_NAME,
                        FS_MODE_OPEN_WRITE, 
                        &ulToken,
                        &lFileHandle);
    if(lRetVal < 0)
    {
        lRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
        //ASSERT_ON_ERROR(FILE_OPEN_WRITE_FAILED);
        Debug("error: open file\r\n");
    }
    
    //
    // write "Old MacDonalds" child song as many times to get just below a 64KB file
    //
    
     lRetVal = sl_FsWrite(lFileHandle,
                    0/*offset*/, (unsigned char *)&cfg, sizeof(cfg));
       
     if (lRetVal < 0)
     {
            lRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
           // ASSERT_ON_ERROR(FILE_WRITE_FAILED);
            Debug("error: write file\r\n");
     }
    
    //
    // close the user file
    //
    lRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
    if (SL_RET_CODE_OK != lRetVal)
    {
        //ASSERT_ON_ERROR(FILE_CLOSE_ERROR);
      Debug("error:close file\r\n");
    }
  
}

void ucfg_read(void)
{
    unsigned long ulToken;
  long lFileHandle;
    long lRetVal = -1;
   
    //
    // open a user file for reading
    //
    lRetVal = sl_FsOpen((unsigned char *)USER_FILE_NAME,
                        FS_MODE_OPEN_READ,
                        &ulToken,
                        &lFileHandle);
    if(lRetVal < 0)
    {
        lRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
        //ASSERT_ON_ERROR(FILE_OPEN_READ_FAILED);
        Debug("error:open file in read\r\n");
        ucfg_init();
        //set_default();
        return;
    }

    //
    // read the data and compare with the stored buffer
    //
    
        lRetVal = sl_FsRead(lFileHandle,
                   0/*offset*/, (unsigned char*)&cfg, sizeof(cfg));
        if ((lRetVal < 0) || (lRetVal != sizeof(cfg)))
        {
            lRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
            //ASSERT_ON_ERROR(FILE_READ_FAILED);
            Debug("error: read file %d\r\n", lRetVal);
            //set_default();
        }

    //
    // close the user file
    //
    lRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
    if (SL_RET_CODE_OK != lRetVal)
    {
        //ASSERT_ON_ERROR(FILE_CLOSE_ERROR);
      Debug("error:close file in read\r\n");
    }
    if (cfg.ver != VERSION) {
      set_default();
    }
}
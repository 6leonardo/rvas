#if !defined(_COMPILE_SETTINGS_H_)
#define _COMPILE_SETTINGS_H_

#define VERSION "Scale-1.0.0"

//#define PRODUCTION
#define PRODUCTION_BOARD
#define IDE_OTA
//#define USE_NTP
//#define HOST_AND_MAC
#define AP_ON_STA_FAIL false
#define STA_RETRY 3



#if !defined(PRODUCTION)
#define USE_SERIAL
#define DEBUG_INIT()  Serial.begin(74880)
#define DEBUGF(args...) Serial.printf(args)
#define DEBUG(x) Serial.print(x)
#define DEBUGLN(x) Serial.println(x)
#else
#define DEBUG_INIT() 
#define DEBUGF(args...) 
#define DEBUG(x) 
#define DEBUGLN(x) 
#endif

#if defined(PRODUCTION_BOARD)
#define PROD_VS_DEMO(prod, demo) (prod)
#else
#define PROD_VS_DEMO(prod, demo) (demo)
#endif

//raspberry wifi
#define WIFI_SID "fabriziobox"
#define WIFI_PWD "laLunaeilSole"

//config wifi
#define AP_SID "sensor"
#define AP_PWD "XxSensor.8"
//local server
#define SERVER "rv.local"
#define PORT 80

struct eepromInfoType0
{
    char AP_SID[32]
	char AP_PWD[32];		
	char STA_SID[32]; 
	char STA_PWD[32]; 
    char SERVER[32];
    int port;
    // DEEP_SLEEP Timeout interval in seconds
    int sleepInterval;
    // DEEP_SLEEP Timeout interval when connecting to AP fails
    int failConnectRetryInterval;
    long utcOffsetInSeconds;
    //
    int initialized_1;
    int initialized_2;
    float scale_1;
    float scale_2;
    float offset_1;
    float offset_2;
    float gas_tare_1;
    float gas_tare_2;
};

extern eepromInfoType0 configuration;
extern void WatchIT(int ms);
#endif

#if !defined(_COMPILE_SETTINGS_H_)
#define _COMPILE_SETTINGS_H_
#include <PubSubClient.h>

#include "pin.h"
#define VERSION "Scale-1.0.0"

//#define PRODUCTION
#define PRODUCTION_BOARD
//#define IDE_OTA
//#define USE_NTP
//#define HOST_AND_MAC
//#define WATCH_DOG 5000

#define AP_ON_STA_FAIL false
#define STA_RETRY 3

#if !defined(PRODUCTION)
#define USE_SERIAL
#define DEBUG_INIT()  Serial.begin(74880)
#define DEBUGF(args...) Serial.printf(args)
#define DEBUG(x) Serial.print(x)
#define DEBUG_LN(x) Serial.println(x)
#else
#define DEBUG_INIT() 
#define DEBUGF(args...) 
#define DEBUG(x) 
#define DEBUG_LN(x) 
#endif

#if defined(PRODUCTION_BOARD)
#define PROD_VS_DEMO(prod, demo) (prod)
#else
#define PROD_VS_DEMO(prod, demo) (demo)
#endif

//raspberry wifi
#define DEFAULT_WIFI_SID "fabriziobox"
#define DEFAULT_WIFI_PWD "xxxxxxxxxxx"

//config wifi
#define DEFAULT_AP_SID "sensor"
#define DEFAULT_AP_PWD "XxSensor.8"

#define MQTT_SERVER "rv.local"
#define MQTT_PORT 1883
#define MQTT_USER "pi"
#define MQTT_PWD "demodemo"
#define MQTT_TIMEOUT 10*1000
#define MQTT_TOPIC_LEVEL_1 "gas/1/level"
#define MQTT_TOPIC_LEVEL_2 "gas/2/level"
/*
#define MQTT_WEIGHT_1 "gas/weight/1"
#define MQTT_WEIGHT_2 "gas/weight/2"
#define MQTT_TARE_1 "gas/tare/1"
#define MQTT_TARE_2 "gas/tare/2"
*/
#define MQTT_TOPIC_WAKEUP "gas/wakeup"
#define MQTT_TOPIC_URL_OPEN "slave/url/open"
#define MQTT_TOPIC_URL_CLOSE "slave/url/close'"
#define MQTT_TOPIC_SLAVE_NAME "gas"



struct eepromInfoType0
{
    char AP_SID[32];
	char AP_PWD[32];		
	char STA_SID[32]; 
	char STA_PWD[32]; 
    char mqtt_server[32];
    char mqtt_user[32];
    char mqtt_pwd[32];
    int mqtt_port;
    int mqtt_timeout;
    // DEEP_SLEEP Timeout interval in seconds
    int sleepInterval;
    // DEEP_SLEEP Timeout interval when connecting to AP fails
    int failConnectRetryInterval;
    long utcOffsetInSeconds;
    int wakeupInterval;
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

extern eepromInfoType0 eeprom;
extern void WatchIT(int ms);
#endif

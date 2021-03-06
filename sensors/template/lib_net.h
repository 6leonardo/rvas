#if !defined(_LIB_NET_H_)
#define _LIB_NET_H_
#include "compile_settings.h"

#include <WiFiClient.h>
#include <ESP8266WiFi.h>
#include <NTPClient.h>


//return of setup(), and also getStatus()
#define WIFI_HOTSPOT 1
#define WIFI_RASPBERRY 2
#define WIFI_TO_BE_CONFIGURED 99
#define WIFI_ERROR 0

class NET {
	private:
		static bool first;
		static bool useAP;
		static int wifiStatus;
		static void macAddress(char *buf);
		static void onWiFiEvent(WiFiEvent_t event);
		static bool connect(char *ssid, char *password);
		static bool createHotspot();
    	static int internal_setup();
	public:
		static int setup(bool startAP);
		static int restart();
		static void loop();
		static int getStatus();
		static int getNetworks(char* buf, int sizeof_buf);
		static long getEpochTime();
};
#endif

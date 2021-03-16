#if !defined(_NET_H_)
#define _NET_H_
#include "compile_settings.h"
#include <ESP8266WiFi.h>

//return of setup(), and also getStatus()
#define WIFI_HOTSPOT 1
#define WIFI_RASPBERRY 2
#define WIFI_TO_BE_CONFIGURED 99
#define WIFI_ERROR 0

class NET
{
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
	static int setup(bool config);
	static int restart();
	static void loop();
	static int getStatus();
	static int getNetworks(char *buf, int sizeof_buf);
	static long getEpochTime();
	static int mqtt_queue_length();
	static bool mqtt_connected();
	static bool mqtt_pop(String &topic, String &payload, bool remove = true);
	static bool mqtt_publish(const char *topic, const char *payload, bool retain = true);
};
#endif

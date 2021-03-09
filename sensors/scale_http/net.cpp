#include "net.h"
#include <ESP8266mDNS.h>
#include <ESP8266httpUpdate.h>
#include <DNSServer.h>
#include <WiFiUdp.h>
#if defined(IDE_OTA)
#include <ArduinoOTA.h>
#endif

#define WIFI_ERROR_TIMEOUT (120 * 1000)
#define DNS_PORT 53
#define WIFI_CONNECT_TIMEOUT (10 * 1000)
#define WIFI_OPEN ENC_TYPE_NONE

WiFiUDP ntpUDP;
DNSServer serverDNS;
NTPClient timeClient(ntpUDP, "pool.ntp.org", eeprom.utcOffsetInSeconds);
IPAddress AP_netMask(255, 255, 255, 0);
IPAddress AP_ip(192, 168, 4, 2);

bool NET::first = false;
int NET::wifiStatus = WIFI_TO_BE_CONFIGURED;
bool NET::useAP = false;

int NET::getStatus()
{
	return wifiStatus;
}

int NET::getNetworks(char *buf, int sizeof_buf)
{
	int n = WiFi.scanNetworks();
	int len = 0;
	buf[0] = 0;
	for (int i = 0; i < n; i++)
	{
		if (len + WiFi.SSID(i).length() + 1 < sizeof_buf)
			len += sprintf(buf + len, "%s|Ch:%d|%ddBm|%s\n", WiFi.SSID(i).c_str(), WiFi.channel(i), WiFi.RSSI(i), WiFi.encryptionType(i) == WIFI_OPEN ? "open" : "enc");
		else
		{
			n = i;
			break;
		}
	}
	DEBUG_LN("scan networks:");
	DEBUG_LN(buf);
	return n;
}

void NET::macAddress(char *buf)
{
	uint8_t mac[WL_MAC_ADDR_LENGTH];
	WiFi.softAPmacAddress(mac);
	sprintf(buf, "%02X%02X%02X%02X%02X%02X",
			mac[WL_MAC_ADDR_LENGTH - 6],
			mac[WL_MAC_ADDR_LENGTH - 5],
			mac[WL_MAC_ADDR_LENGTH - 4],
			mac[WL_MAC_ADDR_LENGTH - 3],
			mac[WL_MAC_ADDR_LENGTH - 2],
			mac[WL_MAC_ADDR_LENGTH - 1]);
}

void NET::onWiFiEvent(WiFiEvent_t event)
{
	DEBUGF("---------------- wifi event %d\n", event);
	wifiStatus = WIFI_TO_BE_CONFIGURED;
}

bool NET::connect(char *ssid, char *password)
{
	char host[32];
	char mac[10];

	DEBUGF("WIFI connect to %s, key %s\n", ssid, password);

	macAddress(mac);
#if defined(HOST_AND_MAC)
	sprintf(host, "%s_%s", eeprom.AP_SID, mac);
#else
	sprintf(host, "%s", eeprom.AP_SID);
#endif
	WiFi.mode(WIFI_STA);
	WiFi.disconnect();
	WatchIT(50);

	DEBUGF("NET CONNECT .. status %d\n", WiFi.status());

	WiFi.mode(WIFI_STA);
	WiFi.hostname(host);
	//router 40mhz casino...meglio il B
	//WiFi.setPhyMode(WIFI_PHY_MODE_11B);
	WiFi.begin(ssid, password);

	unsigned long time = millis();
	while (WiFi.status() != WL_CONNECTED && time + WIFI_CONNECT_TIMEOUT > millis())
	{
		DEBUG(".");
		yield();
		WatchIT(250);
	}

	if (WiFi.status() != WL_CONNECTED)
		return false;

	DEBUGF("\nhostname: %s  IP:%s", host, WiFi.localIP().toString().c_str());

	if (MDNS.begin(host))
	{
		DEBUG_LN("mDNS responder started");
	}
	else
	{
		DEBUG_LN("Error setting up MDNS responder!");
	}
	WatchIT(500);
#if defined(USE_NTP)
	timeClient.begin();
#endif
	return WiFi.status() == WL_CONNECTED;
}

bool NET::createHotspot()
{
	char mac[10];
	char host[32];
	macAddress(mac);
#if defined(HOST_AND_MAC)
	sprintf(host, "%s_%s", eeprom.AP_SID, mac);
#else
	sprintf(host, "%s", eeprom.AP_SID);
#endif

	WiFi.disconnect();
	WiFi.mode(WIFI_AP);
	WiFi.hostname(host);
	WiFi.softAPConfig(AP_ip, AP_ip, AP_netMask);
	WiFi.softAP(host, eeprom.AP_PWD);
	DEBUGF("SSID: %s PASSWORD: %s\n", host, eeprom.AP_PWD);
	for (int i = 0; i < 3; i++)
	{
		yield();
		WatchIT(200);
	}
	DEBUG("IP address: ");
	DEBUG_LN(WiFi.softAPIP());
	for (int i = 0; i < 3; i++)
	{
		yield();
		WatchIT(200);
	}
	serverDNS.setTTL(100);
	serverDNS.setErrorReplyCode(DNSReplyCode::NoError);
	serverDNS.start(DNS_PORT, "*", AP_ip); // captive
	for (int i = 0; i < 3; i++)
	{
		yield();
		WatchIT(200);
	}
	DEBUG_LN("DNS Server Started, captive portal");

	return true;
}

int NET::restart()
{
	switch (wifiStatus)
	{
	case WIFI_TO_BE_CONFIGURED:
	case WIFI_ERROR:
		break;

	case WIFI_RASPBERRY:
		DEBUG_LN("mDNS Server Stopped");
		MDNS.end();
		break;

	case WIFI_HOTSPOT:
		DEBUG_LN("DNS Server Stopped");
		serverDNS.stop();
		break;
	}
	wifiStatus = WIFI_TO_BE_CONFIGURED;
	internal_setup();
}

//0 hotspot
//1 connected to server lan
int NET::internal_setup()
{
	if (first)
	{
		first = false;
		WiFi.onEvent(onWiFiEvent, WIFI_EVENT_STAMODE_DISCONNECTED);
	}

	int count = STA_RETRY;
	int connected = false;
	while (!(connected = connect(eeprom.STA_SID, eeprom.STA_PWD)) && --count > 0)
	{
		yield();
		WatchIT(100);
	}

	if (connected)
		return wifiStatus = WIFI_RASPBERRY;

	if (useAP || AP_ON_STA_FAIL)
	{
		DEBUG_LN("create hostspot wlan");
		if (createHotspot())
			return wifiStatus = WIFI_HOTSPOT;
		else
			return wifiStatus = WIFI_ERROR;
	}
	return wifiStatus = WIFI_ERROR;
}

int NET::setup(bool config)
{

	useAP = config;
	static bool oneshoot = false;
	int ret = internal_setup();

#if defined(IDE_OTA)
	ArduinoOTA.onStart([]() {
		String type;
		if (ArduinoOTA.getCommand() == U_FLASH)
			type = "sketch";
		else // U_SPIFFS
			type = "filesystem";

		// NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
		DEBUG_LN("Start updating " + type);
	});
	ArduinoOTA.onEnd([]() {
		DEBUGF("\nEnd");
	});
	ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
		Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
	});
	ArduinoOTA.onError([](ota_error_t error) {
		DEBUGF("Error[%u]: ", error);
		if (error == OTA_AUTH_ERROR)
			DEBUG_LN("Auth Failed");
		else if (error == OTA_BEGIN_ERROR)
			DEBUG_LN("Begin Failed");
		else if (error == OTA_CONNECT_ERROR)
			DEBUG_LN("Connect Failed");
		else if (error == OTA_RECEIVE_ERROR)
			DEBUG_LN("Receive Failed");
		else if (error == OTA_END_ERROR)
			DEBUG_LN("End Failed");
	});
	DEBUG_LN("OTA STARTING");
	ArduinoOTA.begin();
	DEBUG_LN("OTA STARTED");
#endif

	return ret;
}

void NET::loop()
{
	static unsigned long time = 0;
	static bool forcentp = true;

	switch (wifiStatus)
	{
	case WIFI_ERROR:
	case WIFI_TO_BE_CONFIGURED:
		forcentp = true;
		if (time == 0 || time + WIFI_ERROR_TIMEOUT < millis())
		{
			time = millis();
			//da controllare
			//se si passa da hotspot a client... mdns e dns da fermare....
			internal_setup();
		}
		break;
	case WIFI_HOTSPOT:
		forcentp = true;
		time = 0;
		serverDNS.processNextRequest();
		WatchIT(1);
		break;
	case WIFI_RASPBERRY:
		//time = 0;
		MDNS.update();
#if defined(USE_NTP)
		if (forcentp)
		{
			timeClient.forceUpdate();
			forcentp = false;
			DEBUG_LN(timeClient.getEpochTime());
			struct timeval n = {.tv_sec = timeClient.getEpochTime()};
			//struct timezone tz = {60, 0};
			settimeofday(&n, NULL);
		}
		if ((time + 24 * 60 * 60 * 1000) < millis())
		{
			if (timeClient.update())
			{
				DEBUG_LN(timeClient.getEpochTime());
				struct timeval n = {.tv_sec = timeClient.getEpochTime()};
				//struct timezone tz = {60, 0};
				settimeofday(&n, NULL);
			}
			time = millis();
		}
#endif
		WatchIT(1);
		break;

	default:
		break;
	}

#if defined(IDE_OTA)
	ArduinoOTA.handle();
#endif
}

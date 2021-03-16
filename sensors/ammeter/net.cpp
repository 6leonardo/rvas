#include "net.h"
#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiClient.h>
#include <ESP8266mDNS.h>
//#include <ESP8266httpUpdate.h>
#include <DNSServer.h>
#include <WiFiUdp.h>
#include <PubSubClient.h>
#if defined(IDE_OTA)
#include <ArduinoOTA.h>
#endif

#define WIFI_ERROR_TIMEOUT (120 * 1000)
#define DNS_PORT 53
#define WIFI_CONNECT_TIMEOUT (10 * 1000)
#define WIFI_OPEN ENC_TYPE_NONE

#if defined(REMOTE_DEBUG)
RemoteDebug Debug;
#endif

WiFiUDP ntpUDP;
DNSServer serverDNS;
NTPClient timeClient(ntpUDP, "pool.ntp.org", eeprom.utcOffsetInSeconds);
IPAddress AP_netMask(255, 255, 255, 0);
IPAddress AP_ip(192, 168, 4, 2);

bool NET::first = false;
int NET::wifiStatus = WIFI_TO_BE_CONFIGURED;
bool NET::useAP = false;

WiFiClient wifi_mqtt;
PubSubClient mqtt(wifi_mqtt);

#define MQTT_QUEUE_LEN 10
typedef struct
{
	String topic;
	String payload;
} mqtt_el;

mqtt_el mqtt_queue[MQTT_QUEUE_LEN];
int mqtt_start = 0;
int mqtt_end = 0;
int mqtt_len = 0;

void mqtt_callback(char *topic, byte *payload, unsigned int length)
{
	DEBUG("Message arrived [");
	DEBUG(topic);
	DEBUG("] ");
	for (int i = 0; i < length; i++)
	{
		DEBUG((char)payload[i]);
	}
	DEBUG_LN();

	if (mqtt_len < MQTT_QUEUE_LEN)
	{
		char buff[500];
		for (int i = 0; i < length; i++)
			buff[i] = (char)payload[i];
		buff[length] = 0;
		mqtt_queue[mqtt_end].topic = topic;
		mqtt_queue[mqtt_end].payload = buff;
		mqtt_end = (1 + mqtt_end) % MQTT_QUEUE_LEN;
		mqtt_len++;
	}
}

int NET::mqtt_queue_length()
{
	return mqtt_len;
}

bool NET::mqtt_pop(String &topic, String &payload, bool remove)
{
	if (mqtt_len == 0)
		return false;

	topic = mqtt_queue[mqtt_start].topic;
	payload = mqtt_queue[mqtt_start].payload;
	if (remove)
	{
		mqtt_queue[mqtt_start].topic = "";
		mqtt_queue[mqtt_start].payload = "";
		mqtt_start = (1 + mqtt_start) % MQTT_QUEUE_LEN;
		mqtt_len--;
	}
	return true;
}

bool NET::mqtt_publish(const char *topic, const char *payload, bool retain)
{
	if (!mqtt.connected())
		return false;

	DEBUGF("publish [%s] [%s] %d\n", topic, payload, (int)retain);

	mqtt.publish(topic, payload, retain);
	return true;
}

bool NET::mqtt_connected()
{
	return mqtt.connected();
}

bool mqtt_reconnect()
{
	static bool first = true;
	static unsigned long time = 0;

	// Loop until we're reconnected
	if (!mqtt.connected())
	{
		// Attempt to connect
		if (first || time + 5000 < millis())
		{
			DEBUG_LN("Attempting MQTT connection...");
			first = false;
			time = millis();
			char host[32];
			NET::getHostname(host);
			bool mqtt_status = false;
			if (*eeprom.mqtt_user == 0)
				mqtt_status = mqtt.connect(host);
			else
				mqtt_status = mqtt.connect(host, eeprom.mqtt_user, eeprom.mqtt_pwd);

			if (mqtt_status)
			{
				DEBUG_LN("connected");
				mqtt.subscribe(MQTT_TOPIC_WAKEUP);
				return true;
			}
			else
			{
				DEBUG("failed, rc=");
				DEBUG(mqtt.state());
				DEBUG_LN(" try again in 5 seconds");
			}
			WatchIT(1);
		}
		return false;
	}
	return true;
}
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

void NET::getHostname(char *host)
{
#if defined(HOST_AND_MAC)
	char mac[10];
	macAddress(mac);
	sprintf(host, "%s_%s", eeprom.AP_SID, mac);
#else
	sprintf(host, "%s", eeprom.AP_SID);
#endif
}

bool NET::connect(char *ssid, char *password)
{
	char host[32];

	DEBUGF("WIFI connect to %s, key %s\n", ssid, password);
	getHostname(host);
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
	char host[32];
	getHostname(host);

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

	if (ret && wifiStatus == WIFI_RASPBERRY)
	{
		mqtt.setServer(eeprom.mqtt_server, eeprom.mqtt_port);
		mqtt.setCallback(mqtt_callback);
	}

#if defined(REMOTE_DEBUG)
	char host[32];
	NET::getHostname(host);
	Debug.begin(host);
	Debug.setResetCmdEnabled(true);
	Debug.showProfiler(true);
	Debug.showColors(true);
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
		if (mqtt_reconnect())
			mqtt.loop();

		WatchIT(1);
		break;

	default:
		break;
	}

#if defined(IDE_OTA)
	ArduinoOTA.handle();
#endif

#if defined(REMOTE_DEBUG)
	Debug.handle();
#endif
}

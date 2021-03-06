#include <ESP8266HTTPClient.h>
#include <HX711.h>

#include "compile_settings.h"
#include "net.h"
#include "web.h"
#include "eeprom.h"
#define LEVEL_1 "gas/level/1"
#define LEVEL_2 "gas/level/2"
#define WEIGHT_1 "gas/weight/1"
#define WEIGHT_2 "gas/weight/2"
#define WEIGHT_1 "gas/tare/1"
#define WEIGHT_2 "gas/tare/2"
#define CMD_1 "gas/cmd/1"
#define CMD_2 "gas/cmd/2"

WiFiClient client;

HX711 scale1;
HX711 scale2;

float weight1;
float weight2;

bool configMode;

//ESP.wdtDisable();
void WatchIT(int ms) {
	delay(ms);
#if defined(WATCH_DOG)	
	ESP.wdtFeed();
#endif
}

void setup(void)
{

#if defined(WATCH_DOG)
	ESP.wdtEnable(WATCH_DOG);
#endif
	
	DEBUG_LN("Starting");
	pinMode(CONFIG_BUTTON, INPUT);

	unsigned long time = millis();
	configMode = false;
	configMode = digitalRead(CONFIG_BUTTON) == HIGH;
	DEBUGF("ConfigMode: %d\n", configMode);
	DEBUG_LN("EEPROM INIT");
	Persistent::setup();
	DEBUG_LN("NET INIT");
	int status = NET::setup(configMode);
	DEBUG("WIFI STATUS:");
	DEBUG_LN(status);
	DEBUG_LN("WEB INIT");

	if (configMode)
		WEB::setup();
}

void loop(void)
{
	static unsigned long started_at = millis();
	static bool first_loop = true;

	static unsigned long display_refresh = millis();

	WatchIT(1);
	NET::loop();
	WatchIT(1);
	if (configMode)
	{
		WEB::loop();
		WatchIT(1);
	}
	first_loop = false;
	yield();
}


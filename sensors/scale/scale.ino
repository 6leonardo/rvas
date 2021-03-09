#include <ESP8266HTTPClient.h>
#include <HX711.h>

#include "compile_settings.h"
#include "net.h"
#include "web.h"
#include "eeprom.h"

WiFiClient client;
bool reset = false;
HX711 scale1;
HX711 scale2;

float weight1;
float weight2;
unsigned long started_at;
bool configMode;

//ESP.wdtDisable();
void WatchIT(int ms)
{
	delay(ms);
#if defined(WATCH_DOG)
	ESP.wdtFeed();
#endif
}

void getState()
{
	started_at = millis();
	char buf[300];
	sprintf(buf, "{ \"scale1\": {\"state\":%d, \"weight\":%.2f, \"tare\":%.2f, \"level\":%.2f} ,\"scale2\": {\"state\":%d, \"weight\":%.2f, \"tare\":%.2f, \"level\":%.2f}}",
			eeprom.initialized_1, weight1 / 1000, eeprom.gas_tare_1 / 1000, (weight1 - eeprom.gas_tare_1) / 100,
			eeprom.initialized_2, weight2 / 1000, eeprom.gas_tare_2 / 1000, (weight2 - eeprom.gas_tare_2) / 100);
	WEB::responseJson(buf);
}

void doCommand()
{
	const char *cmd = WEB::serverWeb.arg("cmd").c_str();
	const char *scalen = WEB::serverWeb.arg("scale").c_str();
	const char *param = WEB::serverWeb.arg("param").c_str();
	HX711 &scale = *scalen == '1' ? scale1 : scale2;
	if (!strcmp(cmd, "dotare"))
	{
		scale.tare(120);
		scale.set_scale();
	}
	else if (!strcmp(cmd, "doscale"))
	{
		float units = scale.get_units(100);
		float offset = scale.get_offset();
		float weight = 0;
		sscanf(param, "%f", &weight);
		DEBUGF("units %.2f offset %.2f weight:%.2f\n", units, offset, weight);
		DEBUGF("scale %.2f", units / weight);
		if (*scalen == '1')
		{
			eeprom.initialized_1 = 1;
			eeprom.offset_1 = offset;
			eeprom.scale_1 = units / weight;
			scale.set_scale(units / weight);
			reset = true;
		}
		else
		{
			eeprom.initialized_2 = 1;
			eeprom.offset_2 = offset;
			eeprom.scale_2 = units / weight;
			scale.set_scale(units / weight);
			reset = true;
		}
		Persistent::save();
	}
	else if (!strcmp(cmd, "new"))
	{
		float weight = 0;
		float tare = 0;
		sscanf(param, "%f", &weight);
		if (weight == 0)
		{
			float units = scale.get_units(60);
			tare = units - 10000;
		}
		else
			tare = weight;

		if (*scalen == '1')
			eeprom.gas_tare_1 = tare;
		else
			eeprom.gas_tare_2 = tare;
		Persistent::save();
	}

	char buf[50];
	sprintf(buf, "{\"code\":%d}", 1);
	WEB::responseJson(buf);
}

void hibernate()
{
	WiFi.disconnect();
	delay(1000);
	DEBUG_LN("off");
	uint64_t t = 1e6;
	t *= eeprom.sleepInterval;
	scale1.power_down();
	ESP.deepSleep(t, WAKE_RF_DEFAULT); //WAKE_RF_DEFAULT); //, WAKE_RFCAL);
	delay(1000);
}

void setup(void)
{
	pinMode(CONFIG_BUTTON, INPUT);
	configMode = false;
	configMode = digitalRead(CONFIG_BUTTON) == LOW;
	//configMode = true;
#if defined(WATCH_DOG)
	ESP.wdtEnable(WATCH_DOG);
#endif
	DEBUG_INIT();
	DEBUG_LN("Starting");
	DEBUGF("reset reason %s\n", ESP.getResetReason().c_str());

	DEBUGF("ConfigMode: %d\n", (int)configMode);

	DEBUG_LN("EEPROM INIT");
	Persistent::setup();

	DEBUG_LN("Initialize HX711 sensors");
	scale1.begin(LOADCELL_1_DOUT_PIN, LOADCELL_1_SCK_PIN, LOADCELL_1_GAIN);
	if (eeprom.initialized_1)
	{
		while (!scale1.is_ready())
		{
			scale1.wait_ready(1000);
			yield();
		}

		//scale1.power_up();
		DEBUGF("scale1 offset:%f scale:%f\n", eeprom.offset_1, eeprom.scale_1);
		scale1.set_offset(eeprom.offset_1);
		scale1.set_scale(eeprom.scale_1);
	}
	scale2.begin(LOADCELL_2_DOUT_PIN, LOADCELL_2_SCK_PIN, LOADCELL_2_GAIN);
	if (eeprom.initialized_2)
	{
		while (!scale2.is_ready())
		{
			scale2.wait_ready(1000);
			yield();
		}

		//scale2.power_up();
		DEBUGF("scale2 offset:%f scale:%f\n", eeprom.offset_2, eeprom.scale_2);
		scale2.set_offset(eeprom.offset_2);
		scale2.set_scale(eeprom.scale_2);
	}

	DEBUG_LN("NET INIT");
	int status = NET::setup(configMode);
	DEBUG("WIFI STATUS:");
	DEBUG_LN(status);
	DEBUG_LN("WEB INIT");

	if (configMode)
	{
		WEB::addHandler("/state", getState);
		WEB::addHandler("/cmd", doCommand);
		WEB::setup();
	}
}
void tasks()
{
	NET::loop();
	WatchIT(1);
	if (configMode)
	{
		WEB::loop();
		WatchIT(1);
	}
}
void loop(void)
{
	static bool first_loop = true;
	static unsigned long start_time = millis();
	static unsigned long sent_time = 0;
	static bool mqtt_answer = false;
	float count;
	float weight;
	char msg[40];

	if (millis() - start_time > 1000 * 60 * 60 * 24 + 365 * 10)
		start_time = millis();

	if (eeprom.initialized_1)
	{
		weight = count = 0;
		while (++count < 15)
		{
			weight += scale1.get_units(1);
			tasks();
			if (reset)
				reset = (bool)(weight = count = 0);
		}
		weight1 = weight / (count - 1);
	}

	if (eeprom.initialized_2)
	{
		weight = count = 0;
		while (++count < 15)
		{
			weight += scale2.get_units(1);
			tasks();
			if (reset)
				reset = (bool)(weight = count = 0);
		}
		weight2 = weight / (count - 1);
	}

	if (NET::getStatus() == WIFI_RASPBERRY && NET::mqtt_connected())
	{
		if (sent_time == 0 || sent_time + 1000 < millis())
		{
			if (eeprom.initialized_1)
			{
				sprintf(msg, "%.2f", (weight1/1000- eeprom.gas_tare_1) * 10);
				NET::mqtt_publish(MQTT_TOPIC_LEVEL_1, msg);
			}

			if (eeprom.initialized_2)
			{
				sprintf(msg, "%.2f", (weight2/1000 - eeprom.gas_tare_2) * 10);
				NET::mqtt_publish(MQTT_TOPIC_LEVEL_2, msg);
			}
		}
		String topic;
		String payload;
		if (NET::mqtt_pop(topic, payload))
		{
			if (topic == MQTT_TOPIC_WAKEUP)
			{
				mqtt_answer = true;
				if (payload == "on")
				{
					NET::mqtt_publish(MQTT_TOPIC_WAKEUP, "off");
					if (!configMode)
					{
						WEB::addHandler("/state", getState);
						WEB::addHandler("/cmd", doCommand);
						WEB::setup();
						configMode = true;
					}
					sprintf(msg, "%s %s", MQTT_TOPIC_SLAVE_NAME, WiFi.localIP().toString().c_str());
					NET::mqtt_publish(MQTT_TOPIC_URL_OPEN, msg, false);
				}
			}
		}

		sent_time = millis();
	}

	if (!configMode && (mqtt_answer || start_time + eeprom.mqtt_timeout < millis()))
	{
		if(configMode && NET::mqtt_connected()) 
			NET::mqtt_publish(MQTT_TOPIC_URL_CLOSE, MQTT_TOPIC_SLAVE_NAME, false);

		hibernate();
	}

	tasks();

	if (started_at + eeprom.wakeupInterval < millis())
		hibernate();

	first_loop = false;
	yield();
}

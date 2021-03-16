#include <ESP8266HTTPClient.h>
#include "src/Adafruit_INA219/Adafruit_INA219.h"
#include "TCA9548A.h"

#include "compile_settings.h"
#include "net.h"
#include "web.h"
#include "eeprom.h"

WiFiClient client;
bool reset = false;


TCA9548A I2CMux(0x70);

Adafruit_INA219 ina[5] = {Adafruit_INA219(0x40), Adafruit_INA219(0x40), Adafruit_INA219(0x40), Adafruit_INA219(0x40), Adafruit_INA219(0x40)};

unsigned long started_at;
unsigned long from_begin = 0;
bool configMode;
#if defined(REMOTE_DEBUG)
RemoteDebug debug;
#endif

//ESP.wdtDisable();
void WatchIT(int ms)
{
	delay(ms);
#if defined(WATCH_DOG)
	ESP.wdtFeed();
#endif
}
#define N_INA 5

typedef struct {
	float shuntVolt;
	float busVolt;
	float current;
	float power;
	float mWh;
} INA219;

INA219 sensors[N_INA];

void getState()
{
	started_at = millis();
	char buf[1024];
	char json[1024];
	strcpy(json,"[");

	for(int i=0;i<5;i++) {
		sprintf(buf, "{\"sv\":%.2f,\"bv\":%.2f,\"c\":%.2f,\"p\":%.2f,\"mwh\":%.2f}",
			sensors[i].shuntVolt,
			sensors[i].busVolt,
			sensors[i].current,
			sensors[i].power,
			sensors[i].mWh/(millis()-from_begin));
		if(i!=0)
			strcat(json,",");
		strcat(json,buf);
	}
	strcat(json,"]");
	WEB::responseJson(json);
}

void doCommand()
{

	const char *cmd = WEB::serverWeb.arg("cmd").c_str();
	const char *scalen = WEB::serverWeb.arg("scale").c_str();
	const char *param = WEB::serverWeb.arg("param").c_str();

	if (!strcmp(cmd, "dotare"))
	{

		//hx.set_offset(0);
	}
	else if (!strcmp(cmd, "doscale"))
	{
		Persistent::save();
	}
	else if (!strcmp(cmd, "new"))
	{
		/*
		Persistent::save();
	*/
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
	ESP.deepSleep(t, WAKE_RF_DEFAULT); //WAKE_RF_DEFAULT); //, WAKE_RFCAL);
	delay(1000);
}

String setupLog;

void init_ina219(int i)
{
	I2CMux.openChannel(i);
	delay(10);
	if (!ina[i].begin())
	{
		DEBUGF("%d Failed to find INA219 chip\n", i);
		while (1)
			delay(10);
	}
	String vars;
	ina[i].setCalibration(16, 0.1, 320, 2, vars);
	setupLog += "shunt: ";
	setupLog += i;
	setupLog += "\n";
	setupLog += vars;
	//ina[i].setCalibration_test();
}

void read_ina219(int i, float &shuntVoltage, float &busVoltage, float &current_mA, float &power_mW)
{
	delay(10);
	I2CMux.openChannel(i);
	delay(10);
	shuntVoltage = ina[i].getShuntVoltage_mV();
	busVoltage = ina[i].getBusVoltage_V();
	current_mA = ina[i].getCurrent_mA();
	power_mW = ina[i].getPower_mW();
	delay(10);
	I2CMux.closeAll();
	delay(10);
}

void setup(void)
{
	pinMode(CONFIG_BUTTON, INPUT);
	configMode = false;
	configMode = digitalRead(CONFIG_BUTTON) == LOW;
	configMode = true;
	//configMode = true;
#if defined(WATCH_DOG)
	ESP.wdtEnable(WATCH_DOG);
#endif

	DEBUG_INIT();
	DEBUG_LN("Starting");
	DEBUGF("reset reason %s\n", ESP.getResetReason().c_str());
	DEBUGF("ConfigMode: %d\n", (int)configMode);
	Persistent::setup();

	DEBUG_LN("EEPROM INIT");
	DEBUG_LN("NET INIT");
	int status = NET::setup(configMode);
	DEBUG("WIFI STATUS:");
	DEBUG_LN(status);

	DEBUG_LN("INA INIT");
	I2CMux.begin(Wire);
	I2CMux.closeAll();

	//Wire.setClock(400000);

	for (int i = 0; i < N_INA; i++)
		init_ina219(i);

	I2CMux.closeAll();

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
	float volt;
	char msg[40];

	if (millis() - start_time > 1000 * 60 * 60 * 24 + 365 * 10)
		start_time = millis();

	unsigned long time = millis();
	if (sent_time == 0 || sent_time + 2000 < millis())
	{

		for (int i = 0; i < N_INA; i++)
		{
			float shuntVoltage = 0;
			float busVoltage = 0;
			float current_mA = 0;
			float power_mW = 0;

			tasks();
			if (sent_time != 0)
			{
				read_ina219(i, sensors[i].shuntVolt, sensors[i].busVolt, sensors[i].current, sensors[i].power);
				sensors[i].mWh += sensors[i].power  * ((time - sent_time));
				LOG("[ %2d ] shmV:%6.2f busV:%6.2f curmA:%7.2f powermW:%7.2f mW/h:%8.2f", i, sensors[i].shuntVolt, sensors[i].busVolt, sensors[i].current, sensors[i].power, sensors[i].mWh / (time - from_begin));
				//LOG("%s\n", setupLog.c_str());
			}
			else
			{

				sensors[i].mWh=0;
				from_begin = time;
			}
		}
		sent_time = time;
	}

	/*
	if (NET::getStatus() == WIFI_RASPBERRY && NET::mqtt_connected())
	{
		if (sent_time == 0 || sent_time + 1000 < millis())
		{
			if (eeprom.initialized_1)
			{
				sprintf(msg, "%.2f", volt1);
				NET::mqtt_publish(MQTT_TOPIC_LEVEL_1, msg);
			}

			if (eeprom.initialized_2)
			{
				sprintf(msg, "%.2f", volt2);
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
	*/

	unsigned long job = millis();

	tasks();

	if (!configMode && (mqtt_answer || start_time + eeprom.mqtt_timeout < millis()))
		hibernate();

	if (started_at + eeprom.wakeupInterval < millis())
	{
		if (configMode && NET::mqtt_connected())
			NET::mqtt_publish(MQTT_TOPIC_URL_CLOSE, MQTT_TOPIC_SLAVE_NAME, false);

		hibernate();
	}

	first_loop = false;
	yield();
}

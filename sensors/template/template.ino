#include "compile_settings.h"
#include <ESP8266WiFi.h>
#include "lib_net.h"
#include "lib_web.h"
#include "lib_eeprom.h"

//------

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <HX711.h>
#include <EEPROM.h>


// HX711 circuit wiring
const int LOADCELL_1_DOUT_PIN = 2;
const int LOADCELL_1_SCK_PIN = 3;

const int LOADCELL_2_DOUT_PIN = 2;
const int LOADCELL_2_SCK_PIN = 3;

const int CONFIG_BUTTON = 1

//#define INTERN


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

float weight1 float weight2

	void
	setup()
{
	//ESP.eraseConfig();
	pinMode(16, INPUT_PULLUP);
//	pinMode(2, OUTPUT);
//	digitalWrite(2, HIGH);
#if defined(FUMO)
	pinMode(FUMO_D, INPUT);
#endif
	delay(100);
	WiFi.persistent(false);
	Serial.begin(74880);
	delay(500);
	Serial.println();
	Serial.println("******** BEGIN ***********");
	Serial.println("- set ESP STA mode");
	WiFi.mode(WIFI_STA);
	Serial.println("- connecting to wifi");
	WiFi.begin(ssid, password);
	Serial.println("");
	while (WiFi.status() != WL_CONNECTED)
	{
		if (counter > 20)
		{
			Serial.println("- can't connect, going to sleep");
			hibernate(failConnectRetryInterval);
		}
		delay(500);
		Serial.print(".");
		counter++;
	}

	Serial.println("- wifi connected");
	readDHTSensor();
	readGas();
	sendHttpRequest();
	Serial.println("- got back to sleep");
	hibernate(sleepInterval);
}

void sendHttpRequest()
{
	HTTPClient http;
	int code = 0;
	String payload;
	String resp;
	http.begin(client, server, port, buildRequest(TEMP, temp));
	code = http.GET();
	payload = http.getString();
	resp = "";
	resp = resp + "1. " + code + " " + payload;
	Serial.println(resp);
	http.end();
	http.begin(client, server, port, buildRequest(HUM, hum));
	code = http.GET();
	payload = http.getString();
	resp = "";
	resp = resp + "2. " + code + " " + payload;
	Serial.println(resp);
	http.end();
	http.begin(client, server, port, buildRequest(GAS, fumo_a));
	code = http.GET();
	payload = http.getString();
	resp = "";
	resp = resp + "3. " + code + " " + payload;
	Serial.println(resp);
	http.end();
}

void readGas()
{
#if defined(FUMO)
	fumo_a = analogRead(FUMO_A);
	fumo_d = digitalRead(FUMO_D);
#else
	fumo_a = random(0, 1000);
	fumo_d = 0;
#endif
}

void readDHTSensor()
{
	delay(200);
#if defined(DHTPIN)
	hum = dht.readHumidity();
	temp = dht.readTemperature();
#else
	temp = random(0, 30);
	hum = random(0, 100);
#endif

	if (isnan(hum) || isnan(temp))
	{
		temp = 0.00;
		hum = 0.00;
	}
}

String buildRequest(char *name, float value)
{
	char buf[100];
	sprintf(buf, "/add?name=%s&value=%f", name, value);
	return String(buf);
}

void hibernate(int pInterval)
{
	WiFi.disconnect();
	delay(1000);
	Serial.print("off");
	uint64_t t = 1e6;
	t *= pInterval;

	ESP.deepSleep(t, WAKE_RF_DEFAULT); //WAKE_RF_DEFAULT); //, WAKE_RFCAL);
	delay(100);
}

void loop() { delay(100); }

/*
	data = "temp=";
	data += String(t);
	data += "&hum=";
	data += String(h);
	data += "&dev=";
	data += N_DEV;
	http.begin(serverHost);
	http.addHeader("Content-Type", "application/x-www-form-urlencoded");
	http.POST(data);
	http.end();
*/

/**
 *
 * HX711 library for Arduino - example file
 * https://github.com/bogde/HX711
 *
 * MIT License
 * (c) 2018 Bogdan Necula
 *
**/

void setup()
{

	DEBUG_INIT();
	DEBUGF("Initializing the scale\n");
	
	scale1.begin(LOADCELL_1_DOUT_PIN, LOADCELL_1_SCK_PIN);
	scale2.begin(LOADCELL_2_DOUT_PIN, LOADCELL_2_SCK_PIN);

	// Initialize library with data output pin, clock input pin and gain factor.
	// Channel selection is made by passing the appropriate gain:
	// - With a gain factor of 64 or 128, channel A is selected
	// - With a gain factor of 32, channel B is selected
	// By omitting the gain factor parameter, the library
	// default "128" (Channel A) is used here.

	Serial.println("Before setting up the scale:");
	Serial.print("read: \t\t");
	Serial.println(scale.read()); // print a raw reading from the ADC

	Serial.print("read average: \t\t");
	Serial.println(scale.read_average(20)); // print the average of 20 readings from the ADC

	Serial.print("get value: \t\t");
	Serial.println(scale.get_value(5)); // print the average of 5 readings from the ADC minus the tare weight (not set yet)

	Serial.print("get units: \t\t");
	Serial.println(scale.get_units(5), 1); // print the average of 5 readings from the ADC minus tare weight (not set) divided
		// by the SCALE parameter (not set yet)

	scale.set_scale(2280.f); // this value is obtained by calibrating the scale with known weights; see the README for details
	scale.tare();			 // reset the scale to 0

	Serial.println("After setting up the scale:");	

	Serial.print("read: \t\t");
	Serial.println(scale.read()); // print a raw reading from the ADC

	Serial.print("read average: \t\t");
	Serial.println(scale.read_average(20)); // print the average of 20 readings from the ADC

	Serial.print("get value: \t\t");
	Serial.println(scale.get_value(5)); // print the average of 5 readings from the ADC minus the tare weight, set with tare()

	Serial.print("get units: \t\t");
	Serial.println(scale.get_units(5), 1); // print the average of 5 readings from the ADC minus tare weight, divided
		// by the SCALE parameter set with set_scale

	Serial.println("Readings:");
}

void loop()
{
	Serial.print("one reading:\t");
	Serial.print(scale.get_units(), 1);
	Serial.print("\t| average:\t");
	Serial.println(scale.get_units(10), 1);

	scale.power_down(); // put the ADC in sleep mode
	delay(5000);
	scale.power_up();
}

//-----

eepromInfoType0 configuration;
OutputCommand commands[MAX_COMMANDS];
bool configMode = false;

void setup(void)
{

	DEBUGLN("Starting");

	DEBUG_LN("Waiting 5 Seconds for config mode");
	unsigned long time = millis();
	configMode = false;
	while (time + 5000 > millis())
	{
		DEBUG("x");
		unsigned long wait = millis();
		while (wait + 500 > millis())
		{
			yield();
			configMode = configMode || (!DIGITAL::rawRead(INPUT_CONFIG) == HIGH);
		}
	}
	DEBUGLN("");
	DEBUG("ConfigMode: ");
	DEBUGLN(configMode);

	DEBUGLN("EEPROM INIT");
	Persistent::setup();
	DEBUGLN("NET INIT");
	int status = NET::setup();
	DEBUG("WIFI STATUS:");
	DEBUGLN(status);
	DEBUGLN("WEB INIT");
	WEB::setup();
}


void loop(void)
{
	static unsigned long display_refresh = millis();
	WatchIT(1);
	NET::loop();
	WatchIT(1);
	WEB::loop();
	WatchIT(1);
	RTC::loop();
	yield();
}

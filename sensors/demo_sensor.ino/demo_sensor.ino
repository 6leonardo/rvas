/*
Geekstips.com
IoT project - Communication between two ESP8266 - Talk with Each Other
ESP8266 Arduino code example
*/
#include <DHT.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#define DHTPIN 14
#define FUMO_A 17 
//A0
#define FUMO_D 5
#define FUMO
// AP Wi-Fi credentials

const char *ssid = "xxxxxx";
const char *password = "xxxxxx";

// Local web-server address
String server = "rv.local";
int port = 80;
// DEEP_SLEEP Timeout interval in seconds
int sleepInterval = 30;
// DEEP_SLEEP Timeout interval when connecting to AP fails
int failConnectRetryInterval = 2;
int counter = 0;

//#define INTERN

#if defined(INTERN)
#define TEMP "sensor/int/temp"
#define HUM "sensor/int/hum"
#define GAS "sensor/int/gas/1"
#else
#define TEMP "sensor/ext/temp"
#define HUM "sensor/ext/hum"
#define GAS "sensor/ext/gas/1"
#endif


#if defined(DHTPIN)
DHT dht(DHTPIN, DHT11);
#endif
WiFiClient client;

float hum;
float temp;
float fumo_a;
float fumo_d;

void setup()
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
	int code=0;
	String payload;
	String resp;
	http.begin(client,server,port,buildRequest(TEMP,temp));
	code=http.GET();
	payload=http.getString();
	resp="";
	resp=resp+"1. "+code+" "+payload;
	Serial.println(resp);
	http.end();
	http.begin(client,server,port,buildRequest(HUM,hum));
	code=http.GET();
	payload=http.getString();
	resp="";
	resp=resp+"2. "+code+" "+payload;
	Serial.println(resp);
	http.end();
	http.begin(client,server,port,buildRequest(GAS,fumo_a));
	code=http.GET();
	payload=http.getString();
	resp="";
	resp=resp+"3. "+code+" "+payload;
	Serial.println(resp);
	http.end();

}

void readGas() {
#if defined(FUMO)
	fumo_a = analogRead(FUMO_A);
	fumo_d = digitalRead(FUMO_D);
#else
	fumo_a=random(0,1000);
	fumo_d=0;
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

String buildRequest(char* name, float value)
{
	char buf[100];
	sprintf(buf,"/add?name=%s&value=%f",name,value);
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


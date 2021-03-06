#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include "web.h"
#include "net.h"
#include "eeprom.h"


#define WEB_PORT 80

ESP8266WebServer serverWeb;

void responseTextPlain(const char *txt = NULL)
{
	serverWeb.send(200, "text/plain", txt ? txt : "OK");
}

void responseJson(const char *json)
{
	serverWeb.send(200, "application/json", json ? json : "{}");
}

void WEB::handleGetNetworks()
{
	char buf[512];
	int sessionId = 0;

	NET::getNetworks(buf, sizeof(buf));
	//in js togliere l'ultimo new line
	responseTextPlain(buf);
}

void WEB::handleSetNetwork()
{
	int sessionId = 0;

	strncpy(eeprom.STA_SID, serverWeb.arg("ssid").c_str(), sizeof(eeprom.STA_SID) - 1);
	strncpy(eeprom.STA_PWD, serverWeb.arg("key").c_str(), sizeof(eeprom.STA_PWD) - 1);

	Persistent::saveeeprom();

	if (NET::restart() == WIFI_RASPBERRY)
		responseTextPlain("Connected");
	else
		responseTextPlain("KO!");
}

void WEB::handleReadInputs()
{
	/*
	int sessionId = 0;

	if (!authenticatedOrError(sessionId))
		return;

	char json[256];
	sprintf(json, "{\"ALARM_1\":%d,\"ALARM_2\":%d,\"ALARM_3\":%d,\"ALARM_4\":%d,\"PORTA\":%d}",
			!DIGITAL::read(INPUT_ALLARM_1),
			!DIGITAL::read(INPUT_ALLARM_2),
			!DIGITAL::read(INPUT_ALLARM_3),
			!DIGITAL::read(INPUT_ALLARM_4),
			!DIGITAL::read(INPUT_PORTA));
	responseJson(json);
	*/
}

void WEB::handleWriteOutput()
{
	/*
	int sessionId = 0;

	if (!authenticatedOrError(sessionId))
		return;

	int output = 0;
	int pin = 0;
	int value = 0;
	char *cmd = NULL;
	sscanf(serverWeb.arg("output").c_str(), "%d", &output);
	sscanf(serverWeb.arg("value").c_str(), "%d", &value);
	switch (output)
	{
	case 1:
		pin = OUTPUT_PORTA;
		cmd = "PORTA";
		break;
	case 2:
		pin = OUTPUT_CANCELLO;
		cmd = "CANCELLO";
		break;
	case 3:
		pin = OUTPUT_BOX;
		cmd = "BOX";
		break;
	case 4:
		pin = OUTPUT_ALARM;
		cmd = "CHIAVE ALLARME";
	}

	if (pin > 0)
	{
		int found = -1;
		for (int i = 0; i < MAX_COMMANDS; i++)
		{
			if (commands[i].command == pin)
			{
				found = i;
				break;
			}
		}
		if (found == -1)
		{
			for (int i = 0; i < MAX_COMMANDS; i++)
			{
				if (commands[i].count == 0 && !commands[i].on)
				{
					found = i;
					break;
				}
			}
		}

		if (found > -1)
		{
			if (commands[found].count <= 0 && !commands[found].on)
			{
				char log[200];
				sprintf(log, "utente: %s, comando: %s-%d",
						getUserName(sessionId),
						cmd,
						value);
				DB::writeEvent(log);
				commands[found].command = pin;
				commands[found].time = millis() - 60 * 1000;
				commands[found].on = false;
				commands[found].count = value;
			}
		}
	}
	*/
	responseTextPlain("Salvato");
}

String WEB::getContentType(String filename)
{
	if (filename.endsWith(".html"))
		return "text/html";
	else if (filename.endsWith(".css"))
		return "text/css";
	else if (filename.endsWith(".js"))
		return "application/javascript";
	else if (filename.endsWith(".ico"))
		return "image/x-icon";
	else if (filename.endsWith(".json"))
		return "application/json";
	return "text/plain";
}


bool WEB::handleFileRead(String path)
{	
	if (path.endsWith("/"))
		path += "index.html";				   
	String contentType = getContentType(path); 
	String pathWithGz = path + ".gz";
	if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path))
	{								   
		if (SPIFFS.exists(pathWithGz)) 
			path += ".gz";			  

		File file = SPIFFS.open(path, "r");					   
		size_t sent = serverWeb.streamFile(file, contentType); 
		file.close();										   
		Serial.println(String("\tSent file: ") + path);
		return true;
	}
	Serial.println(String("\tFile Not Found: ") + path);
	return false; 
}

void WEB::handleFileUpload()
{ 
	static File fsUploadFile;

	HTTPUpload &upload = serverWeb.upload();
	if (upload.status == UPLOAD_FILE_START)
	{
		String filename = upload.filename;
		if (!filename.startsWith("/"))
			filename = "/" + filename;
		if (filename.endsWith("site.js"))
			filename = "/js/site.js";
		if (filename.endsWith("db.json"))
			filename = "/db/db.json";
		if (filename.endsWith("log.txt"))
			filename = "/db/log.txt";
		if (filename.endsWith("measures.txt"))
			filename = "/db/measures.txt";

		DEBUG("handleFileUpload Name: ");
		DEBUG_LN(filename);
		fsUploadFile = SPIFFS.open(filename, "w"); 
		filename = String();
	}
	else if (upload.status == UPLOAD_FILE_WRITE)
	{
		if (fsUploadFile)
			fsUploadFile.write(upload.buf, upload.currentSize); 
	}
	else if (upload.status == UPLOAD_FILE_END)
	{
		if (fsUploadFile)
		{						  
			fsUploadFile.close(); 
			DEBUG("handleFileUpload Size: ");
			DEBUG_LN(upload.totalSize);
			serverWeb.sendHeader("Location", "/xx_upload"); 
			serverWeb.send(303);
		}
		else
		{
			serverWeb.send(500, "text/plain", "500: couldn't create file");
		}
	}
}

void WEB::setup()
{
	SPIFFS.begin(); 
	serverWeb.onNotFound([]() {									 
		if (!handleFileRead(serverWeb.uri()))					 
			serverWeb.send(404, "text/plain", "404: Not Found"); 
	});
	serverWeb.on("/get_input.php", handleReadInputs);
	serverWeb.on("/get_networks.php", handleGetNetworks);
	serverWeb.on("/set_network.php", handleSetNetwork);
	serverWeb.on("/xx_upload", HTTP_GET, []() {					
		if (!handleFileRead("/upload.html"))					 
			serverWeb.send(404, "text/plain", "404: Not Found");
	});

	serverWeb.on(
		"/xx_upload", HTTP_POST,
		[]() {
			serverWeb.send(200);
		},
		handleFileUpload);

	const char *headerkeys[] = {"User-Agent", "Cookie"};
	size_t headerkeyssize = sizeof(headerkeys) / sizeof(char *);
	serverWeb.collectHeaders(headerkeys, headerkeyssize);
	serverWeb.begin(WEB_PORT);
	MDNS.addService("http", "tcp", WEB_PORT);
	DEBUG_LN("HTTP server started");
}

void WEB::loop()
{
	unsigned long epoch = millis() / 1000;
	serverWeb.handleClient();
	MDNS.update();
}

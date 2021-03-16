#include <ESP8266mDNS.h>
#include "web.h"
#include "net.h"
#include "eeprom.h"


#define WEB_PORT 80

ESP8266WebServer WEB::serverWeb;

void WEB::responseTextPlain(const char *txt)
{
	serverWeb.send(200, "text/plain", txt ? txt : "OK");
}

void WEB::responseJson(const char *json)
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

	Persistent::save();

	if (NET::restart() == WIFI_RASPBERRY)
		responseTextPlain("Connected");
	else
		responseTextPlain("KO!");
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

void WEB::addHandler(char* url, void(*handler)()) {
	serverWeb.on(url,handler);
}

void WEB::setup()
{
	SPIFFS.begin(); 
	serverWeb.onNotFound([]() {									 
		if (!handleFileRead(serverWeb.uri()))					 
			serverWeb.send(404, "text/plain", "404: Not Found"); 
	});
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

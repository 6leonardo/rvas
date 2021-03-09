
#if !defined(_WEB_H)
#define _WEB_H
#include "compile_settings.h"
#include <ESP8266WebServer.h>

class WEB
{
private:
	static void handleFileUpload();

	static void handleGetNetworks();
	static void handleSetNetwork();

	static String getContentType(String filename);
	static bool handleFileRead(String path);

public:
	static ESP8266WebServer serverWeb;
	static void responseTextPlain(const char *txt = NULL);
	static void responseJson(const char *json);
	static void setup();
	static void addHandler(char* url, void(*handler)());
	static void loop();
};
#endif

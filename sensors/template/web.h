
#if !defined(_WEB_H)
#define _WEB_H
#include "compile_settings.h"

class WEB
{
private:
	static void handleReadInputs();
	static void handleWriteOutput();
	static void handleFileUpload();

	static void handleGetNetworks();
	static void handleSetNetwork();

	static void handleGetUpdates();
	static void handleDoUpdates();

	static String getContentType(String filename);
	static bool handleFileRead(String path);
	static void handlePermitLog();

public:
	static void setup();
	static void loop();
};
#endif

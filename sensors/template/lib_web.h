
#if !defined(_LIB_WEB_H)
#define _LIB_WEB_H
#include "compile_settings.h"

class WEB
{
private:
	static void handleReadInputs();
	static void handleWriteOutput();
	static void handleFileUpload();
	static void handleReadUsers();
	static void handleWriteUser();
	static void handleReadLan();
	static void handleWriteLan();
	static void handleBroadlinkLearn();
	static void handleBroadlinkCommands();
	static void handleBroadlinkSend();
	static void handleBroadlinkMessages();
	static void handleBroadlinkExists();

	//static void handleSetSettings();
	//static void handleGetSettings();
	//static void handleGetCell();

	static void handleGetNetworks();
	static void handleSetNetwork();
	static void handleGetPermitWifi();

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

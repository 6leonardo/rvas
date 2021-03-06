#include <EEPROM.h>
#include "lib_eeprom.h"

#define VALORE_SENTINELLA 0x76
#define ADDRESS_SENTINELLA 0

template <typename T>
int EEPROM_writeAnything(int ee, const T &value)
{
	const byte *p = (const byte *)(const void *)&value;
	unsigned int i;
	for (i = 0; i < sizeof(value); i++)
	{
		EEPROM.write(ee++, *p++);
	}
	EEPROM.commit(); //note here the commit! necessario per esp266
	return i;
}

template <typename T>
int EEPROM_readAnything(int ee, T &value)
{
	byte *p = (byte *)(void *)&value;
	unsigned int i;
	for (i = 0; i < sizeof(value); i++)
		*p++ = EEPROM.read(ee++);
	return i;
}

void Persistent::setup()
{

	EEPROM.begin(4096);

	byte sentinella;

	EEPROM_readAnything(ADDRESS_SENTINELLA, sentinella);

	if (sentinella != VALORE_SENTINELLA)
		init();
	else
	{
		loadConfiguration();
	}
}

void Persistent::loadConfiguration()
{
	EEPROM_readAnything(4, configuration); // memorizza in flash
}

void Persistent::saveConfiguration()
{
	EEPROM_writeAnything(4, configuration); // memorizza in flash
	WatchIT(20);
}

void Persistent::init()
{
	EEPROM_writeAnything(ADDRESS_SENTINELLA, VALORE_SENTINELLA);
	strcpy(configuration.AP_SID, AP_SID);
	strcpy(configuration.AP_PWD, AP_PWD);
	strcpy(configuration.STA_SID, WIFI_SID);
	strcpy(configuration.STA_PWD, WIFI_PWD);
	strcpy(configuration.SERVER, SERVER);
	configuration.sleepInterval = 30;
    configuration.failConnectRetryInterval = 2;
	configuration.utcOffsetInSeconds = 3600;
	configuration.port=PORT;
	configuration.initialized_1 = 0;
	configuration.initialized_2 = 0;
	configuration.scale_1 = 0;
	configuration.scale_2 = 0;
	configuration.offset_1 = 0;
	configuration.offset_2 = 0;
	configuration.gas_tare_1 = 0;
	configuration.gas_tare_2 = 0;
	saveConfiguration();
}

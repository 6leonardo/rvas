#include <EEPROM.h>
#include "eeprom.h"

#define VALORE_SENTINELLA 0x76
#define ADDRESS_SENTINELLA 0

eepromInfoType0 eeprom;


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
		loadeeprom();
	}
}

void Persistent::loadeeprom()
{
	EEPROM_readAnything(4, eeprom); // memorizza in flash
}

void Persistent::saveeeprom()
{
	EEPROM_writeAnything(4, eeprom); // memorizza in flash
	WatchIT(20);
}

void Persistent::init()
{
	EEPROM_writeAnything(ADDRESS_SENTINELLA, VALORE_SENTINELLA);
	strcpy(eeprom.AP_SID, DEFAULT_AP_SID);
	strcpy(eeprom.AP_PWD, DEFAULT_AP_PWD);
	strcpy(eeprom.STA_SID, DEFAULT_WIFI_SID);
	strcpy(eeprom.STA_PWD, DEFAULT_WIFI_PWD);
	strcpy(eeprom.SERVER, RASP_SERVER);
	eeprom.sleepInterval = 30;
    eeprom.failConnectRetryInterval = 2;
	eeprom.utcOffsetInSeconds = 3600;
	eeprom.port=PORT;
	eeprom.initialized_1 = 0;
	eeprom.initialized_2 = 0;
	eeprom.scale_1 = 0;
	eeprom.scale_2 = 0;
	eeprom.offset_1 = 0;
	eeprom.offset_2 = 0;
	eeprom.gas_tare_1 = 0;
	eeprom.gas_tare_2 = 0;
	saveeeprom();
}

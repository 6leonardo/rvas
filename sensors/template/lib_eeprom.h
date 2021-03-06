
#if !defined(_LIB_EEPROM_)
#define _LIB_EEPROM_
#include "compile_settings.h"

class Persistent {
	private:
		static void init();
		static void loadConfiguration();

	public:
		static void setup();
		static void saveConfiguration();
};

#endif
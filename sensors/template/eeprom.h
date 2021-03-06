
#if !defined(_EEPROM_)
#define _EEPROM_
#include "compile_settings.h"

class Persistent {
	private:
		static void init();
		static void loadeeprom();

	public:
		static void setup();
		static void saveeeprom();
};

#endif
#include "main.h"

class DisplayOled
{
	private:




	public:
		void init();
		void setFont(DCfont Font);
		void writeString(uint8_t x, uint8_t y, const char *String);
		void clearScreen();
		

};
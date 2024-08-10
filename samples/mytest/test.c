#include <c64/joystick.h>
#include <c64/vic.h>
#include <c64/sprites.h>
#include <c64/memmap.h>
#include <c64/rasterirq.h>
#include <c64/sid.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

//#pragma compile("crt.c")

byte * const Screen = (byte *)0x0400;
byte * const Color = (byte *)0xd800;

// multiples of 5us
static void delay(char n)
{
	__asm {
		ldx n
	l1:
		dex
		bne	l1
	}
}

int main(void)
{
    for(;;)
    {
        for (byte offset = 0; offset <= 254; offset++) 
        {
            vic_waitBottom();

            for (byte index = 0; index <= 254; index++) 
            {
                byte value = offset + index;
                Screen[index] = value;
                Color[index] = value;
            }

vic_waitBottom();
            delay(200);
vic_waitBottom();
            delay(200);
vic_waitBottom();
            delay(200);
vic_waitBottom();
            delay(200);
vic_waitBottom();
            delay(200);
vic_waitBottom();
            delay(200);
vic_waitBottom();
            delay(200);
vic_waitBottom();
            delay(200);
vic_waitBottom();
            delay(200);
vic_waitBottom();
            delay(200);            
        }
    }

	return 0;
}
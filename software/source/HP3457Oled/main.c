/*
 * HP3457Oled.c
 *
 * Created: 30.01.2022 18:13:05
 * Author : xyphro
 */ 

#include <avr/io.h>
#include "oled.h"
#include "pari2c.h"

#include <util/delay.h>
#include "hp_charset.h"
#include "hpdispprot.h"




int main(void)
{	
	oled_init();
	hpdp_init();
	
	DDRD &= ~(1<<3);
	DDRD |= (1<<5);
	
	uint16_t i=0;
    while (1) 
    {
		uint8_t update = hpdp_update();
		if ( update & HPDP_UPDATE_DIGITS )
		{
			oled_repaint_digits(53);
		}
		if ( update & HPDP_UPDATE_ANNUCIATORS )
		{
			oled_repaint_annuciators(53-16);
		}
    }
}


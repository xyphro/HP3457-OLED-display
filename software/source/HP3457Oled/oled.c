
#include "oled.h"
#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include "gfxelements.h"
#include "pari2c.h"
#include "hp_charset.h"
#include "annuciatorsel.h"

uint8_t oled_chars[12]; // character content of all 12 digits
uint8_t oled_dots [12]; // dot content of all 12 digits

//************************************************************************
// PD7 = RESET
// PC5 = SCL
// PC4, PC3, PC2, PC1, PC0, PC7, PC6 = SDA0..6
// PB2, PB1, PB0, PB7, PB6           = SDA7..11
// PD6 = POWER (high active)
//
// Experimentally determined pixel range:
// line 30 = BOTTOM, line 127 = TOP => 98 pixels
// line order: BOTTOM to TOP
//************************************************************************

#define OLED_SLADDR 0x78


static void ssd1306_init(void)
{
	
	PORTD &= ~(1<<7);
	_delay_ms(100);
	PORTD |= (1<<7);
	_delay_ms(100);

	pari2c_start();
	pari2c_writebyte_bc(OLED_SLADDR);
	pari2c_writebyte_bc(0x00);//--Control byte
	pari2c_writebyte_bc(0xae);//--turn off oled panel

	pari2c_writebyte_bc(0xa8);//--set multiplex ratio(1 to 64)
	pari2c_writebyte_bc(0x1f);//--1/32 duty

	pari2c_writebyte_bc(0xd3);//-set display offset
	pari2c_writebyte_bc(0x00);//-not offset

	pari2c_writebyte_bc(0x40);//--set start line address

	pari2c_writebyte_bc(0xa1);//--set segment re-map 128 to 1

	pari2c_writebyte_bc(0xC0);//--Set COM Output Scan Direction 64 to 1

	pari2c_writebyte_bc(0x81);//--set contrast control register
	pari2c_writebyte_bc(0x87); // contrast level

	pari2c_writebyte_bc(0xd5);//--set display clock divide ratio/oscillator frequency
	pari2c_writebyte_bc(0x80);//--set divide ratio

	pari2c_writebyte_bc(0xd9);//--set pre-charge period
	pari2c_writebyte_bc(0x22);

	pari2c_writebyte_bc(0xda);//--set COM pins
	pari2c_writebyte_bc(0x12);

	pari2c_writebyte_bc(0xdb);//--set vcomh
	pari2c_writebyte_bc(0x20);

	pari2c_writebyte_bc(0x8d);//--set vcomh
	pari2c_writebyte_bc(0x10); // EXTERNAL VCC

	pari2c_writebyte_bc(0xa6);//--set normal display

	pari2c_writebyte_bc(0xa4);//Disable Entire Display On

	pari2c_writebyte_bc(0xaf);//--turn on oled panel

	pari2c_writebyte_bc(0x20);
	pari2c_writebyte_bc(0x00);

	pari2c_stop();
}

void ssd1306_set_region(uint8_t x1, uint8_t x2, uint8_t y1, uint8_t y2)
{
	pari2c_start();
	pari2c_writebyte_bc(OLED_SLADDR);
	pari2c_writebyte_bc(0x00);
	pari2c_writebyte_bc(0x21);
	pari2c_writebyte_bc(y1);
	pari2c_writebyte_bc(y2);
	pari2c_stop();
	
	pari2c_start();
	pari2c_writebyte_bc(OLED_SLADDR);
	pari2c_writebyte_bc(0x00);
	pari2c_writebyte_bc(0x22);
	pari2c_writebyte_bc(x1);
	pari2c_writebyte_bc(x2);
	pari2c_stop();

}

void oled_clear(void)
{
	ssd1306_set_region(0, 3, 0, 127);
	
	pari2c_start();
	pari2c_writebyte_bc(OLED_SLADDR);
	pari2c_writebyte_bc(0x40);
	for (uint8_t i=0; i<128; i++)
	{
		pari2c_writebyte_bc(0x00);
		pari2c_writebyte_bc(0x00);
		pari2c_writebyte_bc(0x00);
		pari2c_writebyte_bc(0x00);
	}

	pari2c_stop();
	
}



void oled_init(void)
{
	PORTD |= (1<<7);
	DDRD  |= (1<<7); // /RES = OUT, HIGH
	PORTD &= ~(1<<6);
	DDRD  |=  (1<<6); // POWER = OUT, LOW
	
	pari2c_init();
	
	_delay_ms(100);
	PORTD |=  (1<<6); // power 7.5V on
	
	_delay_ms(200);
	ssd1306_init();
	oled_clear();
	
	// fill display memory with empty default values
	for (uint8_t digit=0; digit<12; digit++)
	{
		oled_chars[digit] = 32; // "space"
		oled_dots [digit] = 0; // annuciators / punctuation = OFF
	}
}

void oled_power(bool on)
{
	if (on)
		PORTD |=  (1<<6);
	else
		PORTD &= ~(1<<6);
}

// To ease layout and avoid trace crossings, the displays have been ordered in a
// strange order. Below table enables to get the ordering correct
const uint8_t oled_reloc[12] PROGMEM = {11, 10, 9, 8, 7, 4, 3, 2, 1, 0, 6, 5};
	
void oled_repaint_digits(uint8_t yposSegment)
{
	uint8_t dots, writeval, dots_all[12];
	uint16_t segments;
	uint16_t segments_all[12];
	
	uint8_t i, j;
	
	j = 11; i = pgm_read_byte( &(oled_reloc[j])); segments_all[i] = pgm_read_word( &(hp_charset[ oled_chars[j] ]) ); dots_all[i]     = oled_dots[j];
	j = 10; i = pgm_read_byte( &(oled_reloc[j])); segments_all[i] = pgm_read_word( &(hp_charset[ oled_chars[j] ]) ); dots_all[i]     = oled_dots[j];
	j =  9; i = pgm_read_byte( &(oled_reloc[j])); segments_all[i] = pgm_read_word( &(hp_charset[ oled_chars[j] ]) ); dots_all[i]     = oled_dots[j];
	j =  8; i = pgm_read_byte( &(oled_reloc[j])); segments_all[i] = pgm_read_word( &(hp_charset[ oled_chars[j] ]) ); dots_all[i]     = oled_dots[j];
	j =  7; i = pgm_read_byte( &(oled_reloc[j])); segments_all[i] = pgm_read_word( &(hp_charset[ oled_chars[j] ]) ); dots_all[i]     = oled_dots[j];
	j =  6; i = pgm_read_byte( &(oled_reloc[j])); segments_all[i] = pgm_read_word( &(hp_charset[ oled_chars[j] ]) ); dots_all[i]     = oled_dots[j];
	j =  5; i = pgm_read_byte( &(oled_reloc[j])); segments_all[i] = pgm_read_word( &(hp_charset[ oled_chars[j] ]) ); dots_all[i]     = oled_dots[j];
	j =  4; i = pgm_read_byte( &(oled_reloc[j])); segments_all[i] = pgm_read_word( &(hp_charset[ oled_chars[j] ]) ); dots_all[i]     = oled_dots[j];
	j =  3; i = pgm_read_byte( &(oled_reloc[j])); segments_all[i] = pgm_read_word( &(hp_charset[ oled_chars[j] ]) ); dots_all[i]     = oled_dots[j];
	j =  2; i = pgm_read_byte( &(oled_reloc[j])); segments_all[i] = pgm_read_word( &(hp_charset[ oled_chars[j] ]) ); dots_all[i]     = oled_dots[j];
	j =  1; i = pgm_read_byte( &(oled_reloc[j])); segments_all[i] = pgm_read_word( &(hp_charset[ oled_chars[j] ]) ); dots_all[i]     = oled_dots[j];
	j =  0; i = pgm_read_byte( &(oled_reloc[j])); segments_all[i] = pgm_read_word( &(hp_charset[ oled_chars[j] ]) ); dots_all[i]     = oled_dots[j];
	
	ssd1306_set_region(0, 3, yposSegment, yposSegment+50-1);

	pari2c_start();
	pari2c_writebyte_bc(OLED_SLADDR);
	pari2c_writebyte_bc(0x40);
	

	// Draw 7 segment digits including colon/dot/comma
	uint16_t idx = 0;
	uint8_t val0, val1, idxs[3], dat[3];
	while (true)
	{
		val0 = pgm_read_byte(&(segmentmap[idx++]));
		if (val0 == 0xff)
			break;
		val1   = pgm_read_byte(&(segmentmap[idx++]));
		dat[0] = pgm_read_byte(&(segmentmap[idx++]));
		dat[1] = pgm_read_byte(&(segmentmap[idx++]));
		dat[2] = pgm_read_byte(&(segmentmap[idx++]));
		
		idxs[0] = val0 >> 4;
		idxs[1] = val1 >> 4;
		idxs[2] = val1 & 0xf;

		// construct I2c writebytes		
		for (uint8_t segmentno = 0; segmentno < 12; segmentno++)
		{
			segments = segments_all[segmentno];
			dots     = dots_all [segmentno];
			
			writeval = 0;
			if (idx >= kommalimits[2])
				segments |= ((((uint16_t)dots>>2) & 0x01)<<15);
			else if (idx >= kommalimits[1])
				segments |= ((((uint16_t)dots>>1) & 0x01)<<15);
			else if (idx >= kommalimits[0])
				segments |= ((((uint16_t)dots>>0) & 0x01)<<15);
		
			// build current write value (max 3 segments per writebyte)
			if ( ((uint16_t)1<<idxs[0]) & segments )
				writeval |= dat[0];
			if ( ((uint16_t)1<<idxs[1]) & segments )
				writeval |= dat[1];
			if ( ((uint16_t)1<<idxs[2]) & segments )
				writeval |= dat[2];
				
			pari2c_writebytes[segmentno] = writeval;
		}

		// handle repeat counter
		do
		{
			pari2c_writebyte();
			val0--;
		} while ((val0 & 0xf) < 15);

		
	}
	pari2c_stop();
}


void oled_repaint_annuciators(uint8_t yposAnnuciators)
{
	// Draw annuciators
	ssd1306_set_region(0, 3, yposAnnuciators, yposAnnuciators+11-1);
	pari2c_start();
	pari2c_writebyte_bc(OLED_SLADDR);
	pari2c_writebyte_bc(0x40);
	for (uint8_t bytecount=0; bytecount < (11*4); bytecount++)
	{
		for (uint8_t segmentno=0; segmentno < 12; segmentno++)
		{
			uint8_t *ptr;
			bool ison;
						
			
			ptr = pgm_read_word( &(annuciators[ 11-segmentno ]) );

			ison = !!(oled_dots[ segmentno ] & (1<<4));

			if (ison)
				pari2c_writebytes[ pgm_read_byte( &(oled_reloc[segmentno])) ] = pgm_read_byte( &(ptr[bytecount]) );
			else
				pari2c_writebytes[ pgm_read_byte( &(oled_reloc[segmentno])) ] = 0;
		}
		pari2c_writebyte();
	}
	
	pari2c_stop();


}

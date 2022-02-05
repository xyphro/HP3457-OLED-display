
#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include "pari2c.h"


//************************************************************************
// PC5 = common SCL line
// PC4, PC3, PC2, PC1, PC0, PC7, PC6 = SDA0..6
// PB2, PB1, PB0, PB7, PB6           = SDA7..11
//************************************************************************

// port masks (1 = bit is used as SDA line
#define PARI2C_MASK_PORTC (0b00011111)
#define PARI2C_MASK_PORTB (0b11011111)

uint8_t pari2c_writebytes[16];

#define PARI2C_SCL_HIGH {PORTC |=  (1<<5);}
#define PARI2C_SCL_LOW  {PORTC &= ~(1<<5);}
	
#define PARI2C_SDA_HIGH_ALL {PORTC |=  PARI2C_MASK_PORTC; PORTB |=  PARI2C_MASK_PORTB;}
#define PARI2C_SDA_LOW_ALL  {PORTC &= ~PARI2C_MASK_PORTC; PORTB &= ~PARI2C_MASK_PORTB;}

#define Q_DEL _delay_loop_2(3)
#define H_DEL _delay_loop_2(5)



void pari2c_init(void)
{
	PORTC |= PARI2C_MASK_PORTC;
	DDRC  |= PARI2C_MASK_PORTC; // all PORTC SDA lines = OUT, HIGH
	PORTB |= PARI2C_MASK_PORTB;
	DDRB  |= PARI2C_MASK_PORTB; // all PORTB SDA lines = OUT, HIGH
	
	PORTC |= (1<<5);
	DDRC  |= (1<<5); // SCL = OUT, HIGH
}

void pari2c_start(void)
{
	PARI2C_SCL_HIGH;
	H_DEL;
	
	PARI2C_SDA_LOW_ALL;
	H_DEL;
}

void pari2c_stop(void)
{
	PARI2C_SDA_LOW_ALL;
	H_DEL;
	PARI2C_SCL_HIGH;
	Q_DEL;
	PARI2C_SDA_HIGH_ALL;
	H_DEL;
}

void pari2c_writebyte_bc(uint8_t data)
{
	uint8_t i;
	
	for(i=0; i<8; i++)
	{
		PARI2C_SCL_LOW;
		Q_DEL;
		
		if (data & 0x80)
			PARI2C_SDA_HIGH_ALL
		else
			PARI2C_SDA_LOW_ALL;
		
		H_DEL;
		
		PARI2C_SCL_HIGH;
		H_DEL;
		
		data=data<<1;
	}
	
	//The 9th clock (ACK Phase)
	PARI2C_SCL_LOW;
	Q_DEL;
	
	DDRC &= ~PARI2C_MASK_PORTC;
	DDRB &= ~PARI2C_MASK_PORTB;
	PORTC |= PARI2C_MASK_PORTC;
	PORTB |= PARI2C_MASK_PORTB;
	
	PARI2C_SDA_HIGH_ALL;
	H_DEL;
	
	PARI2C_SCL_HIGH;
	H_DEL;
	
	PARI2C_SCL_LOW;
	H_DEL;
	
	DDRC |= PARI2C_MASK_PORTC;
	DDRB |= PARI2C_MASK_PORTB;
}

void pari2c_writebyte(void)
{
	uint8_t i, j, bitvalue;
	uint8_t mask, pb, pc;
	
	bitvalue = 0x80;
	for(i=0; i<8; i++)
	{
		PARI2C_SCL_LOW;
		//Q_DEL;
		//
		
		pb = PORTB & ~PARI2C_MASK_PORTB;
		pc = PORTC & ~PARI2C_MASK_PORTC;
		mask=1;
		
		j = 0; if (pari2c_writebytes[ 0] & bitvalue) pb |=  (1<<j);
		j = 1; if (pari2c_writebytes[ 1] & bitvalue) pb |=  (1<<j);
		j = 2; if (pari2c_writebytes[ 2] & bitvalue) pb |=  (1<<j);
		j = 3; if (pari2c_writebytes[ 3] & bitvalue) pb |=  (1<<j);
		j = 4; if (pari2c_writebytes[ 4] & bitvalue) pb |=  (1<<j);
		j = 6; if (pari2c_writebytes[ 5] & bitvalue) pb |=  (1<<j);
		j = 7; if (pari2c_writebytes[ 6] & bitvalue) pb |=  (1<<j);
		j = 0; if (pari2c_writebytes[ 7] & bitvalue) pc |=  (1<<j);
		j = 1; if (pari2c_writebytes[ 8] & bitvalue) pc |=  (1<<j);
		j = 2; if (pari2c_writebytes[ 9] & bitvalue) pc |=  (1<<j);
		j = 3; if (pari2c_writebytes[10] & bitvalue) pc |=  (1<<j);
		j = 4; if (pari2c_writebytes[11] & bitvalue) pc |=  (1<<j);
		
		
		PORTB = pb;
		PORTC = pc;
		
		PARI2C_SCL_HIGH;
		Q_DEL;
		
		bitvalue >>= 1;
	}
	
	//The 9th clock (ACK Phase)
	PARI2C_SCL_LOW;
	Q_DEL;
	
	DDRC &= ~PARI2C_MASK_PORTC;
	DDRB &= ~PARI2C_MASK_PORTB;
	PORTC |= PARI2C_MASK_PORTC;
	PORTB |= PARI2C_MASK_PORTB;
	
	PARI2C_SDA_HIGH_ALL;
	H_DEL;
	
	PARI2C_SCL_HIGH;
	H_DEL;
	
	PARI2C_SCL_LOW;
	H_DEL;
	
	DDRC |= PARI2C_MASK_PORTC;
	DDRB |= PARI2C_MASK_PORTB;
}
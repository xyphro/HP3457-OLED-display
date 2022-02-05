

#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "hp_charset.h"
#include "hpdispprot.h"

#include "oled.h"



/***********************************************************************
 * Connection scheme:
 * PD0(RXD)    = INA ^ ISA (data) (Actually it would not be required to do a HW XOR, but here it helped to save 
 *                                 money for a 8 line isolator IC. In case you don't want galvanic isolation, you 
 *                                 could optimize the XOR IC away).
 * PD3(INT1)   = SYNC (high = command, low = data)
 * PD4(XCK/T0) = O2 (clock)
 * PD2(INT0)   = PWO (power up)
 **
 * Principle: 
 * The HP3457 sends datagrams that are command + data payload based.
 * The SYNC line identifies if the data is a command (SYNC = HIGH) or data (SYNC = LOW).
 * A command has a bitlength of 10 bits, the data payload is variable in length as function of the received command
 * 
 * PD3 is programmed to generate an interrupt on a rising edge (=start of a command).
 *   Once the interrupt arrives, To will be setup to generate an interrupt for every rising edge of the O2=clock line.
 * This timer interrupt does receive bits and store them for later processing.
 *
 * The actual display content will be flagged as valid so that it can be updated if no command is received within 10 ms
 *
 * Credits to Xi from EEvblog for decoding!
 ***********************************************************************/

// Timeout value based on 7k8125 Hz clock (78 cycles ~= 10ms)
// If no commands are received within this period, the display will be updated
#define TIMEOUT_VALUE (78)


// Below commands are known commands and most of them interpreted. Unknown commands are not handled and get simply ignored
#define	CMD_WRITE_REG_A (0x0a) /* = 0x0A x4 = 0x028 = write registers A for the 12 digits */
#define	CMD_WRITE_REG_B (0x1a) /* = 0x1A x4 = 0x068 = write registers B for the 12 digits */
#define CMD_WRITE_REG_C (0x2a) /* = 0x2A x4 = 0x0A8 = write registers C for the 12 digits */
#define	CMD_ANNUNCIATOR (0xbc) /* = 0xBC x4 = 0x2F0 = write the annunciators (eg the little arrows that are below the digits). The annunciators will follow in the data frame ("INA" line): there is one bit per annunciator, and the top right annunciator is transmitted first */
#define	CMD_TOGGLE_DISP (0xc8) /* = 0xC8 x4 = 0x320 = toggle the display ON/OFF (I don't know why this command is sent everytime (the sequence is always FC, B8, C8, BC, ...), maybe the unknow command B8 tells to switch the display OFF, then the C8 command swithes the display ON?) */
#define	CMD_SELECT_DISP (0xfc) /* = 0xFC x4 = 0x3F0 = select the display if the following data field (eg the data on "INA" line) contains 0xFD (that's always the case) */

volatile uint8_t hpdp_dat = 0;
volatile uint8_t hpdp_expectedsz = 0;
volatile uint8_t hpdp_sz = 0;
volatile bool    hpdp_nextiscmd;
volatile bool	hpdp_cmdisknown;
volatile uint8_t hpdp_msg[6], hpdp_cmd;
volatile uint8_t hpdp_updateflag; // update with information which part of the display should be update



// PD3 interrupt - triggered on every rising edge of the PD3 pin 
ISR(INT1_vect)
{
	TCNT0 = -3; // Skip first 2 bits after SYNC edge
	TCCR0 = (7<<1); // enable Timer 0 (precondition here is, that TCNT is -3)
	hpdp_nextiscmd = true;
}

uint8_t cnt=0;

uint8_t kommaxlate[] = {0, 2, 6, 3}; // none, period, colon, comma

// called for every received bit (occurs at a rate of about 55kHz)
ISR(TIMER0_OVF_vect)
{
	TCNT0 = -1; // ensure that next interrupt occurs
	
	hpdp_dat = hpdp_dat >> 1;
	hpdp_dat |= (PIND & (1<<0)) << 7;
	
	//PORTD |= (1<<5);
	//PORTD &= ~(1<<5);
	
	cnt++;
	if (cnt >= 8)
	{ // reception of a byte (cmd or data) done.
		if (hpdp_nextiscmd)
		{
			bool reset_timeout = false;
			hpdp_cmdisknown = true;
			switch(hpdp_dat)
			{
				case CMD_WRITE_REG_A:
				case CMD_WRITE_REG_B:
				case CMD_WRITE_REG_C:
					hpdp_updateflag |= HPDP_UPDATE_DIGITS; 
					hpdp_expectedsz = 6;
					reset_timeout = true;
					break;
				case CMD_SELECT_DISP:
					hpdp_expectedsz = 1;
					break;
				case CMD_ANNUNCIATOR:
					hpdp_updateflag |= HPDP_UPDATE_ANNUCIATORS; 
					reset_timeout = true; // fallthrough is intended
				case CMD_TOGGLE_DISP:
					hpdp_expectedsz = 2;
					break;
				default:
					hpdp_expectedsz = 0;
					hpdp_cmdisknown = false;
					break;
			}
			hpdp_sz = 0;
			hpdp_cmd = hpdp_dat;
			
			hpdp_nextiscmd = false;
			TCNT0 = -3; // Skip 2 pulses after transition from command to data phase
			if (reset_timeout)
			{
				// Timer 1 is set as generic upcounting counter and used as timeout to flag,
				// that the display should be updated.
				TCNT1H = (0xffff-TIMEOUT_VALUE) >> 8; TCNT1L = (0xffff-TIMEOUT_VALUE) & 0xff; // reset timer
				TCCR1B = (5<<0); // 8 MHz / 1024 = 7k8125 Hz => 16 bit timer value wraps every 8.4s, 8 bit LSB wraps every 32m768s
			}
		}
		else
		{ // we received data
			hpdp_msg[hpdp_sz++] = hpdp_dat;
		}
		cnt = 0;
		hpdp_dat  = 0;
		
		// finished receiving a full command with payload data?
		if (hpdp_sz >= hpdp_expectedsz)
		{
			TCCR0 = (0<<1); // disable Timer 0
			cnt = 0;
			
			if (hpdp_cmdisknown)
			{
				// Trigger command dispatching
				
				if ( (hpdp_cmd == CMD_WRITE_REG_A) )
				{
					for(uint8_t i=0; i<6; i++)
					{
						oled_chars[i*2+0] = (oled_chars[i*2+0] & 0xf0) | (hpdp_msg[i] & 0x0f);
						oled_chars[i*2+1] = (oled_chars[i*2+1] & 0xf0) | (hpdp_msg[i] >> 4);
					}
				} else if ( (hpdp_cmd == CMD_WRITE_REG_B) )
				{					
					for(uint8_t i=0; i<6; i++)
					{
						oled_chars[i*2+0] = (oled_chars[i*2+0] & 0x0f) | ((hpdp_msg[i] << 4 )  & 0x30);
						oled_chars[i*2+1] = (oled_chars[i*2+1] & 0x0f) | ((hpdp_msg[i] & 0xf0) & 0x30);
						
						oled_dots [i*2+0] = (oled_dots [i*2+0] & 0xf0) | kommaxlate[ ((hpdp_msg[i]>>2) & 0x03) ];
						oled_dots [i*2+1] = (oled_dots [i*2+1] & 0xf0) | kommaxlate[ ((hpdp_msg[i]>>6) & 0x03) ];
					}
				}  else if ( (hpdp_cmd == CMD_WRITE_REG_C) )
				{ // bit 4 of characters
					for(uint8_t i=0; i<6; i++)
					{
						uint8_t tmp;
						tmp = (oled_chars[i*2+0] & 0x3f) | ((hpdp_msg[i] & 1) << 6);
						if (tmp >= 80)
							tmp = 32;
						oled_chars[i*2+0] = tmp;
						
						tmp = (oled_chars[i*2+1] & 0x3f) | (((hpdp_msg[i]>>4) & 1) << 6);
						if (tmp >= 80)
							tmp = 32;
						oled_chars[i*2+1] = tmp;
					}
				} else if ( (hpdp_cmd == CMD_ANNUNCIATOR) )
				{ // 12 single bit annuciator values
					uint8_t dat;
					dat = hpdp_msg[0];
					for(uint8_t i=0; i<8; i++)
					{
						if (dat & 1)
							oled_dots[i + 0] |=  (1<<4);
						else
							oled_dots[i + 0] &= ~(1<<4);
						dat >>= 1;
					}

					dat = hpdp_msg[1];
					//dat = 0xff;
					for(uint8_t i=0; i<4; i++)
					{
						if (dat & 1)
							oled_dots[i + 8] |=  (1<<4);
						else
							oled_dots[i + 8] &= ~(1<<4);
						dat >>= 1;
					}

				}
				
			}
		}
	}
}


void hpdp_init(void)
{
	hpdp_updateflag = 0;
	
	PORTD &= ~( (1<<0) | (1<<3) | (1<<4) | (1<<2) ); // all pins = no pullup
	DDRD  &= ~( (1<<0) | (1<<3) | (1<<4) | (1<<2) ); // all pins = input
	
	// Enable rising edge interrupt on PD3 = SYNC
	MCUCR |= (3<<2); // trigger INT1 on rising edge
	GIFR  |= (1<<7); // Clear INT1 interrupt flag
	GICR  |= (1<<7); // enable INT1 (It will stay permanently enabled)
	
	// Setup Timer 0 (Used for clock detection)
	//TCCR0 =   (6<<1); // Clock on falling edge of O2 =PD4(T0) pin
	TCCR0 =   (0<<1); // Clock on falling edge of O2 =PD4(T0) pin
	TCNT0 = -3; // generate a timer interrupt after skipping 2 clock cycles
	TIFR |= (1<<0); // clear any pending interrupt
	TIMSK |= (1<<0); // enable timer0 overflow interrupt (an interrupt occurs when wraparround from 0xff to 0 happens)
	
	sei(); // global interrupt enable
}

uint8_t hpdp_update(void)
{
	uint8_t doupdate = (TIFR & (1<<2));
	if (doupdate)
	{
		TCCR1B = 0; // turn timer off to avoid continous updates
		TIFR |= (1<<2); // clear interrupt to avoid retriggers
		doupdate = hpdp_updateflag;
		hpdp_updateflag ^= doupdate;
	}
	
	return doupdate;
}


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
#define	CMD_WRA12L  (0x0a) /* = 0x0A x4 = 0x028 = write registers A for the 12 digits */
#define	CMD_WRB12L  (0x1a) /* = 0x1A x4 = 0x068 = write registers B for the 12 digits */
#define CMD_WRC12L  (0x2a) /* = 0x2A x4 = 0x0A8 = write registers C for the 12 digits */
#define	CMD_WRAB6L  (0x3a) /* = 0x3A x4 = 0x0E8 = shift left 6 digits and write registers AB for the 6 digits */
#define	CMD_WRABC4L (0x4a) /* = 0x4A x4 = 0x128 = shift left 4 digits and write registers ABC for the 4 digits */
#define	CMD_WRAB6R  (0x5a) /* = 0x5A x4 = 0x168 = shift right 6 digits and write registers AB for the 6 digits */
#define	CMD_WRABC4R (0x6a) /* = 0x6A x4 = 0x1a8 = shift right 4 digits and write registers ABC for the 4 digits */
#define	CMD_WRA1L   (0x7a) /* = 0x7A x4 = 0x1e8 = shift left 1 digit and write registers A for 1 digit */
#define	CMD_WRB1L   (0x8a) /* = 0x8A x4 = 0x228 = shift left 1 digit and write registers B for 1 digit */
#define	CMD_WRC1L   (0x9a) /* = 0x9A x4 = 0x268 = shift left 1 digit and write registers C for 1 digit */
#define	CMD_WRA1R   (0xaa) /* = 0xAA x4 = 0x2a8 = shift right 1 digit and write registers A for 1 digit */
#define	CMD_WRB1R   (0xba) /* = 0xBA x4 = 0x2e8 = shift right 1 digit and write registers B for 1 digit */
#define	CMD_WRAB1L  (0xca) /* = 0xCA x4 = 0x328 = shift left 1 digit and write registers AB for 1 digit */
#define	CMD_WRAB1R  (0xda) /* = 0xDA x4 = 0x368 = shift right 1 digit and write registers AB for 1 digit */
#define	CMD_WRABC1L (0xea) /* = 0xEA x4 = 0x3a8 = shift left 1 digit and write registers ABC for 1 digit */
#define	CMD_WRABC1R (0xfa) /* = 0xFA x4 = 0x3e8 = shift left 1 digit and write registers ABC for 1 digit */
#define	CMD_WRITAN  (0xbc) /* = 0xBC x4 = 0x2F0 = write the annunciators (eg the little arrows that are below the digits). The annunciators will follow in the data frame ("INA" line): there is one bit per annunciator, and the top right annunciator is transmitted first */

#define	CMD_DISOFF       (0x8C) /* = 0x8C x4 = 0x230 = Turn display off */
#define	CMD_DISTOG       (0xC8) /* = 0xC8 x4 = 0x320 = toggle the display ON/OFF (I don't know why this command is sent everytime (the sequence is always FC, B8, C8, BC, ...), maybe the unknow command B8 tells to switch the display OFF, then the C8 command swithes the display ON?) */
#define	CMD_PERSCLT      (0xfc) /* = 0xFC x4 = 0x3F0 = select the display if the following data field contains 0xF 0xD (that's always the case for HP3457a) */

volatile uint8_t hpdp_dat = 0;
volatile uint8_t hpdp_expectedsz = 0;
volatile uint8_t hpdp_sz = 0;
volatile bool    hpdp_nextiscmd;
volatile bool	hpdp_cmdisknown;
volatile uint8_t hpdp_msg[12], hpdp_cmd;
volatile uint8_t hpdp_updateflag; // update with information which part of the display should be update

volatile uint8_t hdpd_reg_a[12];  // 4 bit registers from that display content will be assembled
volatile uint8_t hdpd_reg_b[12];  // the character code is (reg_c&1<<6) | (reg_b&3)<<4 | reg_a
volatile uint8_t hdpd_reg_c[12];  // the period is encoded in reg_b bit 2 and 3

volatile bool    hpdp_displayselected = true; // used to handle the PERSCLT command - the default value can be incorrect - could not confirm it yet
volatile bool    hpdp_displayon       = true;  // Stores the on/off state of the display - the default value on has to be right, because HP3852a does never send a command to turn the display on

uint8_t cnt=0; // bitcounter for display command receiver

// PD3 interrupt - triggered on every rising edge of the PD3 pin 
ISR(INT1_vect)
{
	if (PIND & (1<<2))
	{ // only decode when PWO signal is high
		TCNT0 = -3; // Skip first 2 bits after SYNC edge
		TCCR0 = (7<<1); // enable Timer 0 (precondition here is, that TCNT is -3)
		hpdp_nextiscmd = true;
		cnt = 0;
	}
}


uint8_t kommaxlate  [] = {0, 2, 6, 3}; // none, period, colon, comma
uint8_t datalenxlate[] = {12, 12, 12, 12, 12, 12, 12, 1, 1, 1, 1, 1, 2, 2, 3, 3}; // payload count of all write command

// below function will trigger a display update 10ms after it was called
static void hpdp_scheduledisplayupdate(void)
{
	hpdp_updateflag |= HPDP_UPDATE_DIGITS;
	hpdp_updateflag |= HPDP_UPDATE_ANNUCIATORS;
	
	// Timer 1 is set as generic up counting counter and used as timeout to flag,
	// that the display should be updated.
	TCNT1H = (0xffff-TIMEOUT_VALUE) >> 8; TCNT1L = (0xffff-TIMEOUT_VALUE) & 0xff; // reset timer
	TCCR1B = (5<<0); // 8 MHz / 1024 = 7k8125 Hz => 16 bit timer value wraps every 8.4s, 8 bit LSB wraps every 32m768s
}

// called for every received bit (occurs at a rate of about 55kHz)
ISR(TIMER0_OVF_vect)
{
	bool reset_timeout = false;
	TCNT0 = -1; // ensure that next interrupt occurs
	if (PIND & (1<<2))
	{ // only decode when PWO signal is high. PWO acts here similar but not exactly to an enable
		hpdp_dat = hpdp_dat >> 1;
		hpdp_dat |= (PIND & (1<<0)) << 7;
	
		cnt++;
		if ( (hpdp_nextiscmd) && (cnt >= 8) )
		{
			hpdp_cmdisknown = true;
			if ((hpdp_dat & 0x0f) == 0x0a)
			{ // command is a write to display registers (CMD_WR*)
				if (hpdp_displayselected) // only update display if it is selected
					hpdp_expectedsz = datalenxlate[hpdp_dat>>4];
				else
				    hpdp_cmdisknown = false;
			}
			else
			{ // another command received
				switch(hpdp_dat)
				{
					case CMD_DISOFF:
						//hpdp_displayon = false; // Note that this command is never sent on HP347a, but DISTOG is. Another unknown command 0x2e0 is always sent before. HP3457a does not work correctly when this is uncommented
						//hpdp_scheduledisplayupdate();
						break;
					case CMD_DISTOG:
						//hpdp_displayon = !hpdp_displayon; // Note that this command is never sent on HP347a, but DISTOG is. Another unknown command 0x2e0 is always sent before. HP3457a does not work correctly when this is uncommented
						//hpdp_scheduledisplayupdate();
						break;
					case CMD_PERSCLT:
						hpdp_expectedsz = 2;
						break;
					case CMD_WRITAN:
						if (hpdp_displayselected) // only update display if it is selected
							hpdp_expectedsz = 3;
						else
						    hpdp_cmdisknown = false;
						break;
					default:
						hpdp_expectedsz = 0;
						hpdp_cmdisknown = false;
						TCCR0 = (0<<1); // disable Timer 0, we don't process any more data
						break;
				}
			}
			hpdp_sz = 0;
			hpdp_cmd = hpdp_dat;
			cnt = 0;
			hpdp_nextiscmd = false;
			TCNT0 = -3; // Skip 2 pulses after transition from command to data phase
			if ((hpdp_cmdisknown) && (hpdp_expectedsz == 0) )
			{ // command has no payload, so turn off further data reception
				hpdp_cmdisknown = false;
				TCCR0 = (0<<1); // disable Timer 0, we don't process any more data
			}
		}
		else if ( (cnt >= 4) && (!hpdp_nextiscmd) )
		{ // we received data. The payload is processed in Nibbles = 4 bit groups and not as bytes
			cnt = 0;
			hpdp_msg[hpdp_sz++] = hpdp_dat >> 4;
			
			// finished receiving a full command with payload data?
			if (hpdp_sz >= hpdp_expectedsz)
			{
				TCCR0 = (0<<1); // disable Timer 0 - we received all payload data
				bool update_display = true;
				// Trigger command dispatching
				switch (hpdp_cmd)
				{
					case CMD_PERSCLT:
						{
							hpdp_displayselected = (hpdp_msg[0] == 0xd) && (hpdp_msg[1] == 0xf);
						}
						break;
					case CMD_WRA12L:
						{
							for (uint8_t i=0; i<12; i++)
								hdpd_reg_a[i] = hpdp_msg[i];
						}
						break;
					case CMD_WRB12L:
						{
							for (uint8_t i=0; i<12; i++)
								hdpd_reg_b[i] = hpdp_msg[i];
						}
						break;
					case CMD_WRC12L:
						{
							for (uint8_t i=0; i<12; i++)
								hdpd_reg_c[i] = hpdp_msg[i];
						}
						break;
					case CMD_WRAB6L:
						for (uint8_t i=0; i<6; i++)
						{
							hdpd_reg_a[i]   = hdpd_reg_a[i+6];
							hdpd_reg_b[i]   = hdpd_reg_b[i+6];
							hdpd_reg_a[i+6] = hpdp_msg[i*2  ];
							hdpd_reg_b[i+6] = hpdp_msg[i*2+1];
						}
						break;
					case CMD_WRABC4L:
						{
							for (uint8_t i=0; i<(12-4); i++)
							{
								hdpd_reg_a[i]   = hdpd_reg_a[i+4];
								hdpd_reg_b[i]   = hdpd_reg_b[i+4];
								hdpd_reg_c[i]   = hdpd_reg_c[i+4];
							}
							uint8_t j=8;
							for (uint8_t i=0; i<(4*3); i+=3)
							{
								hdpd_reg_a[j  ] = hpdp_msg[i  ];
								hdpd_reg_b[j  ] = hpdp_msg[i+1];
								hdpd_reg_c[j++] = hpdp_msg[i+2];
							}
						}
						break;
					case CMD_WRAB6R:
						{
							uint8_t j=0;
							for (int8_t i=5; i>=0; i--)
							{
								hdpd_reg_a[i+6] = hdpd_reg_a[i];
								hdpd_reg_b[i+6] = hdpd_reg_b[i];
								hdpd_reg_a[i  ] = hpdp_msg[j++];
								hdpd_reg_b[i  ] = hpdp_msg[j++];
							}
						}
						break;
					case CMD_WRABC4R:
						{
							for (int8_t i=(12-4-1); i>=0; i--)
							{
								hdpd_reg_a[i+4]   = hdpd_reg_a[i];
								hdpd_reg_b[i+4]   = hdpd_reg_b[i];
								hdpd_reg_c[i+4]   = hdpd_reg_c[i];
							}
							uint8_t j=3;
							for (uint8_t i=0; i<(4*3); i+=3)
							{
								hdpd_reg_a[j  ] = hpdp_msg[i  ];
								hdpd_reg_b[j  ] = hpdp_msg[i+1];
								hdpd_reg_c[j--] = hpdp_msg[i+2];
							}
						}
						break;
					case CMD_WRA1L:
						for (uint8_t i=0; i<(12-1); i++)
							hdpd_reg_a[i]   = hdpd_reg_a[i+1];
						hdpd_reg_a[11] = hpdp_msg[0];
						break;
					case CMD_WRB1L:
						for (uint8_t i=0; i<(12-1); i++)
							hdpd_reg_b[i]   = hdpd_reg_b[i+1];
						hdpd_reg_b[11] = hpdp_msg[0];
						break;
					case CMD_WRC1L:
						for (uint8_t i=0; i<(12-1); i++)
							hdpd_reg_c[i]   = hdpd_reg_c[i+1];
						hdpd_reg_c[11] = hpdp_msg[0];
						break;
					case CMD_WRA1R:
						for (int8_t i=12-1-1; i>=0; i--)
							hdpd_reg_a[i+1]   = hdpd_reg_a[i];
						hdpd_reg_a[0] = hpdp_msg[0];
						break;
					case CMD_WRB1R:
						for (int8_t i=12-1-1; i>=0; i--)
							hdpd_reg_b[i+1]   = hdpd_reg_b[i];
						hdpd_reg_b[0] = hpdp_msg[0];
						break;
					case CMD_WRAB1L:
						for (uint8_t i=0; i<(12-1); i++)
						{
							hdpd_reg_a[i]   = hdpd_reg_a[i+1];
							hdpd_reg_b[i]   = hdpd_reg_b[i+1];
						}
						hdpd_reg_a[11] = hpdp_msg[0];
						hdpd_reg_b[11] = hpdp_msg[1];
						break;
					case CMD_WRAB1R:
						for (int8_t i=12-1-1; i>=0; i--)
						{
							hdpd_reg_a[i+1]   = hdpd_reg_a[i];
							hdpd_reg_b[i+1]   = hdpd_reg_b[i];
						}
						hdpd_reg_a[ 0] = hpdp_msg[0];
						hdpd_reg_b[ 0] = hpdp_msg[1];
						break;
					case CMD_WRABC1L:
						for (uint8_t i=0; i<(12-1); i++)
						{
							hdpd_reg_a[i]   = hdpd_reg_a[i+1];
							hdpd_reg_b[i]   = hdpd_reg_b[i+1];
							hdpd_reg_c[i]   = hdpd_reg_c[i+1];
						}
						hdpd_reg_a[11] = hpdp_msg[0];
						hdpd_reg_b[11] = hpdp_msg[1];
						hdpd_reg_c[11] = hpdp_msg[2];
						break;
					case CMD_WRABC1R:
						for (int8_t i=12-1-1; i>=0; i--)
						{
							hdpd_reg_a[i+1]   = hdpd_reg_a[i];
							hdpd_reg_b[i+1]   = hdpd_reg_b[i];
							hdpd_reg_c[i+1]   = hdpd_reg_c[i];
						}
						hdpd_reg_a[ 0] = hpdp_msg[0];
						hdpd_reg_b[ 0] = hpdp_msg[1];
						hdpd_reg_c[ 0] = hpdp_msg[2];
						break;
					case CMD_WRITAN:
						{
							uint16_t dat;
							dat = ((uint16_t)(hpdp_msg[2])<<8) | ((uint16_t)(hpdp_msg[1])<<4) | ((uint16_t)hpdp_msg[0]);
							for(uint8_t i=0; i<12; i++)
							{
								if (dat & 1)
									oled_dots[i] |=  (1<<4);
								else
									oled_dots[i] &= ~(1<<4);
								dat >>= 1;
							}
						}
						break;
					default:
						update_display = false;
				}
				if (update_display)
				{
					hpdp_scheduledisplayupdate();
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
		if (doupdate)
		{
			if (hpdp_displayon)
			{
				for(uint8_t i=0; i<12; i++)
				{
					uint8_t ch;
					ch = ((hdpd_reg_c[i]&1)<<6) | ((hdpd_reg_b[i] & 0x3)<<4) | hdpd_reg_a[i];
					if (ch >= (sizeof(hp_charset)/sizeof(hp_charset[0])))
						ch = 32; // the charactermap of HP has only 80 characters, not full 128
					oled_chars[i] = ch;
					oled_dots [i] = (oled_dots [i] & 0xf0) | kommaxlate[(hdpd_reg_b[i]>>2)];
				}
			}
			else
			{
				for(uint8_t i=0; i<12; i++)
				{
					oled_chars[i] = 32; // space character
					oled_dots [i] = 0;  // no dots/comma/...
				}
				
			}
		}
	}
	
	return doupdate;
}
#ifndef __ANNUCIATORSEL_H
#define __ANNUCIATORSEL_H

#include <stdint.h>
#include <stdbool.h>
#include <avr/pgmspace.h>
#include "icons.h"

#define HP3457_ORIGINAL 0
#define HP3457_ICONS 1
#define HP3478_ICONS 2


#define ANNUCIATORS_PERSONALITY HP3457_ORIGINAL

// This defines for each digit the annuciator icon to be used.
// HP has different generations of display types. Some display the annuciator as text, some as 
// triangles with a printed text underneath the display.
// The annuciator bitmaps are taken from file icons.h
// The definition is done from left side of the display towards the right side.
// Unused icons will be stripped away by the compiler, so they don't consume FLASH memory.
// The Icons are generated using the Python script convertIcons2C.py into icons.h/.c files. 
// If you want to update the bitmaps or add a few, make a new .GIF file (with 1 bit black and white palette) 
// rerun the python script.
const uint8_t * PROGMEM const annuciators[12] PROGMEM = 
{
	
#if ANNUCIATORS_PERSONALITY == HP3457_ORIGINAL	
	icon_triangle, icon_triangle, icon_triangle, icon_triangle, icon_triangle, icon_triangle, icon_triangle, icon_triangle, icon_triangle, icon_triangle, icon_triangle, icon_triangle
#elif   ANNUCIATORS_PERSONALITY == HP3457_ICONS
	icon_sample_big, icon_rem_big, icon_srq_big, icon_adrs_big, icon_acpdc_big, icon_4wo_big, icon_azoff_big, icon_mrng_big, icon_math_big, icon_rear_big, icon_err_big, icon_shift_big
#elif ANNUCIATORS_PERSONALITY == HP3478_ICONS
	icon_srq_big, icon_lstn_big, icon_tlk_big, icon_rmt_big, icon_math_big, icon_azoff_big, icon_2ohm_big, icon_4ohm_big, icon_mrng_big, icon_strg_big, icon_cal_big, icon_shift_big
#else
	#error "#define ANNUCIATORS_PERSONALITY of display (HP3457_ORIGINAL, HP3457_ICONS or HP3478_ICONS)"
#endif

};


#endif
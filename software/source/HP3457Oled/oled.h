#ifndef __OLED_H
#define __OLED_H

#include <stdint.h>
#include <stdbool.h>

extern uint8_t oled_chars[12];
extern uint8_t  oled_dots[12];

void oled_init(void);

void oled_power(bool on);

void oled_repaint_digits(uint8_t yposSegment);
void oled_repaint_annuciators(uint8_t yposAnnuciators);

#endif
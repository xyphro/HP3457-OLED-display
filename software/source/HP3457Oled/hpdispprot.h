#ifndef __HPDISPPROT_H
#define __HPDISPPROT_H

#include <stdint.h>
#include <stdbool.h>

#define HPDP_UPDATE_DIGITS      (1<<0)
#define HPDP_UPDATE_ANNUCIATORS (1<<1)


void hpdp_init(void);

// check if the display should be updated and which part
uint8_t hpdp_update(void);

#endif
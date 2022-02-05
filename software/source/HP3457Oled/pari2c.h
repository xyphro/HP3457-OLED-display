#ifndef __PARI2C_H
#define __PARI2C_H

#include <stdint.h>
#include <stdbool.h>

extern uint8_t pari2c_writebytes[16];

void pari2c_init(void);
void pari2c_start(void);
void pari2c_stop(void);
void pari2c_writebyte_bc(uint8_t data);
void pari2c_writebyte(void);


#endif
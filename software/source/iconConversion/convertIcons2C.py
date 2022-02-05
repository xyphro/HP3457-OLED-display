import os
import numpy as np
from PIL import Image

files = os.listdir() 

hdat = '#ifndef __ICONS_H\n#define __ICONS_H\n\n#include <stdint.h>\n#include <avr/pgmspace.h>\n\n'
sdat = '#include "icons.h"\n\n'
for file in files:
    if file.endswith('.gif') and file.startswith('icon_'):
        print(file)
        im = Image.open(file)
        
        variablename = file.split('.')[0]
        
        hdat = hdat + 'extern const uint8_t %s[] PROGMEM;\n' % variablename
        
        sdat = sdat + 'const uint8_t %s[] PROGMEM = \n' % variablename
        sdat = sdat + '{\n'
        for col in range(0, 4):
            sdat = sdat + '\t'
            for y_ in range(0, im.size[1]):
                y = im.size[1]-1-y_
    
                dat = 0
                for x_ in range(0, 8):
                    x = col*8+x_
                    c = im.getpixel((x, y))
                    if c == 1:
                        dat = dat | (1<<x_)
                        
                sdat = sdat + '0x%02x, ' % dat
            
            sdat = sdat + ' // column #%2d\n' % col
        sdat = sdat + '};\n\n'
       
hdat = hdat + '\n#endif'

f = open('..\HP3457Oled\icons.c', 'w')
f.write(sdat)
f.close()

f = open('..\HP3457Oled\icons.h', 'w')
f.write(hdat)
f.close()

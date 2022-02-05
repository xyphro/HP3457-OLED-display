import numpy as np
from PIL import Image
im = Image.open('segmentsV2.gif')

komma = 7
dot1 = 1
dot2 = 4
empty = 15

# Colormap translation table:
idx = 0
xlat = []
for i in range(0, 20):
    if  (i == dot1) or (i == dot2) or (i == komma):
        newidx = 15
        pass;
    elif (i ==empty):
        newidx = -1
    else:
       newidx = idx
       idx = idx + 1
    xlat.append(newidx);
        

# translate pixel data
height = im.size[1]
width  = im.size[0]

a = im.tobytes()

counter = 0
inkomma = False
previnkomma = False
out = []
xcols = []
kommalimit = []
prevvalues = []; prevcnt = 0
for col in range(0, 4):
    for y_ in range(0, height):
        y = height-1-y_ # mirror
        
        cols = []
        for x_ in range(0, 8):
            x = col*8 + x_
            color = xlat[im.getpixel((x, y))]
            cols.append(color)

        uniquecols = list( np.sort(np.unique(cols)) )
        if uniquecols[0] == -1:
            uniquecols= uniquecols[1:]
        while len(uniquecols) < 3:
            uniquecols.append(-1)            
        
        dats= [0, 0, 0]
        for x_ in range(0, 8):
            
            for d in range(0, 3):
                dat = dats[d]
                if (cols[x_] == uniquecols[d]) and (uniquecols[d] >= 0):
                    dat = dat | (1 << x_);
                dats[d] = dat
                
        xcols.append(uniquecols[0])
        xcols.append(uniquecols[1])
        xcols.append(uniquecols[2])
          
        if uniquecols[0] == -1:
            uniquecols[0] = 0;
        if uniquecols[1] == -1:
            uniquecols[1] = 0;
        if uniquecols[2] == -1:
            uniquecols[2] = 0;
            

        newvalues = []
        newvalues.append((uniquecols[0]<<4) | 0)
        newvalues.append((uniquecols[1]<<4) | uniquecols[2])
        newvalues.append(dats[0])
        newvalues.append(dats[1])
        newvalues.append(dats[2])
        
        # Compress (count repetition)
        if len(prevvalues) == 0:
            isequal = False
        else:
            isequal = True
            for i in range(0, 5):
                if ( (newvalues[i] == prevvalues[i]) and (i > 0)) or ( (i == 0) and ((newvalues[0] & 0xf0) == (prevvalues[0] & 0xf0) )):
                    pass;
                else:
                    isequal = False
                    
        #isequal = False
        
        counterbackup = counter
        
        if not isequal:
            out.extend(newvalues)
            counter = counter + 1
        else:
            cnt = out[-5] & 0xf;
            if cnt == 15:
                out.extend(newvalues)
                counter = counter + 1
            else:
                out[-5] = out[-5] +1
        prevvalues = newvalues
        
        inkomma = ((out[-5]>>4) == 15) or ((out[-4]>>4) == 15) or ((out[-4] & 0xf) == 15)
        if not previnkomma and inkomma:
            kommalimit.append(counterbackup)
        previnkomma = inkomma
        
        
            
        
# Generate C array code:
        
s =     '// idx0 | repetition\n'
s = s + '// idx1 | idx2\n'
s = s + '// pixel[idx0]\n'
s = s + '// pixel[idx1]\n'
s = s + '// pixel[idx2]\n'
s = s + 'const uint8_t segmentmap[] PROGMEM = {\n'
i = 0
line = ''
for o in out:
    line = line + '0x%02x, ' % o
    
    if len(line) >= 6*5:
        s = s + line + '\n'
        line = ''
if len(line) > 0:
    s = s + line + '\n'
    line = ''
s = s + '0xff\n'
s = s + '};\n'
    
s = s + 'const uint16_t kommalimits[] = {'
for o in kommalimit:
    s= s+ '0x%04x, ' % (o*5)
s = s + '};\n'


#print(s)
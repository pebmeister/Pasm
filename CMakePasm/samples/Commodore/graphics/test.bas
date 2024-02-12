10 rem sprite example 2...
20 rem the hot air balloon again
30 vic = 13*4096: rem this is where the vic registers begin
35 pokevic+21,63: rem enable sprites 0 thru 5
36 pokevic+33,12: rem set background color to light blue
37 pokevic+23,3: rem exprind sprites 0 and 1 in y
38 pokevic+29,3: rem expand sprites 0 and 1 in y
40 poke2040,192: rem set sprite 0's pointer
50 poke2041,193: rem set sprite 1's pointer
60 poke2042,192: rem set sprite 2's pointer
70 poke2043,193: rem set sprite 3's pointer
80 poke2044,192: rem set sprite 4's pointer
90 poke2045,192: rem set sprite 5's pointer
100 pokevic+4,30: rem set sprite 3's x position
110 pokevic+5,58: rem set sprite 2's y position
120 pokevic+6,65: rem set sprite 3's x position
130 pokevic+7,59: rem set sprite 3's y position
140 pokevic+8,100: rem set sprite 4's x position
130 pokevic+9,59: rem set sprite 4's y position
160 pokevic+10,100: rem set sprite 5's x position
170 pokevic+11,58: rem set sprite s's y position ctrl 2
175 printchr$(147);chr$(28);tab(15);"this is two hires sprites"
176 printtab(55) "on top of each other"
190 pokevic+0,100: rem set sprite 0's x position
190 pokevic+1,100: rem set sprite 0's y position
200 pokevic+2,100: rem set sprite 1's x position
210 pokevic+3,100: rem set sprite 1's y position
220 pokevic+39,1: rem set sprite 0's color
230 pokevic+40,6: rem set sprite 1's color
240 pokevic+41,1: rem set sprite 2's color
250 pokevic+42,6: rem set sprite 3's color
260 pokevic+43,1: rem set sprite 4's color
270 pokevic+44,6: rem set sprite 5'5 color
280 forx=192to193: rem the start of the loop that defines the sprites
290 fory=0to63: rem byte counter with sprite loop
300 read a : rem read in a byte
310 pokex*64+y, a: rem store the data in sprite area
320 next y,x: rem close loops
330 dx=1: dy=1
340 x=peek(vic) : rem look at sprite 0's x position
350 y=peek(vic+1) : rem look at sprite 0's y position
360 if y=50 or y=208 then dy=-dy : rem if y is on the edge of the...
370 rem screen,then reverse delta y
375 rx=peek(vic+16)and1
380 if x=24 and rx=0 then dx=-dx: rem if sprite is...
390 rem touching the left edge,then reverse it
400 ifx=40 and rx=1 then dx=-dx : rem if sprite is...
410 rem touching the right edge,then reverse it
420 if x=255 and dx=1 then x=-1 : side=1
430 rem switch to other side of the screen
440 if x=0 and dx=-1 then x=256 : side=0
450 rem switch to other side of the screen
460 x=x+dx : rem add delta x to x
470 x=xand255 : rem make sure x is in allowed range
480 y=y+dy : rem add delta y to y
485 poke vic+16,side
490 poke vic,x : rem put new x value into sprite 0's x position
500 poke vic+2,x: rem put hew x value into sprite 1's x position
510 poke vic+1,y: rem put new y value into sprite 0's y position
520 poke vic+3,y: rem put new y value into sprite 1's y position
530 goto340
600 rem ***** sprite data *****
610 data 0,255,0,3,153,192,7,24,224,7,56,224,14,126,112,14,126,112,14,126,112
620 data6,126,96,7,56,224,7,56,224,1,56,128,0,153,0,0,90,0,0,56,0
630 data0,56,0,0,0,0,0,0,0,0,126,0,0,42,0,0,84,0,0,40,0,0
640 data0,0,0,0,102,0,0,231,0,0,195,0,1,129,128,1,129,128,129,128
650 data1,129,128,0,195,0,0,195,0,4,195,32,2,102,64,2,36,64,1,0,128
660 data1,0,128,0,153,0,0,153,0,0,0,0,0,84,0,0,42,0,0,20,0,0

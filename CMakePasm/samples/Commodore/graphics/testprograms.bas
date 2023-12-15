10 dim m$(12)
20 rv$= chr$(18) : ro$=chr$(146): cl$=chr$(147) : ho$=chr$(19)
30 ch = 0 : cd$=chr$(17):cu$=chr$(145) : cr$=chr$(13) 
40 for i=0 to 11 : read m$(i) : next
45 print cl$; : hgroff
50 ? ho$; tab(15); chr$(154); "tests"
60 for i = 0 to 12: if ch=i then print rv$; chr$(5);
70 print m$(i);chr$(154)
80 next
90 get a$
100 if a$=cd$ then ch = ch + 1 : goto 400
110 if a$=cu$ then ch = ch - 1 : goto 400
120 if a$<>cr$ then 90
400 d$=a$
410 geta$:ifa$=d$then 410
420 ifch<0thench=0
440 ifch>11thench=10
450 ifd$<>cr$then50 
460 ifch=11 then ?cl$:hgroff:end
500 on ch + 1 gosub 1000,2000,3000,4000,5000,6000,7000,8000,9000,10000,11000
510 geta$:ifa$=""then 510
520 goto 45
1000 hgr:hfcol12:hbcol13:hcls
1010 hbcol1
1020 y = 0
1030 for n = 0 to 7 step 2
1040 hfcol 2 + n : hrect 16 + n ,8 + y*32, 79 - n, 15
1050 hfcol 3 + n : hrect 112 + n + 1 ,8 + y*32, 79 -n -1, 15
1060 y = y + 1
1070 next 
1080 return
2000 hgr:hfcol12:hbcol13:hcls
2010 hbcol1
2020 y = 0
2030 for n = 0 to 7 step 2
2040 hfcol 2 + n : hrect 16, 8 + y*32, 79 + n, 15
2050 hfcol 3 + n : hrect 112 ,8 + y*32, 79 + n + 1, 15
2060 y = y + 1
2070 next
2080 return
3000 hgr:hfcol12:hbcol13:hcls
3010 hbcol 1
3020 y = 0
3030 for n = 0 to 7 step 2
3040 hfcol 2 + n : hrect 16, y  * 32  + 8 + n, 79, 15 -n
3050 hfcol 3 + n : hrect 112, y  * 32  + 8 + n + 1 , 79, 15 - n  -1
3060 y = y + 1
3070 next
3080 return
4000 hgr:hfcol12:hbcol13:hcls
4010 hbcol1
4020 y = 0
4030 for n = 0 to 7 step 2
4040 hfcol 2 + n: hrect 16, y  * 32  + 8, 79, 15 + n
4050 hfcol 3 + n : hrect 112, y  * 32 + 8, 79, 15 + n  + 1
4060 y = y + 1
4070 next
4080 return
5000 hgr:hfcol12:hbcol13:hcls
5010 hbcol1
5020 y = 0
5030 for n = 0 to 7 step 2
5040 hfcol 2 + n : hfrect 16 + n ,8 + y*32, 79 - n, 15
5050 hfcol 3 + n : hfrect 112 + n + 1 ,8 + y*32, 79 -n -1, 15
5060 y = y + 1
5070 next 
5080 return
6000 hgr:hfcol12:hbcol13:hcls
6010 hbcol1
6020 y = 0
6030 for n = 0 to 7 step 2
6040 hfcol 2 + n : hfrect 16, 8 + y*32, 79 + n, 15
6050 hfcol 3 + n : hfrect 112, 8 + y*32, 79 + n + 1, 15
6060 y = y + 1
6070 next
6080 return
7000 hgr:hfcol12:hbcol13:hcls
7010 hbcol1
7020 y = 0
7030 for n = 0 to 7 step 2
7040 hfcol 2 + n : hfrect 16, y  * 32  + 8 + n, 79, 15 -n
7050 hfcol 3 + n : hfrect 112, y  * 32  + 8 + n + 1, 79, 15 - n  -1
7060 y = y + 1
7070 next
7080 return
8000 hgr:hfcol12:hbcol13:hcls
8010 hbcol1
8020 y = 0
8030 for n = 0 to 7 step 2
8040 hfcol 2 + n : hfrect 16, y  * 32  + 8, 79, 15 + n
8050 hfcol 3 + n : hfrect 112, y  * 32 + 8, 79, 15 + n  + 1
8060 y = y + 1
8070 next
8080 return
9000 hgr:hfcol12:hbcol13:hcls
9010 rem hbcol1
9020 xmax=319:ymax=199:xs = int(xmax/2): ys = int(ymax/2)
9030 x1 = 0 : x2 = int(xmax/4) : x3 = int(xmax*3/4) : x4=xmax
9040 y1 = 0 : y2 = int(ymax/4) : y3 = int(ymax*3/4) : y4=ymax
9050 hfcol 2: hline xs,ys,x1,y1
9060 hfcol 3: hline xs,ys,x2,y1
9070 hfcol 4: hline xs,ys,x3,y1
9080 hfcol 5: hline xs,ys,x4,y1
9090 hfcol 6: hline xs,ys,x4,y2
9100 hfcol 7: hline xs,ys,x4,y3
9110 hfcol 8: hline xs,ys,x4,y4
9120 hfcol 9: hline xs,ys,x3,y4
9130 hfcol 10: hline xs,ys,x2,y4
9140 hfcol 11: hline xs,ys,x1,y4
9150 hfcol 14: hline xs,ys,x1,y3
9160 hfcol 15: hline xs,ys,x1,y2
9170 hfcol 2: hline xs,ys,xs,y0
9180 hfcol 3: hline xs,ys,xs,y4
9190 hfcol 4: hline xs,ys,x4,ys
9200 hfcol 5: hline xs,ys,x1,ys
9210 return
10000 hgr:hfcol12:hbcol13:hcls
10010 rem hbcol1
10020 y = 0
10030 for n = 0 to 7 step 2
10040 hfcol 2 + n : hcir 32, y  * 40  + 32 + n, 16
10050 hfcol 3 + n : hcir 112, y  * 40  + 32 + n + 1, 16
10060 y = y + 1
10070 next
10080 return
11000 hgr:hfcol12:hbcol13:hcls
11010 rem hbcol1
11020 y = 0
11030 for n = 0 to 7 step 2
11040 hfcol 2 + n : hfcir 32, y  * 40  + 32 + n, 16
11050 hfcol 3 + n : hfcir 112, y  * 40  + 32 + n + 1, 16
11060 y = y + 1
11070 next
11080 return
20000 data "rect x start","rect x end"
20010 data "rect y start","rect y end"
20020 data "fill rect x start","fill rect x end"
20030 data "fill rect y start","fill rect y end"
20040 data "line", "circle","fill circle", "exit"

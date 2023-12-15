
  SUPERMON+ INSTR #  10000 I  "SUPERMON+ IS A NEW VERSION OF r( "'SUPERMON'  THE REASON FOR THE NEW 2 "VERSION IS TO PROVIDE IDENTICAL À7 "COMMANDS TO THOSE OF THE BUILT-IN ä< "MONITOR OF THE COMMODORE 128. A "  THE MOST VISIBLE CHANGES FROM 3F "EARLIER VERSIONS OF SUPERMON ARE: ^K "  --DECIMAL OR BINARY INPUT ALLOWED; P "  --DISK STATUS AND COMMANDS (@); ¨U "  --LOOSER (EASIER) SYNTAX. ³d  9000 Ñn " NUMBER CONVERSION " âx "   $2000 ï " $2000 ü " +8192 
 " &20000 !  " %10000000000000 'ª  P´ "     IN THE ABOVE EXAMPLE THE USER |¾ "HAS ASKED FOR THE NUMERIC EQUIVALENTS ¨È "TO HEXADECIMAL 2000.  THE REPLY SHOWS ÔÒ "THE VALUE IN HEX ($), IN DECIMAL (+), ûÜ "IN OCTAL (&), AND IN BINARY (%). &æ "  THE USER COULD ASK FOR A NUMBER TO Qð "BE CONVERTED FROM ANY OF THESE BASES yú "BY GIVING THE APPROPRIATE PREFIX. ,9000  6" IMPORTANT NOTE -- " Ë@" AT ANY TIME IN THE FOLLOWING TEXT, öJ"YOU MAY ENTER ANY NUMBER IN ANY BASE !T"AND CONVERSION WILL BE DONE FOR YOU. 0^"EXAMPLE: Bh"   M +4096 jr" ...  WILL CAUSE A MEMORY DISPLAY |"FROM DECIMAL ADDRESS 4096.  IN THE ¿"DISPLAY, THE HEX ADDRESS ($1000) WILL Û"BE SHOWN.  SIMILARLY, ü"   A +2048 LDA #%10000000 &	¤" ... WILL BE CONVERTED TO ASSEMBLE: A	®"   A $0400 LDA #$80 j	¸"   IF YOU DON'T GIVE A PREFIX, THE 	Â"MONITOR WILL ASSUME HEXADECIMAL. 	ô9000 ¸	þ" REGISTER DISPLAY " Å	"   R â	"   PC  SR AC XR YR SP þ	"; 0000 01 02 03 04 05 -
&"     DISPLAYS THE REGISTER VALUES SAVED Y
0"WHEN SUPERMON+ WAS ENTERED.  VALUES 
:"MAY BE CHANGED BY TYPING OVER THE ¯
D"DISPLAY FOLLOWED BY A RETURN CHARACTER. Í
N"  PC - PROGRAM COUNTER ê
X"  SR - STATUS REGISTER b"  AC, XR, YR - A, X, AND  Y REGISTERS 1l"  SP - STACK POINTER ;¼9000 TÐ"   M 200 209 Ú">0200 4D 20 32 30 30 20 32 30:M 200 20" ³ä">0208 39 00 00 04 00 04 00 04:9......." ßî"     DISPLAY MEMORY FROM 0200 HEX TO 	ø"0209 HEX.  DISPLAY IS IN LINES OF 8 4"SO ADDRESSES $200 TO $20F ARE SHOWN. ^"  IF ONE ADDRESS ONLY IS GIVEN THEN "12 LINES (96 LOCATIONS) WILL BE SHOWN. ¸ "IF NO ADDRESS IS GIVEN DISPLAY WILL GO Õ*"FROM THE LAST ADDRESS. 4" EQUIVALENT ASCII CHARACTERS ARE SHOWN !>"IN REVERSE AT THE RIGHT. MH"VALUES ARE CHANGED BY TYPING OVER THE {R"DISPLAY FOLLOWED BY A RETURN CHARACTER. 9000  " EXIT TO BASIC " ­"   X ×¢"     RETURN TO BASIC READY MODE. ¬"WHEN YOU WISH TO RETURN TO SUPERMON+, ¶"COMMAND SYS 8. %è9000 Bò" SIMPLE ASSEMBLER " cü"   A 2000 LDA #+18 {"  (CHANGES  TO:)" "A 2000 A9 12    LDA #$12 »"A 2002 ..NEXT INSTRUCTION å$"     IN THE ABOVE EXAMPLE THE USER ."STARTED ASSEMBLY AT 2000 HEX.  THE :8"FIRST INSTRUCTION WAS LOAD A REGISTER hB"WITH IMMEDIATE 18 DECIMAL. IN FOLLOWING L"LINES THE USER NEED NOT TYPE THE ºV"A AND ADDRESS.  THE SIMPLE ASSEMBLER è`"PROMPTS WITH THE NEXT ADDRESS.  TO EXIT j"THE ASSEMBLER TYPE A RETURN AFTER THE .t"THE ADDRESS PROMPT. W~"  PREVIOUS LINES MAY BE CHANGED BY ~"TYPING OVER THE RIGHT HAND PART. °9000 ¤Ä"   D 2000 2004 ÃÎ". 2000 A9 12    LDA #$12 åØ". 2002 9D 00 80 STA $8000,X â"     DISASSEMBLES INSTRUCTIONS FROM ;ì"2000 TO 2004 HEX.  IF ONE ADDRESS IS gö"GIVEN, 20 BYTES WILL BE DISASSEMBLED.  "IF NO ADDRESS, START FROM THE LAST ¤
"USED ADDRESS. Î"  CODE MAY BE REASSEMBLED BY MOVING ø"THE CURSOR BACK AND TYPING OVER THE ("RIGHT HAND PART. x9000 2" FILL MEMORY " S"   F 1000 1100 FF "     FILLS THE MEMORY FROM 1000 HEX TO ¦ "1100 HEX WITH THE BYTE FF HEX. °Ü9000 Åæ" GO (RUN)" Ùð"   G 1000 ú"     GO TO ADDRESS 1000 HEX AND BEGIN "RUNNING CODE. F"  IF NO ADDRESS IS GIVEN, THE ADDRESS l"FROM THE <PC> REGISTER IS USED. "" JUMP (SUBROUTINE)" ,"   J 1000 È6"     CALL ADDRESS 1000 HEX AND BEGIN ô@"RUNNING CODE.  RETURN TO THE MONITOR. þ¤9000 " HUNT MEMORY " :¢"   H C000 D000 'READ h¬"     HUNT THRU MEMORY FROM C000 HEX TO ¶"D000 HEX FOR THE ASCII STRING READ AND ÅÀ"PRINT THE ADDRESS WHERE IT IS FOUND.  A ñÊ"MAXIMUM OF 32 CHARACTERS MAY BE USED. Ô"   H C000 D000 20 D2 FF IÞ"     HUNT MEMORY FROM C000 HEX TO D000 vè"HEX FOR THE SEQUENCE OF BYTES 20 D2 FF ¤ò"AND PRINT THE ADDRESS.  A MAXIMUM OF 32 ½ü"BYTES MAY BE USED. ÇV	9000 ×`	" LOAD " åa	"   L b	"   LOAD ANY PROGRAM FROM CASSETTE #1. ;c	"    L ";Ç(34);"RAM TEST";Ç(34) gd	"   LOAD FROM CASSETTE #1 THE PROGRAM e	"NAMED RAM TEST. ­j	"   L ";Ç(34);"RAM TEST";Ç(34);",08 Ût	" LOAD FROM DISK (DEVICE 8) THE PROGRAM ó~	"NAMED RAM TEST. 	"THIS COMMAND LEAVES BASIC POINTERS .	"UNCHANGED. 8º	9000 I
" SAVE" 
"   S ";Ç(34);"PROGRAM NAME";Ç(34);",01,0800,0C80" ± 
"     SAVE TO CASSETTE #1 MEMORY FROM Ýª
"0800 HEX UP TO BUT NOT INCLUDING 0C80 ´
"HEX AND NAME IT PROGRAM NAME. A¾
"   S ";Ç(34);"0:PROGRAM NAME";Ç(34);",08,1200,1F50" nÈ
"    SAVE TO DISK DRIVE #0 MEMORY FROM Ò
"1200 HEX UP TO BUT NOT INCLUDING 1F50 ÀÜ
"HEX AND NAME IT PROGRAM NAME. Êæ
9000 èð
" TRANSFER MEMORY " 
ú
"   T 1000 1100 5000 8"     TRANSFER MEMORY IN THE RANGE 1000 f"HEX TO 1100 HEX AND START STORING IT AT ~"ADDRESS 5000 HEX. J9000 ¥T" COMPARE MEMORY " Ç^"   C 1000 1100 5000 ôh"     COMPARE MEMORY IN THE RANGE 1000 r"HEX TO 1100 HEX WITH MEMORY STARTING :|"AT ADDRESS 5000 HEX. D®9000 [¸" CHECK DISK " iÂ"   @ Ì"     GET DISK STATUS MESSAGE Ö"   @9 Çà"     GET DISK UNIT 9 STATUS MESSAGE Øê"   @,$0 úô"     GET DRIVE 0 DIRECTORY þ"   @,S0:TEMP :"     SCRATCH FILE 'TEMP' FROM DISK D9000 a" OUTPUT TO PRINTER" &"CALL SUPERMON+ FROM BASIC WITH: ©0"  OPEN 4,4:CMD 4:SYS 8" Ò:"ALL COMMANDS WILL GO THE PRINTER. ùD"  WHEN COMPLETE, RETURN TO BASIC N"WITH X AND COMMAND: 1X"  PRINT#4:CLOSE 4" ;b9000 h¬"        SUMMARY                     " ±"$,+,&,% NUMBER CONVERSION £¶"G GO (RUN) À»"J JUMP  (SUBROUTINE) áÀ"L LOAD FROM TAPE OR DISK úÊ"M MEMORY DISPLAY Ô"R REGISTER DISPLAY 4Þ"S SAVE TO TAPE OR DISK Lè"X EXIT TO BASIC g"A SIMPLE ASSEMBLER ~"D DISASSEMBLER $"F FILL MEMORY ª."H HUNT MEMORY ÄL"T TRANSFER MEMORY ÝQ"C COMPARE MEMORY ûV"@ DISK STATUS/COMMAND  9000 1ª"SUPERMON WILL LOAD ITSELF INTO THE ^´"TOP OF MEMORY .. WHEREVER THAT HAPPENS {¾"TO BE ON YOUR MACHINE. ¨Ü"BE SURE TO NOTE THE SYS COMMAND WHICH Õæ"LINKS SUPERMON TO THE COMMODORE.  IT  ð"MAY BE USED TO RECONNECT THE MONITOR ,ú"IF IT IS ACCIDENTALLY DISCONNECTED BY U"USE OF THE RUN-STOP/RESTORE KEYS." _9000 h@ 30 «(#" HIT ANY KEY TO CONTINUE "; ½2#I²1¤10:¡A$: Ñ<#¡A$:A$²""§9020 û'"         S U P E R M O N   +   " ' '"COMMANDS - USER INPUT IN  REVERSE " - 8'   
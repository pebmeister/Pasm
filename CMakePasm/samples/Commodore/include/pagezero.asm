.print off

; ======================================================================================
;   Memory locations 0-255 ($0-$FF) have a special significance in 6502
;   machine language programming (the 6510 microprocessor in the
;   Commodore 64 shares the same instruction set as the 6502). Since
;   these addresses can be expressed using just one byte, instructions
;   which access data stored in these locations are shorter and execute
;   more quickly than do instructions which operate on addresses in
;   higher memory, which require two bytes.
;   Because of this relatively fast execution time, most 6502
;   software makes heavy use of zero-page locations. The 64 is no exception,
;   and uses this area for many important system variables and pointers.
;   In addition, locations 0 and 1 have special Input/Output functions on the 6510.
;   In the case of the 64, this on-chip I/O port is
;   used to select the possible combinations of ROM, as we will see below,
;   and to control cassette I/O.
; ======================================================================================

D6510   = $00
; ======================================================================================
;
; D6510         $00
;
; 6510 On-Chip I/O DATA Direction Register
; Bit 0: Direction of Bit 0 I/O on port at next address. Default =1 (output)
; Bit 1: Direction of Bit 1 I/O on port at next address. Default =1 (output)
; Bit 2: Direction of Bit 2 I/O on port at next address. Default=l (output)
; Bit 3: Direction of Bit 3 I/O on port at next address. Default=l (output)
; Bit 4: Direction of Bit 4 I/O on port at next address. Default=0 (input)
; Bit 5: Direction of Bit 5 I/O on port at next address. Default=l (output)
; Bit 6: Direction of Bit 6 I/O on port at next address. Not used.
; Bit 7: Direction of Bit 7 I/O on port at next address. Not used.
;
; This location is the first of a number of hardware registers.
; Although they can be written to and/or read like RAM,
; they are connected to hardware devices, and their contents affect the
; operation of the devices.
; Each bit of this Data Direction Register determines whether the
; contents of the corresponding bit on the Internal I/O Port (see location 1)
; can be written to by peripheral devices. If the bit is set to 0, it
; indicates the direction of data flow as Input, which means that the
; corresponding bit of the I/O Port will be affected by peripheral devices.
; If the bit is set to 1, it indicates Output. On the 64, only Bits 0-5
; are significant. On power-up, this register is set to 239 ($EF),
; which indicates that all bits, except for Bit 4 (which senses the cassette switch),
; are set up for Output.
; ======================================================================================

R6510   = $01
; ======================================================================================
;
; R6510         $01
;
; Bit 0: LORAM signal. Selects ROM or RAM at 40960 ($A000). 1 = BASIC, 0=RAM
; Bit 1: HIRAM signal. Selects ROM or RAM at 57344 ($E000). l=Kernal, 0=RAM
; Bit 2: CHAREN signal. Selects character ROM or I/O devices. 1 =I/O, 0=ROM
; Bit 3: Cassette Data Output line.
; Bit 4: Cassette Switch Sense. Reads 0 if a button is pressed, 1 if not.
; Bit 5: Cassette Motor Control. A 1 turns the motor on, a 0 turns it off.
; Bits 6-7: Not connected
;
; The chief function of this register is to determine which blocks of
; RAM and ROM the 6510 microprocessor will address. The Commodore 64
; comes with 64K RAM, even though it normally does not use
; all of that RAM at once. In addition, it has an 8K BASIC Interpreter
; ROM, an 8K Operating System Kernal ROM, a 4K Character Generator
; ROM, a Sound Interface Device (SID), a 6566 Video Interface
; Controller (VIC-II), and two 6526 Complex Interface Adapter chips.
; To address all of these at once would require 88K, 24K past the
; addressing limit of the 6510 microprocessor. In order to allocate
; address space, the I/O Port is used to affect the addressing lines, and
; thus determine which segments of RAM and ROM will be addressed
; at any one time.
; ======================================================================================

UNUSED1 = $02
; ======================================================================================
;
; UNUSED        $02
;
; unused
; ======================================================================================

ADRAY1  = $03
; ======================================================================================
;
; ADRAY1        $03 - $04
;
; This vector points to the address of the BASIC routine which converts
; a floating point number to an integer. In the current Kernal
; version, the address that it points to is 45482 ($B1AA). Disassembly
; of the ROMs indicates that BASIC does not use this vector. However,
; it may be of real assistance to the programmer who wishes to
; use data that is stored in floating point format. The parameter that is
; passed by the USR command is available only in that format, for example.
; Since it is extremely difficult to decipher and use a floating point
; number, the simplest way to deal with such data is to use the conversion
; routines that are built into BASIC to change it into a two byte
; signed integer. This could be accomplished by jumping directly
; into the BASIC ROM, if you know the location of the routine. It is
; preferable to use this vector because it will always point to the location
; of the routine. Therefore, if the address changes in future versions
; of the 64 or future Commodore computers, you won't have to
; modify your program to make it work with them.
; See the entry for the USR vector at 785 ($311) for an explanation
; of how to use this routine in connection with the USR command.
; ======================================================================================

ADRAY2  = $05
; ======================================================================================
;
; ADRAY2        $05 - $06
;
; Vector: Routine to Convert a Number from Integer to Floating Point
; This vector points to the address of the BASIC routine which converts
; an integer to a floating point number. This routine is currently
; located at 45969 ($B391). BASIC does not appear to reference this
; location. It is available for use by the programmer who needs to
; make such a conversion for a machine language program that interacts
; with BASIC. For an explanation of how to use this routine in
; connection with the USR command, see the entry for the USR vector
; at 785 ($311)
; ======================================================================================

CHARAC  = $07
; ======================================================================================
;
; CHARAC        $07
;
; Search Character for Scanning BASIC Text Input
; This location and the next are used heavily by the BASIC routines
; that scan the text that comes into the buffer at 512 ($200), in order
; to detect significant characters such as quotes, comma, the colon
; which separates BASIC statements, and end-of-line. The ASCII
; values of such special characters are usually stored here.
; This location is also used as a work area by other BASIC
; routines that do not involve scanning text.
; ======================================================================================

ENDCHAR = $08
; ======================================================================================
;
; ENDCHAR       $08
;
; Search Character for Statement Terminator or Quote
; Like location 7, this location is used as a work byte during the
; tokenization of a BASIC statement. Most of the time, its value is 0 or 34.
; ======================================================================================

TRMPOS  = $09
; ======================================================================================
;
; TRMPOS        $09
;
; Column Position of the Cursor before the Last TAB or SPC
; TRMPOS is used by TAB and SPC. The cursor column position prior
; to the TAB or SPC is moved here from 211 ($D3), and is used to calculate
; where the cursor ends up after one of these functions is invoked.
; Note that the value contained here shows the position of the
; cursor on a logical line. Since one logical line can be up to two physical
; lines long, the value stored here can range from 0 to 79.
; ======================================================================================

VERCK   = $0A
; ======================================================================================
;
; VERCK         $0A
;
; Flag: LOAD or VERIFY
; BASIC uses one Kernal routine to perform either the LOAD or VERIFY
; function, depending on whether the Accumulator (.A) is set to 0
; or 1 upon entry to the routine. BASIC sets the value of VERCK to 0
; for a LOAD, or 1 for a VERIFY. Its contents are passed to the Kernal
; LOAD routine, which in turn stores it in location 147 ($93).
; ======================================================================================

COUNT   = $0B
; ======================================================================================
;
; COUNT         $0B
;
; Index into the Text Input Buffer/Number of Array Subscripts
; The routines that convert the text in the input buffer at 512 ($20b)
; into lines of executable program tokens, and the routines that link
; these program lines together, use this location as an index into the
; input buffer area. When the job of converting text to tokens is finished,
; the value in this location is equal to the length of the
; tokenized line.
; The routines which build an array or locate an element in an array
; use this location to calculate the number of DIMensions called
; for and the amount of storage required for a newly created array, or
; the number of subscripts specified when referencing an array element.
; ======================================================================================

DIMFLG  = $0C
; ======================================================================================
;
; DIMFLG        $0C
;
; Flags for the Routines That Locate or Build an Array
; This location is used as a flag by the routines that build an array or
; reference an existing array. It is used to determine whether a variable
; is an array, whether the array has already been DIMensioned, and
; whether a new array should assume the default dimensions.
; ======================================================================================

VALTYP  = $0D
; ======================================================================================
;
; VALTYP        $0D
;
; Flag: Type of Data (String or Numeric)
; This flag is used internally to indicate whether data being operated
; upon is string or numeric. A value of 255 ($FF) in this location indicates
; string data, while a 0 indicates numeric data. This determination
; is made every time a variable is located or created.
; ======================================================================================

INTFLG  = $0E
; ======================================================================================
;
; INTFLG        $0E
;
; Flag: Type of Numeric Data (Integer or Floating Point)
; If data which BASIC is using is determined to be numeric, it is further
; classified here as either a floating point number or as an integer.
; A 128 ($80) in this location identifies the number as an integer, and
; indicates a floating point number.
; ======================================================================================

GARBFLG = $0F
; ======================================================================================
;
; GARBFLG       $0F
;
; Flag for LIST, Garbage Collection, and Program Tokenization
; The LIST routine uses this byte as a flag to let it know when it has
; come to a character string in quotes. It will then print the string,
; rather than search it for BASIC keyword tokens.
; The garbage collection routine uses this location as a flag to indicate
; that garbage collection has already been tried before adding a
; new string. If there is still not enough memory, an OUT OF MEMORY
; message will result.
; This location is also used as a work byte for the process of converting
; a line of text in the BASIC input buffer (512, $200) into a
; linked program line of BASIC keyword tokens.
; ======================================================================================

SUBFLG  = $10
; ======================================================================================
;
; SUBFLG        $10
;
; Flag: Subscript Reference to an Array or User-Defined Function
; Call (FN)
; This flag is used by the PTRGET routine which finds or creates
; a variable, at the time it checks whether the name of a variable is
; valid. If an opening parenthesis is found, this flag is set to indicate
; that the variable in question is either an array variable or a userdefined function.
; You should note that it is perfectly legal for a user-defined function (FN)
; to have the same name as a floating point variable. Moreover, it
; is also legal to redefine a function. Using a FN name in an
; already defined function results in the new definition of the function.
; ======================================================================================

INPFLG  = $11
; ======================================================================================
;
; INPFLG        $11
;
; Flag: Is Data Input to GET, READ or INPUT?
; Since the keywords GET, INPUT, and READ perform similar functions,
; BASIC executes some of the same instructions for all three.
; There are also many areas of difference, however, and this flag indicates
; which of the three keywords is currently being executed, so
; that BASIC will know whether or not to execute the instructions
; which relate to the areas in which the commands differ (152
; ($98) = READ, 64 ($40)= GET, 0=INPUT).
; As a result, INPUT will show the ? prompt, will echo characters
; back to the screen, and will wait for a whole line of text ended by a
; carriage return. GET gives no prompt and accepts one character
; without waiting. The colon character and the comma are valid data
; for GET, but are treated as delimiters between data by INPUT and
; READ. As each command has its own error messages, this flag is used
; to determine the appropriate message to issue in case of an error.
; ======================================================================================

TANSGN  = $12
; ======================================================================================
;
; TANSGN        $12
;
; Flag: Sign of the Result of the TAN or SIN Function
; This location is used to determine whether the sign of the value re
; turned by the functions SIN or TAN is positive or negative.
; Additionally, the string and numeric comparison routines use
; this location to indicate the outcome of the comparison. For a com
; parison of variable A to variable B, the value here will be 1 if A is
; greater than B, 2 if A equals B, and 4 if A is less than B. If more than
; one comparison operator was used to compare the two variables
; (e.g., >= or <= ), the value here will be a combination of the
; above values.
; ======================================================================================

CHANNL  = $13
; ======================================================================================
;
; CHANNL        $13
;
; Current I/O Channel (CMD Logical File) Number
; Whenever BASIC inputs or outputs data, it looks here to determine
; which I/O device is currently active for the purpose of prompting or
; output control. It uses location 184, $B8 for purposes of deciding
; what device actually to input from or output to.
; When the default input device (number 0, the keyboard) or output
; device (number 3, the display screen) is used, the value here will
; be a zero, and the format of prompting and output will be the standard
; screen output format.
; When another device is used, the logical file number (CMD
; channel number) will be placed here. This lets the system know that
; it may have to make some subtle changes in the way it performs the
; I/O operation. For example, if TAB is used with the PRINT command,
; cursor right characters are used if the device PRINTed to is
; the screen. Otherwise, spaces are output when the number here is
; other than zero (the assumption being that you can't tab a printer
; like you can the screen).
; Likewise, the ? prompt for INPUT is suppressed if the file number
; here is nonzero, as is the EXTRA IGNORED message, and input
; of a carriage return by itself is ignored, rather than being treated as a
; null string (""). Therefore, by OPENing the screen as a device, and
; issuing the CMD statement, you can force the suppression of the ?,
; prompt, and the other effects above.
; CMD places the new output file number here, and calls the
; Kernal to open the device for output, leaving it LISTENing for output
; (such as the READY prompt, which is diverted to the new device).
; Many routines reset this location and UNLISTEN the device, defeating
; the CMD and once again sending output to the screen. If an
; error message has to be displayed, for example, this location will be
; reset and the message will be displayed on the screen. GET, GET#,
; INPUT, INPUT#, and PRINT# all will reset this location after the
; I/O is completed, effectively redirecting output back to the screen.
; PRINT and LIST are the only I/O operations that will not undo the CMD.
; This location can also be used to fool BASIC into thinking that
; data it is reading from the tape is actually being entered into the
; keyboard in immediate mode.
; For a look at a technique that uses a different approach to accomplish
; the same thing for disk or tape users, see location 512
; ($200), the keyboard buffer.
; ======================================================================================

LINNUM  = $14
; ======================================================================================
;
; LINNUM        $14
;
; Integer Line Number Value
; The target line number for GOTO, LIST, ON, and GOSUB is stored
; here in low-byte, high-byte integer format, as is the number of a
; BASIC line that is to be added or replaced.
; LIST saves the highest line number to list (or 65535; program is to be listed to the end) at this location. < 1
; GOTO tests the target line number to see if it is greater than the
; line number currently being executed. If it is greater, GOTO starts its
; search for the target line at the current line number. If it is not J
; greater, GOTO must search for the target line from the first line of
; the program. It is interesting to note that the test is of the most
; significant byte only. Therefore, INT (TARGETLINE/256) must be
; greater than INT(CURRENT LINE/256) in order for the search to
; start with the current line, instead of at the beginning of the
; program. PEEK, POKE, WAIT, and SYS use this location as a pointer to
; the address which is the subject of the command.
; ======================================================================================

TEMPPT  = $16
; ======================================================================================
;
; TEMPPT        $16
;
; Pointer to the Next Available Space in the Temporary String Stack
; This location points to the next available slot in the temporary string
; descriptor stack located at 25-33 ($19-$21). Since that stack has room
; for three descriptors of three bytes each, this location will point to 25
; ($19) if the stack is empty, to 28 ($1C) if there is one entry, to 31
; ($1F) if there are two entries, and to 34 ($22) if the stack is full.
; If BASIC needs to add an entry to the temporary string descriptor
; stack, and this location holds a 34, indicating that the stack is
; full, the FORMULA TOO COMPLEX error message is issued. Otherwise,
; the entry is added, and three is added to this pointer.
; ======================================================================================

LASTPT  = $17
; ======================================================================================
;
; LASTPT        $17
;
; Pointer to the Address of the Last String in the Temporary String Stack
; This pointer indicates the last slot used in the temporary string descriptor
; stack. Therefore, the value stored at 23 ($17) should be 3
; less than that stored at 22 ($16), while 24 ($18) will contain a 0.
; ======================================================================================

TEMPST  = $19
; ======================================================================================
;
; TEMPST        $19
;
; Descriptor Stack for Temporary Strings
; The temporary string descriptor stack contains information about
; temporary strings which have not yet been assigned to a string variable.
; An example of such a temporary string is the literal string
; "HELLO" in the statement PRINT "HELLO".
; Each three-byte descriptor in this stack contains the length of
; the string, and its starting and ending locations, expressed as displacements
; within the BASIC storage area.
; ======================================================================================

INDEX   = $22
; ======================================================================================
; Miscellaneous Temporary Pointers and Save Area
; This area is used by many BASIC routines to hold temporary pointers
; and calculation results.
; ======================================================================================

RESHO   = $26                           ;   floating point work area 26-2a
TXTTAB  = $2B                           ;   basic start 2b-2c
VARTAB  = $2D                           ;   pointer to start of the basic variable table 2d-2e
ARYTAB  = $2F                           ;   pointer to start of basic array storage 2f-30
STREND  = $31                           ;   pointer to endof basic array storage area + 1 32-32
FRETOP  = $33                           ;   pointer to bottom of the string text storage area 33-34
FRESPC  = $35                           ;   temporary pointer for strings 35-36
MEMSIZ  = $37                           ;   pointer to highest address used by basic 37-38
CURLIN  = $39                           ;   current line number (FF in $3a signifies immediate mode) $39-3a
OLDLIN  = $3B                           ;   previous line number (used by cont) 3b-3c
OLDTXT  = $3D                           ;   pointer to current basic statement 3d-3e
DATLIN  = $3F                           ;   current data line number 3f-40
DATPTR  = $41                           ;   pointer to current data item 42-42
INPPTR  = $43                           ;   pointer to source of get read or input info 43-44
VARNAM  = $45                           ;   pointer to current basic variable name 46-46
VARPNT  = $47                           ;   pointer to current basic variable value 47-48
FORPNT  = $49                           ;   temporary pointer to index used by for 49-4a
OPPTR   = $4B                           ;   math operator table displacent 4b-4c
OPMASK  = $4D                           ;   math comparison operation mask
DEFPNT  = $4E                           ;   pointer to current FN description 4e-4f
DSCPNT  = $50                           ;   temporary pointer to current string description 50-52
FOUR6   = $53                           ;   constant for garbage collection
JMPER   = $54                           ;   jump to function instruction 84-86
UNUSED2 = $57                           ;   numeric work area 57-60
FAC1    = $61                           ;   fac1 61-66
FACEXP  = $61                           ;   fac1 exponent
FACHO   = $62                           ;   fac1 mantissa 62-65
FACSSGN = $66                           ;   fac1 sign
SGNFLG  = $67                           ;   number of terms in series evaluation
BITS    = $68                           ;   overflow digit
FAC2    = $69                           ;   fac2 69-6e
ARGEXP  = $69                           ;   fac2 exponent
ARGHO   = $6A                           ;   fac2 mantissa 6a-6d
ARGSGN  = $6E                           ;   fac2 sign
FACOV   = $70                           ;   low order mantissa fac1
FBUFPTR = $71                           ;   series evaluation pointer 71-72
CHRGET  = $73                           ;   subroutine get next basic text char 73-8a
CHRGOT  = $79                           ;   TXTPTR is already updated then call this from wedge
TXTPTR  = $7A                           ;
POINTB  = $7C                           ;
RNDX    = $8B                           ;   random seed value 8b-8f
STATUS  = $90                           ;   kernal io status
STKEY   = $91                           ;   was stop key pressed
SVXT    = $92                           ;   timing constant for tape
VERCK2  = $93                           ;   flag for load or verify 0 = load 1 = verify
C3PO    = $94                           ;   serial bus output char was buffered
BSOUR   = $95                           ;   bufferd char for serial bus
SYNO    = $96                           ;   cassette block syncronization number
XSAV    = $97                           ;   temporary x register save
LDTND   = $98                           ;   number of open io files
DFLTN   = $99                           ;   default input device (0=keyboard)
DFLTO   = $9A                           ;   default output device (3 = screen)
PRTY    = $9B                           ;   tape parity
DPSW    = $9C                           ;   tape byte received
MSGFLG  = $9D                           ;   kernal message control
PTR1    = $9E                           ;   tape pass1 error log
PTR2    = $9F                           ;   tape pass2 error log
TIME    = $A0                           ;   jiffy clock a0-a2
TMPDATA = $A3                           ;   temp data area a3-a4
CNTDN   = $A5                           ;   cassette syncronzation countdown
BUFPNT  = $A6                           ;   count of charactors in tape io buffer
INBIT   = $A7                           ;   cassette temp storage
BITCI   = $A8                           ;   rs232   - cassette temp storage
RINONE  = $A9                           ;   rs232 check for start bit
RIDATA  = $AA                           ;   rs232 input byte
RIPRTY  = $AB                           ;   rs232 input parity
SAL     = $AC                           ;   start address of screen load/scrolling ac-ad
EAL     = $AE                           ;   end of program ae-af
CMP0    = $B0                           ;   tape timing b0-b1
TAPE1   = $B2                           ;   start of tape buffer b2-b3
BITTS   = $B4                           ;   rs232 bit counter
NXTBIT  = $B5                           ;   rs232 next bit to send
RODATA  = $B6                           ;   rs232 output byte buffer
FNLEN   = $B7                           ;   length of current filename
LA      = $B8                           ;   current logical file number
SA      = $B9                           ;   current secondary address
FA      = $BA                           ;   current device number
FNADR   = $BB                           ;   pointer current file name bb-bc
ROPRTY  = $BD                           ;   rs232 output parity
FSBLK   = $BE                           ;   cassette read write block count
MYCH    = $BF                           ;   tape input buffer
CAS1    = $C0                           ;   tape motor interlock
STAL    = $C1                           ;   io start address c1-c2
MEMUSS  = $C3                           ;   tape load temp address c3-c4
LSTX    = $C5                           ;   matrix coordinate of last key pressed
NDX     = $C6                           ;   number of characters in keyboard buffer
RVS     = $C7                           ;   print characters in reverse
INDX    = $C8                           ;   end of logical line input
LXSP    = $C9                           ;   cursor at start of input c9-ca
SFDX    = $CB                           ;   matrix coordinate of current key pressed
BLNSW   = $CC                           ;   cursor blink enable
BLNCT   = $CD                           ;   countdown to blink cursor
GDBLN   = $CE                           ;   character under cursor
BLNON   = $CF                           ;   was the last charactor blink on or off
CRSW    = $D0                           ;   input from keyboard or screen
PNT     = $D1                           ;   pointer address of current screen line d1-d2
PNTR    = $D3                           ;   cursor column on on current line
QTSW    = $D4                           ;   editor in quote mode?
LNMX    = $D5                           ;   msx length of physical screen line
TBLX    = $D6                           ;   current cursor physical line number
UNUSED3 = $D7                           ;   temp stprage area for ascii value of last char
INSRT   = $D8                           ;   insert mode
LDTB1   = $D9                           ;   screen line link table d9-f2
USER    = $F3                           ;   pointer address of current screen color ram f3-f4
KEYTAB  = $F5                           ;   keyboard decode table f5-f6
RIBUF   = $F7                           ;   pointer rs232 input buffer f7-f8
ROBUF   = $F9                           ;   pointer rs232 ouput buffer f9-fa
FREKZP  = $FB                           ;   free 0 page fb-fe
BASZPT  = $FF                           ;   temporary floatingpoint to ascii

.print on

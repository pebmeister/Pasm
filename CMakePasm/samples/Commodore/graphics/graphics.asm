         ; written by Paul Baxter

        .inc "macros.asm"
        .inc "pagezero.asm"
        .inc "basic.asm"
        .inc "kernal.asm"

;********************************************
        BITMAPBASE = $2000
        COLORBASE = $0400

        ; Some zero page has multiple uses

        XCOORD = LINNUM
        LOC = UNUSED2
        DX = BITTS
        DY = BITCI
        XS = RIDATA
        YS = RIPRTY + 1
        K1 = OLDLIN
        K2 = FREKZP
        _D = FREKZP + 2
        _X = LINNUM
        _Y = RESHO
        _CMD = FREKZP
        TMP = UNUSED2 + 2
        CMDTABLE = UNUSED2
        BASICTEXT = RESHO
        CMDJUMP = UNUSED2
        WEDGE_TMP = LINNUM
        CURLOC = BITTS
        COUNTER = UNUSED2
        MASKB = RESHO
        MASKE = FREKZP
        YCOUNT = FREKZP + 2
        YCOL = PTR1
        XD   = PTR1
        COLORPTR = TMPDATA
        X0 = FORPNT
        X1 = OPPTR
        Y0 = PRTY
        Y1 = MSGFLG
        R = FREKZP
        PK = OLDLIN
        CURX = BITCI
        CURY = FREKZP + 2
        CX = RIDATA
        CY = RIPRTY + 1
        XDIST  = INDEX
        YDIST  = INDEX + 2
        StartX = DSCPNT
        YMAX = OLDLIN
        XMAX = BLNCT
        FixedA = FREKZP
        FixedB = FAC1
        FixedC = FAC2
        CIRJUMP = VERCK
        COLOR = UNUSED3

        IGONE_SV = $3FC

;********************************************

        .org $8000

        .WORD   @COLD_START     ; Cartridge cold-start vector
        .WORD   @WARM_START     ; Cartridge warm-start vector
        .BYTE   $C3, $C2, $CD, $38, $30     ; CBM8O - Autostart key

@COLD_START
;   KERNAL RESET ROUTINE
        stx $D016               ; SCROLX - Turn on VIC for PAL / NTSC check
        jsr $FDA3               ; IOINIT - Init CIA chips
        jsr $FD50               ; RANTAM - Clear/test system RAM
        jsr $FD15               ; RESTOR - Init KERNAL RAM vectors
        jsr $FF5B               ; CINT   - Init VIC and screen editor

;   BASIC RESET  Routine
        jsr $E453               ; Init BASIC RAM vectors
        jsr $E3BF               ; Main BASIC RAM Init routine
        jsr $E422               ; Power-up message / NEW command
        ldx #$FB
        txs                     ; Reduce stack pointer for BASIC

@WARM_START
;   START YOUR PROGRAM HERE

        lda #5                  ; CHANGE BORDER COLOUR
        sta BORDER              ;

        ;
        ;   install wedge
        ;
@SETWEDGE
        NEWBYTES = 3
        sei

        MOVE16 IGONE, IGONE_SV
        MOVE16I IGONE, WEDGE

        lda #0                  ; peform a NEW for basic
        ldy #NEWBYTES
-
        sta (TXTTAB),y
        dey
        bne -

        cli

        jmp (IGONE_SV)

;********************************************
;*                                          *
;*  WEDGE                                   *
;*                                          *
;* TXTPTR - pointer to BASIC text           *
;* IGONE  - jump vector back to BASIC       *
;*                                          *
;* ---------------------------------------- *
;*        PAGE ZERO USAGE                   *
;* ---------------------------------------- *
;*  CMDTABLE    = UNUSED2      2 bytes      *
;*  BASICTEXT   = RESHO        2 bytes      *
;*  CMDJUMP     = UNUSED2      2 bytes      *
;*  WEDGE_TMP   = LINNUM       2 bytes      *
;*  _CMD        = FREKZP       1 byte       *
;* ---------------------------------------- *
;*                                          *
;********************************************
WEDGE
        ; move the address of the table to pagezero
        MOVE16I CMDTABLE, _Commands
        MOVE16 TXTPTR, BASICTEXT

        ; init _cmd INDEX TO 0
        ; also init y to 1
        ldy #0
        sty _CMD
        iny

; -------------------------------------

@SkipSpace
        lda (BASICTEXT),y
        cmp #' '
        bne @MatchCmd
        INC16 BASICTEXT
        jmp @SkipSpace

; -------------------------------------

@MatchCmd
        lda (BASICTEXT),y
        ; if it's a null then its the end of a command
        beq @NextCmd

        ; if its a colon let basic handle it
        cmp #':'
        beq @NextCmd

        ; compare to the table entry
        cmp (CMDTABLE),y
        bne @NextCmd

        ; get the next char
        iny
        jmp @MatchCmd

; -------------------------------------

@NextCmd
        lda (CMDTABLE),y
        beq @ExecuteCmd

        inc _CMD

; -------------------------------------

        ldy #0
@CmdTableInc
        INC16 CMDTABLE
        lda (CMDTABLE),y
        bne @CmdTableInc
        iny
        lda (CMDTABLE),y
        beq @ExitToBasic
        INC16 CMDTABLE
        iny
        jmp @MatchCmd

; -------------------------------------

@ExecuteCmd
        dey
        sty WEDGE_TMP
        lda #0
        sta WEDGE_TMP + 1

        ADD16 BASICTEXT, WEDGE_TMP, BASICTEXT
        MOVE16 BASICTEXT, TXTPTR

        ; multiply _CMD by two to get jump offset
        lda _CMD
        asl

        ; move offset to x
        ; move jump entry to
        ; pagezero jump
        tax

        lda _Jump,x
        sta CMDJUMP
        lda _jump + 1,x
        sta CMDJUMP + 1

        jsr @Jump

; -------------------------------------

@Cleanup
        jsr CHRGOT
        beq @Exit2
        jsr CHRGET
        jmp @Cleanup

; -------------------------------------

@Exit2
        DEC16 TXTPTR

; -------------------------------------
@ExitToBasic
        jmp (IGONE_SV)

@Jump
        ; execute the command
        jmp (CMDJUMP)

; -------------------------------------
; each command
; 1st byte space or 0 for end of table
; 0 terminates the string entry
; -------------------------------------
_Commands
        .byte " H", FRE_TOK, "CT", 0
        .byte " HLINE", 0
        .byte " HPLOT", 0
        .byte " HRECT", 0
        .byte " HGROFF", 0
        .byte " HFCOL", 0
        .byte " HBCOL", 0
        .byte " HCLS", 0
        .byte " HGR", 0
        .byte " HCIR", 0
        .byte " HFCIR", 0
        .byte " HBEZ", 0
        .byte " HTEST", 0

        .byte 0

 ; offsets to commands
_Jump
        .word HGRFillRect
        .word HGRLine
        .word HGRPlot
        .word HGRRect
        .word HGROff
        .word HGRFColor
        .word HGRBColor
        .word HGRCls
        .word HGROn
        .word HGRCircle
        .word HGRFillCircle
        .word HGRBez
        .word HGRTest

;********************************************
;*                                          *
;*  HGROn                                   *
;*                                          *
;*  Enter HGR Mode                          *
;*                                          *
;********************************************
HGROn
        ; hires on
        lda $D018
        ora #%00001000
        sta $D018

        lda $D011
        ora #%00100000
        sta $D011
        rts

;********************************************
;*                                          *
;*  HGRCLS                                  *
;*                                          *
;*  Clear graphics bitmap and colors        *
;*                                          *
;* ---------------------------------------- *
;*        CALLS                             *
;* ---------------------------------------- *
;*  ClearColor                              *
;* ---------------------------------------- *
;********************************************
HGRCls
        ldx #0
        lda #0
        .var n = 8000/255, i
@_ClrLoop
        .for i = 0 .to n -1
            sta BITMAPBASE + ($0100 * i),x
        .next i
        inx
        bne @_ClrLoop
@_ClrLoop2
        sta BITMAPBASE + ($0100 * n),x
        inx
        cpx #8000-n*255
        bne @_ClrLoop2

        jmp ClearColor

;********************************************
;*                                          *
;*  HGRBColor                               *
;*     Set HGR Background COLOR             *
;*                                          *
;* ---------------------------------------- *
;*        PAGE ZERO USAGE                   *
;* ---------------------------------------- *
;*  TMP     = UNUSED2 + 2       1 byte      *
;*  COLOR   = CMP0              1 byte      *
;* ---------------------------------------- *
;********************************************
HGRBColor
        jsr GETBYTC
        txa
_HGRBColor
        and #$0F
        sta TMP
        lda COLOR
        and #$F0
        ora TMP
        sta COLOR
        rts

;********************************************
;*                                          *
;*  HGRFColor                               *
;*     Set HGR Forground COLOR              *
;*                                          *
;* ---------------------------------------- *
;*        PAGE ZERO USAGE                   *
;* ---------------------------------------- *
;*  TMP     = UNUSED2 + 2       1 byte      *
;*  COLOR   = CMP0              1 byte      *
;* ---------------------------------------- *
;********************************************
HGRFColor
        jsr GETBYTC
        txa
_HGRFColor
        and #$0F
        asl
        asl
        asl
        asl
        sta TMP

        lda COLOR
        and #$0F
        ora TMP
        sta COLOR
        rts

;********************************************
;*                                          *
;*  ClearColor                              *
;*     clear Color memory                   *
;*                                          *
;* ---------------------------------------- *
;*        PAGE ZERO USAGE                   *
;* ---------------------------------------- *
;*  COLOR   = CMP0              1 byte      *
;* ---------------------------------------- *
;********************************************
ClearColor
        lda COLOR
        ldx #0
@Clear3
        sta COLORBASE + ($0100 * 0),x
        sta COLORBASE + ($0100 * 1),x
        sta COLORBASE + ($0100 * 2),x
        inx
        bne @Clear3
@Clear4
        sta COLORBASE + ($0100 * 3),x
        inx
        cpx #1000-3*255
        bne @Clear4
        rts

;********************************************
;*                                          *
;*  HGROff                                  *
;*                                          *
;********************************************
HGROff
        ; hires off
        lda $D018
        and #~ %00001000
        sta $D018

        lda $D011
        and #~ %00100000
        sta $D011
        rts

;********************************************
;*                                          *
;*  HGRPlot                                 *
;*                                          *
;* Parses Basic Text and plot a point       *
;*                                          *
;* ---------------------------------------- *
;*        PAGE ZERO USAGE                   *
;* ---------------------------------------- *
;*  TXTPTR                                  *
;* ---------------------------------------- *
;*        CALLS                             *
;* ---------------------------------------- *
;   GETNUM                                  *
;   RangeCheckXY                            *
;   _Plot                                   *
;* -----------------------------------------*
;********************************************
HGRPlot
        INC16 TXTPTR
        jsr GETNUM ; get address in $14/$15 y next integer at X

        jsr RangeCheckXY

;********************************************
;*                                          *
;*  _Plot                                   *
;*       XCOORD     IN  2 byte x coordinate *
;*       X register IN  y coordinate        *
;*                                          *
;* Plot a single point                      *
;*                                          *
;* ---------------------------------------- *
;*        PAGE ZERO USAGE                   *
;* ---------------------------------------- *
;*  LOC         = UNUSED2                   *
;*  COLOR       = CMP0                      *
;*  YCOL        = PTR1                      *
;*  COLORPTR    = TMPDATA                   *
;*  XCOORD      = LINNUM                    *
;* ---------------------------------------- *
;*        CALLS                             *
;* ---------------------------------------- *
;*   CalcPlot                               *
;*   CalcColorByte                          *
;* ---------------------------------------- *
;********************************************
_Plot
        stx YCOL
        MOVE16 XCOORD, COLORPTR

        ; Calc Bitmap location
        jsr CalcPlot
        ldy #0
        lda (LOC),y
        ora MaskStart,x
        sta (LOC),y

        ; Calc color map location
        jsr CalcColorByte
        lda COLOR
        ldy #0
        sta (COLORPTR),y
        rts

;********************************************
;*                                          *
;* CalcColorByte                            *
;*                                          *
;* COLORPTR = x location                    *
;* YCOL = y location                        *
;*                                          *
;* ---------------------------------------- *
;*        PAGE ZERO USAGE                   *
;* ---------------------------------------- *
;*  YCOL        = PTR1          2 bytes     *
;*  COLORPTR    = TMPDATA       2 bytes     *
;* ---------------------------------------- *
;********************************************
CalcColorByte
        ; YCOL + 0 has y value
        ; zero out high byte
        lda #0
        sta YCOL + 1
        RSHIFT16 COLORPTR, 3  ; COLORPTR = x / 8
        RSHIFT16 YCOL, 3      ; ycol int(y/8)*40
        MULT40 YCOL

        ; COLORPTR = COLORPTR + ycol
        ADD16 COLORPTR, YCOL, COLORPTR

        ; COLORPTR = COLORPTR + COLORBASE
        ADD16I COLORPTR, COLORBASE, COLORPTR
        rts

;********************************************
;*                                          *
;*  CalcPlot                                *
;*      XCOORD IN  2 byte x coordinate      *
;*      X reg  IN  y coordinate             *
;*                                          *
;*      LOC    OUT  byte of bitmap          *
;*      X reg  OUT  bit of bitmap           *
;*                                          *
;* ---------------------------------------- *
;*        PAGE ZERO USAGE                   *
;* ---------------------------------------- *
;*  LOC         = UNUSED2       2 byte      *
;*  XCOORD      = LINNUM        2 bytes     *
;*  TMP         = UNUSED2 + 2   2 bytes     *
;********************************************
CalcPlot
; 320*int(y/8)=32*[8*int(y/8)] + 8*[8*int(y/8)]
        lda #0
        sta LOC
        txa             ; X register has Y coordinate
        lsr
        lsr
        lsr
        sta LOC + 1     ; high-byte of 32*8*int(y/8)

;-------------------------------------------------

        lsr
        ror LOC
        lsr
        ror LOC         ; low byte of 8*[8*int(y/8)]

;-------------------------------------------------

        adc LOC + 1
        adc #>BITMAPBASE
        sta LOC+1       ; high byte of memory=BITMAPBASE + 320*int(y/8)

;-------------------------------------------------

        lda XCOORD
        and #%11111000  ; 8*int(x/8)
        adc LOC
        sta LOC         ; set address memory block of bytes

;-----------------------------------------------

        lda XCOORD + 1
        adc LOC + 1
        sta LOC + 1

;-----------------------------------------------

        txa
        and #%00000111 ; int(y/7), byte offset inside bLOCk
        tay

;-----------------------------------------------

        lda XCOORD
        and #%00000111 ; int(x/7), bit inside byte
        tax

;-----------------------------------------------

        ; add y to LOC
        sty TMP
        lda #0
        sta TMP + 1
        ADD16 TMP, LOC, LOC

;-----------------------------------------------

        ; given: x (0 - 7)
        ; x = 7 - x
        MOVE8I TMP, 7
        stx TMP + 1
        SUB8 TMP, TMP + 1, TMP
        ldx TMP
        rts

;********************************************
;*                                          *
;*                                          *
;*  HGRLine                                 *
;*                                          *
;* This routine draws a line                *
;*                                          *
;* ---------------------------------------- *
;*        PAGE ZERO USAGE                   *
;* ---------------------------------------- *
;*  TXTPTR                    2 bytes       *
;*  X0           = FORPNT     2 bytes       *
;*  Y0           = PRTY       1 byte        *
;*  X1           = OPPTR      2 bytes       *
;*  Y1           = MSGFLG     1 byte        *
;*  XCOORD       = LINNUM     2 bytes       *
;* ---------------------------------------- *
;*        CALLS                             *
;* ---------------------------------------- *
;*  GETNUM                                  *
;*  RangeCheckXY                            *
;*  _Line                                   *
;********************************************
 HGRLine
        INC16 TXTPTR

        jsr GETNUM ; get address in LINNUM $14/$15 y next integer at X
        jsr RangeCheckXY

        stx Y0
        MOVE16 XCOORD, X0

        jsr CHKCOM
        jsr GETNUM ; get address in LINNUM $14/$15 y next integer at X
        jsr RangeCheckXY

        stx Y1
        MOVE16 XCOORD, X1

;********************************************
;*                                          *
;* _Line                                    *
;*                                          *
;*    X0 - start X                          *
;*    Y0 - start Y                          *
;*    X1 - end X                            *
;*    Y1 - end Y                            *
;*                                          *
;*    optimized for horizontal lines        *
;*                                          *
;* ---------------------------------------- *
;*        PAGE ZERO USAGE                   *
;* ---------------------------------------- *
;*  X0           = FORPNT     2 bytes       *
;*  Y0           = PRTY       1 byte        *
;*  X1           = OPPTR      2 bytes       *
;*  Y1           = MSGFLG     1 byte        *
;*  XDIST        = INDEX      2 bytes       *
;* ---------------------------------------- *
;********************************************
_Line
        lda Y0
        cmp Y1
        bne NotHorizontal

;---------------------------------------
        BLE16 X0, X1, @X0_Less_Equal
;---------------------------------------
; Swap X0, Y0 and X1, Y1
        SWAP16 X0, X1
        ldx Y0
        ldy Y1
        stx Y1
        sty Y0

;---------------------------------------
;
; At this point X0 MUST be less than X1!!!
;
;---------------------------------------

@X0_Less_Equal
        MOVE16 X0,XCOORD
        SUB16 X1, X0, XDIST

;---------------------------------------
CalcPlotHLine
        MOVE16 XCOORD, COLORPTR
        MOVE8 Y0, YCOL
        ldx Y0

        ; Calculate 1st byte and bit of BitMap
        jsr CalcPlot
        MOVE16 LOC, CURLOC

        ; Draw the horizontal Line
        jsr HGRHorizontalLine

        ; Set the Lines color memory
        jmp HCPlotLine

;---------------------------------------

NotHorizontal
        MOVE8I DY+1, 0

        ; DX% = abs(x0%-x1%)
        ; set XS to add or substract
        ;
        BLT16 X0, X1, @X1_larger

;-------------------------------------------
        SUB16 X0, X1, DX
        lda #1
        bne @Calc_DY

@X1_larger
        SUB16 X1, X0, DX
        lda #0
@Calc_DY
        sta XS

;-------------------------------------------
        ; DY = abs(y0% - y1%)
        ; set YS to add or subtract
        lda Y0
        cmp Y1
        bcc @Y1_larger

        SUB8 Y0, Y1, DY
        lda #1
        bne @Done_calc

@Y1_larger
        SUB8 Y1, Y0, DY
        lda #0
@Done_calc
        sta YS
;-------------------------------------------
        ; y is 8 bit so if this is not 0 its larger
        lda DX + 1

        ; using reverse logic because branch is > 128
        beq @DX_DY_compare
        jmp @DX_larger

@DX_DY_compare
        lda DX
        cmp DY

        ; using reverse logic because branch is > 128
        bcc @DY_larger
        jmp @DX_larger

@DY_larger
        ; K1 = 2 * DX
        MOVE16 DX, K1
        LSHIFT16 K1, 1

        ; K2%=2*(DX%-DY%)
        SUB16 DX, DY, K2
        LSHIFT16 K2, 1

        ; d%=K1%-DY
        SUB16 K1, DY, _D
;-------------------------------------------
        ; y=y0%
        lda Y0
        sta _Y

        ; x = x0
        MOVE16 X0, _X
;-------------------------------------------
; for y=y0% to y1% step YS%
@Y_loop_start
        lda _Y
        cmp Y1

        bne @Y_loop_continue
        jmp @Exitline

@Y_loop_continue
        ; plot
        ldx _Y
        jsr _Plot
; -------------------------------------------
        ; if d%<0 then y_loop_d0_minus
        lda _D + 1
        bmi @Y_loop_d0_minus

        ; x = x + XS
        lda XS
        beq @Y_loop_X_inc

        DEC16 _X
        jmp  @Y_loop_add_d

@Y_loop_X_inc
        INC16 _X

@Y_loop_add_d
        ; d% = d% + K2%
        ADD16 _D, K2, _D
        jmp @Y_loop_next
; -------------------------------------------
@y_loop_d0_minus
        ; d%=d%+K1%
        ADD16 _D, K1, _D
; -------------------------------------------
@y_loop_next
        ; y = y + YS
        lda YS
        beq @Y_loop_next_inc

        DEC8 _Y
        jmp @Y_loop_start

@Y_loop_next_inc
        inc _Y
        jmp @Y_loop_start
; -------------------------------------------
@DX_larger
        ; K1 = 2 * DY
        MOVE16 DY, K1
        LSHIFT16 K1, 1

        ; K2%=2*(DY%-DX%)
        SUB16 DY, DX, K2
        LSHIFT16 K2, 1

        ; d%=K1%-DX
        SUB16 K1, DX, _D
; -------------------------------------------
        ; y=y0%
        lda Y0
        sta _Y

        ; x = x0
        MOVE16 X0, _X
;-------------------------------------------
; for x=x0% to x1% step XS%
@X_loop_start

        lda _X + 1
        cmp X1 + 1
        bne @X_loop_continue

        lda _X
        cmp X1
        bne @X_loop_continue

        jmp @Exitline

@X_loop_continue
        ; plot
        ldx _Y
        jsr _Plot
; -------------------------------------------
        ; if d%<0 then x_loop_d0_minus
        lda _D + 1
        bmi @X_loop_d0_minus

        ; y = y + YS
        lda YS
        beq @X_loop_Y_inc

        DEC8 _Y
        jmp  @X_loop_add_d

@X_loop_Y_inc
        inc _Y

@X_loop_add_d
        ; d% = d% + K2%
        ADD16 _D, K2, _D
        jmp @X_loop_next

@X_loop_d0_minus
        ; d%=d%+K1%
        ADD16 _D, K1, _D
; -------------------------------------------
@X_loop_next
        ; x = x + XS
        lda XS
        beq @x_loop_next_inc

        DEC16 _X
        jmp @X_loop_start

@X_loop_next_inc
        INC16 _X
        jmp @X_loop_start
; -------------------------------------------
@Exitline
        ldx _Y
        jmp _Plot

;********************************************
;
;  HCPlotLine
;
; COLORPTR - X start
; XDIST    - X Distance
; YCOL     - Y Location
;
; Plot color memory line
;
;********************************************
HCPlotLine
        ; Save start, distance
        MOVE16 COLORPTR, StartX

        ; get the start byte
        jsr CalcColorByte

        ; check for XDIST > 8
        MOVE16 XDIST, XD
        lda XD + 1
        bne @Continue

        lda XD
        cmp #9
        bcc @LoopExit

@Continue
        INC16 XD

        lda StartX
        and #%00000111
        beq @InitLoop

; -------------------------------------------
        ; subtract 8 - N from XD

        sta TMP

        sec
        lda #8
        sbc TMP
        sta TMP
        SUB168 XD, TMP, XD

        ; set color Memory
        ldy #0
        lda COLOR
        sta (COLORPTR),y
        INC16 COLORPTR

; -------------------------------------------

        ; set y to 0
@InitLoop
        ldy #0
@Loop
        lda XD + 1  ; check that there is more to process
                    ; check high byte first
        bne @LoopContinue
        lda XD      ; check low byte is at least 8 pixels
        cmp #08
        bcc @LoopExit

; -------------------------------------------
@LoopContinue
        ; set color Memory
        lda COLOR
        sta (COLORPTR),y
        INC16 COLORPTR

        ; subtract 8 from distance
        SUB16I XD, 8, XD
        jmp @Loop

; -------------------------------------------
@LoopExit
        ; check for last byte
        lda XD
        beq @Exit

        ; set color Memory
        lda COLOR
        sta (COLORPTR),y
@Exit
        rts

;********************************************
;
;  HGRHorizontalLine
;
; CURLOC IN POINTER to bitmap byte
; XDIST IN x pixel distance
; X register IN bit in first byte of bitmap
;
;********************************************
HGRHorizontalLine
        ; If there are 8 bits to set then we can use bytes
        ; instead of using bits
        ;
        ; set the first potentially partial byte
        ; set x to number of bits in first byte
        lda XDIST + 1   ; load HI byte
        bne @HFirstByte ; if its not 0 then it > 8 bits
        cpx XDIST       ; see if bit is within distance
        bcc @HFirstByte ; yes so we leave x unmodified

;----------------------------------------------
; special case for < 8 pixels
@StartByte
        ldy #0
        lda (CURLOC),y
        ldy XDIST       ; set y to XDIST
        beq @StartExit
@StartByteLoop
        ora MaskStart,x
        dex
        dey
        bne @StartByteLoop
        sta (CURLOC),y
@StartExit
        rts

;----------------------------------------------
@HFirstByte
        stx  COUNTER    ; save in counter
        lda #0          ; zero out high byte
        sta COUNTER + 1

;----------------------------------------------
        ; subtract COUNTER from distance of first byte
        ; and save in COUNTER
        SUB16 XDIST, COUNTER, COUNTER
;----------------------------------------------
        ; Plot the first byte
        ldy #0
        lda (CURLOC),y
        ora MaskData,x
        sta (CURLOC),y
;----------------------------------------------
; Horizontal Line - Loop
; here we plot bytes instead of bits
;----------------------------------------------
@HLoop1
        ; test that there are at least 8 bits to plot
        lda COUNTER + 1 ; get the high byte
        bne @HLoop_Cont ; if its set there are more than 8 bits left
        lda COUNTER     ; load lo byte
        beq @HLoop1End
        cmp #8          ; compare it to 8
        beq @HLoop1End  ; if it is equal then go to the last byte
        bcs @HLoop_Cont ; continue if > 8
        jmp @HLoop1End  ; less than 8 go to last byte

; -------------------------------------------
@HLoop_Cont             ; add 8 to current pointer
        ADD16I CURLOC, 8, CURLOC
        lda #$FF        ; load a with FF
        sta (CURLOC),y  ; store the byte

                        ; Increment COUNTER by 8
        SUB16I COUNTER, 8, COUNTER
        jmp @HLoop1     ; goto to byte loop

; -------------------------------------------
@HLoop1End
        ldx COUNTER     ; see if there are bits left
        beq @HLoop1EXIT ; if no then exit

;----------------------------------------------
; Horizontal Lines Last Byte
;----------------------------------------------
        ADD16I CURLOC, 8, CURLOC
@HLastByte
        lda (CURLOC),y
        ora MaskDataEnd,x
        sta (CURLOC),y
@HLoop1EXIT
        rts

;********************************************
;
;  HGRFillRect
;
;********************************************
HGRFillRect
        INC16 TXTPTR    ; advance basic textptr

        ; get the start position, xdistance and ydistance

        jsr GETNUM      ; get address in $14/$15 y next integer at X
        jsr RangeCheckXY

        stx Y0
        MOVE16 XCOORD, X0

        jsr CHKCOM
        jsr GETNUM      ; get address in $14/$15 y next integer at X
        jsr RangeCheckXY

        stx YDIST
        MOVE16 XCOORD, XDIST

;********************************************
;
;  HFillRect
;
; X0        -  xstart
; Y0        -  ystart
; XDIST     -  xdistance
; YDIST     -  ydistance
;
;********************************************
HFillRect
        ADD8 Y0, YDIST, YMAX
        ADD16 X0, XDIST, X1

; -------------------------------------------
@YLOOP
        lda Y0
        sta Y1
        jsr _Line

        ldy Y0
        cpy YMAX
        beq @Exit
        iny
        sty Y0
        jmp @YLOOP
@Exit
        rts

;********************************************
;
;  HGRRect
;
;  Parse Basic text for
;  x, y, xdist, ydist
;
;  Call HRect to draw rectangle
;
;********************************************
HGRRect
        INC16 TXTPTR    ; advance basic textptr

        ; get the start position, xdistance and ydistance

        jsr GETNUM ; get address in $14/$15 y next integer at X
        jsr RangeCheckXY
        stx Y0
        MOVE16 XCOORD, X0

        jsr CHKCOM
        jsr GETNUM ; get address in $14/$15 y next integer at X
        jsr RangeCheckXY
        stx YDIST
        MOVE16 XCOORD, XDIST

;********************************************
;
;  HRect
;
;  X0       - x start
;  Y0       - y start
;  XDIST    - x distance
;  YDIST    - y distance
;********************************************
HRect
        ADD16 X0, XDIST, XMAX
        ADD8 Y0, YDIST, YMAX

        ; ------------------------------
        ;
        ; Top Horizontal Line
        ;
        ; ------------------------------
        MOVE16 XMAX, X1
        MOVE8 Y0, Y1
        pha
        jsr _Line

        ; ------------------------------
        ;
        ; Bottom Horizontal Line
        ;
        ; ------------------------------
        lda YMAX
        sta Y0
        sta Y1
        jsr _Line

        ; ------------------------------
        ;
        ; Left Vertical Line
        ;
        ; ------------------------------
        MOVE16 X0, X1
        pla
        sta Y0
        jsr _Line

        ; ------------------------------
        ;
        ; Right Vertical Line
        ;
        ; ------------------------------
        MOVE16 XMAX, X0
        MOVE16 X0, X1
        jsr _Line
        rts

;********************************************
;
;  CirclePlot
;
;  This will fill or plot for circles
;
;********************************************
CirclePlot
        * = $CE00
CirSave         .ds 1
CX_MINUS_CURY   .ds 2
CX_MINUS_CURX   .ds 2
CX_PLUS_CURY    .ds 2
CX_PLUS_CURX    .ds 2

CY_MINUS_CURY   .ds 1
CY_MINUS_CURX   .ds 1
CY_PLUS_CURY    .ds 1
CY_PLUS_CURX    .ds 1
        * = CirclePlot

        SUB16 CX, CURX, CX_MINUS_CURX
        ADD16 CX, CURX, CX_PLUS_CURX
        SUB168 CX, CURY, CX_MINUS_CURY
        ADD168 CX, CURY, CX_PLUS_CURY

        SUB8 CY, CURX, CY_MINUS_CURX
        ADD8 CY, CURX, CY_PLUS_CURX
        SUB8 CY, CURY, CY_MINUS_CURY
        ADD8 CY, CURY, CY_PLUS_CURY

        lda CirSave
        asl
        tax
        lda JumpTable, x
        sta CIRJUMP
        inx
        lda JumpTable, x
        sta CIRJUMP + 1
        jmp (CIRJUMP)

JumpTable
        .word PlotCirclePixels
        .word PlotFillCircle

;********************************************
;
;  HGRFillCircle
;
;  Fill a circle from BASIC
;  x, y, radius
;
; Bresenham's algorithm
;
;*******************************************
HGRFillCircle
        lda #1
        sta CirSave
        jmp HGRCircle2

;********************************************
;
;  HGRCircle
;
;  Parse Basic text for
;  x, y, radius
;
; Bresenham's algorithm
;
;********************************************
HGRCircle
        lda #0
        sta CirSave

HGRCircle2
        INC16 TXTPTR    ; advance basic textptr

        ; get the start position, xdistance and ydistance

        jsr GETNUM ; get address in $14/$15 y next integer at X
        stx CY
        MOVE16 XCOORD, CX
        jsr GETBYTC
        stx R

        jsr RangeCheckCircle
_HCir
                            ; pk = 3 - 2 * r;
        lda #0
        sta PK + 1
        MOVE8 R, PK         ; pk = r
        LSHIFT16 PK,1       ; pk = r * 2
        MOVE16I TMP, 3;     ; tmp = 3
        SUB16 TMP, PK, PK   ; pk = tmp - pk

        MOVE16I CURX, 0     ; x = 0
        MOVE8 R, CURY       ; y = r

        ; PlotPixels(x, y, xc, yc);
        jsr CirclePlot

@StartWhile
        ; while (x < y)
        lda CURX + 1
        bne @ContinueWhile
        lda CURX
        cmp CURY
        bcs @Exit

@ContinueWhile
        ; if (pk <= 0)
        lda PK+1
        bmi @PKLessEqualZero
        bne @PKGreaterZero
        lda PK
        bne @PKGreaterZero

@PKLessEqualZero
        ; pk = pk + (4 * x) + 6;
        MOVE16 CURX, TMP
        LSHIFT16 TMP, 2
        ADD16 PK, TMP, PK
        ADD16I PK, 6, PK

        ;++x;
        INC16 CURX
        jmp @EndWhile

@PKGreaterZero
        ; pk = pk + (4 * (x - y)) + 10;
        SUB168 CURX, CURY, TMP
        LSHIFT16 TMP, 2
        ADD16I TMP, 10, TMP
        ADD16 PK, TMP, PK

        ; ++x;
        INC16 CURX
        ; --y;
        DEC8 CURY

@EndWhile

        ; PlotPixels(x, y, xc, yc);
        jsr CirclePlot
        jmp @StartWhile
@Exit
        rts

;********************************************
;
;  PlotCirclePixels
;
;  plot circle pixels
;
;*******************************************
PlotCirclePixels

;==============================================

        ; PutPixel(CX - CURX, CY + CURY);
        MOVE16 CX_MINUS_CURX, XCOORD
        ldx CY_PLUS_CURY
        jsr _Plot

;----------------------------------------------
        ; PutPixel(CX + CURX, CY + CURY);
        MOVE16 CX_PLUS_CURX, XCOORD
        ldx CY_PLUS_CURY
        jsr _Plot

;==============================================
        ; PutPixel(CX - CURX,  CY - CURY);
        MOVE16 CX_MINUS_CURX, XCOORD
        ldx CY_MINUS_CURY
        jsr _Plot

;----------------------------------------------

        ; PutPixel(CX + CURX, CY - CURY);
        MOVE16 CX_PLUS_CURX, XCOORD
        ldx CY_MINUS_CURY
        jsr _Plot

;==============================================

        ; PutPixel(CX - CURY, CY + CURX);
        MOVE16 CX_MINUS_CURY, XCOORD
        ldx CY_PLUS_CURX
        jsr _Plot

;----------------------------------------------

        ; PutPixel(CX + CURY, CY + CURX);
        MOVE16 CX_PLUS_CURY, XCOORD
        ldx CY_PLUS_CURX
        jsr _Plot

;==============================================

        ; PutPixel(CX - CURY, CY - CURX);
        MOVE16 CX_MINUS_CURY, XCOORD
        ldx CY_MINUS_CURX
        jsr _Plot

;----------------------------------------------

        ; PutPixel(CX + CURY, CY - CURX);
        MOVE16 CX_PLUS_CURY, XCOORD
        ldx CY_MINUS_CURX
        jmp _Plot

;********************************************
;
;  PlotFillCircle
;
;  fill circle pixels
;
;*******************************************

PlotFillCircle

        ; Line (CX - CURX, CY + CURY) TO (CX + CURX, CY + CURY);
        MOVE16 CX_MINUS_CURX, X0
        MOVE16 CX_PLUS_CURX, X1
        MOVE8 CY_PLUS_CURY, Y0
        MOVE8 Y0, Y1
        jsr _Line

;----------------------------------------------

        ; Line (CX - CURX,  CY - CURY) TO (CX + CURX, CY - CURY);
        MOVE16 CX_MINUS_CURX, X0
        MOVE16 CX_PLUS_CURX, X1
        MOVE8 CY_MINUS_CURY, Y0
        MOVE8 Y0, Y1
        jsr _Line

;----------------------------------------------

        ; Line(CX - CURY, CY + CURX) TO (CX + CURX, CY + CURX)
        MOVE16 CX_MINUS_CURY, X0
        MOVE16 CX_PLUS_CURY, X1
        MOVE8 CY_PLUS_CURX, Y0
        MOVE8 Y0, Y1
        jsr _Line

;----------------------------------------------

        ; Line(CX - CURX, CY - CURX) to (CX + CURX, CY - CURX)
        MOVE16 CX_MINUS_CURY, X0
        MOVE16 CX_PLUS_CURY, X1
        MOVE8 CY_MINUS_CURX, Y0
        MOVE8 Y0, Y1
        jmp _Line

;********************************************
;
;  RangeCheckXY
;
;  Check XCOORD and x range
;
;*******************************************
RangeCheckXY
        cpx # 200
        bcs RangeError

        lda XCOORD + 1
        beq @Exit
        cmp #2
        bcs RangeError


        lda XCOORD
        cmp # 320 - 256
        bcs RangeError
@Exit
        rts
RangeError
        ldx # ILLEGALQUANITY
        jmp ERROR

;********************************************
;
;  RangeCheckCircle
;
;  Range check Circle parameters
;
;*******************************************
RangeCheckCircle
        lda CY
        cmp R
        bcc RangeError

        adc R
        tax

        lda CX + 1
        bne @CheckMax

        lda CX
        cmp R
        bcc RangeError

@CheckMax
        ADD168 CX,R,XCOORD
        jmp RangeCheckXY

;----------------------------------------------

MaskStart
        .byte %00000001 ; 0
        .byte %00000010 ; 1
        .byte %00000100 ; 2
        .byte %00001000 ; 3
        .byte %00010000 ; 4
        .byte %00100000 ; 5
        .byte %01000000 ; 6
        .byte %10000000 ; 7

MaskData
        .byte %00000001 ; 0
        .byte %00000011 ; 1
        .byte %00000111 ; 2
        .byte %00001111 ; 3
        .byte %00011111 ; 4
        .byte %00111111 ; 5
        .byte %01111111 ; 6
        .byte %11111111 ; 7

MaskDataEnd
        .byte %00000000
        .byte %10000000 ; 1
        .byte %11000000 ; 2
        .byte %11100000 ; 3
        .byte %11110000 ; 4
        .byte %11111000 ; 5
        .byte %11111100 ; 6
        .byte %11111110 ; 7
        .byte %11111111 ; 8

;********************************************
;
;  HGRBez
;
;  Draw a Bezier curve
;
;*******************************************

BezParams
        * = $CF50
BX0     .ds 2
BY0     .ds 1
BX1     .ds 2
BY1     .ds 1
BX2     .ds 2
BY2     .ds 1
        * = BezParams

HGRBez
;----------------------------------
; local variables
;----------------------------------
        * = $CF00
@LASTX  .ds 2
@LASTY  .ds 1
@A1     .ds 4
@B1     .ds 4
@C1     .ds 4
@ONE_MINUS_T .ds 4
@A      .ds 4
@B      .ds 4
@C      .ds 4
@FIXX   .ds 4
@FIXY   .ds 4
@IDX    .ds 1

;----------------------------------
        * = HGRBez

        INC16 TXTPTR    ; advance basic textptr

        ; get the start position, xdistance and ydistance
        jsr GETNUM ; get address in $14/$15 y next integer at X
        stx BY0
        MOVE16 XCOORD, BX0

        jsr CHKCOM
        jsr GETNUM ; get address in $14/$15 y next integer at X
        stx BY1
        MOVE16 XCOORD, BX1

        jsr CHKCOM
        jsr GETNUM ; get address in $14/$15 y next integer at X
        stx BY2
        MOVE16 XCOORD, BX2

_HBez    = *

        MOVE16 BX0, @LASTX
        MOVE8 BY0, @LASTY

        MOVE8I @IDX,1
@WHILE_IDX
        ;     while (idex <= max)
        lda @IDX
        cmp #128
        bcc @ContinueWhile
        rts

@ContinueWhile
;----------------------------------------------
; calculate a1,b1 and c1
; use table lookup
;----------------------------------------------
        lda @IDX
        asl
        tay

        MOVE16I FixedA, @A1_table
        lda (FixedA),y
        sta @A1 + 2
        iny
        lda (FixedA),y
        sta @A1 + 3

        dey
        MOVE16I FixedA, @B1_table
        MOVE16I @B1,0
        lda (FixedA),y
        sta @B1 + 2
        iny
        lda (FixedA),y
        sta @B1 + 3

        dey
        MOVE16I FixedA, @C1_table
        MOVE16I @C1,0
        lda (FixedA),y
        sta @C1 + 2
        iny
        lda (FixedA),y
        sta @C1 + 3

;-----------------------------------------
;        Calculate FixedX
;-----------------------------------------
        ;   a = fix16_mul(a1, x0);
        MOVE32 @A1, FixedA
        MOVE16I FixedB, 0
        MOVE16 BX0, FixedB + 2
        jsr FIX16_MUL
        MOVE32 FixedC, @A

        ;   b = fix16_mul(b1, x1);
        MOVE32 @B1, FixedA
        MOVE16 BX1, FixedB + 2
        jsr FIX16_MUL
        MOVE32 FixedC, @B

        ;  c = fix16_mul(c1, x2);
        MOVE32 @C1, FixedA
        MOVE16 BX2, FixedB + 2
        jsr FIX16_MUL
        MOVE32 FixedC, @C

        ; fix16_t x_fixed = fix16_add(a, b);
        ; x_fixed = fix16_add(x_fixed, c);
        ADDFIX16 @A, @B, @FIXX
        ADDFIX16 @FIXX, @C, @FIXX

;-----------------------------------------
;        Calculate FixedY
;-----------------------------------------
        ;   a = fix16_mul(a1, y0);
        MOVE32 @A1, FixedA
        MOVE16I FixedB, 0
        MOVE16I FixedB + 2, 0
        MOVE8 BY0, FixedB + 2
        jsr FIX16_MUL
        MOVE32 FixedC, @A

        ;   b = fix16_mul(b1, y1);
        MOVE32 @B1, FixedA
        MOVE8 BY1, FixedB + 2
        jsr FIX16_MUL
        MOVE32 FixedC, @B

        ;  c = fix16_mul(c1, y2);
        MOVE32 @C1, FixedA
        MOVE8 BY2, FixedB + 2
        jsr FIX16_MUL
        MOVE32 FixedC, @C

        ; fix16_t y_fixed = fix16_add(a, b);
        ; y_fixed = fix16_add(x_fixed, c);
        ADDFIX16 @A, @B, @FIXY
        ADDFIX16 @FIXY, @C, @FIXY

;----------------------------------------

        ; putpixel(put_x, put_y); // putting pixel

        lda @FIXX + 2
        cmp @LASTX
        bne @Plot_Line

        lda @FIXX + 3
        cmp @LASTX + 1
        bne @Plot_Line

        lda @FIXY + 2
        cmp @LASTY
        beq @No_Line

@Plot_Line
        MOVE16 @FIXX + 2, X0
        MOVE8 @FIXY + 2, Y0
        MOVE16 @LASTX, X1
        MOVE8 @LASTY, Y1

        MOVE16 @FIXX + 2, @LASTX
        MOVE8 @FIXY + 2, @LASTY

        jsr @RangeCheckLine
        jsr _Line

@NO_Line
        inc @IDX
        jmp @WHILE_IDX

;---------------------------
@RangeCheckLine
        lda Y0
        cmp # 200
        bcs @RangeError

        lda Y1
        cmp # 200
        bcs @RangeError

; Check X0
        lda X0 + 1
        beq @CheckX1
        cmp #2
        bcs @RangeError

        lda X0
        cmp # 320 - 256
        bcs @RangeError

@CheckX1
        lda X1 + 1
        beq @Exit
        cmp #2
        bcs @RangeError

        lda X1
        cmp # 320 - 256
        bcs @RangeError
@Exit
        rts

@RangeError
        ldx # ILLEGALQUANITY
        jmp ERROR

; ****** Bezier Curve tables ***********
@A1_table
        .word $0000
        .word $fc04
        .word $f810
        .word $f424
        .word $f040
        .word $ec64
        .word $e890
        .word $e4c4
        .word $e100
        .word $dd44
        .word $d990
        .word $d5e4
        .word $d240
        .word $cea4
        .word $cb10
        .word $c784
        .word $c400
        .word $c084
        .word $bd10
        .word $b9a4
        .word $b640
        .word $b2e4
        .word $af90
        .word $ac44
        .word $a900
        .word $a5c4
        .word $a290
        .word $9f64
        .word $9c40
        .word $9924
        .word $9610
        .word $9304
        .word $9000
        .word $8d04
        .word $8a10
        .word $8724
        .word $8440
        .word $8164
        .word $7e90
        .word $7bc4
        .word $7900
        .word $7644
        .word $7390
        .word $70e4
        .word $6e40
        .word $6ba4
        .word $6910
        .word $6684
        .word $6400
        .word $6184
        .word $5f10
        .word $5ca4
        .word $5a40
        .word $57e4
        .word $5590
        .word $5344
        .word $5100
        .word $4ec4
        .word $4c90
        .word $4a64
        .word $4840
        .word $4624
        .word $4410
        .word $4204
        .word $4000
        .word $3e04
        .word $3c10
        .word $3a24
        .word $3840
        .word $3664
        .word $3490
        .word $32c4
        .word $3100
        .word $2f44
        .word $2d90
        .word $2be4
        .word $2a40
        .word $28a4
        .word $2710
        .word $2584
        .word $2400
        .word $2284
        .word $2110
        .word $1fa4
        .word $1e40
        .word $1ce4
        .word $1b90
        .word $1a44
        .word $1900
        .word $17c4
        .word $1690
        .word $1564
        .word $1440
        .word $1324
        .word $1210
        .word $1104
        .word $1000
        .word $0f04
        .word $0e10
        .word $0d24
        .word $0c40
        .word $0b64
        .word $0a90
        .word $09c4
        .word $0900
        .word $0844
        .word $0790
        .word $06e4
        .word $0640
        .word $05a4
        .word $0510
        .word $0484
        .word $0400
        .word $0384
        .word $0310
        .word $02a4
        .word $0240
        .word $01e4
        .word $0190
        .word $0144
        .word $0100
        .word $00c4
        .word $0090
        .word $0064
        .word $0040
        .word $0024
        .word $0010
        .word $0004
        .word $0000
@B1_table
        .word $0000
        .word $03f8
        .word $07e0
        .word $0bb8
        .word $0f80
        .word $1338
        .word $16e0
        .word $1a78
        .word $1e00
        .word $2178
        .word $24e0
        .word $2838
        .word $2b80
        .word $2eb8
        .word $31e0
        .word $34f8
        .word $3800
        .word $3af8
        .word $3de0
        .word $40b8
        .word $4380
        .word $4638
        .word $48e0
        .word $4b78
        .word $4e00
        .word $5078
        .word $52e0
        .word $5538
        .word $5780
        .word $59b8
        .word $5be0
        .word $5df8
        .word $6000
        .word $61f8
        .word $63e0
        .word $65b8
        .word $6780
        .word $6938
        .word $6ae0
        .word $6c78
        .word $6e00
        .word $6f78
        .word $70e0
        .word $7238
        .word $7380
        .word $74b8
        .word $75e0
        .word $76f8
        .word $7800
        .word $78f8
        .word $79e0
        .word $7ab8
        .word $7b80
        .word $7c38
        .word $7ce0
        .word $7d78
        .word $7e00
        .word $7e78
        .word $7ee0
        .word $7f38
        .word $7f80
        .word $7fb8
        .word $7fe0
        .word $7ff8
        .word $8000
        .word $7ff8
        .word $7fe0
        .word $7fb8
        .word $7f80
        .word $7f38
        .word $7ee0
        .word $7e78
        .word $7e00
        .word $7d78
        .word $7ce0
        .word $7c38
        .word $7b80
        .word $7ab8
        .word $79e0
        .word $78f8
        .word $7800
        .word $76f8
        .word $75e0
        .word $74b8
        .word $7380
        .word $7238
        .word $70e0
        .word $6f78
        .word $6e00
        .word $6c78
        .word $6ae0
        .word $6938
        .word $6780
        .word $65b8
        .word $63e0
        .word $61f8
        .word $6000
        .word $5df8
        .word $5be0
        .word $59b8
        .word $5780
        .word $5538
        .word $52e0
        .word $5078
        .word $4e00
        .word $4b78
        .word $48e0
        .word $4638
        .word $4380
        .word $40b8
        .word $3de0
        .word $3af8
        .word $3800
        .word $34f8
        .word $31e0
        .word $2eb8
        .word $2b80
        .word $2838
        .word $24e0
        .word $2178
        .word $1e00
        .word $1a78
        .word $16e0
        .word $1338
        .word $0f80
        .word $0bb8
        .word $07e0
        .word $03f8
        .word $0000
@C1_table
        .word $0000
        .word $0004
        .word $0010
        .word $0024
        .word $0040
        .word $0064
        .word $0090
        .word $00c4
        .word $0100
        .word $0144
        .word $0190
        .word $01e4
        .word $0240
        .word $02a4
        .word $0310
        .word $0384
        .word $0400
        .word $0484
        .word $0510
        .word $05a4
        .word $0640
        .word $06e4
        .word $0790
        .word $0844
        .word $0900
        .word $09c4
        .word $0a90
        .word $0b64
        .word $0c40
        .word $0d24
        .word $0e10
        .word $0f04
        .word $1000
        .word $1104
        .word $1210
        .word $1324
        .word $1440
        .word $1564
        .word $1690
        .word $17c4
        .word $1900
        .word $1a44
        .word $1b90
        .word $1ce4
        .word $1e40
        .word $1fa4
        .word $2110
        .word $2284
        .word $2400
        .word $2584
        .word $2710
        .word $28a4
        .word $2a40
        .word $2be4
        .word $2d90
        .word $2f44
        .word $3100
        .word $32c4
        .word $3490
        .word $3664
        .word $3840
        .word $3a24
        .word $3c10
        .word $3e04
        .word $4000
        .word $4204
        .word $4410
        .word $4624
        .word $4840
        .word $4a64
        .word $4c90
        .word $4ec4
        .word $5100
        .word $5344
        .word $5590
        .word $57e4
        .word $5a40
        .word $5ca4
        .word $5f10
        .word $6184
        .word $6400
        .word $6684
        .word $6910
        .word $6ba4
        .word $6e40
        .word $70e4
        .word $7390
        .word $7644
        .word $7900
        .word $7bc4
        .word $7e90
        .word $8164
        .word $8440
        .word $8724
        .word $8a10
        .word $8d04
        .word $9000
        .word $9304
        .word $9610
        .word $9924
        .word $9c40
        .word $9f64
        .word $a290
        .word $a5c4
        .word $a900
        .word $ac44
        .word $af90
        .word $b2e4
        .word $b640
        .word $b9a4
        .word $bd10
        .word $c084
        .word $c400
        .word $c784
        .word $cb10
        .word $cea4
        .word $d240
        .word $d5e4
        .word $d990
        .word $dd44
        .word $e100
        .word $e4c4
        .word $e890
        .word $ec64
        .word $f040
        .word $f424
        .word $f810
        .word $fc04
        .word $0000

;****************************************************
;*                                                  *
;* FIX16_MUL                                        *
;*                                                  *
;*  fix16 multiply                                  *
;*                                                  *
;*  16bit integer    16bit Fraction                 *
;*  Little Endian    Little Endian                  *
;*                                                  *
;*  FixedA  IN:  4 bytes 1st number to multiply     *
;*  FixedB  IN:  4 bytes 2nd number to multiply     *
;*  FixedC OUT:  4 bytes result                     *
;*                                                  *
;*  Each argument is divided to 16-bit parts.       *
;*  destroys a, x, y                                *
;*                                                  *
;****************************************************
FIX16_MUL
@A              = FixedA
@B              = FixedA + 2
@C              = FixedB
@D              = FixedB + 2

    @AC         = UNUSED2
    @AD         = BITCI
    @TMP        = TMPDATA
    @BD         = TAPE1
    @CB         = ARGSGN
    @AD_CB      = TEMPST
    @P_HI       = VERCK
    @P_LO       = UNUSED2

    ;   int32_t AC = A*C;
    ;   int32_t AD_CB = A*D + C*B;
    ;   uint32_t BD = B*D;
    FMUL @A, @C, @AC
    FMUL @A, @D, @AD
    FMUL @C, @B, @CB
    FMUL @B, @D, @BD

    ADDFIX16 @AD, @CB, @AD_CB

    ;   int32_t product_hi = AC + (AD_CB >> 16);
    MOVE16I @TMP, 0
    MOVE16 @AD_CB, @TMP + 2
    ADDFIX16 @AC, @TMP, @P_HI

    MOVE16I @TMP + 2, 0
    MOVE16 @AD_CB + 2, @TMP
    ADDFIX16 @BD, @TMP, @P_LO

    ; return (product_hi << 16) | (product_lo >> 16);
    MOVE16 @P_HI + 2, FixedC
    MOVE16 @P_LO, FixedC + 2

    rts

HGRTest
        * = $CC00
        @MenuIndex  .ds 1       ; menu index
        @Choice     .ds 1       ; user selected menu
        @MenuItem   .ds 2       ; menu text
        @N          .ds 2       ;
        @Y          .ds 1       ;
        @X0         .ds 2       ;
        @Y0         .ds 1       ;
        @RectJump   .ds 1       ;
        * = HGRTest

        lda #$45
        sta Color

        ; set initial menu choice
        lda #0
        sta @Choice

@MainLoop
        ; Turn off graphics
        ; and clear the screen

        jsr HGROff

        ; Clear Text screen
        lda #147
        jsr CHROUT

        ; position cursor top center
        ldx #0
        ldy #16
        clc
        jsr Plot

        ; draw the title
        lda #<@TITLE
        ldy #>@TITLE
        jsr STROUT

@Menu_Refresh
        ; Initialize MenuItem Pointer
        MOVE16I @MenuItem, @Menu
        lda #0
        sta @MenuIndex

@DrawMenuLoop
        ; Position cursor
        ldx @MenuIndex
        inx
        ldy #0
        clc
        jsr Plot

        ; If its the selected menu
        ; turn revese on
        lda @MenuIndex
        cmp @Choice
        bne @DrawMenuItem

        lda #18     ; Reverse On
        jsr CHROUT

@DrawMenuItem
        lda @MenuItem
        ldy @MenuItem + 1
        jsr STROUT

        lda @MenuIndex
        cmp @Choice
        bne @MenuLoopEnd

        lda #146    ; Reverse Off
        jsr CHROUT

@MenuLoopEnd
        INC8 @MenuIndex
        MOVE16 @MenuItem, TMP
        ldy #0

@NextMenuItem
        INC16 TMP
        lda (TMP),y
        bne @NextMenuItem

        ; check for last menu item
        INC16 TMP
        MOVE16 TMP, @MenuItem
        lda (TMP),y
        bne @DrawMenuLoop

;-----------------------------------

@Input_Get
        jsr GETIN
        beq @Input_Get

        cmp #$11    ;down
        beq @Down

        cmp #$91    ;up
        beq @Up

        cmp #$0d    ; Enter
        beq @Enter

        jmp @Input_Get

;-----------------------------------

@Up
        lda @Choice
        beq @Input_Get
        dec @Choice
        jmp @Menu_Refresh

;-----------------------------------

@Down
        lda @Choice
        cmp #12
        bcs @Input_Get
        inc @Choice
        jmp @Menu_Refresh

;-----------------------------------

@Enter
        lda @Choice
        bne @L1
        MOVE8I @RectJump, 0
        jmp @Rect_StartX
@L1
        cmp #1
        bne @L2

        MOVE8I @RectJump, 0
        jmp @Rect_EndX
@L2
        cmp #2
        bne @L3
        MOVE8I @RectJump, 0
        jmp @Rect_StartY
@L3
        cmp #3
        bne @L4
        MOVE8I @RectJump, 0
        jmp @Rect_EndY
@L4
        cmp #4
        bne @L5
        MOVE8I @RectJump, 1
        jmp @Rect_StartX
@L5
        cmp #5
        bne @L6
        MOVE8I @RectJump, 1
        jmp @Rect_EndX
@L6
        cmp #6
        bne @L7
        MOVE8I @RectJump, 1
        jmp @Rect_StartY
@L7
        cmp #7
        bne @L8
        MOVE8I @RectJump, 1
        jmp @Rect_EndY
@L8
        cmp #8
        bne @L9
        jmp @Line
@L9
        cmp #9
        bne @L10
        Lda #0
        sta CirSave
        jmp @Circle
@L10
        cmp #10
        bne @L11
        Lda #1
        sta CirSave
        jmp @Circle        
@L11
        cmp #11
        bne @L12
        jmp @Bez
@L12
        cmp #12
        bne @LEnd
        jmp @Exit
@LEnd
        jmp @Input_Get

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;  RECT X Start
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
@Rect_StartX
        lda #$CD
        sta Color

        jsr HGRCls
        jsr HGROn

        MOVE8I @Y,0
        MOVE8I @Y0,8
        MOVE16I @N,0

@Rect_X_Start_NLoop
; Left Rect

        ; set forecolor
        lda @N
        clc
        adc #2
        jsr _HGRFColor
        lda #1
        jsr _HGRBColor

        ; set X0
        ADD16I @N, 16, X0

        ; set Y0
        MOVE8 @Y0, Y0

        ; set XDist
        MOVE16I XDIST, 79
        SUB16 XDIST, @N, XDIST

        ; Set YDist
        MOVE8I YDIST, 15

        ; Frame or fill rectangle
        jsr @FillFrameRect

; Right Rect

        ; set forecolor
        lda @N
        clc
        adc #3
        jsr _HGRFColor

        ; set X0`
        ADD16I @N, 113, X0

        ; set Y0
        MOVE8 @Y0, Y0

        ; set XDist
        MOVE16I XDIST, 78
        SUB16 XDIST, @N, XDIST

        ; Set YDist
        MOVE8I YDIST, 15

        ; Frame or fill rectangle
        jsr @FillFrameRect

        ; increment Y
        ADD8I @Y0, 32, @Y0
        INC8  @Y

        ; Next N
        ADD16I @N, 2, @N
        lda @N
        cmp #8
        bcs @EndStartX
        jmp @Rect_X_Start_NLoop
@EndStartX
        jmp @WaitForPress

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;  RECT X END
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
@Rect_EndX
        lda #$CD
        sta Color

        jsr HGRCls
        jsr HGROn

        MOVE8I @Y,0
        MOVE8I @Y0,8
        MOVE16I @N,0

@Rect_X_End_NLoop

; Left Rect
        ; set forecolor
        lda @N
        clc
        adc #2
        jsr _HGRFColor
        lda #1
        jsr _HGRBColor

;------------------------------------
        ; set X0
        MOVE16I X0, 16

        ; set Y0
        MOVE8 @Y0, Y0

        ; set XDist
        MOVE16I XDIST, 79
        SUB16 XDIST, @N, XDIST

        ; Set YDist
        MOVE8I YDIST, 15

        ; Frame or fill rectangle
        jsr @FillFrameRect

;------------------------------------

; Right Rect

        ; set forecolor
        lda @N
        clc
        adc #3
        jsr _HGRFColor

        ; set X0
        MOVE16I X0, 112

        ; set Y0
        MOVE8 @Y0, Y0

        ; set XDist
        MOVE16I XDIST, 78
        SUB16 XDIST, @N, XDIST

        ; Set YDist
        MOVE8I YDIST, 15

        ; Frame or fill rectangle
        jsr @FillFrameRect

;------------------------------------

        ; increment Y
        ADD8I @Y0, 32, @Y0
        INC8  @Y

        ; Next N
        ADD16I @N, 2, @N
        lda @N
        cmp #8
        bcs @EndEndX
        jmp @Rect_X_End_NLoop
@EndEndX
        jmp @WaitForPress

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;  RECT Y Start
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
@Rect_StartY
        lda #$CD
        sta Color

        jsr HGRCls
        jsr HGROn

        MOVE8I @Y,0
        MOVE8I @Y0,8
        MOVE16I @N,0

@Rect_Y_Start_NLoop

;---------------------------------
; Left Rect 

        ; set forecolor
        lda @N
        clc
        adc #2
        jsr _HGRFColor
        lda #1
        jsr _HGRBColor

        ; set X0
        MOVE16I X0, 16

        ; set Y0
        MOVE8 @Y0, Y0
        ADD8 Y0, @N, Y0

        ; set XDist
        MOVE16I XDIST, 79

        ; Set YDist
        MOVE8I YDIST, 15
        SUB8 YDIST, @N, YDIST

        ; Frame or fill rectangle
        jsr @FillFrameRect

;---------------------------------
; Right Rect
       ; set forecolor
        lda @N
        clc
        adc #3
        jsr _HGRFColor

        ; set X0
        MOVE16I X0, 112

        ; set Y0
        MOVE8 @Y0, Y0
        ADD8  Y0, @N, Y0
        INC8 Y0

        ; set XDist
        MOVE16I XDIST, 79

        ; Set YDist
        MOVE8I YDIST, 14
        SUB8 YDIST, @N, YDIST

        ; Frame or fill rectangle
        jsr @FillFrameRect

        ; increment Y
        ADD8I @Y0, 32, @Y0
        INC8  @Y

        ; Next N
        ADD16I @N, 2, @N
        lda @N
        cmp #8
        bcs @EndStartY
        jmp @Rect_Y_Start_NLoop
@EndStartY
        jmp @WaitForPress

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;  RECT Y End
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
@Rect_EndY
        lda #$CD
        sta Color

        jsr HGRCls
        jsr HGROn

        MOVE8I @Y,0
        MOVE8I @Y0,8
        MOVE16I @N,0

@Rect_Y_End_NLoop

; Left Rect
        ; set forecolor
        lda @N
        clc
        adc #2
        jsr _HGRFColor
        lda #1
        jsr _HGRBColor

        ; set X0
        MOVE16I X0, 16

        ; set Y0
        MOVE8 @Y0, Y0

        ; set XDist
        MOVE16I XDIST, 79

        ; Set YDist
        MOVE8I YDIST, 15
        SUB8 YDIST, @N, YDIST

        ; Frame or fill rectangle
        jsr @FillFrameRect

 ; Right Rect
       ; set forecolor
        lda @N
        clc
        adc #3
        jsr _HGRFColor

        ; set X0
        MOVE16I X0, 112

        ; set Y0
        MOVE8 @Y0, Y0

        ; set XDist
        MOVE16I XDIST, 79

        ; Set YDist
        MOVE8I YDIST, 14
        SUB8 YDIST, @N, YDIST

        ; Frame or fill rectangle
        jsr @FillFrameRect

        ; increment Y
        ADD8I @Y0, 32, @Y0
        INC8  @Y

        ; Next N
        ADD16I @N, 2, @N
        lda @N
        cmp #8
        bcs @EndEndY
        jmp @Rect_Y_End_NLoop
@EndEndY
        jmp @WaitForPress

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
@FillFrameRect
        ; Frame or fill rectangle
        lda @RectJump
        beq @FrameRect1

        jsr HFillRect
        rts
@FrameRect1
        jmp HRect

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;  Line
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
@Line
        lda #$CD
        sta Color

        jsr HGRCls
        jsr HGROn

        lda #2
        jsr _HGRFColor
        MOVE16I X0, 159
        MOVE8I Y0, 99
        MOVE16I X1, 0
        MOVE8I Y1, 0
        jsr _Line
        
        lda #3
        jsr _HGRFColor
        MOVE16I X1, 79
        jsr _Line
        
        lda #4
        jsr _HGRFColor
        MOVE16I X1, 239
        jsr _Line
        
        lda #5
        jsr _HGRFColor
        MOVE16I X1, 319
        jsr _Line
        
        lda #6
        jsr _HGRFColor
        MOVE8I Y1, 49
        jsr _Line
        
        lda #7
        jsr _HGRFColor
        MOVE8I Y1, 149
        jsr _Line
        
        lda #8
        jsr _HGRFColor
        MOVE8I Y1, 199        
        jsr _Line
        
        lda #9
        jsr _HGRFColor
        MOVE16I X1, 239        
        jsr _Line

        lda #10
        jsr _HGRFColor
        MOVE16I X1, 79        
        jsr _Line

        lda #11
        jsr _HGRFColor
        MOVE16I X1, 0
        jsr _Line

        lda #14
        jsr _HGRFColor
        MOVE8I Y1, 149   
        jsr _Line

        lda #15
        jsr _HGRFColor
        MOVE8I Y1, 49   
        jsr _Line

        lda #2
        jsr _HGRFColor
        MOVE16I X1, 159
        MOVE8I Y1, 0
        jsr _Line

        lda #3
        jsr _HGRFColor
        MOVE8I Y1, 199
        jsr _Line

        lda #4
        jsr _HGRFColor
        MOVE16I X1, 319
        MOVE8I Y1, 99
        jsr _Line

        lda #5
        jsr _HGRFColor
        MOVE16I X1, 0
        jsr _Line

        jmp @WaitForPress

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;  Circle
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
@Circle
        lda #$CD
        sta Color

        jsr HGRCls
        jsr HGROn

        MOVE8I @Y,0
        MOVE8I @Y0,32
        MOVE16I @N,0

@Circle_Loop
;---------------------------------
; Left Circle

        lda @N
        clc
        adc #2
        jsr _HGRFColor

        MOVE16I CX, 32
        MOVE8 @Y0, CY
        ADD8 CY, @N, CY        
        MOVE8I R, 16
        jsr _HCir

;---------------------------------
; Right Circle

        lda @N
        clc
        adc #3
        jsr _HGRFColor

        MOVE16I CX, 112
        MOVE8 @Y0, CY
        ADD8 CY, @N, CY
        INC8 CY
        MOVE8I R, 16
        jsr _HCir

        ; increment Y
        ADD8I @Y0, 40, @Y0
        INC8  @Y

        ; Next N
        ADD16I @N, 2, @N
        lda @N
        cmp #8
        bcs @CircleEnd
        jmp @Circle_Loop
        
@CircleEnd
        jmp @WaitForPress

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;  Bez
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
@Bez
        lda #$CD
        sta Color

        jsr HGRCls
        jsr HGROn

        MOVE8I @Y,0
        MOVE8I @Y0,32
        MOVE16I @N,0

@Bez_Loop
;---------------------------------
; Left Bez

        lda @N
        clc
        adc #2
        jsr _HGRFColor

        MOVE16I BX0, 32
        MOVE16I BX1, 300
        MOVE16I BX2, 32
        
        MOVE8 @Y0, BY0
        ADD8 BY0, @N, BY0        
        ADD8I BY0, 16, BY1
        ADD8I BY1, 16, BY2
        jsr _HBez
                        
;---------------------------------
; Right Bez

        lda @N
        clc
        adc #3
        jsr _HGRFColor

        MOVE16I BX0, 182
        MOVE16I BX1, 450
        MOVE16I BX2, 182
        
        MOVE8 @Y0, BY0
        ADD8 BY0, @N, BY0        
        ADD8I BY0, 16, BY1
        ADD8I BY1, 16, BY2
        jsr _HBez
        
        ; increment Y
        ADD8I @Y0, 40, @Y0
        INC8  @Y

        ; Next N
        ADD16I @N, 2, @N
        lda @N
        cmp #8
        bcs @BezEnd
        jmp @Bez_Loop
        
@BezEnd
;        jmp @WaitForPress  Fall through to @WaitForPress

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
@WaitForPress
        jsr GETIN
        beq @WaitForPress
        jmp @MainLoop

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;  EXIT
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
@Exit
        lda #147
        jsr CHROUT
        rts

;------------------------------------

@TITLE
        .byte "TEST", 0

;------------------------------------
@MENU
        .byte "RECT X START",0
        .byte "RECT X END",0
        .byte "RECT Y START",0
        .byte "RECT Y END",0
        .byte "FILL RECT X START",0
        .byte "FILL RECT X END",0
        .byte "FILL RECT Y START",0
        .byte "FILL RECT Y END",0
        .byte "LINE",0
        .byte "CIRCLE",0
        .byte "FILL CIRCLE",0
        .byte "BEZIER CURVE",0
        .byte "EXIT",0
        .byte 0,0
        rts

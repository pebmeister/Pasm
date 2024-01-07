         ; written by Paul Baxter

        .inc "macros.asm"
        .inc "pagezero.asm"
        .inc "basic.asm"
        .inc "kernal.asm"

;********************************************
        BITMAPBASE = $2000
        COLORBASE = $0400

        ; Some zero page has multiple uses

        XCOORD = LINNUM ; DO NOT CHANGE set by basic parse routine
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
        CURLOCB = BITCI
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
        FixedA      = FREKZP
        FixedB      = FAC1
        FixedC      = FAC2  

        IGONE_SV .equ $3FC

        ;************************************
        ; The values below are globals must
        ; not be modified by basic
        ; The values below will work if you do
        ; not use tape
        ;************************************
        CIRJUMP = CNTDN
        COLOR = CMP0

        .org $8000

        .WORD	@COLD_START		; Cartridge cold-start vector
        .WORD	@WARM_START	    ; Cartridge warm-start vector
        .BYTE	$C3, $C2, $CD, $38, $30		; CBM8O - Autostart key

@COLD_START
;	KERNAL RESET ROUTINE
        stx $D016				; Turn on VIC for PAL / NTSC check
        jsr $FDA3				; IOINIT - Init CIA chips
        jsr $FD50				; RANTAM - Clear/test system RAM
        jsr $FD15				; RESTOR - Init KERNAL RAM vectors
        jsr $FF5B				; CINT   - Init VIC and screen editor
 

;	BASIC RESET  Routine
        jsr $E453				; Init BASIC RAM vectors
        jsr $E3BF				; Main BASIC RAM Init routine
        jsr $E422				; Power-up message / NEW command
        ldx #$FB
        txs					    ; Reduce stack pointer for BASIC
	
@WARM_START
;	START YOUR PROGRAM HERE

        lda #5		            ; CHANGE BORDER COLOUR 
        sta BORDER		        ; 
 
        ;
        ;   install wedge
        ;  
@SETWEDGE 
        NEWBYTES .equ  3
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

@SkipSpace
        lda (BASICTEXT),y
        cmp #' '
        bne @MatchCmd
        INC16 BASICTEXT
        jmp @SkipSpace

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

@Cleanup
        jsr CHRGOT
        beq @Exit2
        jsr CHRGET
        jmp @Cleanup
@Exit2
        DEC16 TXTPTR

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
;
;  HGRBColor
;     Set HGR Background COLOR
;
;********************************************
HGRBColor
        jsr GETBYTC
        txa
        and #$0F
        sta TMP
        lda COLOR
        and #$F0
        ora TMP
        sta COLOR
        rts

;********************************************
;
;  HGRFColor
;     Set HGR Forground COLOR
;
;********************************************
HGRFColor
        jsr GETBYTC
        txa
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
;
;  ClearColor
;     clear Color memory
;
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
;
;  HGROff
;
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
;
;  HGRPlot
;
; Parses Basic Text and plot a point
;
; Calls
;   GETNUM $B7EB
;   _Plot
;
; HGRPlot x,y
;
;********************************************
HGRPlot
        INC16 TXTPTR
        jsr GETNUM ; get address in $14/$15 y next integer at X

        jsr RangeCheckXY

;********************************************
;
;  _Plot
;       XCOORD     IN  2 byte x coordinate
;       X register IN  y coordinate
;
; Plot a single point
; Must not be called Plot because it collides with Kernal
; PLOT    = $E50A
;
; Calls
;   CalcPlot
;
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
;
; CalcColorByte
;
; COLORPTR = x location
; YCOL = y location
;
; Warning:
;     Destroys both COLORPTR and YCOL
;********************************************
CalcColorByte
        lda #0
        sta YCOL + 1
        RSHIFT16 COLORPTR, 3  ; COLORPTR = x / 8
        RSHIFT16 YCOL, 3      ; ycol int(y/8)*40
        MULT40 YCOL

        ADD16 COLORPTR, YCOL, COLORPTR  ; COLORPTR = COLORPTR + ycol
        ADD16I COLORPTR, COLORBASE, COLORPTR ; COLORPTR = COLORPTR + COLORBASE
        rts

;********************************************
;
;  CalcPlot
;       XCOORD IN  2 byte x coordinate
;       X reg  IN  y coordinate
;
;       LOC    OUT  byte of bitemap
;       X reg  OUT  bit of bitmap
;
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
;
;  HGRLine
;
; This routine draws a line
;
; Calls:
;  GETNUM
;  _Line
;
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
;
; _Line
;
;    X0 - start X
;    Y0 - start Y
;    X1 - end X
;    Y1 - end Y
;
;    X0 must be less than X1
; optimized for horizontal lines
;********************************************
_Line   
        ldx Y0
        cpx Y1
        bne NotHorizontal

;---------------------------------------        
        lda X0 + 1
        cmp X1 + 1
        bcc _Line2
;---------------------------------------
        lda X0
        cmp X1
        bcc _Line2
;---------------------------------------
        MOVE16 X1, XCOORD
        SUB16 X0, X1, XDIST
                
        jmp CalcPlotHLine

;********************************************
;*                                          *
;*  _Line2                                  *
;*    Draw Horizontal line                  *
;*    X0 must be less than X1               *
;*                                          *
;*    X0 - start X                          *
;*    Y0 - start Y                          *
;*    X1 - end X                            *
;*    Y1 - end Y                            *
;*                                          *
;********************************************
_Line2
        MOVE16 X0,XCOORD
        SUB16 X1, X0, XDIST

        lda XDIST + 1
        bne CalcPlotHLine
        
        lda XDIST
        cmp #$08
        bcc NotHorizontal               

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
        lda X0 + 1
        cmp X1 + 1
        bcc @X1_larger

        lda X0
        cmp X1
        bcc @X1_larger

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
        ; Save start, distance and end
        MOVE16 COLORPTR, StartX

        ; get the start byte
        jsr CalcColorByte

        MOVE16 XDIST, XD
        INC16 XD

        lda StartX
        and #%00000111
        beq @InitLoop

; -------------------------------------------
        ; if XD < 8 we cant do this
        tay
        
        lda XD + 1
        bne @Sub
        lda XD
        cmp #8
        bcc @Exit
        
        ; subtract 8 - N from XD
@Sub
        tya
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
; XDIST IN x distance
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
        ldx XDIST       ; set x to XDIST
        jmp @HLastByte

;----------------------------------------------
@HFirstByte
        stx  COUNTER    ; save in counter
        lda #0          ; zero out high byte
        sta COUNTER + 1
;----------------------------------------------
        ; substract COUNTER from distance of first byte
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
        cmp #8          ; compare it to 8
        beq @HLoop1End  ; if it is equal then go to the last byte
        bcs @HLoop_Cont ; continue if > 8
        jmp @HLoop1End  ; less than 8 go to last byte

; -------------------------------------------
@HLoop_Cont             ; add 8 to current pointer
        ADD16I CURLOC, 8, CURLOC
        lda #$FF        ; load a with FF
        sta (CURLOC),y  ; store the byte

                        ; Increrment COUNTER by 8
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
        lda Y0

; -------------------------------------------
@YLOOP
        sta Y1
        jsr _Line2

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
        jsr _Line2

        ; ------------------------------
        ;
        ; Bottom Horizontal Line
        ;
        ; ------------------------------
        lda YMAX
        sta Y0
        sta Y1
        jsr _Line2

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
;  SetCircleJump
;
;  set routine for circle to plot or fill
;
;********************************************
SetCircleJump
        asl
        tax
        lda JumpTable, x
        sta CIRJUMP
        inx
        lda JumpTable, x
        sta CIRJUMP + 1
        rts

;********************************************
;
;  JumpCircle
;
;  SetCircleJump MUST be called first
;  This will fill or plot for circles
;
;********************************************
JumpCircle
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
        jsr SetCircleJump
        jmp HGRCircle2

;********************************************
;
;  HGRFillCircle
;
;  Fill a circle from asm
;
;*******************************************
HGRFillCircle2
        lda #1
        jsr SetCircleJump
        jmp HGRCircle3

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
        jsr SetCircleJump

HGRCircle2
        INC16 TXTPTR    ; advance basic textptr

        ; get the start position, xdistance and ydistance

        jsr GETNUM ; get address in $14/$15 y next integer at X
        stx CY
        MOVE16 XCOORD, CX
        jsr GETBYTC
        stx R

        jsr RangeCheckCircle

HGRCircle3
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
        jsr JumpCircle

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
        jsr JumpCircle
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
        ; PutPixel(CX - CURX, CY + CURY);
        SUB16 CX, CURX, XCOORD
        ADD8 CY, CURY, Y1
        tax
        jsr _Plot

;----------------------------------------------

        ; PutPixel(CX + CURX, CY + CURY);
        ADD16 CX, CURX, XCOORD
        ldx Y1
        jsr _Plot

;----------------------------------------------

        ; PutPixel(CX - CURX,  CY - CURY);
        SUB16 CX, CURX, XCOORD
        SUB8 CY, CURY, Y1
        tax
        jsr _Plot

;----------------------------------------------

        ; PutPixel(CX + CURX, CY - CURY);
        ADD16 CX, CURX, XCOORD
        ldx Y1
        jsr _Plot

;----------------------------------------------

        ; PutPixel(CX - CURY, CY + CURX);
        SUB168 CX, CURY, XCOORD
        ADD8 CY, CURX, Y1
        tax
        jsr _Plot

;----------------------------------------------

        ; PutPixel(CX + CURY, CY + CURX);
        ADD168 CX, CURY, XCOORD
        ldx Y1
        jsr _Plot

;----------------------------------------------

        ; PutPixel(CX - CURY, CY - CURX);
        SUB168 CX, CURY, XCOORD
        SUB8 CY, CURX, Y1
        tax
        jsr _Plot

;----------------------------------------------

        ; PutPixel(CX + CURY, CY - CURX);
        ADD168 CX, CURY, XCOORD
        ldx Y1
        jmp _Plot

;********************************************
;
;  PlotFillCircle
;
;  fill circle pixels
;
;*******************************************
PlotFillCircle
        ; Line(CX - CURY, CY + CURX) to (CX + CURY, CY + CURX)
        SUB168 CX, CURY, X0
        ADD8 CY, CURX, Y0
        ADD168 CX, CURY, X1
        MOVE8 Y0, Y1
        jsr _Line2

;----------------------------------------------

        ; Line (CX - CURY, CY - CURX) to (CX + CURY, CY - CURX)
        SUB168 CX, CURY, X0
        SUB8 CY, CURX, Y0
        ADD168 CX, CURY, X1
        MOVE8 Y0, Y1
        jsr _Line2

;----------------------------------------------

        ; Line(CX - CURX, CY + CURY) to (CX + CURX, CY + CURY)
        SUB16 CX, CURX, X0
        ADD8 CY, CURY, Y0
        ADD16 CX, CURX, X1
        MOVE8 Y0, Y1
        jsr _Line2

;----------------------------------------------

        ; Line(CX - CURX, CY - CURY) to (CX + CURX, CY - CURY)
        SUB16 CX, CURX, X0
        SUB8 CY, CURY, Y0
        ADD16 CX, CURX, X1
        MOVE8 Y0, Y1
        jmp _Line2


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
HGRBez  

;----------------------------------
; local variables
;----------------------------------
        * = $CE00
@LASTX  .ds 2
@LASTY  .ds 1
@A1     .ds 4
@B1     .ds 4
@C1     .ds 4
@X0     .ds 2
@Y0     .ds 1
@X1     .ds 2
@Y1     .ds 1
@X2     .ds 2
@Y2     .ds 1
@T      .ds 4
@ONE_MINUS_T .ds 4
@A      .ds 4
@B      .ds 4
@C      .ds 4
@FIXX   .ds 4
@FIXY   .ds 4

;----------------------------------
        * = HGRBez
        
        INC16 TXTPTR    ; advance basic textptr

        ; get the start position, xdistance and ydistance
        jsr GETNUM ; get address in $14/$15 y next integer at X
        stx @Y0
        MOVE16 XCOORD, @X0

        jsr CHKCOM
        jsr GETNUM ; get address in $14/$15 y next integer at X
        stx @Y1
        MOVE16 XCOORD, @X1
        
        jsr CHKCOM
        jsr GETNUM ; get address in $14/$15 y next integer at X
        stx @Y2
        MOVE16 XCOORD, @X2

        ;   const auto inc = fix16_from_dbl(1.0 / sz);
        ;   auto t = F16(0);
        ;   constexpr auto max = F16(1);
        ;   constexpr auto two = F16(2);
        MOVE16I @T, 0        
        MOVE16I @T + 2, $0
        
        MOVE16 @X0, @LASTX
        MOVE8 @Y0, @LASTY
        
@WHILE_T
        ;     while (t <= max)horacemann        
        lda @T
        beq @ContinueWhile
        rts

@ContinueWhile
        ;  const auto one_minus_t = F16(1) - t;
        SUBFIX16 @ONE, @T, @ONE_MINUS_T
        
;----------------------------------------------      
; calculate a1,b1 and c1
;----------------------------------------------        
        ; (1 - t) * (1 - t)
        ; const fix16_t a1 = fix16_mul(one_minus_t, one_minus_t);
        MOVE32 @ONE_MINUS_T, FixedA
        MOVE32 @ONE_MINUS_T, FixedB
        jsr FIX16_MUL
        MOVE32 FixedC, @A1

        ; 2 * (1 - t) * t
        ; fix16_t b1 = fix16_mul(two, one_minus_t);
        ; b1 = fix16_mul(b1, t);
        MOVE32 @TWO, FixedA
        MOVE32 @ONE_MINUS_T, FixedB
        jsr FIX16_MUL
        MOVE32 FixedC, @B1

        MOVE32 @B1,FixedA
        MOVE32 @T, FixedB
        jsr FIX16_MUL
        MOVE32 FixedC, @B1

        ;  c1 = fix16_mul(t, t);
        MOVE32 @T, FixedA
        MOVE32 @T, FixedB
        jsr FIX16_MUL
        MOVE32 FixedC, @C1
        
;-----------------------------------------
;        Calculate FixedX
;-----------------------------------------
        ;   a = fix16_mul(a1, x0);
        MOVE32 @A1, FixedA
        MOVE16I FixedB, 0
        MOVE16 @X0, FixedB + 2
        jsr FIX16_MUL
        MOVE32 FixedC, @A   

        ;   b = fix16_mul(b1, x1);        
        MOVE32 @B1, FixedA
        MOVE16 @X1, FixedB + 2              
        jsr FIX16_MUL
        MOVE32 FixedC, @B
     
        ;  c = fix16_mul(c1, x2);
        MOVE32 @C1, FixedA
        MOVE16 @X2, FixedB + 2
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
        MOVE8 @Y0, FixedB + 2
        jsr FIX16_MUL
        MOVE32 FixedC, @A

        ;   b = fix16_mul(b1, y1);
        MOVE32 @B1, FixedA
        MOVE8 @Y1, FixedB + 2
        jsr FIX16_MUL
        MOVE32 FixedC, @B
        
        ;  c = fix16_mul(c1, y2);
        MOVE32 @C1, FixedA
        MOVE8 @Y2, FixedB + 2
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
        bne @DO_Line
        
        lda @FIXX + 3       
        cmp @LASTX + 1
        bne @DO_Line

        lda @FIXY + 2
        cmp @LASTY
        beq @No_Line

@DO_Line
        ; X0 MUST BE <= X1!
        
        ;16-bit number comparison...
        ;
        lda @FIXX + 3         ;MSB of 1st number
        cmp @LASTX + 1        ;MSB of 2nd number
        bcc @X0_Lower_Same    ;X0 < X1
        ;
        bne @X1_Larger        ;X1 > X0
        ;
        lda @FIXX + 2         ;LSB of 1st number
        cmp @LASTX            ;LSB of 2nd number
        bcc @X0_Lower_Same    ;X0 < X1
        ;
        beq @X0_Lower_Same    ;X0 = X1
        ;
        bne @X1_Larger        ;X > Y
         
@X0_Lower_Same       
        MOVE16 @FIXX + 2, X0
        MOVE8 @FIXY + 2, Y0
        MOVE16 @LASTX, X1
        MOVE8 @LASTY, Y1
        jmp @DrawLine
        
@X1_Larger
        MOVE16 @FIXX + 2, X1
        MOVE8 @FIXY + 2, Y1
        MOVE16 @LASTX, X0
        MOVE8 @LASTY, Y0
 
@DrawLine
        jsr @RangeCheckLine
        jsr _Line

        MOVE16 @FIXX + 2, @LASTX
        MOVE8 @FIXY + 2, @LASTY
                
@NO_Line        
        ; t += inc;
        ADDFIX16 @T, @INC, @T
        jmp @WHILE_T

;---------------------------

@RangeCheckLine
        lda Y0
        cmp # 200
        bcs @RangeError

        lda Y1
        cmp # 200
        bcs @RangeError

@CheckX1
        lda X1 + 1
        beq @Exit

        lda X1
        cmp # 320 - 256
        bcs @RangeError

@Exit
        rts
@RangeError
        ldx # ILLEGALQUANITY
        jmp ERROR

@INC    .word 0, $0210
@ONE    .word 1, 0
@TWO    .word 2, 0
    
;****************************************************
;*                                                  *
;* FIX16_MUL                                        *
;*                                                  *
;*  fix16 multiply                                  *
;*                                                  *
;*  16bit integer    16bit Fraction                 *
;*                                                  *
;*  FixedA  IN:  4 bytes 1st number to multiply     *
;*  FixedB  IN:  4 bytes 2nd number to multiply     *
;*  FixedC OUT:  4 bytes result                     *
;*                                                  *
;*	// Each argument is divided to 16-bit parts.    *
;*  destroys a, x, y                                *
;*                                                  *
;****************************************************

FIX16_MUL

    @A          = FixedA
    @B          = FixedA + 2
    @C          = FixedB
    @D          = FixedB + 2

    @AC         = UNUSED2
    @AD         = BITCI
    @TMP        = TMPDATA
    @BD         = TAPE1
    @CB         = ARGSGN

    * = $CF00    
    @AD_CB      .ds 4
    @P_HI       .ds 4
    @P_LO       .ds 4
    * = FIX16_MUL

    ;	int32_t AC = A*C;
    ;	int32_t AD_CB = A*D + C*B;
    ;	uint32_t BD = B*D;
    FMUL @A, @C, @AC, @TMP
    FMUL @A, @D, @AD, @TMP
    FMUL @C, @B, @CB, @TMP
    FMUL @B, @D, @BD, @TMP
   
    ADDFIX16 @AD, @CB, @AD_CB 
  
    ;	int32_t product_hi = AC + (AD_CB >> 16);
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

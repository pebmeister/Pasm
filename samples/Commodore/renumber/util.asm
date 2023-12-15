;********************************************
;*                                          *
;*  LASTLINE                                *
;*                                          *
;*  set the Z flag if we are at the end OF  *
;*  basic program.                          *
;*  nxtln must point to current line        *
;*                                          *
;*  destroys a,y                            *
;*                                          *
;********************************************
LASTLINE
        ;
        ;   load the pointer to next line
        ;   00 00 is the end of program
        ;
        ldy #NXOFF
        lda (NXTLN),y
        bne +
        iny
        lda (NXTLN),y
+
        rts

;********************************************
;*                                          *
;*  BINBCD16                                *
;*                                          *
;*  convert 16 bit number to bcd and a      *
;*   string                                 *
;*  bnvar must contain number to convert    *
;*  bcd on exit contains bcd number 3 bytes *
;*                                          *
;*  bcdstr on exit contains bcd string      *
;*                                          *
;*  destroys    a,y                         *
;*                                          *
;********************************************
BINBCD16
        ;
        ;   set decimal code
        ;   clear result
        ;
        sed
        lda #0
        sta BCD
        sta BCD + 1
        sta BCD + 2

        ;
        ;   loop 16 bits
        ;
        ldy #16
-
        asl BNVAR               ;   shift bnvar left by 1
        rol BNVAR + 1
        lda BCD                 ;   shift into bcd
        adc BCD
        sta BCD
        lda BCD + 1             ;   shift into bcd + 1
        adc BCD + 1
        sta BCD + 1
        lda BCD + 2             ;   shift into bcd + 2
        adc BCD + 2
        sta BCD + 2

        dey                     ;   check for end of loop
        bne -

        ;
        ;   clear decimal mode
        ;
        cld

        ;
        ;   convert bcd 3 bytes to a string
        ;
        BCD2STR BCD + 0, BCDSTR + 3
        BCD2STR BCD + 1, BCDSTR + 1
        lda BCD + 2
        and #$0F
        ora #'0'
        sta BCDSTR
        rts
   
;********************************************
;*                                          *
;*  BCDLENGTH                               *
;*                                          *
;*  get length of BCDSTR                    *
;*  store result in BCDLEN                  *
;*                                          *
;*  destroys    a,y                         *
;*                                          *
;********************************************
BCDLENGTH
        ;
        ;   loop through 5 chars
        ;
        ldy #0
        sty BCDLEN
-
        lda BCDSTR,y
        cmp #'0'
        bne +

        cpy #4
        beq +

        ldx BCDLEN
        beq ++
+
        sty BCDLEN
        lda #5
        sec
        sbc BCDLEN
        sta BCDLEN
        rts
+
        iny
        bne -
     
FROM    = MINIDX
TO      = MAXIDX
SIZEL   = CURIDX
SIZEH   = CURIDX + 1
;********************************************
;*                                          *
;* MOVELINESDOWN                            *
;*                                          *
;*  move basic lines to get rid of extra    *
;*  space                                   *
;*                                          *
;********************************************
MOVELINESDOWN
        ;
        ;   save nxtln
        ;
        MOVE16 NXTLN,+

        ;
        ;   set MINIDX to endptr
        ;
        MOVE16 NXTLN,MINIDX

        clc
        lda LININS
        adc MINIDX
        sta MINIDX
        lda #0
        adc MINIDX + 1
        sta MINIDX + 1

        ;
        ;   find out how many more bytes we need
        ;
        sec
        lda LNUMLEN
        sbc BCDLEN
        sta LASTIDX

        ;
        ;   fix basic variables
        ;
        sec
        lda STREND
        sbc LASTIDX
        sta STREND
        lda STREND+1
        sbc #0
        sta STREND+1

        ;
        ;   subtract from the line start pointers
        ;
-
        ldy #NXOFF
        sec
        lda (NXTLN),y
        sta MAXIDX
        sbc LASTIDX
        sta (NXTLN),y
        iny
        lda (NXTLN),y
        sta MAXIDX + 1
        sbc #0
        sta (NXTLN),y

        ;
        ;   copy maxidx to nxtln
        ;
        MOVE16 MAXIDX,NXTLN

        ;
        ;   check for end of basic
        ;
        jsr LASTLINE
        bne -

        ;
        ;   figure out numbytes to move
        ;
        SUB16 MAXIDX,MINIDX,CURIDX

        ;
        ;   add EOL and end of basic marker
        ;
        ADD16I CURIDX,4,CURIDX

        ;
        ;   set max to new dest address
        ;         
        clc
        lda MINIDX
        adc LASTIDX
        sta MAXIDX
        lda MINIDX + 1
        adc #0
        sta MAXIDX + 1

        ;
        ;   we need to swap maxidx and minidx
        ;
        MOVE16 MINIDX,NXTLN
        MOVE16 MAXIDX,MINIDX
        MOVE16 NXTLN,MAXIDX

        jsr MOVEDOWN

        ;
        ;   restore nxtln
        ;
        MOVE16 +,NXTLN
        rts

+       .word 0

;********************************************
;*                                          *
;* MOVELINESUP                              *
;*                                          *
;*  move basic lines to add room            *
;*                                          *
;********************************************
MOVELINESUP
        ;
        ;   save nxtln
        ;
        MOVE16 NXTLN,+
        ;
        ;   set MINIDX to endptr
        ;
        MOVE16 NXTLN,MINIDX

        clc
        lda LININS
        adc MINIDX
        sta MINIDX
        lda #0
        adc MINIDX + 1
        sta MINIDX + 1

        ;
        ;   find out how many more bytes we need
        ;
        sec
        lda BCDLEN
        sbc LNUMLEN
        sta LASTIDX

        ;
        ;   fix basic variables
        ;
        clc
        adc STREND
        sta STREND
        lda #0
        adc STREND+1
        sta STREND+1

        ;
        ;   add to the line start pointers
        ;
-
        ldy #NXOFF
        clc
        lda (NXTLN),y
        sta MAXIDX
        adc LASTIDX
        sta (NXTLN),y
        iny
        lda (NXTLN),y
        sta MAXIDX + 1
        adc #0
        sta (NXTLN),y

        ;
        ;   copy maxidx to nxtln
        ;
        MOVE16 MAXIDX,NXTLN

        ;
        ;   check for end of basic
        ;
        jsr LASTLINE
        bne -

        ;
        ;   figure out numbytes to move
        ;
        SUB16 MAXIDX,MINIDX,CURIDX

        ;
        ;   add EOL and end of basic marker
        ;
        ADD16I CURIDX,4,CURIDX

        ;
        ;   set max to new dest address
        ;
        clc
        lda MINIDX
        adc LASTIDX
        sta MAXIDX
        lda MINIDX + 1
        adc #0
        sta MAXIDX + 1

        jsr MOVEUP

        ;
        ;   restore nxtln
        ;
        MOVE16 +,NXTLN
        rts

+       .word 0

;********************************************
;*                                          *
;* Move memory up                           *
;*                                          *
;* FROM = source start address              *
;*   TO = destination start address         *
;* SIZE = number of bytes to move           *
;*                                          *
;********************************************
MOVEUP
        ldx SIZEH               ; the last byte must be moved first
        clc                     ; start at the final pages of from and to
        txa
        adc FROM + 1
        sta FROM + 1
        clc
        txa
        adc TO + 1
        sta TO + 1
        inx                     ; allows the use of bne after the dex below
        ldy SIZEL
        beq ++
        dey                     ; move bytes on the last page first
        beq +
-
        lda (FROM),y
        sta (TO),y
        dey
        bne -
+
        lda (FROM),y            ; handle Y = 0 separately
        sta (TO),y
+
        dey
        dec FROM + 1            ; move the next page (if any)
        dec TO + 1
        dex
        bne -

        rts

;********************************************
; Move memory down                          *
;                                           *
; FROM = source start address               *
;   TO = destination start address          *
; SIZE = number of bytes to move            *
;                                           *
;********************************************
MOVEDOWN
        ldy #0
        ldx SIZEH
        beq +
-
        lda (FROM),y            ;   move a page at a time
        sta (TO),y
        iny
        bne -
        inc FROM + 1
        inc TO + 1
        dex
        bne -
+
        ldx SIZEL
        beq +
-
        lda (FROM),y            ;   move the remaining bytes
        sta (TO),y
        iny
        dex
        bne -
+
        rts


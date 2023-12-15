;********************************************
;*                                          *
;* INSERTLINENUM                            *
;*                                          *
;*  insert new linenumber in basic program  *
;*                                          *
;********************************************
INSERTLINENUM
@BCDSZ  = 5

        PUSHAY
        ;
        ;   we found the line
        ;   now calculate the new line number based on index
        ;
        lsr CURIDX + 1          ; curidx >>= 1
        ror CURIDX

        ;
        ; multiply by line increment and store in product
        ;
        MULT16 CURIDX,INCR,PRODUCT

        ;
        ;   add start line
        ;   store in BNVAR
        ;
        ADD16 LSTART,PRODUCT,BNVAR

        jsr BINBCD16

        jsr BCDLENGTH

        lda LNUMLEN
        cmp BCDLEN
        bne +

        ;
        ;   line numbers are the same length
        ;
        ldy LININS
        ;
        ;   get offset to bcdstring
        ;
-
        sec
        lda #@BCDSZ
        sbc BCDLEN
        tax
        ;
        ;   copy string into line
        ;
-
        lda BCDSTR,x
        sta (NXTLN),y
        iny
        inx
        cpx #@BCDSZ
        bne -

        tya
        tax
        dex

        POPAY
        rts
+
        bcc +

        ;
        ;   new line number is smaller
        ;
        jsr MOVELINESDOWN
-
        ldy LININS
        bne ---

        ;
        ;   new line number is larger
        ;
+
        jsr MOVELINESUP
        jmp -


;********************************************
;*                                          *
;* GETLINENUM                               *
;*                                          *
;*  get line number in basic line           *
;*  (nxtln),y points to lnumber     		*
;*                                          *
;*  on exit lnum = line number              *
;*          LININS = y offset               *
;*                                          *
;********************************************
GETLINENUM
        ;
        ;   clear lnum
        ;
        lda #0
        sta LNUM
        sta LNUM + 1
        sta LNUMLEN
        sty LININS
-
        lda (NXTLN),y
        cmp #'0'
        bcc +
        cmp #'9' + 1
        bcs +

        ;
        ;   increment line length
        ;
        inc LNUMLEN

        ;
        ;   a now contains a digit
        ;   multiply lnum * 10
        ;   add digit
        ;
        PUSHAY                  ;   save registers
        MULT10 LNUM             ;   multiply by 10
        POPAY                   ;   restore registers

        and #~'0'               ;   mask off string
        clc                     ;   add to lnum
        adc LNUM
        sta LNUM
        lda LNUM + 1
        adc #0
        sta LNUM + 1
        iny
        bne -                   ;   get the next digit
+
        rts


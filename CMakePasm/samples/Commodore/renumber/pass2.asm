;********************************************
;*                                          *
;* PASS2                                    *
;*                                          *
;*  reset current line to beginning         *
;*  find goto gosub then go to              *
;*  lookup line in table                    *
;*  if it fits replace if not move lines    *
;*  so it does fit                          *
;*                                          *
;********************************************
PASS2
        lda BORDER
        pha

        ;
        ;   set NXTLN
        ;
        MOVE16 TXTTAB,NXTLN     ;   load start of basic
                                ;   into next line
        ;
        ;   clear quote flag
        ;
-
        lda #0
        sta INQUOTE

        ;
        ;   check for last line of basic
        ;
        jsr LASTLINE
        bne +                   ;   continue

        pla
        sta BORDER

        ;
        ;   exit
        ;
        rts

        ;
        ;   init y to point to first char in line   -1
        ;
+
        ldy #TXTOFF - 1
        lda #0
-
        ;
        ;   reg a contains prevch
        ;   skip spaces
        ;
        cmp #SPACE
        beq +                   ;   dont save a in prevch

        ;
        ;   save non space
        ;
        sta PREVCH
+
        ;
        ;   get the next char in line
        ;
        iny                     ;   increment offset
        lda (NXTLN),y           ;   get the char
        beq +++                 ;   00 signifies eol

        ;
        ;   see if we got a quote
        ;
        cmp #QUOTE
        beq ++

        ;
        ;   see if we are in a string
        ;
        ldx INQUOTE
        bne -                   ;   if we are in a string skip

        ;
        ;   start looking for tokens
        ;
        cmp #GOTO_TOK           ;   check for goto
        beq +
        cmp #GOSUB_TOK          ;   check for gosub
        beq +
        cmp #THEN_TOK           ;   check for then
        beq +
        cmp #TO_TOK             ;   check for to
        bne -
        ldx PREVCH              ;   check for go before to
        cpx #GO_TOK
        bne -
+
-
        iny                     ;   increment offset
        lda (NXTLN),y           ;   get the char
        beq ++                  ;   00 signifies eol

        cmp #':'                ;   if we have a colon continue with rest of line
        beq --

        cmp #SPACE              ;   if we have a space skip it
        beq -

        cmp #','                ;   if we have a comma skip it
        beq -

        cmp #'0'
        bcc --                  ;   less than '0'
        cmp #'9' + 1            ;   greater than '9'
        bcs --

        ;
        ;   we have a line number
        ;

        jsr GETLINENUM          ;   parse linenumber in basic line
        jsr FINDLINENUM         ;   look up old linenumber and get new line number
        jsr INSERTLINENUM       ;   insert the new line number

        txa                     ;   x contains current pointer in line
        tay
        jmp -

        ;
        ;   we got a quote
        ;
+
        lda INQUOTE             ;   load quote flag
        eor #1                  ;   flip the bit
        sta INQUOTE             ;   save the result
        jmp --                  ;   continue with rest of line

        ;
        ;   get the next line ptr
        ;   store in nxtln
        ;
+
        ldy #NXOFF              ; offset from line offset
        lda (NXTLN),y           ; load next line offset
        tax                     ; save in x
        iny
        lda (NXTLN),y           ; get next byte
        stx NXTLN               ; set next line
        sta NXTLN + 1

        INC BORDER

        jmp ---


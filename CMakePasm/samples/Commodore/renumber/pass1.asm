;****************************************
;*                                      *
;* PASS1                                *
;*                                      *
;*  create table for old and new lines  *
;*  update line numbers                 *
;*  gotos etc will be fixes in pass2    *
;*                                      *
;****************************************
PASS1
        ;
        ; check for last line of basic
        ;
        jsr LASTLINE
        bne +
        jmp ++++
+
        ;
        ; save original line number in table
        ;
        jsr SAVETBL

        ;
        ; update the line number in basic
        ;
        ldy #LNOFF
        lda LNUM
        sta (NXTLN),y
        lda LNUM + 1
        iny
        sta (NXTLN),y

        ;
        ; add line increment to line number
        ;
        ADD16 LNUM,INCR,LNUM
        bcc ++

        ;
        ;   We wrapped. Line number too big
        ;   restore original lines and then error out

        ;
        ;   set NXTLN
        ;
        MOVE16 TXTTAB,NXTLN     ;   load start of basic into next line   
        MOVE16 TBL,TBLTMP
-
        ldy #0
        lda (TBLTMP),y
        ldy #LNOFF
        sta (NXTLN),y
        ldy #1
        lda (TBLTMP),y
        ldy #LNOFF + 1
        sta (NXTLN),y

        lda TBLSZ
        bne +
        lda TBLSZ + 1
        bne +

        sec
        rts
+
        DEC16 TBLSZ
        ADD16I TBLTMP,2,TBLTMP

        ldy #NXOFF
        lda (NXTLN),y
        tax
        iny
        lda (NXTLN),y
        stx NXTLN
        sta NXTLN + 1
        jmp -

        ;
        ; get the next line ptr
        ; store in nxtln
        ;
+
        ldy #NXOFF
        lda (NXTLN),y
        tax
        iny
        lda (NXTLN),y
        stx NXTLN
        sta NXTLN + 1

        ;
        ; increment table size
        ;
        INC16 TBLSZ

        jmp PASS1

+
        ;
        ; clear carry to signal success
        ;
        clc
        rts

;****************************************
;*                                      *
;* SAVETBL                              *
;*                                      *
;*  add the current line to the table   *
;*  nxtln must point to start of line   *
;*                                      *
;*  destroys a,y                        *
;*                                      *
;****************************************
SAVETBL
        ;
        ; move line number to bnvar
        ;
        ldy #LNOFF
        lda (NXTLN),y
        tax
        iny
        lda (NXTLN),y
        stx BNVAR
        sta BNVAR + 1

        ;
        ; tbltmp points to next spot in table
        ; store line number contained bnvar in table
        ;
        ldy #0
        lda BNVAR
        sta (TBLTMP),y
        lda BNVAR + 1
        iny
        sta (TBLTMP),y

        ;
        ; increment table ptr by 2 bytes
        ;
        ADD16I TBLTMP,2,TBLTMP
        rts


;********************************************
;*                                          *
;*  WEDGE                                   *
;*                                          *
;********************************************
WEDGE
        lda CURLIN + 1
        cmp #$FF
        beq +

        jmp (IGONE_SV)

+
        ldy #1
-
        lda @RENUMCMD,y
        beq +

        cmp (TXTPTR),y
        bne +++++
        iny
        bne -
+
        dey
-
        jsr CHRGET
        dey
        bne -
        ;
        ;   init line number to 100
        ;
        MOVE16I LSTART,100

        ;
        ;   init line increment
        ;
        MOVE16I INCR,10

        ldy #1
        lda (TXTPTR),y
        beq +++

        jsr CHRGET
+
        jsr @GETLN
        ldy LNUMLEN
        beq +++
        jsr @VALIDATE

        MOVE16 LNUM,LSTART

        dey
        beq +
-
        jsr CHRGET
        dey
        bne -
+
        ldy #1
        lda (TXTPTR),y
        beq +

        jsr CHRGET
        cmp #','
        bne ++

        ldy #1
        lda (TXTPTR),y
        beq +

        jsr CHRGET

        jsr @GETLN
        ldy LNUMLEN
        beq ++
        jsr @VALIDATE

        MOVE16 LNUM,INCR

        dey
        beq +
-
        jsr CHRGET
        dey
        bne -
+
        jsr START
        bcc +

        ldx #ILLEGALQUANITY
        jmp ERROR

+
        jmp (IGONE_SV)
@GETLN
        MOVE16 TXTPTR,NXTLN
        ldy #0
        jmp GETLINENUM
@RENUMCMD
        .byte " RENUM",0

@VALIDATE
        PUSHAY
        lda LNUMLEN         ; Line length must be less than 6
        cmp #6
        bcs ++

        cmp #5
        bcc +

        ldy LININS          ; get next char
        lda (NXTLN),y
        cmp #'7'
        bcs ++

        iny
        lda (NXTLN),y
        cmp #'4'
        bcs ++
+
        POPAY
        rts
+
        ldx #ILLEGALQUANITY
        jmp ERROR


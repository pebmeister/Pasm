;
; written by Paul Baxter
;
        *=  $C000
        TBLLOC  = $A000

        NEWBYTES = 3            ; number of 0 bytes used to perform basic new

;********************************************
;*                                          *
;*  INSTALL                                 *
;*                                          *
;********************************************
INSTALL

        cli

        ;
        ;   install wedge
        ;
        MOVE16 IGONE,IGONE_SV
        MOVE16I IGONE,WEDGE

        ;
        ;   set NXTLN
        ;
        lda TXTTAB              ;   load start of basic
        sta NXTLN               ;   into next line
        sta STREND
        lda TXTTAB + 1
        sta NXTLN + 1
        sta STREND + 1

        lda #0                  ; peform a NEW for basic
        ldy #NEWBYTES
-
        sta (NXTLN),y
        dey
        bne -

        ADD16I STREND,2,STREND
        MOVE16 MEMSIZ,FRETOP

        lda STREND
        sta VARTAB
        sta ARYTAB
        lda STREND + 1
        sta VARTAB + 1
        sta ARYTAB + 1

        sei

        jmp WARM


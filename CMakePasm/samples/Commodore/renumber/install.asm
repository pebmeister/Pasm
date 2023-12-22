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
        ;
        ;   install wedge
        ;
        sei
        
        MOVE16 IGONE, IGONE_SV
        MOVE16I IGONE, WEDGE

        lda #0                  ; peform a NEW for basic
        ldy #NEWBYTES
-
        sta (TXTTAB),y
        dey
        bne -

        ;
        ;   set NXTLN
        ;
        lda TXTTAB              ;   load start of basic
        sta NXTLN               ;   into next line
        sta STREND
        lda TXTTAB + 1
        sta NXTLN + 1
        sta STREND + 1

        lda STREND
        sta VARTAB
        sta ARYTAB
        lda STREND + 1
        sta VARTAB + 1
        sta ARYTAB + 1

        cli
        
        jmp WARM
        
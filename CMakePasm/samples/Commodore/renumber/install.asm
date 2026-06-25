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

		jsr SCRTCH           	; Call the BASIC "NEW" routine


        ;
        ;   set NXTLN
        ;
        lda TXTTAB              ;   load start of basic
        sta NXTLN               ;   into next line
        lda TXTTAB + 1
        sta NXTLN + 1

        cli
        
        jmp WARM
        

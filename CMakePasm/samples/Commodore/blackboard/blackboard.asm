;
; BlackBoard
;
    .org $8000
;
    COLOR   .equ $10
    BASE    .equ $2000
    SCROLY  .equ $D011
    VMCSB   .equ $D011
    COLMAP  .equ $0400
;
    HMAX    .equ 320
    VMAX    .equ 200
    HMID    .equ 160
    VMID    .equ 100
;
    SCRLEN  .equ 8000
    MAPLEN  .equ 1000
;
    TEMPA   .equ $FB    
    TEMPB   .equ TEMPA+2
;
    TABPTR  .equ TEMPA
    TABSIZ  .equ $9000
;
    HPSN    .equ TABSIZ+2
    VPSN    .equ HPSN+2
    CHAR    .equ VPSN+1
    ROW     .equ CHAR+1
    LINE    .equ ROW+1
    BYTE    .equ LINE*1
    BITT    .equ BYTE+2
;
    MPRL    .equ BITT+1
    MPRH    .equ MPRL+1
    MPDL    .equ MPRH+1
    MPDH    .equ MPDL+1
    PRODL   .equ NPDH+1
    PRODH   .equ PRODL+1
;
    FILVAL  .equ PRODH+1
    JSV     .equ FILVAL+1
;
; CIAPRA .equ $DC00
;
            JMP START
;
; BLOCK FILL ROUTINE
;
BLKFIL      LDA FILVAL
            LDX TABSIZ+1
            BEQ PARTPG
            LDY #0
FULLPG      STA (TABPTR),Y
            INY
            BNE FULLPG
            INC TABPTR+1
            
            BNE FULLPG
PARTPG      LDX TABSIZ
            BEQ FINI
            LDY #O
PARTLP      STA (TABPTR),Y
            INY
            DEX
            BNE PARTLP
FINI        RTS
;
; 16-BIT MULTIPLICATION ROUTINE
;
MULT16      LDA #O
            STA PRODL
            STA PRODH
            LDX #17
            
;
; BlackBoard
;
    .org $8000
;
    COLOR   .equ $10
    BASE    .equ $2000
    SCROLY  .equ $D011
    VMCSB   .equ $D018
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
    BYTE    .equ LINE+1
    BITT    .equ BYTE+2
;
    MPRL    .equ BITT+1
    MPRH    .equ MPRL+1
    MPDL    .equ MPRH+1
    MPDH    .equ MPDL+1
    PRODL   .equ MPDH+1
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
            DEX
            BNE FULLPG
PARTPG      LDX TABSIZ
            BEQ FINI
            LDY #0
PARTLP      STA (TABPTR),Y
            INY
            DEX
            BNE PARTLP
FINI        RTS
;
; 16-BIT MULTIPLICATION ROUTINE
;
MULT16      LDA #0
            STA PRODL
            STA PRODH
            LDX #17
            CLC
MULT        ROR PRODH
            ROR PRODL
            ROR MPRH
            ROR MPRL
            BCC CTDOWN
            CLC
            LDA MPDL
            ADC PRODL
            STA PRODL
            LDA MPDH
            ADC PRODH
            STA PRODH
CTDOWN      DEX
            BNE MULT
            RTS
;
; PLOT ROUTINE
;
; ROW-VPSN/8 (8-BIT DIVIDE)
;
PLOT        LDA VPSN
            LSR A
            LSR A
            LSR A
            STA ROW
;
; CHAR-HPSN/8 (16-BIT DIVIDE)
;
            LDA HPSN
            STA TEMPA
            LDA HPSN+1
            STA TEMPA+1
            LDX #3
DLOOP       LSR TEMPA+1
            ROR TEMPA
            DEX
            BNE DLOOP
            LDA TEMPA
            STA CHAR
;
;
;
            LDA VPSN
            AND #7
            STA LINE
;
; BITT=7-(HPSN AND 7)
;
            LDA HPSN
            AND #7
            STA BITT
            SEC
            LDA #7
            SBC BITT
            STA BITT
;
; BYTE-BASE+ROW*HMAX+8*CHAR+LINE
;
; FIRST MULTIPLY ROW HMAX
;
            LDA ROW
            STA MPRL
            LDA #0
            STA MPRH
            LDA #<HMAX
            STA MPDL
            LDA #>HMAX
            STA MPDH
            JSR MULT16
            LDA MPRL
            STA TEMPA
            LDA MPRL+1
            STA TEMPA+1
;
; ADD PRODUCT TO BASE
;
            CLC
            LDA #<BASE
            ADC TEMPA
            STA TEMPA
            LDA #>BASE
            ADC TEMPA+1
            STA TEMPA+1
;
; MULTIPLY 8 * CHAR
;
            LDA #8
            STA MPRL
            LDA #0
            STA MPRH
            LDA CHAR
            STA MPDL
            LDA #0
            STA MPDH
            JSR MULT16
            LDA MPRL
            STA TEMPB
            LDA MPRH
            STA TEMPB+1
;
; ADD LINE
;
            CLC
            LDA TEMPB
            ADC LINE
            STA TEMPB
            LDA TEMPB+1
            ADC #0
            STA TEMPB+1
;
; BYTE = TEMPA + TEMPB
;
            CLC
            LDA TEMPA
            ADC TEMPB
            STA TEMPB
            LDA TEMPA+1
            ADC TEMPB+1
            STA TEMPB+1
;
; POKE BYTE,PEEK(BYTE)
;
            LDX BITT
            INX
            LDA #0
            SEC
SQUARE      ROL
            DEX
            BNE SQUARE
            LDY #0
            ORA (TEMPB), Y
            STA (TEMPB), Y
            RTS
;
; MAIN ROUTINE STARTS 
;
; FIRST DEFINE BIT MAP 
; HIGH-RESOLUTION GRAPHICS
;
START       LDA #$18
            STA VMCSB
;
            LDA SCROLY
            ORA #32
            STA SCROLY
;
; SELECT GRAPHICS BANK 1
;
            LDA $DD02
            ORA #$03
            STA $DD02
;
            LDA $DD00
            ORA #$03
            STA $DD00
;
; CLEAR BIT MAP
;
            LDA #0
            STA FILVAL
            LDA #<BASE
            STA TABPTR
            LDA #>BASE
            STA TABPTR+1
            LDA #<SCRLEN
            STA TABSIZ
            LDA #>SCRLEN
            STA TABSIZ+1
            JSR BLKFIL
;
; SET BKG AND LINE COLORS
;
            LDA #COLOR
            STA FILVAL
            LDA #<COLMAP
            STA TABPTR
            LDA #>COLMAP
            STA TABPTR+1
            LDA #<MAPLEN
            STA TABSIZ
            LDA #>MAPLEN
            STA TABSIZ+1
            JSR BLKFIL
;
; DRAW HORIZONTAL LINE
;
            LDA #VMID
            STA VPSN
            LDA #0
            STA HPSN
            STA HPSN+1
AGIN        JSR PLOT
            INC HPSN
            BNE NEXT
            INC HPSN+1
NEXT        LDA HPSN+1
            CMP #>HMAX
            BCC AGIN
            LDA HPSN
            CMP #<HMAX
            BCC AGIN
;
; DRAW VERTICAL LINE
;
            LDA #0
            STA VPSN
POINT       LDA #<HMID
            STA HPSN
            LDA #>HMID
            STA HPSN+1
            JSR PLOT
            INC HPSN
            BNE SKIP
            INC HPSN+1
SKIP        JSR PLOT
            LDX VPSN
            INX
            STX VPSN
            CPX #VMAX
            BCC POINT
INF         JMP INF

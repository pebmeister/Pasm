        .inc "kernal.asm"
        .inc "pagezero.asm"
        .inc "basic.asm"

        .org $102               ; this MUST BE the autostart address

        lda #$7F                ;   suppress any irq&nmi
        sta CIAICR              ;   to disallow abort load by R/S
        sta CIACRA 
        jsr RESTOR              ;   restore i/o-vex (just for sure...)

        lda #<MSG               ;   print message (optional)
        ldy #>MSG
        jsr STROUT

        lda #NAMELEN
        ldx #<NAME
        ldy #>NAME
        jsr SETNAM              ;   set name of file to load

        lda FA                  ;   last device number - should be 8
        tax
        tay
        jsr SETLFS              ;   open 8,8,8

        lda #0                  ;   load... (lda #1 would verify)
        sta MSGFLG              ;   flag progam mode (to suppress 'searching for...' msg)
        jsr LOAD                ;   ...filename,8,8

        lda #$81                ;  restore any irq&nmi
        sta CIAICR              ;
        sta CIACRA

        jmp EXECADDR            ;   start it

                                ;   enter your (short!) text here
MSG     .db " "
NAME    .str "PONG",0           ;   enter name of prg-to-load here

        NAMELEN = * - NAME - 1
        EXECADDR = $C000        ;   set to the 1st instruction's address of prg-to-load

                                ;   now comes the smart part:
        .fill $01, $200-*

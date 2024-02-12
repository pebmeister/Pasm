        .print off

;********************************************
;*                                          *
;* PUSH16                                   *
;*                                          *
;*  push 16 bit value                       *
;*                                          *
;*  destroys a                              *
;*                                          *
;********************************************
        .macro PUSH16
        lda \1
        pha
        lda \1 + 1
        pha
        .endm

;********************************************
;*                                          *
;* POP16                                    *
;*                                          *
;*  pop 16 bit value                        *
;*                                          *
;*  destroys a                              *
;*                                          *
;********************************************
        .macro POP16
        pla
        sta \1 + 1
        pla
        sta \1
        .endm

;********************************************
;*                                          *
;* STORE                                    *
;*                                          *
;*  store a variable                        *
;*                                          *
;*  \1  Source                              *
;*  \2  Destination                         *
;*  \3  Length      (Must be less than 255) *
;*                                          *
;*                                          *
;*  destroys a, y                           *
;*                                          *
;********************************************
        .macro STORE
.if \3 > 2
        ldy #\3 -1
@l1
        lda \1,y
        sta \2,y
        dey
        bpl @l1
.else
        lda \1
        sta \2
.if \3 > 1
        lda \1 + 1
        sta \2 + 1
.endif
.endif
        .endm

;********************************************
;*                                          *
;* STOREI                                   *
;*                                          *
;*  store a variable                        *
;*                                          *
;*  \1  Source numeric value                *
;*  \2  Destination                         *
;*  \3  Length      (Must be less than 255) *
;*                                          *
;*                                          *
;*  destroys a, y                           *
;*                                          *
;********************************************
        .macro STOREI
.if \3 > 2
        lda #\1
        ldy #\3 -1
@l1
        sta \2,y
        dey
        bpl @l1
.else
        lda #\1
        sta \2
.if \3 > 1
        sta \2 + 1
.endif
.endif
        .endm

;********************************************
;*                                          *
;* MOVE8                                    *
;*                                          *
;*  move a 8 bit variable                   *
;*                                          *
;*  \1  Source                              *
;*  \2  Destination                         *
;*                                          *
;*  destroys a                              *
;*                                          *
;********************************************
        .macro MOVE8
        lda \1
        sta \2
        .endm

;********************************************
;*                                          *
;* MOVE8I                                   *
;*                                          *
;*  move a 8 bit value to a variable        *
;*                                          *
;*  \1  Variable                            *
;*  \2  Value                               *
;*                                          *
;*  destroys a                              *
;*                                          *
;********************************************
        .macro MOVE8I
        lda #\2
        sta \1
        .endm

;********************************************
;*                                          *
;* MOVE16                                   *
;*                                          *
;*  move a 16 bit variable                  *
;*                                          *
;*  \1  Source                              *
;*  \2  Destination                         *
;*                                          *
;*  destroys a                              *
;*                                          *
;********************************************
        .macro MOVE16
        lda \1
        sta \2
        lda \1 + 1
        sta \2 + 1
        .endm

;********************************************
;*                                          *
;* MOVE32                                   *
;*                                          *
;*  move a 32 bit variable                  *
;*                                          *
;*  \1  Source                              *
;*  \2  Destination                         *
;*                                          *
;*  destroys a                              *
;*                                          *
;********************************************
        .macro MOVE32
        lda \1
        sta \2
        lda \1 + 1
        sta \2 + 1
        lda \1 + 2
        sta \2 + 2
        lda \1 + 3
        sta \2 + 3
        .endm

;********************************************
;*                                          *
;* MOVE16I                                  *
;*                                          *
;*  move a 16 bit value to a variable       *
;*                                          *
;*  \1  Variable                            *
;*  \2  Value                               *
;*                                          *
;*  destroys a                              *
;*                                          *
;********************************************
        .macro MOVE16I
        lda #<\2
        sta \1
        lda #>\2
        sta \1 + 1
        .endm

;********************************************
;*                                          *
;* ADD8                                     *
;*                                          *
;*  add 8 bit variables                     *
;*                                          *
;*  \1  Variable 1                          *
;*  \2  Variable 2                          *
;*  \3  Destination                         *
;*                                          *
;*  destroys a                              *
;*                                          *
;********************************************
        .macro ADD8
        clc
        lda \1
        adc \2
        sta \3
        .endm

;********************************************
;*                                          *
;* ADD8I                                    *
;*                                          *
;*  add 8 bit variables                     *
;*                                          *
;*  \1  variable 1                          *
;*  \2  number 2                            *
;*  \3  Destination                         *
;*                                          *
;*  destroys a                              *
;*                                          *
;********************************************
        .macro ADD8I
        clc
        lda \1
        adc #\2
        sta \3
        .endm

;********************************************
;*                                          *
;* ADD16                                    *
;*                                          *
;*  add 16 bit variables                    *
;*                                          *
;*  /1  Variable 1                          *
;*  /2  Variable 2                          *
;*  /3  Destination                         *
;*                                          *
;*  destroys a                              *
;*                                          *
;********************************************
        .macro ADD16
        clc
        lda \1
        adc \2
        sta \3
        lda \1 + 1
        adc \2 + 1
        sta \3 + 1
        .endm

;********************************************
;*                                          *
;* ADD32                                    *
;*                                          *
;*  add 32 bit variables                    *
;*                                          *
;*  /1  Variable 1                          *
;*  /2  Variable 2                          *
;*  /3  Destination                         *
;*                                          *
;*  destroys a                              *
;*                                          *
;********************************************
        .macro ADD32
        clc
        lda \1
        adc \2
        sta \3
        lda \1 + 1
        adc \2 + 1
        sta \3 + 1
        lda \1 + 2
        adc \2 + 2
        sta \3 + 2
        lda \1 + 3
        adc \2 + 3
        sta \3 + 3
        .endm

;********************************************
;*                                          *
;* ADDFIX16                                 *
;*                                          *
;*  add 16 bit fixed point variables        *
;*                                          *
;*  /1  Variable 1                          *
;*  /2  Variable 2                          *
;*  /3  Destination                         *
;*                                          *
;*  destroys a                              *
;*                                          *
;********************************************
        .macro ADDFIX16
        clc
        lda \1 + 2
        adc \2 + 2
        sta \3 + 2
        lda \1 + 3
        adc \2 + 3
        sta \3 + 3
        lda \1
        adc \2
        sta \3
        lda \1 + 1
        adc \2 + 1
        sta \3 + 1
        .endm

;********************************************
;*                                          *
;* ADDFIX16I                                *
;*                                          *
;*  add 16 bit fixed point variables        *
;*                                          *
;*  /1  Variable 1                          *
;*  /2  number                              *
;*  /3  Destination                         *
;*                                          *
;*  destroys a                              *
;*                                          *
;********************************************
        .macro ADDFIX16I
        clc
        lda #\2
        adc \1 + 2
        sta \3 + 2
        lda #0
        adc \1 + 3
        sta \3 + 3
        lda #0
        adc \1
        sta \3
        lda #0
        adc \1 + 1
        sta \3 + 1
        .endm

;********************************************
;*                                          *
;* ADD16I                                   *
;*                                          *
;*  add value to a 16 bit variable          *
;*                                          *
;*  \1  Variable                            *
;*  \2  Value                               *
;*  \3  Destination                         *
;*                                          *
;*  destroys a                              *
;*                                          *
;********************************************
        .macro ADD16I
        clc
        lda \1
        adc #<\2
        sta \3
        lda \1 + 1
        adc #>\2
        sta \3 + 1
        .endm

;********************************************
;*                                          *
;* ADD168                                   *
;*                                          *
;*  add 8 bit to 16 bit variable            *
;*                                          *
;*  \1  Variable 1      16 bit              *
;*  \2  Variable 2      8 bit               *
;*  \3  Destination     16 bit              *
;*                                          *
;*  destroys a                              *
;*                                          *
;********************************************
        .macro ADD168
        clc
        lda \1
        adc \2
        sta \3
        lda \1 + 1
        adc #0
        sta \3 + 1
        .endm

;********************************************
;*                                          *
;* INC8                                     *
;*                                          *
;*  increment an 8 bit variable             *
;*                                          *
;*  \1  Variable                            *
;*                                          *
;*  destroys a                              *
;*                                          *
;********************************************
        .macro INC8
        inc \1
        .endm

;********************************************
;*                                          *
;* INC16                                    *
;*                                          *
;*  increment a 16 bit variable             *
;*                                          *
;*  \1  Variable                            *
;*                                          *
;*  destroys a                              *
;*                                          *
;********************************************
        .macro INC16
        inc \1
        bne @exit
        inc \1 + 1
@exit
        .endm

;********************************************
;*                                          *
;* SUB8                                     *
;*                                          *
;*  subtract 8 bit variables                *
;*                                          *
;*  \1  Variable                            *
;*  \2  Variable to subtract                *
;*  \3  Destination                         *
;*                                          *
;*  destroys a                              *
;*                                          *
;********************************************
        .macro SUB8
        sec
        lda \1
        sbc \2
        sta \3
        .endm

;********************************************
;*                                          *
;* SUB8I                                    *
;*                                          *
;*  subtract 8 bit variables                *
;*                                          *
;*  /1  Variable                            *
;*  /2  number to subtract                  *
;*  /3  Destination                         *
;*                                          *
;*  destroys a                              *
;*                                          *
;********************************************
        .macro SUB8I
        sec
        lda \1
        sbc #\2
        sta \3
        .endm

;********************************************
;*                                          *
;* SUB16                                    *
;*                                          *
;*  subtract 16 bit variables               *
;*                                          *
;*  \1  Variable                            *
;*  \2  Variable to subtract                *
;*  \3  Destination                         *
;*                                          *
;*  destroys a                              *
;*                                          *
;********************************************
        .macro SUB16
        sec
        lda \1
        sbc \2
        sta \3
        lda \1 + 1
        sbc \2 + 1
        sta \3 + 1
        .endm

;********************************************
;*                                          *
;* SUB16I                                   *
;*                                          *
;*  subtract a value from 16 bit variable   *
;*                                          *
;*  \1  Variable                            *
;*  \2  Value to subtract                   *
;*  \3  Destination                         *
;*                                          *
;*  destroys a                              *
;*                                          *
;********************************************
        .macro SUB16I
        sec
        lda \1
        sbc #<\2
        sta \3
        lda \1 + 1
        sbc #>\2
        sta \3 + 1
        .endm

;********************************************
;*                                          *
;* SUB168                                   *
;*                                          *
;*  subtract 8 bit from 16 bit variable     *
;*                                          *
;*  \1  Variable                            *
;*  \2  Variable to subtract                *
;*  \3  Destination                         *
;*                                          *
;*  destroys a                              *
;*                                          *
;********************************************
        .macro SUB168
        sec
        lda \1
        sbc \2
        sta \3
        lda \1 + 1
        sbc #0
        sta \3 + 1
        .endm

;********************************************
;*                                          *
;* SUBFIX16                                 *
;*                                          *
;*  subtract 16 bit variables               *
;*                                          *
;*  \1  Variable                            *
;*  \2  Variable to subtract                *
;*  \3  Destination                         *
;*                                          *
;*  destroys a                              *
;*                                          *
;********************************************
        .macro SUBFIX16
        sec
        lda \1 + 2
        sbc \2 + 2
        sta \3 + 2
        lda \1 + 3
        sbc \2 + 3
        sta \3 + 3

        lda \1
        sbc \2
        sta \3
        lda \1 + 1
        sbc \2 + 1
        sta \3 + 1

        .endm

;********************************************
;*                                          *
;* DEC8                                     *
;*                                          *
;*  Decrement a 8 bit variable              *
;*                                          *
;*  \1  Variable                            *
;*                                          *
;*  destroys a                              *
;*                                          *
;********************************************
        .macro DEC8
        dec \1
        .endm

;********************************************
;*                                          *
;* DEC16                                    *
;*                                          *
;*  Decrement a 16 bit variable             *
;*                                          *
;*  \1  Variable                            *
;*                                          *
;*  destroys a                              *
;*                                          *
;********************************************
        .macro DEC16
        SUB16I \1,1,\1
        .endm

;********************************************
;*                                          *
;*  RSHIFT8                                 *
;*                                          *
;*  8bit shift                              *
;*                                          *
;*  \1  Variable                            *
;*  \2  bits to shift                       *
;*                                          *
;*  destroys    nothing                     *
;*                                          *
;********************************************
        .macro RSHIFT8
        .var n
        n = \2
        .while n > 0
            lsr \1
            n = n -1
        .wend
        .endm

;********************************************
;*                                          *
;*  RSHIFT16                                *
;*                                          *
;*  16bit shift                             *
;*                                          *
;*  \1  Variable                            *
;*  \2  bits to shift                       *
;*                                          *
;*  destroys    nothing                     *
;*                                          *
;********************************************
        .macro RSHIFT16
        .var n
        n = \2
        .while n > 0
            lsr \1 + 1
            ror \1

            n = n -1
        .wend
        .endm

;********************************************
;*                                          *
;*  RSHIFT32                                *
;*                                          *
;*  32bit shift                             *
;*                                          *
;*  \1  Variable                            *
;*  \2  bits to shift                       *
;*                                          *
;*  destroys    nothing                     *
;*                                          *
;********************************************
        .macro RSHIFT32
        .var n
        n = \2
        .while n > 0
            lsr \1 + 3
            ror \1 + 2
            ror \1 + 1
            ror \1

            n = n -1
        .wend
        .endm

;********************************************
;*                                          *
;*  LSHIFT8                                 *
;*                                          *
;*  8bit shift                              *
;*                                          *
;*  \1  Variable                            *
;*  \2  btits to shift                      *
;*                                          *
;*  destroys    nothing                     *
;*                                          *
;********************************************
        .macro LSHIFT8
        .var n
        n = \2
        .while n > 0
            asl \1
            n = n -1
        .wend
        .endm

;********************************************
;*                                          *
;*  LSHIFT16                                *
;*                                          *
;*  16bit shift                             *
;*                                          *
;*  \1  Variable                            *
;*  \2  bits to shift                       *
;*                                          *
;*  destroys    nothing                     *
;*                                          *
;********************************************
        .macro LSHIFT16
        .var n
        n = \2
        .while n > 0
            asl \1
            rol \1 + 1
            n = n -1
        .wend
        .endm

;********************************************
;*                                          *
;*  LSHIFT32                                *
;*                                          *
;*  32bit shift                             *
;*                                          *
;*  \1  Variable                            *
;*  \2  bits to shift                       *
;*                                          *
;*  destroys    nothing                     *
;*                                          *
;********************************************
        .macro LSHIFT32
        .var n
        n = \2
        .while n > 0
            asl \1
            rol \1 + 1
            rol \1 + 2
            rol \1 + 3
            n = n -1
        .wend
        .endm
;********************************************
;*                                          *
;* MULT10                                   *
;*                                          *
;*  multiplies 2 byte number by 10          *
;                                           *
;*  n * 10 = n * 8 + n * 2                  *
;*                                          *
;*  \1  Variable                            *
;*                                          *
;*  destroys a,x,y                          *
;*                                          *
;********************************************
        .macro MULT10
        asl \1
        rol \1 + 1              ;   multiply by 2

        ;
        ;   save the number * 2
        ;
        ldx \1                  ;   save low byte in x
        ldy \1 + 1              ;   save high byte in y

        ;
        ;   continue shifting for multiply by 4 and 8
        ;
        asl \1
        rol \1 + 1              ;   multiply by 4
        asl \1
        rol \1 + 1              ;   multiply by 8

        ;
        ;   now add up the result
        ;
        clc                     ;   clear carry
        txa                     ;   load low byte of 2x
        adc \1                  ;   add to 8x low byte
        sta \1                  ;   store low byte
        tya                     ;   load the high byte of 2x
        adc \1 + 1              ;   add to 8x high
        sta \1 + 1              ;   store high byte
        .endm

;********************************************
;*                                          *
;* MULT40                                   *
;*                                          *
;*  multiplies 2 byte number by 40          *
;                                           *
;*  \1  Variable                            *
;*                                          *
;*  n * 40 = 4 * (n * 8 + n * 2)            *
;*                                          *
;*  destroys a,x,y                          *
;*                                          *
;********************************************
        .macro MULT40
        asl \1
        rol \1 + 1              ;   multiply by 2

        ;
        ;   save the number * 2
        ;
        ldx \1                  ;   save low byte in x
        ldy \1 + 1              ;   save high byte in y

        ;
        ;   continue shifting for multiply by 4 and 8
        ;
        asl \1
        rol \1 + 1              ;   multiply by 4
        asl \1
        rol \1 + 1              ;   multiply by 8

        ;
        ;   now add up the result
        ;
        clc                     ;   clear carry
        txa                     ;   load low byte of 2x
        adc \1                  ;   add to 8x low byte
        sta \1                  ;   store low byte
        tya                     ;   load the high byte of 2x
        adc \1 + 1              ;   add to 8x high
        sta \1 + 1              ;   store high byte

        asl \1
        rol \1 + 1              ;   multiply by 2
        asl \1
        rol \1 + 1              ;   multiply by 4

        .endm

;********************************************
;*                                          *
;* MULT5                                    *
;*                                          *
;*  multiplies 2 byte number by 5           *
;                                           *
;*  \1  Variable                            *
;*                                          *
;*  n * 5 = n * 4 + n                       *
;*                                          *
;*  destroys a,x,y                          *
;*                                          *
;********************************************
        .macro MULT5
        ;
        ;   save the number
        ;
        ldx \1                  ;   save low byte in x
        ldy \1 + 1              ;   save high byte in y

        ;
        ;   continue shifting for multiply by 4
        ;
        LSHIFT16 \1, 2

        ;
        ;   now add up the result
        ;
        clc                     ;   clear carry
        txa                     ;   load low byte of 2x
        adc \1                  ;   add to 4x low byte
        sta \1                  ;   store low byte
        tya                     ;   load the high byte of x
        adc \1 + 1              ;   add to 4x high
        sta \1 + 1              ;   store high byte
        .endm

;********************************************
;*                                          *
;*  MULT8                                   *
;*                                          *
;*  8bit multiplication                     *
;*                                          *
;*  \1  Variable a                          *
;*  \2  Variable b                          *
;*  \3  Destination                         *
;*                                          *
;********************************************
            .macro MULT8
            lda #00
            ldx #08
@Loop
            lsr \2
            bcc @NoAdd

            clc
            adc \1
@NoAdd
            rol \1
            dex
            bne @Loop

            sta \3
            .endm

;********************************************
;*                                          *
;*  MULT16                                  *
;*                                          *
;*  16bit multiplication     32bit result   *
;*                                          *
;*  \1  Variable a     16-bit               *
;*  \2  Variable b     16 bit               *
;*  \3  Destination    32 bit               *
;*                                          *
;********************************************
            .macro MULT16

            PUSH16 \2

            lda #0      ;initialize result to 0
            sta \3 + 2

            ldx #16     ;there are 16 bits in num2
@l1
            lsr \2 + 1  ;get low bit of num2
            ror \2
            bcc @l2     ; 0 or 1?

            tay         ; if 1, add num1 (hi byte of result is in a)
            clc
            lda \1
            adc \3 + 2
            sta \3 + 2

            tya
            adc \1 + 1
@l2
            ror         ;"stairstep" shift
            ror \3 + 2
            ror \3 + 1
            ror \3

            dex
            bne @l1

            sta \3 + 3

            POP16 \2
            .endm

;********************************************
;*                                          *
;*  MULT16                                  *
;*                                          *
;*  16bit multiplication     32bit result   *
;*  \2 is destroyed                         *
;*                                          *
;*  \1  Variable a     16-bit               *
;*  \2  Variable b     16 bit               *
;*  \3  Destination    32 bit               *
;*                                          *
;********************************************
            .macro MULT_F_16


            lda #0      ;initialize result to 0
            sta \3 + 2
            ldx #16     ;there are 16 bits in num2
@l1
            lsr \2 + 1  ;get low bit of num2
            ror \2
            bcc @l2     ; 0 or 1?

            tay         ; if 1, add num1 (hi byte of result is in a)
            clc
            lda \1
            adc \3 + 2
            sta \3 + 2

            tya
            adc \1 + 1
@l2
            ror         ;"stairstep" shift
            ror \3 + 2
            ror \3 + 1
            ror \3

            dex
            bne @l1

            sta \3 + 3

            .endm


;********************************************
;*                                          *
;*  SWAP16                                  *
;*                                          *
;*  Swap 2 16 bit values                    *
;*                                          *
;*  \1  Variable a     16-bit               *
;*  \2  Variable b     16 bit               *
;*                                          *
;********************************************
            .macro SWAP16
            ldx \1
            ldy \1 + 1
            lda \2
            sta \1
            lda \2 + 1
            sta \1 + 1
            stx \2
            sty \2 + 1
            .endm

;********************************************
;*                                          *
;*  FMUL                                    *
;*                                          *
;*  16 bit multiplication 32bit result      *
;*  swap hi and low words                   *
;*                                          *
;*  \1  Variable a     16-bit               *
;*  \2  Variable b     16 bit               *
;*  \3  Destination    32 bit               *
;*                                          *
;********************************************
            .macro FMUL
            MULT16 \1, \2, \3
            SWAP16 \3, \3 + 2
            .endm

;********************************************
;*                                          *
;*  MULT16I                                 *
;*                                          *
;*  16bit multiplication                    *
;*                                          *
;*  \1  number a       16-bit               *
;*  \2  Variable b     16 bit               *
;*  \3  Destination    32 bit               *
;*                                          *
;*  destroys    a,x,y                       *
;*                                          *
;********************************************
        .macro MULT16I

        PUSH16 \2

        lda #0      ;initialize result to 0
        sta \3 + 2
        ldx #16     ;there are 16 bits in num2
@l1
        lsr \2 + 1  ;get low bit of num2
        ror \2
        bcc @l2     ; 0 or 1?

        tay         ; if 1, add num1 (hi byte of result is in a)
        clc
        lda #<\1
        adc \3 + 2
        sta \3 + 2

        tya
        adc #>\1
@l2
        ror         ;"stairstep" shift
        ror \3 + 2
        ror \3 + 1
        ror \3

        dex
        bne @l1

        sta \3 + 3

        POP16 \2
        .endm

;********************************************
;*                                          *
;* PUSHREG                                  *
;*                                          *
;*  push registers on the stack             *
;*                                          *
;*  destroys a                              *
;*                                          *
;********************************************
        .macro PUSHREGS
        php
        pha
        tya
        pha
        txa
        pha
        .endm

;********************************************
;*                                          *
;* POPREG                                   *
;*                                          *
;*  restore registers on the stack          *
;*                                          *
;*  destroys a,x,y                          *
;*                                          *
;********************************************
        .macro POPREGS
        pla
        tax
        pla
        tay
        pla
        plp
        .endm

;********************************************
;*                                          *
;* PUSHAY                                   *
;*                                          *
;*  push and y on the stack                 *
;*                                          *
;*  destroys a                              *
;*                                          *
;********************************************
        .macro PUSHAY
        pha
        tya
        pha
        .endm

;********************************************
;*                                          *
;* POPAY                                    *
;*                                          *
;*  restore registers on the stack          *
;*                                          *
;*  destroys a,y                            *
;*                                          *
;********************************************
        .macro POPAY
        pla
        tay
        pla
        .endm

;********************************************
;*                                          *
;* BCD2STR                                  *
;*                                          *
;*  converts 1 byte bcd to 2 byte string    *
;*                                          *
;*  destroys a                              *
;*                                          *
;********************************************
        .macro BCD2STR
        lda \1                  ;   load the byte to convert 
        tax
        lsr                     ;   shift to low nibble
        lsr
        lsr
        lsr
        ora #'0'                ;   or '0' to make it numeric
        sta \2                  ;   save the PETASCII low byte
        txa                     ;   reload the byte to convert
        and #$0F                ;   get the lo nibble
        ora #'0'                ;   or '0' to make it numeric
        sta \2 + 1              ;   save the PETASCII high byte
        .endm

;********************************************
;*                                          *
;* SQUARE                                   *
;* Calculates the 16 bit unsigned integer   *
;* square of the signed 16 bit integer in   *
;* Numberl/Numberh.  The result is always in*
;* the range 0 to 65025                     *
;*                                          *
;* The maximum input range is only +/-255   *
;* and no checking is done to ensure that   *
;* this is so.                              *
;*                                          *
;* This routine is useful if you are trying *
;* to draw circles as for any circle        *
;*                                          *
; x^2+y^2=r^2 where x and y are the         *
;* co-ordinates of any point on the circle  *
;* and r is the circle radius               *
;*                                          *
;* Destroys all registers                   *
;********************************************
        .macro SQUARE
        lda $00         ; clear a
        sta \2          ; clear square low byte
                        ; (no need to clear the high byte, it gets
                        ; shifted out)
        lda \1          ; get number low byte
        ldx \1 + 1      ; get number high  byte
        bpl @NonNeg     ; if +ve don't negate it
                        ; else do a two's complement
        eor #$ff        ; invert
        sec             ; +1
        adc #$00        ; and add it

@NonNeg
        sta @TempSq     ; save abs(number)
        ldx #$08        ; set bit count

@Nextr2Bit
        asl \2          ; low byte *2
        rol \2 + 1      ; high byte *2+carry from low
        asl a           ; shift number byte
        bcc @NoSqAdd    ; don't do add if c = 0
        tay             ; save a
        clc             ; clear carry for add
        lda @TempSq     ; get number
        adc \2          ; add number^2 low byte
        sta \2          ; save number^2 low byte
        lda #$00        ; clear a
        adc \2 + 1      ; add number^2 high byte
        sta \2 + 1      ; save number^2 high byte
        tya             ; get a back

@NoSqAdd:
        dex             ; decrement bit count
        bne @Nextr2Bit  ; go do next bit
        jmp @Exit
@TempSq:                ; temp byte for intermediate result
        .byte $00
@Exit
        .endm

;********************************************
;*                                          *
;*  BEQ16                                   *
;*                                          *
;*  16bit beq                               *
;*                                          *
;*  \1  a       16-bit                      *
;*  \2  b       16 bit                      *
;*  \3  Destination a = b                   *
;*                                          *
;*  destroys    a                           *
;*                                          *
;********************************************
        .macro BEQ16
        lda \1 + 1
        cmp \2 + 1
        bne @exit
        lda \1
        cmp \2
        beq \3
  @exit
        .endm

;********************************************
;*                                          *
;*  BEQ16                                   *
;*                                          *
;*  16bit beq                               *
;*                                          *
;*  \1  a       16-bit                      *
;*  \2  b       16 bit Number               *
;*  \3  Destination a = b                   *
;*                                          *
;*  destroys    a                           *
;*                                          *
;********************************************
        .macro BEQ16I
        lda \1 + 1
        cmp #>\2
        bne @exit
        lda \1
        cmp #<\2
        beq \3
  @exit
        .endm

;********************************************
;*                                          *
;*  BNE16                                   *
;*                                          *
;*  16bit beq                               *
;*                                          *
;*  \1  a       16-bit                      *
;*  \2  b       16 bit                      *
;*  \3  Destination a != b                  *
;*                                          *
;*  destroys    a                           *
;*                                          *
;********************************************
        .macro BNE16
        lda \1 + 1
        cmp \2 + 1
        bne \3
        lda \1
        cmp \2
        bne \3
        .endm

;********************************************
;*                                          *
;*  BNE16I                                  *
;*                                          *
;*  16bit beq                               *
;*                                          *
;*  \1  a       16-bit                      *
;*  \2  b       16 bi value                 *
;*  \3  Destination a != b                  *
;*                                          *
;*  destroys    a                           *
;*                                          *
;********************************************
        .macro BNE16I
        lda \1 + 1
        cmp #>\2
        bne \3
        lda \1
        cmp #<\2
        bne \3
        .endm

;********************************************
;*                                          *
;*  BLT16                                   *
;*                                          *
;*  16bit bcc                               *
;*                                          *
;*  \1  a       16-bit                      *
;*  \2  b       16 bit                      *
;*  \3  Destination a < b                   *
;*                                          *
;*  destroys    a                           *
;*                                          *
;********************************************
        .macro BLT16
        lda \1 + 1
        cmp \2 + 1
        bcc \3
        bne @exit
        lda \1
        cmp \2
        bcc \3
@exit
        .endm

;********************************************
;*                                          *
;*  BLT16I                                  *
;*                                          *
;*  16bit bcc                               *
;*                                          *
;*  \1  a       16-bit                      *
;*  \2  b       16 bit number                *
;*  \3  Destination a < b                   *
;*                                          *
;*  destroys    a                           *
;*                                          *
;********************************************
        .macro BLT16I
        lda \1 + 1
        cmp #>\2
        bcc \3
        bne @exit
        lda \1
        cmp #<\2
        bcc \3
@exit
        .endm

;********************************************
;*                                          *
;*  BLE16                                   *
;*                                          *
;*  16bit branch less than or equal         *
;*                                          *
;*  \1  a       16-bit                      *
;*  \2  b       16 bit                      *
;*  \3  Destination a <= b                  *
;*                                          *
;*  destroys    a                           *
;*                                          *
;********************************************
        .macro BLE16
        lda \1 + 1
        cmp \2 + 1
        bcc \3
        bne @exit
        lda \1
        cmp \2
        bcc \3
        beq \3
@exit
        .endm

;********************************************
;*                                          *
;*  BLE16I                                  *
;*                                          *
;*  16bit branch less than or equal         *
;*                                          *
;*  \1  a       16-bit                      *
;*  \2  b       16 bit number               *
;*  \3  Destination a <= b                  *
;*                                          *
;*  destroys    a                           *
;*                                          *
;********************************************
        .macro BLE16I
        lda \1 + 1
        cmp #>\2
        bcc \3
        bne @exit
        lda \1
        cmp #<\2
        bcc \3
        beq \3
@exit
        .endm

;********************************************
;*                                          *
;*  BGE16                                   *
;*                                          *
;*  16bit Greater than or equal             *
;*                                          *
;*  \1  a       16-bit                      *
;*  \2  b       16 bit                      *
;*  \3  Destination a >= b                  *
;*                                          *
;*  destroys    a                           *
;*                                          *
;********************************************
        .macro BGE16
        lda \1 + 1
        cmp \2 + 1
        beq @bg1
        bcs \3
        bcc @exit       
@bg1
        lda \1
        cmp \2
        bcs \3
@exit
        .endm

;********************************************
;*                                          *
;*  BGE16I                                  *
;*                                          *
;*  16bit Greater than or equal             *
;*                                          *
;*  \1  a       16-bit                      *
;*  \2  b       16 bit number               *
;*  \3  Destination a >= b                  *
;*                                          *
;*  destroys    a                           *
;*                                          *
;********************************************
        .macro BGE16I
        lda \1 + 1
        cmp #>\2
        beq @bg1        
        bcs \3
        bcc @exit       
@bg1
        lda \1
        cmp #<\2
        bcs \3
@exit
        .endm

        .print on
 
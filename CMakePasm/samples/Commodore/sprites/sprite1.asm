        ; sprite example ...
        ; the hot air balloon

        VIC = 13 * 4096         ; this is where the VIC registers begin

        PLOT    = $E50A         ; set text screen position
        STROUT  = $AB1E         ; draws a string
        GETIN   = $F13E         ; get input char

        x_      = $FB
        y_      = $FC
        dx      = $FD
        dy      = $FE
        side    = $61
        rx      = $62

        MINY    = 50
        MAXY    = 208
        MINX    = 24
        MAXEXT  = 40

        .org $C000

        ; ********************
        ; enable sprites
        ; ********************
        lda #%00111111
        sta VIC + 21            ; enable sprites 0 thru 5

        lda #14
        sta VIC + 33            ; set background color

        ; ********************
        ; set sprite pointers
        ; ********************
        ldx #192
        stx 2040                ; set sprite 0's pointer
        stx 2042                ; set sprite 2's pointer
        stx 2044                ; set sprite 4's pointer
        inx
        stx 2041                ; set sprite 1's pointer
        stx 2043                ; set sprite 3's pointer
        stx 2045                ; set sprite 5's pointer

        ; ********************
        ; set sprite positions
        ; ********************
        ldx #02
        ldy #100
        stx VIC + 0             ; set sprite 0's x position
        sty VIC + 1             ; set sprite 0's y position
        stx VIC + 2             ; set sprite 1's x position
        sty VIC + 3             ; set sprite 1's y position
        lda #30
        sta VIC + 4             ; set sprite 2's x position
        lda #58
        sta VIC + 5             ; set sprite 2's y position
        lda #65
        sta VIC + 6             ; set sprite 3's x position
        lda #58
        sta VIC + 7             ; set sprite 3's y position
        lda #100
        sta VIC + 8             ; set sprite 4's x position
        lda #58
        sta VIC + 9             ; set sprite 4's y position
        lda #100
        sta VIC + 10            ; set sprite 5's x position
        lda #58
        sta VIC + 11            ; set sprite 5's y position

        ; ********************
        ; expand sprites
        ; ********************
        lda #%00000011
        sta VIC + 23            ; expand sprites 0 and 1 in y
        sta VIC + 29            ; expand sprites 0 and 1 in x

        ; ********************
        ; set sprite colors
        ; ********************
        ldx #1
        ldy #6
        stx VIC + 39            ; set sprite 0's color
        sty VIC + 40            ; set sprite 1's color
        stx VIC + 41            ; set sprite 2's color
        sty VIC + 42            ; set sprite 3's color
        stx VIC + 43            ; set sprite 4's color
        sty VIC + 44            ; set sprite 5's color

        ; ********************
        ; clear screen print message
        ; ********************
        lda #<@cls
        ldy #>@cls
        jsr STROUT
        ldx #0
        ldy #15
        clc
        jsr PLOT
        lda #<@msg1
        ldy #>@msg1
        jsr STROUT
        ldx #1
        ldy #15
        clc
        jsr PLOT
        lda #<@msg2
        ldy #>@msg2
        jsr STROUT

        ; ********************
        ; set sprite data
        ; ********************
        ldx #0
@sprloop
        lda @spriteData,x
        sta $3000,x
        inx
        cpx #128
        bcc @sprloop

        ; ********************
        ; set sprite x and y direction
        ;
        ; 0 none
        ; 1 left/up
        ; 2 right/down
        ; ********************
        lda #1
        sta dx
        lda #2
        sta dy

        ; set x extend flags
        lda #%00000011
        sta VIC + 16
        sta side
@mainLoop
        jsr GETIN
        beq @continue        
        jmp @exit
@continue
        ldx #3                  ; outer delay
        ldy #0                  ; inner delay
        jsr @delay

        lda VIC
        sta x_                  ; sprite 0's x position
        lda VIC + 1             ; sprite 0's y position
        sta y_

        cmp #MINY
        beq @changedy
        cmp #MAXY
        bne @calcrx

@changedy
        ldx dy
        beq @dyexit             ; 0 = no change

        cpx #1                  ; 1 = up   2 = down
        beq @dydown

        ; change down to up
        dex
        jmp @dyexit
@dydown
        ; change up to down
        inx
@dyexit
        stx dy
@calcrx
        lda VIC + 16
        and #%00000011
        sta rx

        ; if x=24 and rx=0 then dx=-dx
        lda x_
        cmp #MINX
        bne @checkrx
        lda rx
        beq @changedx

        ; ifx=40 and rx=1 then dx=-dx
@checkrx
        lda x_
        cmp #MAXEXT
        bne @notrightedge
        lda rx
        beq @notrightedge

@changedx
        ldx dx
        beq @dxexit             ; no change

        cpx #1                  ; 1 = left   2 = right
        beq @dxright

        ; change to left
        dex
        jmp @dxexit
@dxright
        ; change to right
        inx
@dxexit
        stx dx

        ;  if x=255 and dx=1 then x=-1 : side=3
@notrightedge
        lda x_
        cmp #255
        bne @no_extendx
        lda dx
        cmp #2
        beq @extendx
        bne @no_extendx
@extendx
        ; extend side for sprite 0 and 1
        lda #255
        sta x_
        lda #%00000011
        sta side
        jmp @movex

@no_extendx
        lda x_
        cmp #0
        bne @movex
        lda dx
        cmp #1
        bne @movex

        ; side 0
        ; x_ = 256
        lda #0
        sta side
        lda #255
        sta x_
@movex
        lda dx
        beq @movey
        cmp #2
        beq @moveright

        ; move left
        lda x_
        sec
        sbc #1
        sta x_
        jmp @movey

@moveright
        lda x_
        clc
        adc #1
        sta x_
@movey
        lda dy
        beq @loopend

        cmp #1
        bne @movedown

        lda y_
        sec
        sbc #1
        sta y_
        jmp @loopend

@movedown
        lda y_
        clc
        adc #1
        sta y_

@loopend
        lda side
        sta VIC + 16

        lda x_
        sta VIC + 0         ; put new x value into sprite 0's x position
        sta VIC + 2         ; put new x value into sprite 1's x position

        lda y_
        sta VIC + 1         ; put new x value into sprite 0's y position
        sta VIC + 3         ; put new x value into sprite 1's y position
        jmp @mainLoop

@delay
        dey
        bne @delay
        dex
        bne @delay
        rts
@exit
        lda #246
        sta VIC + 33        ; set background color blue
        lda #0
        sta VIC + 21        ; disable sprites
        lda #<@ecls
        ldy #>@ecls
        jsr STROUT
        rts

@cls    .byte 147, 28, 0
@ecls   .byte 147, 154, 0
@msg1   .text "THIS IS TWO HIRES SPRITES", 0
@msg2   .text "ON TOP OF EACH OTHER", 0

@spriteData
        .print off
        .byte 0,255,0,3,153,192,7,24,224,7,56,224,14,126,112,14,126,112,14,126,112
        .byte 6,126,96,7,56,224,7,56,224,1,56,128,0,153,0,0,90,0,0,56,0
        .byte 0,56,0,0,0,0,0,0,0,0,126,0,0,42,0,0,84,0,0,40,0,0
        .byte 0,0,0,0,102,0,0,231,0,0,195,0,1,129,128,1,129,128,1,129,128
        .byte 1,129,128,0,195,0,0,195,0,4,195,32,2,102,64,2,36,64,1,0,128
        .byte 1,0,128,0,153,0,0,153,0,0,0,0,0,84,0,0,42,0,0,20,0,0
        .print on

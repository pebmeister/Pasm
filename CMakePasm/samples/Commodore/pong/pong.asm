        ; Pong written by Paul Baxter        

        .inc "macros.asm"
        .inc "kernal.asm"
        .inc "basic.asm"

        VIC         = $D000     ; this is where the VIC registers begin
        SID         = $D400     ; this is where the SID chip resides

        TScrn       = 70        ; screen top edge pixels (where ball bounces)
        LScrn       = 24        ; screen left edge pixels (score for right)
        BScrn       = 216       ; screen bottom edge pixels (where ball bounces)
        RScrn       = 325       ; screen right edge pixels (score for left)
        LPad        = 55        ; left paddle pos X pixels
        RPad        = 35        ; right paddle pos X (minus 255) pixels
        PadPosStart = 100       ; paddle y pos start
        BallSLeft   = 55        ; ball start left position pixels
        BallSTop    = 180       ; ball start top position pixels
        SpeedIncX   = $0100     ; Fraction Part =  SpeedIncX / 65535
        TopLnPos    = 2         ; top line pos character row
        BotLnPos    = 21        ; bottom line pos character row
        MaxSpeed    = $8000     ; Max Speed Fraction Part MaxSpeed / 65535
        Joy1        = $DC00     ; Joystick 1 port
        Joy2        = $DC01     ; Joystick 2 port
        JoyMin      = 71        ; Joy min top position
        JoyMax      = 200       ; Joy max bottom position 
        JoyInc      = $1100     ; JoYSpeedtick speed Fraction Part =  JoyInc / 65535
        Black       = 0         ; Color number for black
        White       = 1         ; Color number for white        
        PauseRow    = 5         ; Row for Pause message
        PauseCol    = 10        ; Column for Pause message
        DivCol      = 18        ; Column for middle of screen
        DivChar     = $E6       ; Line down the center
        WinSCore    = $11       ; score needed to win 
        BSPXStart   = $0F00     ; Ball X Speed start Fraction Part =   BSPXStart / 65535
        BSPYStart   = $0300     ; Ball Y Speed start Fraction Part =   BSPYStart / 65535
        AutoCntDown = 6         ; Auto paddle countdown
        RV          = 18        ; Reverse ON
        RVOFF       = 146       ; Reverse OFF

        ; Sid sound parameters
        ; see https://www.c64-wiki.com/wiki/SID
        WAVE        = 16        ; wave type triangle
        VOL         = 15        ; VOLUME
        ATTACK      = 0         ; attack 0 - 15     
        DECAY       = 4         ; decay 0 - 15               
        SUSTAIN     = 0         ; sustain 0 - 15
        RELEASE     = 0         ; release 0 - 15
        FREHI       = 25        ; High frequency
        FRELO       = 177       ; Low frequency

        .org $C000

Start
        jsr @Initialize         ; Initialize the game
        jsr @SplashScreen       ; draw splashscreen (set numplayers)
        jsr @DrawScreen         ; draw main screen
        jsr @WaitFireClear      ; wait for fire not pressed
        jsr @Pause              ; Put the game in Pause mode

;------------------------------------
@MainLoop
        ; If we are paused
        ; then see if player hit fire  button
        lda Pause
        beq @MainContinue     
        jsr @WaitForFire

@MainContinue
        ; move the ball
        jsr @MoveBall

        ; read the joysticks
        jsr @ReadJoySticks

        ; check if ball hit a paddle
        jsr @CheckSpriteCollision

        ; check sound complete
        jsr @CheckSound

        ; Loop
        jmp @MainLoop

;------------------------------------

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;  move ball position
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
@MoveBall
        ; If we are paused
        ; dont move the ball
        lda Pause
        bne @MoveExit

        ; First check X direction
        ; 0 - Right
        ; 1 - Left
        lda XDirection
        bne @MoveRight

        ; Move ball left
        jsr @MoveBallLeft
        jmp @CheckYDirection

@MoveRight
        ; move ball right
        jsr @MoveBallRight

        ; Now check Y direction
@CheckYDirection
        ; 0 - Down
        ; 1 - Up
        lda YDirection
        bne @MoveDown

        ; move up position
        jsr @MoveBallUp
        jmp @UpdateSpritePos
@MoveDown
        jsr @MoveBallDown

        ; Positions are in FixedPoint
        ; Move the int part
        ; to the sprite position
@UpdateSpritePos
        ; move position to sprite
        jmp @PosToSprite
@MoveExit
        rts

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;  move ball left
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
@MoveBallLeft
        ; move ball pos left 
        SUBFIX16 XPos, XSpeed, XPos

        ; if the ball reaches Left Screen Pos
        ; then right player scored
        BGE16I XPos, LScrn, @ExitMove

        ; Play the score sound
        lda #0
        jsr @PlaySound

        ; add 1 to Right Player score
        sed
        lda RScore
        clc
        adc #01
        sta RScore
        cld

        ; display new score
        ; re init the ball speed
        ; and change x direction
        jsr @RightScoreChange

        ; see the Right Player won
        ldx RScore        
        cpx #WinSCore
        bne @RightNotWin

        ; Right won
        jmp @RightWin
        
@RightNotWin
        jmp @Pause

@ExitMove
        rts

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;  move ball right
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
@MoveBallRight
        ; move ball pos right
        ; if the ball reaches right screen pos
        ; then Left player scored
        ADDFIX16 XPos, XSpeed, XPos          
        BLE16I XPos, RScrn, @ExitMove
        
        ; Play score sound
        lda #0
        jsr @PlaySound
        
        ; add 1 to left player score
        sed
        lda LScore
        clc
        adc #01
        sta LScore
        cld

        ; display new score
        ; reinit the ball speed
        ; and change x direction
        jsr @RightScoreChange

        MOVE16I RAutoFireCnt, 600

        ; see the Right Player won
        ldx LScore
        cpx #WinSCore
        bne @LeftNotWin

        ; Left player wins
        lda NumPlayers
        bne @LWin

        jsr @InitBallPos

        lda #1
        sta XDirection
@LWin
        jmp @LeftWin
@LeftNotWin

        jmp @Pause

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;  move ball up
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
@MoveBallUp
        ; move up position
        ; if position hits the top of the screen
        ; change ball y direction
        SUBFIX16 YPos, YSpeed, YPos
        ; see if we hit the top
        BGE16I YPOS, TScrn, @ExitMoveY
@ChangeYDir
        jmp @ChangeYDirection
@ExitMoveY
        rts

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;  move ball down
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
@MoveBallDown
        ; move up position
        ; if position hits the bottom of the screen
        ; change ball y direction
        ADDFIX16 YPos, YSpeed, YPos
        BLE16I  YPos, BScrn, @ExitMoveY
        jmp @ChangeYDirection

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;  change ball x direction
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
@ChangeXDirection
        ; X = 1 - X  
        ; toggles 1 and 0
        lda #1
        sec
        sbc XDirection
        sta XDirection
        rts

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;  change ball y direction
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
@ChangeYDirection
        ; Y = 1 - Y
        ; toggles 1 and 0
        lda #1
        sec
        sbc YDirection
        sta YDirection

        lda #0
        sta Temp + 1
        jsr @RND
        and #%0000111
        sta Temp

        BGE16I YSpeed + 2, MaxSPeed, @ReduceYSpeed

        jsr @RND
        and #%00000010
        bne @AddRandom

        BLE16 YSpeed + 2, Temp, @ChangeYPlaySound
@ReduceYSpeed
        SUB16 YSpeed + 2, Temp, YSpeed + 2

        jmp @ChangeYPlaySound

@AddRandom
        ADD16 YSpeed + 2, Temp, YSpeed + 2

        ; Play sound when changing
        ; Y direction
@ChangeYPlaySound
        lda #1
        jmp @PlaySound

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;  increase ball X speed
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
@CheckSpeedInc
        ; check ball increase x speed
        ; only increase every 10-11 times
        ; if the int part of speed is
        ; not 0 or fraction part
        ; is at MaxSPeed then dont
        ; increase speed
        lda INCCount
        cmp #10
        bcc @NoIncrease

        BNE16I XSPEED, 0, @ResetInc
        BGE16I XSpeed + 2, MaxSpeed, @ResetInc

        ADDFIX16 XSpeed, XInc, XSpeed
 
@ResetInc
        lda #0
        sta INCCount

@NoIncrease
        rts

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;  check if ball hit either paddle
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
@CheckSpriteCollision
        ; if theres no collision exit
        lda VIC + 30
        beq @ColExit

        ; save colision data
        sta Temp
        lda #1
        jsr @PlaySound

        ; increment ball speed incrment count
        inc INCCount

        ; check right paddle collision
        lda Temp
        and #00000010
        bne @ColLeft

        ; Collision Right
        ; reposition ball to
        ; left of the paddle
        lda #1
        sta XPos + 1 
        lda #RPad - 10
        sta XPos
        MOVE16I XPos + 2, 0

        ; set x Direction
        lda #0
        sta XDirection

@SPColUpDateSpritePos
        ; move XPos and YPos to sprite position
        jsr @PosToSprite

        ; increase xspeed on bounce
        jmp @CheckSpeedInc
@ColExit
        rts

@ColLeft
        ; Collision Left
        ; reposition ball to
        ; Right of the paddle
        lda #0
        sta XPos + 1
        lda #LPad + 18
        sta XPos
        MOVE16I XPos + 2, 0
        lda #1
        sta XDirection
        jmp @SPColUpDateSpritePos

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;  move  XPos, YPos to ball sprite
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
@PosToSprite
        ; to move past position 255
        ; must extend right flag of VIC
        ; if its 255 or less
        ; turn off extend right flag

        ; check high byte of int
        lda XPos + 1
        bne @ExtendRight

        ; turn off extend right
        lda VIC + 16
        and #~00000001
        sta VIC + 16

        ; store position in sprite
        jmp @SetSpritePos

@ExtendRight
        ; turn on extend right
        lda VIC + 16
        ora #%00000001
        sta VIC + 16

@SetSpritePos
        lda XPos            ; low int byte of pos
        sta VIC + 0         ; put new x value into sprite 0's x position
        lda YPos
        sta VIC + 1         ; put new y value into sprite 0's y position
        rts

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;  Right score change
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
@RightScoreChange
        ; display new score
        ; re init the ball speed
        ; and change direction
        jsr @DrawScore
        jsr @InitBallSpeed
        jmp @ChangeXDirection

        ; ********************
        ;
        ; ReadJoySticks
        ;
        ; ********************
@ReadJoySticks
        ; Read JoySticks
        jsr @ReadJoy1
        lda NumPlayers
        beq @UseAutoRight
        jmp @ReadJoy2
@UseAutoRight
        jmp @AutoRight        

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;  Read @ReadJoy1
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
@ReadJoy1
        ; Bits of JoyStick need to be flipped
        ; 0 = pressed 
        ; 1 = not pressed
        ; We do this by subtracting 
        ; from %01111111 
        sec
        lda #%01111111
        sbc Joy1
        sta Temp
        and #%00000010
        beq @CheckJoy1Up

        ; Joy1 DOWN
        ; check if paddle is at max
        lda LPadYPos
        cmp #JoyMax
        bcs @Joy1Exit

        ; move paddle down
        ADDFIX16 LPadYPos, PInc, LPadYPos
        lda LPadYPos
        sta VIC + 3
@Joy1Exit
        ; save direction
        ; and fire button state
        lda Temp
        tax
        and #%00010000
        sta LFire
        txa
        and #%00000011
        sta LY
        rts

@CheckJoy1Up
        ; bit 0 is UP
        lda Temp
        and #%00000001
        beq @Joy1Exit

@Joy1Up
        ; Joy1 UP
        ; check for min paddle position        
        lda LPadYPos
        cmp #JoyMin
        bcc @Joy1Exit

        ; move paddle up
        SUBFIX16 LPadYPos, PInc, LPadYPos
        lda LPadYPos
        sta VIC + 3
        jmp @Joy1Exit

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;  Read @ReadJoy2
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
@ReadJoy2
        ; Bits of JoyStick need to be flipped
        ; 0 = pressed 
        ; 1 = not pressed
        ; We do this by subtracting 
        ; from %01111111 
        sec
        lda #%01111111
        sbc Joy2
        sta Temp
        and #%00000010
        beq @CheckJoy2Up

        ; Joy2 DOWN
        ; check for max psition
        lda RPadYPos
        cmp #JoyMax
        bcs @Joy2Exit

        ; move paddle down
        ADDFIX16 RPadYPos, PInc, RPadYPos
        lda RPadYPos
        sta VIC + 5
@Joy2Exit
        ; save direction and 
        ; fire button state
        lda Temp
        tax
        and #%00010000
        sta RFire
        txa
        and #%00000011
        sta RY
        rts

@CheckJoy2Up
        ; bit 0 = up
        lda Temp
        and #%00000001
        beq @Joy2Exit

@Joy2Up
        ; Joy2 UP
        ; check for min position
        lda RPadYPos
        cmp #JoyMin
        bcc @Joy2Exit

        ; move paddle up
        SUBFIX16 RPadYPos, PInc, RPadYPos
        lda RPadYPos
        sta VIC + 5
        jmp @Joy2Exit
 
        ; ********************
        ; initialize
        ; ********************
@Initialize
        jsr @InitSid
        jsr @InitRnd

        ; score
        lda #0
        sta LScore
        sta RScore

        ; Auto fire
        sta RAutoFireCnt
        sta RAutoFireCnt + 1

        ; last paddle Y direction
        sta LY
        sta RY

        ; speed increment count
        sta INCCount

        ; ball/paddle speed increment
        ; high wordsta XInc
        sta XInc + 1
        sta PInc
        sta PInc + 1

        ; ball/paddle speed increment
        ; low word
        MOVE16I XInc + 2, SpeedIncX
        MOVE16I PInc + 2, JoyInc

        ; set x and y direction
        lda #1
        sta XDirection
        jsr @RND
        and #%00000001
        sta YDirection

        lda #AutoCntDown
        sta RAutoCnt

        ; set ball position
        jsr @InitBallPos

        ; set ball speed
        jsr @InitBallSpeed

        ; set position of the ball
        ; 16x16 Fixed Point
        lda #0
        sta Pause
        
        ; ********************
        ; set sprite pointers
        ; ********************
        ldx #192
        stx 2040                ; set sprite 0's pointer
        inx
        stx 2041                ; set sprite 1's pointer
        stx 2042                ; set sprite 2's pointer

        ; ********************
        ; set sprite colors
        ; ********************
        ldx #White              ; white for all sprites
        stx VIC + 39            ; set sprite 0's color
        stx VIC + 40            ; set sprite 1's color
        stx VIC + 41            ; set sprite 2's color

        ldx XPos
        stx VIC + 0             ; set sprite 0's x position
        ldy YPos
        sty VIC + 1             ; set sprite 0's y position
 
        ldx #LPad
        stx VIC + 2             ; set sprite 1's x position
        ldx #PadPosStart
        stx VIC + 3             ; set sprite 1's y position

        stx LPadYPos            ; set left paddle Y position high byte
        stx RPadYPos            ; set right paddle Y position high byte
        ldx #0                  ; set low byte and low word of
        stx LPadYPos + 1        ; left and right paddle to 0
        stx LPadYPos + 2
        stx LPadYPos + 3
        stx RPadYPos + 1
        stx RPadYPos + 2
        stx RPadYPos + 3
        ldx #RPad
        stx VIC + 4             ; set sprite 2's x position
        ldx #PadPosStart
        stx VIC + 5             ; set sprite 2's y position

        lda VIC + 16            ; set sprite 2's extended x
        ora #%00000100
        sta VIC + 16

        ; ********************
        ; set sprite data
        ; ********************
        ldx #0
@SprLoop
        lda @spriteData,x
        sta $3000,x
        inx
        cpx #63 * 2 + 1
        bcc @SprLoop

        ; draw the screen
        jsr @DrawScreen

        ; draw the score
        jsr @DrawScore

        ; ********************
        ; Enable sprites
        ; ********************
        lda #%00000110
        sta VIC + 21            ; enable sprites 1-2 
        rts

        ; ********************
        ; Init ball position
        ; ********************
@InitBallPos
        lda #0
        sta XPos + 1
        sta XPos + 2
        sta XPos + 3
        sta YPos + 1
        sta YPos + 2
        sta YPos + 3
        lda #BallSLeft
        sta XPos
        lda #BallSTop
        sta YPos
        rts

        ; ********************
        ;
        ; InitBall Speed
        ;
        ; ********************
@InitBallSpeed
        ; set speed for x and speed for y
        ; of the ball
        ; 16x16 Fixed Point
        lda #$00
        sta XSpeed + 0
        sta XSpeed + 1
        sta YSpeed
        sta YSpeed + 1
        MOVE16I XSpeed + 2, BSPXStart
        MOVE16I YSpeed + 2, BSPYStart
        ; reset ball speed increase
        lda #0
        sta INCCount
        rts

        ; ********************
        ;
        ; Draw the screen
        ;
        ; ********************
@DrawScreen
        ; set background color
        lda #Black
        sta VIC + 32        ; set boarder color black
        sta VIC + 33        ; set background color black
        
        ; clear screen
        lda #<@Cls
        ldy #>@Cls
        jsr STROUT

        ; draw top line
        ldx #TopLnPos
        ldy #0
        clc
        jsr PLOT
        lda #<@Line
        ldy #>@Line
        jsr STROUT

        ; draw bottom line
        ldx #BotLnPos
        ldy #0
        clc
        jsr PLOT
        lda #<@Line
        ldy #>@Line
        jsr STROUT

        ; draw line down center
        ldx #TopLnPos + 1
        stx Temp
    @LineLoop
        ldy #DivCol
        clc
        jsr PLOT
        lda #DivChar
        jsr CHROUT
        ldx Temp
        inx
        stx Temp
        cpx #BotLnPos
        bcc @LineLoop
        rts

        ; ********************
        ;
        ; Wait for for clear
        ; Wait for player to
        ; release fire button
        ; ********************
@WaitFireClear
        jsr @ReadJoy1
        lda LFire
        bne @WaitFireClear
        rts

        ; ********************
        ;
        ; Draw Score
        ;
        ; ********************
@DrawScore
        ; Set Position
        ldx #0
        ldy #08
        clc
        jsr PLOT

        ; Draw left score
        lda LScore
        jsr @PByte

        ; Set Position
        ldx #0
        ldy #30
        clc
        jsr PLOT

        ; Draw right score
        lda RScore
        jmp @PByte  

        ; ********************
        ;
        ; Draw Byte
        ; Byte is in Temp as BCD
        ; ********************
@PByte
        sta Temp

        ; shift for high nibble
        lsr
        lsr
        lsr
        lsr 
        bne @PHi
        
        ; 0 for high nibble
        ; draw a space
        lda #' '
        jsr CHROUT
        jmp @PLow
@PHi        
        jsr @PChar
@PLow
        ; and $0F for low nibble        
        lda Temp
        and #$0F
@PChar
        clc
        adc #'0'
        jmp CHROUT

        ; ********************
        ;
        ; LeftWin
        ;
        ; Left player won
        ; ********************
@LeftWin
        lda #'1'
        sta WINPLAYER
        jmp @PlayerWins

        ; ********************
        ;
        ; RightWin
        ;
        ; Right player won
        ; ********************
@RightWin
        lda #'2'
        sta WINPLAYER

        ; a Player has won
        ; Pause game and display
        ; winner
@PlayerWins
        ; Pause
        lda #1
        sta Pause

        ; Disable ball sprite
        lda VIC + 21
        and #~00000001
        sta VIC + 21            ; disable sprite 0

        ; reset score to 0
        lda #0
        sta LScore
        sta RScore

        ; set position
        ldx #PauseRow
        ldy #PauseCol
        clc
        jsr PLOT

        ; display PLAYER
        lda #<@RVP
        ldy #>@RVP
        jsr STROUT

        lda #' '
        jsr CHROUT

        ; display player number
        lda WINPLAYER
        jsr CHROUT

        ; display WINS
        lda #<@Wins
        ldy #>@Wins
        jmp STROUT

        ; ********************
        ;
        ; Pause
        ;
        ; ********************
@Pause
        ; set pause flag
        lda #1
        sta Pause

        ; disable sprite 0
        lda VIC + 21
        and #~00000001
        sta VIC + 21

        ; set position
        ldx #PauseRow
        ldy #PauseCol
        clc
        jsr PLOT

        ; draw Player
        lda #<@RVP
        ldy #>@RVP
        jsr STROUT

        lda #' '
        jsr CHROUT

        ; determine player by ball direction
        ldx XDirection
        beq @WaitRightPlayer

        ; draw '1'
        lda #'1'
        jsr CHROUT
        jmp @DisplayPress

@WaitRightPlayer
        ; draw '2'        
        lda #'2'
        jsr CHROUT

@DisplayPress
        ; display Press fire
        lda #<@Press
        ldy #>@Press
        jmp STROUT

        ; ********************
        ;
        ; Wait for Fire
        ;
        ; ********************
@WaitForFire
        ; determine player by ball direction
        lda XDirection
        beq @WaitRight

        ; left Joystick Fire flag
        MOVE16 LPadYPos, YPos

        lda LFire
        beq @Exit

        ; unpause
        jmp @ExitUnPause

@WaitRight
        ; right Joystick fire flag
        lda RFire
        beq @Exit

        ; move ball ypos to match paddle
        MOVE16 RPadYPos, YPos

@ExitUnPause        
        jsr @PosToSprite        ; update ball sprite
        ldx #PauseRow           ; set position
        ldy #PauseCol
        clc
        jsr PLOT
        lda #<@Erase            ; erase pause message
        ldy #>@Erase
        jsr STROUT

        ldx #PauseRow           ; redraw center line
        ldy #DivCol
        clc
        jsr PLOT
        lda #DivChar
        jsr CHROUT

        jsr @InitBallSpeed      ; reinitialize ball speed

        lda VIC + 21
        ora #00000001
        sta VIC + 21            ; enable sprite 0
        lda #0                  ; clear pause flag
        sta Pause
        jmp @DrawScore          ; draw score
@Exit
        rts

        ; ********************
        ;
        ; Init SID
        ;
        ; ********************
@InitSid
        ; set note duration count        
        ldx #0
        txa
        sta NCNT
        ; clear sid registers
@SidInitLoop
        sta SID,x
        inx
        cpx #25
        bcc @SidInitLoop
        rts

        ; ********************
        ;
        ; PlaySound
        ; a contains sound type
        ; ********************
@PlaySound
        tax
        ; turn off any previous sound
        jsr @StopSound

        lda #ATTACK*16+DECAY            ; set attack / decay        
        sta SID + 5
        lda #SUSTAIN*16+RELEASE         ; set sustain / release
        sta SID + 6
        lda #VOL                        ; set volume
        sta SID + 24                    ; set high frequency

        txa
        beq @PlayScore

        lda #FREHI + 2
        jmp @SetLowFreq
@PlayScore
        lda #FREHI
@SetLowFreq
        sta SID + 1
        lda #FRELO                      ; set low frequency
        sta SID

        lda # WAVE | %00000001          ; set wave type and open flag
        sta SID + 4                     ; set note duration
        lda #200
        sta NCNT
        rts

        ; ********************
        ;
        ; Check Sound
        ;       see if a sound
        ;       is done
        ; ********************
@CheckSound
        lda NCNT
        beq @CheckSoundExit
                                        ; check note duration counter
@DecCounter
        dec NCNT                        ; decrement counter
        lda NCNT
        bne @CheckSoundExit
@StopSound
        lda # WAVE & ~ %00000001        ; turn off enable bit
        sta SID + 4

@CheckSoundExit
        rts

        ; ********************
        ;
        ; Auto Right
        ;
        ; ********************
@AutoRight
        lda #0
        sta RFire

        dec RAutoCnt
        lda RAutoCnt
        bne @AutoRightExit

        BEQ16I RAutoFireCnt, 0, @ResetAutoCnt

        DEC16 RAutoFireCnt
        BNE16I RAutoFireCnt, 0, @ResetAutoCnt
        
        lda #1
        sta RFire

@ResetAutoCnt
        lda #AutoCntDown
        sta RAutoCnt

        SUB16I YPOS,7,TEMP
        BEQ16 RPadYPos, TEMP, @AutoRightExit
        BLT16 RPadYPos, TEMP, @MoveRPadUp

        lda RPadYPos
        cmp #JoyMin
        bcc @AutoRightExit

        ; move paddle up
        SUBFIX16 RPadYPos, PInc, RPadYPos
        lda RPadYPos
        sta VIC + 5
        jmp @AutoRightExit

@MoveRPadUp
        lda RPadYPos
        cmp #JoyMax
        bcs @AutoRightExit

        ; move paddle up
        ADDFIX16 RPadYPos, PInc, RPadYPos
        lda RPadYPos
        sta VIC + 5

@AutoRightExit
        rts

        ; ********************
        ;
        ; Initialize Random
        ;
        ; ********************
@InitRnd
        lda #$FF  ; maximum frequency value
        sta $D40E ; voice 3 frequency low byte
        sta $D40F ; voice 3 frequency high byte
        lda #$80  ; noise waveform, gate bit off
        sta $D412 ; voice 3 control register
        rts
@RND
        lda $D41B
        rts

        ; ********************
        ;
        ; Splashscreen
        ;
        ; ********************
@SplashScreen
        jsr @DrawScreen

        lda #0
        sta NumPlayers

@SplashLoop
        ; set position        
        ldx #PauseRow
        ldy #PauseCol -3
        clc
        jsr PLOT

        lda NumPlayers
        bne @NoRev1

        lda #RV
        jsr CHROUT
@NoRev1
        lda #'1'
        jsr CHROUT
        lda #' '
        jsr CHROUT

        ; display PLAYER
        lda #<@Player
        ldy #>@Player
        jsr STROUT
        lda #RVOFF
        jsr CHROUT

        ldx #PauseRow
        ldy #PauseCol + 12
        clc
        jsr PLOT

        lda NumPlayers
        beq @NoRev2

        lda #RV
        jsr CHROUT
@NoRev2
        lda #'2'
        jsr CHROUT
        lda #' '
        jsr CHROUT

        ; display PLAYER
        lda #<@Player
        ldy #>@Player
        jsr STROUT
        lda #'S'
        jsr CHROUT
        lda #RVOFF

        jsr CHROUT

@ReadPlayers
        sec
        lda #%01111111
        sbc Joy1
        beq @ReadPlayers

        tax
        and #%00010000
        bne @SplashExit

        txa
        and #%00000101
        bne @Left

        txa
        and #%00001010
        bne @Right

@Left
        lda NumPlayers
        beq @ReadPlayers
        jmp @ChangeNumPlayers
@Right
        lda NumPlayers
        bne @ReadPlayers

@ChangeNumPlayers
        lda #1
        sec
        sbc NumPlayers
        sta NumPlayers
        jmp @SplashLoop

@SplashExit
        rts



;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;  Sprite definitions
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
@spriteData
        ;  ball
        .byte  %11111111, %00000000, %00000000
        .byte  %11111111, %00000000, %00000000
        .byte  %11111111, %00000000, %00000000
        .byte  %11111111, %00000000, %00000000
        .byte  %00000000, %00000000, %00000000
        .byte  %00000000, %00000000, %00000000
        .byte  %00000000, %00000000, %00000000
        .byte  %00000000, %00000000, %00000000
        .byte  %00000000, %00000000, %00000000
        .byte  %00000000, %00000000, %00000000
        .byte  %00000000, %00000000, %00000000
        .byte  %00000000, %00000000, %00000000
        .byte  %00000000, %00000000, %00000000
        .byte  %00000000, %00000000, %00000000
        .byte  %00000000, %00000000, %00000000
        .byte  %00000000, %00000000, %00000000
        .byte  %00000000, %00000000, %00000000
        .byte  %00000000, %00000000, %00000000
        .byte  %00000000, %00000000, %00000000
        .byte  %00000000, %00000000, %00000000
        .byte  %00000000, %00000000, %00000000
        .byte  %00000000, %00000000, %00000000

        ; paddle
        .byte  %00000000, %00111000, %00000000
        .byte  %00000000, %00111000, %00000000
        .byte  %00000000, %00111000, %00000000
        .byte  %00000000, %00111000, %00000000
        .byte  %00000000, %00111000, %00000000
        .byte  %00000000, %00111000, %00000000
        .byte  %00000000, %00111000, %00000000
        .byte  %00000000, %00111000, %00000000
        .byte  %00000000, %00111000, %00000000
        .byte  %00000000, %00111000, %00000000
        .byte  %00000000, %00111000, %00000000
        .byte  %00000000, %00111000, %00000000
        .byte  %00000000, %00111000, %00000000
        .byte  %00000000, %00111000, %00000000
        .byte  %00000000, %00111000, %00000000
        .byte  %00000000, %00111000, %00000000
        .byte  %00000000, %00111000, %00000000
        .byte  %00000000, %00111000, %00000000
        .byte  %00000000, %00111000, %00000000
        .byte  %00000000, %00111000, %00000000
        .byte  %00000000, %00111000, %00000000
        .byte  %00000000, %00111000, %00000000

@Cls    .byte 147, 5, 0
@Line   .byte $C0,$C0,$C0,$C0,$C0,$C0,$C0,$C0,$C0,$C0
        .byte $C0,$C0,$C0,$C0,$C0,$C0,$C0,$C0,$C0,$C0
        .byte $C0,$C0,$C0,$C0,$C0,$C0,$C0,$C0,$C0,$C0
        .byte $C0,$C0,$C0,$C0,$C0,$C0,$C0,$C0,$C0,$C0
        .byte 0
@RVP    .byte RV
@Player .byte "PLAYER",0
@Press  .byte " PRESS FIRE", RVOFF, 0
@Wins   .byte "   WINS!!!!", RVOFF, 0
@Erase  .byte "                   ",0

        ; ********************
        ; Storage
        ; Since there is no BASIC
        ; to conflict with
        ; use as much  Page Zero 
        ; as I need
        ;
        ; These could be located
        ; else where if needed
        ; ********************

        * = $26
YPos            .ds 4
XPos            .ds 4
RPadYPos        .ds 4
LPadYPos        .ds 4
XSpeed          .ds 4
YSpeed          .ds 4
XInc            .ds 4 
PInc            .ds 4 
Temp            .ds 2
RAutoFireCnt    .ds 2
NCNT            .ds 1
XDirection      .ds 1
YDirection      .ds 1
LScore          .ds 1
RScore          .ds 1
LFire           .ds 1
RFire           .ds 1
Pause           .ds 1
WINPLAYER       .ds 1
LY              .ds 1
RY              .ds 1
INCCount        .ds 1
RAutoCnt        .ds 1
NumPlayers      .ds 1

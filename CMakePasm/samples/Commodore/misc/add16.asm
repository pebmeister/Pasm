/* 
;********************************************
;*                                          *
;*  MULTA16                                 *
;*                                          *
;*  16bit multiplication     32bit result   *
;*                                          *
;*  \1  Variable a     16-bit               *
;*  \2  Variable b     16 bit               *
;*  \3  Destination    32 bit               *
;*                                          *
;********************************************
;16-bit multiply with 32-bit product 
;taken from 6502.org
 
multiplier  = $f7 
multiplicand= $f9 
product     = $fb 
 
mymult16  
            lda #$00
            sta product+2   ; clear upper bits of product
            sta product+3 
            
            ldx #$10        ; set binary count to 16 
shift_r
            lsr multiplier+1    ; divide multiplier by 2 
            ror multiplier
            bcc rotate_r 
            
            lda product+2   ; get upper half of product and add multiplicand
            clc
            adc multiplicand
            sta product+2
            lda product+3 
            adc multiplicand+1
rotate_r

            ror         ; rotate partial product 
            sta product+3 
            ror product+2
            ror product+1 
            ror product 
            
            dex
            bne shift_r
            rts
 */
 
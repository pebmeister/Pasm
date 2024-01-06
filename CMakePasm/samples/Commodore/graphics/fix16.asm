

;****************************************************
;*                                                  *
;* FIX16_MUL                                        *
;*                                                  *
;*  fix16 multiply                                  *
;*                                                  *
;*  16bit integer    16bit Fraction                 *
;*                                                  *
;*  FixedA  IN:  4 bytes 1st number to multiply     *
;*  FixedB  IN:  4 bytes 2nd number to multiply     *
;*  P_HI    OUT: 4 bytes result of fixed multiply   *
;*  P_LO    OUT: 4 bytes result of fixed multiply   *
;*                                                  *
;*	// Each argument is divided to 16-bit parts.    *
;*  destroys a, x, y                                *
;*                                                  *
;****************************************************
FIX16_MUL

@FixedA = $CF00
@FixedB = @FixedA + 4
@A = FixedB + 4
@B = A + 4
@C = B + 4
@D = C + 4
@AC = D + 4
@AD = AC + 4
@CB = AD + 4
@AD_CB = @CB + 4
@AD_CB_HI = @AD_CB + 4
@P_HI = @AD_CB_HI + 4
@P_LO = @P_HI + 4

    ;	int32_t A = (inArg0 >> 16), C = (inArg1 >> 16);
    ;	uint32_t B = (inArg0 & 0xFFFF), D = (inArg1 & 0xFFFF);
	
    MOVE16 @FixedA + 2, @A
    MOVE16I @A + 2, 0
    
    MOVE16 @FixedA, @B
    MOVE16I @B + 2, 0
    
    MOVE16 @FixedB + 2, @C
    MOVE16I @C, 0
    
    MOVE16 @FixedD, @D
    MOVE16I @C, 0
    
    ;	int32_t AC = A*C;
    ;	int32_t AD_CB = A*D + C*B;
    ;	uint32_t BD = B*D;
	MUL16 @A, @C, @AC
    MUL16 @A, @D, @AD
    MUL16 @C, @B, @CB
    ADDFIX16 @AD, @CB, @AD_CB 
    MUL16 @B, @D, @BD

    ;	int32_t product_hi = AC + (AD_CB >> 16);
    MOVE16 @AD_CB + 2, @AD_CB_HI
    MOVE16I 0, AD_CB_HI + 2
    ADDFIX16 @AC, @AD_CB + 2, @P_HI
    
    ; uint32_t ad_cb_temp = AD_CB << 16;
	; uint32_t product_lo = BD + ad_cb_temp;
	; if (product_lo < BD)
	;	product_hi++;
    ADDFIX16 @BD, @AD_CB, @P_LO
    lda @P_LO + 3
    cmp @BD + 3
    bcc @ADD
    lda @P_LO + 2
    cmp @BD + 2
    bcc @ADD
    lda @P_LO + 1
    cmp @BD + 1
    bcc @ADD    
    lda @P_LO
    cmp @BD
    bcs @RETURN
    
@ADD
    ADDFIX16 @P_HI, 1, @P_HI 
@RETURN
 	; return (product_hi << 16) | (product_lo >> 16);
    rts


 
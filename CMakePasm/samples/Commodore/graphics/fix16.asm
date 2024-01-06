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
;*  FixedC OUT:  4 bytes result                     *
;*                                                  *
;*	// Each argument is divided to 16-bit parts.    *
;*  destroys a, x, y                                *
;*                                                  *
;****************************************************
FIX16_MUL

    FixedA      = $CF00
    FixedB      = FixedA + 4
    FixedC      = FixedB + 4
    
    * = FixedC + 4
    @AC         .ds 4
    @AD         .ds 4
    @BD         .ds 4
    @CB         .ds 4
    @AD_CB      .ds 4
    @P_HI       .ds 4
    @P_LO       .ds 4
    @tmp        .ds 4
    * = FIX16_MUL
    
    @A          = FixedA
    @B          = FixedA + 2
    @C          = FixedB
    @D          = FixedB + 2
    
    ;	int32_t AC = A*C;
    ;	int32_t AD_CB = A*D + C*B;
    ;	uint32_t BD = B*D;
	MULT16 @A, @C, @AC
    MOVE16 @AC+2,@tmp
    MOVE16 @AC, @AC+2
    MOVE16 @tmp, @AC

    MULT16 @A, @D, @AD
    MOVE16 @AD+2,@tmp
    MOVE16 @AD, @AD+2
    MOVE16 @tmp, @AD
    
    MULT16 @C, @B, @CB
    MOVE16 @CB+2,@tmp
    MOVE16 @CB, @CB+2
    MOVE16 @tmp, @CB

    MULT16 @B, @D, @BD
    MOVE16 @BD+2,@tmp
    MOVE16 @BD, @BD+2
    MOVE16 @tmp, @BD
    
    ADDFIX16 @AD, @CB, @AD_CB 
  
    ;	int32_t product_hi = AC + (AD_CB >> 16);
    MOVE16I @tmp, 0
    MOVE16 @AD_CB, @tmp + 2
    ADDFIX16 @AC, @tmp, @P_HI
        
    MOVE16I @tmp + 2, 0
    MOVE16 @AD_CB + 2, @tmp
    ADDFIX16 @BD, @tmp, @P_LO
     
 	; return (product_hi << 16) | (product_lo >> 16);
    MOVE16 @P_HI + 2, FixedC 
    MOVE16 @P_LO, FixedC + 2
    
    rts
 
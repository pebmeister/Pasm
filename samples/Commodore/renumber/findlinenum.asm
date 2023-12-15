;********************************************
;*                                          *
;* FINDLINENUM                              *
;*                                          *
;*  find line number in a sorted table      *
;*  using a binary search                   *
;*                                          *
;********************************************
FINDLINENUM
        ;
        ;   init maxidx
        ;
        MOVE16 TBLSZ, MAXIDX   ; Save the Table size

        ;
        ;   init lastidx
        ;
        STOREI $FF, LASTIDX, 2

        ;
        ;   convert maxidx to bytes
        ;
        LSHIFT16 MAXIDX, 1

        ;
        ;   init minidx
        ;
        STOREI 0, MINIDX, 2

-
        ;
        ;   calculate next index to table
        ;   table is sorted so binary search
        ;   maxidx  - minidx
        ;
        SUB16 MAXIDX, MINIDX, CURIDX

        ;
        ;   divide by 2
        ;
        RSHIFT16 CURIDX, 1

        ;
        ;   add minidx
        ;
        ;   curidx += minidx
        ADD16 CURIDX, MINIDX, CURIDX

        ;
        ;   make CURIDX even (low byte) 
        ; 
        lda CURIDX              ;   curidx &= 0xFFFE
        and #~%00000001
        sta CURIDX

        ;
        ;   check to see if we tried this already
        ;
        BNE16 CURIDX, LASTIDX, +
        ;
        ;   we can't find the line
        ;
        rts
+
        ;
        ;   save current index
        ;
        MOVE16 CURIDX, LASTIDX

        ;
        ;   load tbl[curinx]
        ;
        ADD16 TBL, CURIDX, TBLPTR

        ;
        ;   compare high byte
        ;
        ldy #1
        lda (TBLPTR),y
        cmp LNUM + 1
        bne +
        ;
        ;   compare low byte
        ;
        dey
        lda (TBLPTR),y
        cmp LNUM
        beq +++
+
        ;
        ;   not equal
        ;
        bcc +

        ;
        ;   table entry greater than lnum
        ;
        MOVE16 CURIDX, MAXIDX
        jmp -                   ;   try next guess
+
        ;
        ;   table entry less than lnum
        ;
        MOVE16 CURIDX,MINIDX
        jmp -                   ;   try next guess
+
        rts


;********************************************
;*                                          *
;*  START                                   *
;*                                          *
;*  main entry point                        *
;*                                          *
;********************************************
START
        jsr INITPROG            ;   initialize the program
        jsr PASS1               ;   perform pass1

        ;
        ;   check for an error
        ;
        php
        bcs +
        jsr PASS2               ;   perform pass2
+
        jsr RESTORE             ;   put memory back to normal
        plp
        rts                     ;   done


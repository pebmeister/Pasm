;
;   zero page usage
;   we will map basic out so we can use
;   some basic zero page memory locations
;
MINIDX  = VARNAM                        ;   min index for line lookup 2 bytes
MAXIDX  = INPPTR                        ;   max index for line lookup 2 bytes
TBLSZ   = UNUSED2                       ;   size of the table 2 bytes
LASTIDX = UNUSED2 + 2                   ;   last used index 2 bytes
LNUMLEN = FACSSGN                       ;   length of line we are searchng for 1 byte
CURIDX  = INDEX                         ;   current index 2 bytes
NXTLN   = FREKZP                        ;   next basic statement 2 bytes
LNUM    = FREKZP + 2                    ;   line num to use 2 bytes
INCR    = TMPDATA                       ;   line num increment 2 bytes
BNVAR   = OPPTR                         ;   num to convert to bcd 3 bytes
BCD     = OPMASK                        ;   bcd number result 3 bytes
BCDSTR  = OLDLIN                        ;   bcd string result 6 bytes
INQUOTE = UNUSED1                       ;   holds flag if we are in a string 1 byte
PREVCH  = DIMFLG                        ;   prev char of line for 'go to' 1 byte
LDZFLG  = RESHO                         ;   flag for printing leading zeros 1 bytes
TBLPTR  = FORPNT                        ;   pointer for table 2 bytes
TBLTMP  = DATPTR                        ;   used to create table 2 bytes
BCDLEN  = FACEXP                        ;   length of BCD 1 byte
LININS  = FACHO                         ;   line number insert offset 2 byte
PRODUCT = FAC2                          ;   multiplication result


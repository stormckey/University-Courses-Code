;most part is the same as the x0200 in LC-3 but we set the kbsr to x4000
        .orig x0200
        ldi r0,makbdr
        ld r6,os_sp
        ld r0, user_psr
        add r6,r6,#-1
        str r0,r6,#0
        ld r1,allow
        sti r1,mapkbsr
        ld r0 user_pc
        add r6,r6,#-1
        str r0,r6,#0
        rti
allow   .fill x4000
mapkbsr .fill xFE00
makbdr  .fill xFE02
os_sp   .fill x3000
user_psr .fill x8002
user_pc .fill x3000
        .end
;change the address of the interrupt to x2000
        .orig x0180
        .fill x2000
        .end
;----------interrupt----------------------
;we use r7 to prevent the intterup from doinf bad things
;if r7 = #723 then the interrupt will do nothing
;during the input ,r7 will be assigned #723
;after the input we just need to distinguish number and letter and do different operations
;if it's number we add it to r4 ,and prevent r4 from overflowing at the same time
;if it is letter, int will cover r5
        .orig x2000
        st r1,saver1
        st r2,saver2
        st r3,saver3
        ld r1,test
        add r1,r1,r7
        brz skip5
        ldi r2,mapkbdr
        ld r1,negdot2
        add r1,r1,r2
        brz skip5
        ld r1,negnine
        add r1,r1,r2
        brp char
        and r3,r3,#0
        add r3,r3,#1
        sti r3,sent1
        ld r3,ascnum
        add r2,r2,r3
        add r4,r4,r2
        ld r3,bound
        add r3,r3,r4
        brn skip5
        ld r4,correct
        brnzp skip5
char    add r5,r2,#0
        brnzp skip5
skip5   ld r1,saver1
        ld r2,saver2
        ld r3,saver3
        rti
ascnum  .fill #-48
bound   .fill #-18
correct .fill #17
mapkbdr .fill xFE02
test    .fill #-723
negnine .fill #-57
negdot2 .fill #-46
sent1   .fill x4000
saver1  .blkw 1
saver2  .blkw 1
saver3  .blkw 1
        .end
        
        .orig x3000
;-------------read part---------------------
;use r1 to count if we have end a whole line
;use r2 to test if the char is dot or not
;use r3 as dustbin
;use r4 to store the index of the last not dot char
;use r5 to store the char of the body
        ld r7,senti
        and r1,r1,#0
        ld r2,negdot
read    getc
        out
        add r3,r2,r0
        brz skip1
        add r4,r1,#0
        add r5,r0,#0
skip1   add r1,r1,#1
        ld r3,negtwe
        add r3,r3,r1
        brn read
        add r4,r4,#-2
        and r7,r7,#0
        and r0,r0,#0
        add r0,r0,#10
        out
;-----------read part over-----------
;now we have the body char in r5
;now we have the index of the first not-dot char in r4
;what we need to do now is to wait for a while and print the next line
;if there is an interrupt during the wait procedure then it will change r4 or r5 
;what we need to after the interrupt is the same as what we do without it
;we use r1 to count the chars we have output and check if it is equal to r4 every iteration
;if it is equal to r4 , we just output the letter three times

loop1   and r1,r1,#0
loop2   add r1,r1,#-1
        brnp loop2
        ldi r2,sent2
        brp skip2
        add r4,r4,#-1
        brzp skip2
        and r4,r4,#0
skip2   and r1,r1,#0
        sti r1,sent2
begin   ld r0,dot
        not r2,r4
        add r2,r2,#1
        add r2,r2,r1
        brnp skip3
        add r0,r5,#0
        out
        out
        out
        brnzp skip4
skip3   out
skip4   add r1,r1,#1
        ld r2,negend
        add r2,r2,r1
        brn begin
        ld r0,enter
        out
        brnzp loop1
        halt
senti   .fill #723
enter   .fill #10
negend  .fill #-18
dot     .fill #46
negdot  .fill #-46
negtwe  .fill #-20
sent2    .fill x4000
        .end
        
        .orig x4000
        .fill x0000
        .end
        
        
        
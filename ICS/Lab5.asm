            .orig x3000
;-------------read part ----------------
;here we need to read all the data we need and save them in label storage
;r0 is uesd to contain the input
;r1 is used to store the total number of later input lines
;r2 is used to check if it is enter
;r3 is used to tranfer ascii to binary
;ther is an out after every getc to echo

;---------------get count number--------------
            ld r3,asciitran
            ld r6, bottom
            getc
            out
            add r0,r0,r3
            add r1,r0,#0
            getc
            out
            ld r2,negenter
            add r2,r2,r0
            brz lessten
            add r0,r0,r3
            add r1,r0,#10
            getc
            out
            st r1,number
            
;r0 is used to contain the input
;r1 from former part stores the number of later input lines
;r2 is used to store the 1-digit temporarily
;r3 is used to transfer ascii to binary
;r4 is used to check if it is enter or space
;r5 is used to point at the memory address to store the input
lessten     lea r5,storage
            st r1,number
begin       getc
            out
            add r2,r0,r3
            getc
            out
            ld r4,negspace
            add r4,r4,r0
            brz skip1
            add r0,r0,r3
            add r2,r0,#10
            getc
            out
skip1       str r2,r5,#0
            add r5,r5,#1
            getc
            out
            add r2,r0,r3
            getc
            out
            ld r4,negenter
            add r4,r4,r0
            brz skip2
            add r0,r0,r3
            add r2,r0,#10
            getc
            out
skip2       str r2,r5,#0
            add r5,r5,#1
            add r1,r1,#-1
            brp begin
            
;----------------process part----------------
;r1 is always point to an address in storage,and *r1 is the number we try to store in output memory
;r2 is a pointer to an address in storeout and we try to store *r1 in r2
;r5 store the return of subroutine

;first we move r1,and r2 to the next number we try to store and the next address we try to fill
;then we call find subroutine to see if we can find a solution after storing *r1 in r2
;everytime we call find, find will first check if we have store enough number in the output sequence
;if there is enough numbers, then find will return 1
;if there is not enough numbers, find will first check if *r1 is legal to be stored by checking if there is the same numbe 
;that has been stored already ,if there is then find will return 0
;if there is not then find will store *r1 in r2 and try to add the next number to the sequence by move r2 to next address
;and mov r1 to the number in the next line and try both numbers.

            lea r1,storage
            lea r2,storeout
            and r5,r5,#0
            jsr find    ;try if we first put the left number in the first line
            add r5,r5,#0
            brp skip3
            add r1,r1,#1
            jsr find    ;try if we first put the right numbe in the first line
skip3       ld r1,number    ;if there is only number ,output 1
            add r1,r1,#-1
            brz special
            lea r1,storeout ;out put the output sequence
            ld r2,ascii
            ldr r0,r1,#0
            brz over
again       add r3,r0,#-10
            brn less1   ;here is judging whether it is bigger than 9
            ld r0,one
            out
            add r0,r3,#0
less1       add r0,r0,r2
            out
            ld r0,space 
            out
            add r1,r1,#1
            ldr r0,r1,#0
            brz over
            brnzp again
            
over        halt

special     ld r0,one   
            out 
            halt

find        add r6,r6,#-1
            str r7,r6,#0
            add r6,r6,#-1
            str r0,r6,#0
            add r6,r6,#-1
            str r1,r6,#0
            add r6,r6,#-1
            str r2,r6,#0
            add r6,r6,#-1
            str r3,r6,#0
            add r6,r6,#-1
            str r4,r6,#0
            and r5,r5,#0
            
            ld r3,number    ;check if the numbers in the sequence is enough
            lea r4,storeout
            add r3,r3,r4
            not r3,r3
            add r3,r3,#1
            add r3,r3,r2
            brz ret1
            
            lea r3,storeout ;check if *r1 has been stored before if yes ,ret 0 if not store *r1
try         not r0,r3
            ldr r4,r1,#0                 
            add r0,r0,1
            add r0,r0,r2
            brz canst   ;this means you can store
            ldr r0,r3,#0
            not r0,r0
            add r0,r0,#1
            add r0,r0,r4
            brz ret0
            add r3,r3,#1
            brnzp try
            
canst       str r4,r2,#0    ;store *r1 and try next to store numbers in next line
            add r2,r2,#1
            and r3,r1,#1
            brz add2        ;add1 branch means we can get next line by adding 1 to r1
            add r1,r1,#1
            jsr find
            add r5,r5,#0
            brp ret1
            add r1,r1,#1
            jsr find
            brp ret1
            brnzp ret0
add2        add r1,r1,#2    ;add2 branch means we can get next line by adding 2 to r1
            jsr find
            add r5,r5,#0
            brp ret1
            add r1,r1,#1
            jsr find
            brp ret1
            brnzp ret0
            
ret1        and r5,r5,#0    
            add r5,r5,#1

ret0        ldr r4,r6,#0
            add r6,r6,#1
            ldr r3,r6,#0
            add r6,r6,#1
            ldr r2,r6,#0
            add r6,r6,#1
            ldr r1,r6,#0
            add r6,r6,#1
            ldr r0,r6,#0
            add r6,r6,#1
            ldr r7,r6,#0
            add r6,r6,#1
            ret
            
negenter    .fill #-10
negspace    .fill #-32
ascii       .fill #48
asciitran   .fill #-48
space       .fill #32
bottom      .fill xFE00
one         .fill x31
number      .blkw 1
            .end
            
            
            .orig x3100
storage     .blkw 40
storeout    .blkw 16
            .end
        .orig x3000
        
;read part
;use r1 to store the 2's complement of ASCII of ENTER
;use r3 to point the address we want to store some digit 
;use r4 to store -48,so the r0-48 is the digit to store
;use r5 to count the digit
        ld r1,ENTER
        lea r3,DEC
        ld r4,ASCII
        and r5,r5,0
READ    getc
        add r2,r1,r0
        brz ENDREAD
        add r5,r5,#1
;store digit
        add r0,r0,r4
        str r0,r3,0
        add r3,r3,#1
        brnzp READ
        
;here we want to change the digits we stored to binary
;by taking every digit and multiple it with the weight of it then add all the digits up
;use r1 as the pointer to the digit we are now porcessing
;use r2 as the pointer to the wieght of the digit
;use r4 to store the binary number
;use r5 from former step to mark the number of digits
;use r6 to store the digit get form *r1
;use r7 to store the digit weight get from *r2
ENDREAD lea r1,DEC
        lea r2,DECWEI
;move r2 to the right weight pointer
        add r2,r2,#-1
        add r2,r2,r5
        and r4,r4,#0
NEXTDEC ldr r6,r1,#0
        add r1,r1,#1
        ldr r7,r2,#0
        add r2,r2,#-1
;use multiple to get the binary number
MAKEDEC add r6,r6,#0
        brz HERE
        add r4,r4,r7
        add r6,r6,#-1
        brp MAKEDEC
HERE    add r5,r5,#-1
        brp NEXTDEC
        
;here we need to change the binary to hex and ouput it
;we use sub the number by x1000 x0100 x0010 x0001 ,and the time we can sub is the number we should output
;use r1 to count which digit of hex we are output now,initialize it as 4
;use r2 to point at the weight of each hex digit ,which we need to be subed
;use r3 to store the weight from *r2
;use r0 to count how many times we sub in each digit ,then change r0 to ascii and output it
;use r4 as the binary number from former step
;use r6 as a dustbin where we put the result we dont need for long
;use r7 as temporary storage from different memory like #48 #55 etc.
        and r1,r1,0
        add r1,r1,#4
        lea r2,HEXWEI
AGAIN2  ldr r3,r2,#0
        add,r2,r2,#1
        and r0,r0,#0
;for low three digits we can judge whether it's used up by see if it's negative
;but for the highest digit ,we cant for it's stored as unsigned number
;so if we need to branch here and treat the two cases differently

;if it's highest digit
AGAIN1  add r6,r1,#-4
        brz HIFOUR
;if it's not
        add r6,r4,r3
        brn END1
        add r0,r0,1
        add r4,r4,r3
        brnzp AGAIN1
;for highest digit ,we shall stop if we get 0000 in[15:12],and after the add instruction next line ,it will be 1111 
;that's what we need to check as stopping condition
HIFOUR  add r6,r4,r3
;use xF000 to get the [15:12] digits of the result stored in r6 
        ld r7,HEXWEI
        and r6,r6,r7
;examine if it's xF000, which means we shall stop
        ld r7,HEX1000
        add r6,r6,r7
        brz END1
        add r0,r0,1
        add r4,r4,r3
        brnzp AGAIN1
;output the r0 ,by number or by letter
END1    add r6,r0,#-10
        brn LESS10
        ld r7,D55
        add r0,r0,r7
        out
        add r1,r1,#-1
        brp AGAIN2
        brnzp END2
LESS10  ld r7,D48
        add r0,r0,r7
        out
        add r1,r1,#-1
        brp AGAIN2
        
        
        
END2        halt
DEC     .BLKW #5
DECWEI  .FILL #1
        .FILL #10
        .FILL #100
        .FILL #1000
        .FILL #10000
HEXWEI  .FILL xF000
        .FILL xFF00
        .FILL xFFF0
        .FILL xFFFF
D55     .FILL #55
HEX1000 .FILL x1000
D48     .FILL #48
ENTER   .FILL xFFF6
ASCII   .FILL xFFD0
        .END
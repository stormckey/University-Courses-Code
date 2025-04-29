;the list origin  in x8000
;r4 is the pointer to the left of the list ,which is initialized as x8000,and r4 keeps track of the left end
;r5 is the pointer to the right of the list,which is initialized as x8000,and r5 keeps track of the right end
;r6 is the stack pointer,but we dont need to use it actually
;r3 is th pointer to the oprations we stored ,we store opreations in label oper，x0 is the sentinel

;-----------initial---------------
        .orig x3000
        ld r4,base
        ld r5,base
        ld r6,bottom
        ld r1,negent
        lea r3,oper
        
;----------read-------------------
;once we read a character, we output it as echo
;even when it's ENTER,because this means we need to change to next line to output the result
;but we also need to test ENTER, and end the reading procession after ENTER is found
;additionally,we need to store all the operations in memory
;r3 is used as the pointer to store them
read    getc
        out
        add r2,r1,r0
        brz endre
        str r0,r3,#0
        add r3,r3,1
        brnzp read
endre   add r3,r3,1
        and r0,r0,#0
        str r0,r3,#0
        
;----------output------------------------------------
;here we process the operations we stored in label oper and output the result
;r3 is the pointer to the operations, every time we get one operation from r3 and identify which operation it is
;after indentifying ,we do different processions ,if it's an adding procession, we will read the next oprand in the branch part
;once our precession is over ,we will output the result if needed
        lea r3,oper
output  ldr r0,r3,#0;get the opration
        brz done    ;if it's x00 then wne have finished the processions
        add r3,r3,1
        ld r1,negmin ;to indentify which operaion it is
        add r1,r1,r0
        brz charmi
        ld r1,negadd
        add r1,r1,r0
        brz charad
        ld r1,negle
        add r1,r1,r0
        brz charle
;if it is ]
charri  ldr r0,r5,#0
        brz empty1 ;check if the list is empty and output '_' if it is
        and r2,r2,#0
        str r2,r5,#0
        not r2,r4
        add r2,r2,#1
        add r2,r2,r5
        brz skip1
        add r5,r5,#-1
skip1   out
        brnzp output
empty1  ld r0,xiahua
        out
        brnzp output
;if it is -
charmi  ldr r0,r4,#0 
        brz empty2 ;check if the list is empty and output '_' if it is
        and r2,r2,#0
        str r2,r4,#0
        not r2,r4
        add r2,r2,#1
        add r2,r2,r5
        brz skip2
        add r4,r4,#1
skip2   out
        brnzp output
empty2  ld r0,xiahua
        out
        brnzp output
;if it is +
charad  ldr r0,r3,#0
        add r3,r3,#1
        ldr r1,r4,#0
        brnp add1       ;check if it is empty, which means we dont need to move r4 in order to let r4 keep track of the left end
        str r0,r4,#0
        brnzp output
add1    add r4,r4,#-1
        str r0,r4,#0
        brnzp output
;if it is [
charle  ldr r0,r3,#0
        add r3,r3,#1
        ldr r2,r5,#0
        brnp add2       ;check if it is empty, which means we dont need to mov r5 in order to let r5 keep track of the right end
        str r0,r5,#0
        brnzp output
add2    add r5,r5,#1
        str r0,r5,#0
        brnzp output
done    halt
base    .fill x8000
xiahua  .fill #95
bottom  .fill xFE00
negent  .fill #-10
negmin  .fill #-45
negadd  .fill #-43 
negle   .fill #-91
negri   .fill #-93
oper    .blkw 513
        .end
        ;initialize the memory space for the list
        .orig x7E00
        .blkw 1000
        .end
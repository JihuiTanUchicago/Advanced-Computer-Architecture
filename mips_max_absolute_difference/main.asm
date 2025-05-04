.data
array:  .word  3, -1, 5, 8, 0, -6, 4, 12, -2, 9
length: .word  10
newline: .asciiz "\n"

.text
.globl main

main:
    la   $t0, array
    lw   $t1, length 

    lw   $t2, 0($t0) 
    move $t3, $t2  

    li   $t4, 1   

loop:
    bge  $t4, $t1, compute # if i >= length: break

    sll  $t5, $t4, 2
    add  $t6, $t0, $t5     
    lw   $t7, 0($t6)

    blt  $t7, $t2, set_min
    bgt  $t7, $t3, set_max
    j    inc

set_min:
    move $t2, $t7
    j    inc

set_max:
    move $t3, $t7

inc:
    addi $t4, $t4, 1
    j    loop

compute:
    sub  $t8, $t3, $t2

    li   $v0, 1
    move $a0, $t8
    syscall

    li   $v0, 4
    la   $a0, newline
    syscall

    li   $v0, 10
    syscall
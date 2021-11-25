/*
 * MIT License
 *
 * Copyright (c) 2021: imec-COSIC KU Leuven, 3001 Leuven, Belgium 
 * Author: Michiel Van Beirendonck <michiel.vanbeirendonck@esat.kuleuven.be>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
 
.text
.thumb
.syntax unified

.macro GPIO_ToggleBitsA1 scratch0 scratch1

	ldr \scratch0, =0x40020014 @ GPIOA Output reg
	ldr \scratch1, [\scratch0]
	eor \scratch1, \scratch1, #2 @ Pin 1
	str \scratch1, [\scratch0]

.endm

@ puts a random value on the stack
.macro ClearInit scratch0

    ldr \scratch0, =randclear
    ldr \scratch0, [\scratch0]
    push {\scratch0}

.endm

@ discards the random value from the stack
.macro ClearDeInit

    add sp, sp, #4

.endm

@ expects a random value on the stack
.macro ClearLoad scratch0

    ldr \scratch0, [sp]

.endm

@ expects a random value on the stack
.macro ClearStore scratch0

    ldr \scratch0, [sp]
    str \scratch0, [sp, #-4]

.endm

@ expects a random value on the stack
.macro ClearALUShift scratch0 scratch1

    ldr \scratch0, [sp]
    eor \scratch1, \scratch0, \scratch0, ror #16

.endm

@ expects a random value on the stack
.macro ClearFULLALUShift scratch0 scratch1 scratch2 scratch3

    ldr \scratch0, [sp]
    bic \scratch1, \scratch0, \scratch0, ror #16
    and \scratch2, \scratch0, \scratch0, ror #16
    eor \scratch3, \scratch0, \scratch0, ror #16

.endm

@ expects a random value on the stack
.macro ClearAll scratch0 scratch1
    
    ldr \scratch0, [sp]
    and \scratch1, \scratch0, \scratch0, ror #16
    str \scratch0, [sp, #-4]

.endm

.macro ClearAllEor scratch0 scratch1
    
    ldr \scratch0, [sp]
    eor \scratch1, \scratch0, \scratch0, ror #16
    str \scratch0, [sp, #-4]

.endm

@ input : size_t len
.global ClearStack
.func ClearStack, ClearStack
.type ClearStack, %function
ClearStack:
    push {r4, lr}
    mov r2, r0
    mov r4, r0
    sub sp, sp, r4
    mov r0, sp
    ldr r1, =randclear
    ldr r1, [r1]
    bl memset
    add sp, sp, r4
    pop {r4, pc}

.endfunc

.macro SecAnd x0, y0, x1, y1

    ClearAll r0, r0

    bl random_uint32
    and r1, \x0, \y1 
    eor r1, r1, r0
    and \x0, \x0, \y0
    eor \x0, r0, \x0
    and r0, \x1, \y0
    eor r1, r0, r1
    and \x1, \x1, \y1
    eor \x1, \x1, r1

    ClearAll r0, r0

.endm

.macro SecAndBitSub x0, y0, x1, y1

    ClearAll r0, z1

    bl random_uint32
    and r1, \x0, \y1 
    eor r1, r1, r0
    and \x0, \x0, \y0
    eor \x0, r0, \x0
    and r0, \x1, \y0
    eor r1, r0, r1
    and \x1, \x1, \y1
    eor \x1, \x1, r1

    ClearAll \y0, \y1

.endm

.global A2A_C_A
.func A2A_C_A, A2A_C_A
.type A2A_C_A, %function
C_A .req r0
Aptr .req r1
Rptr .req r2
R .req r3
rrr .req r3
A .req r4
R_l .req r5
GAMMA .req r5
A_l .req r6
mod .req r7
n .req lr
A2A_C_A:

    push {r4 - r7, lr}

    ldrh A, [Aptr] 
    ldr GAMMA, [sp, #20]
    ldr mod, [sp, #24]
    ldr n, [sp, #28]

    ClearInit A_l

    sub A, A, rrr
    and A, A, mod
    sub A, A, GAMMA
    and A, A, mod

    ClearALUShift R, R

    ldrh R, [Rptr] 
    and R, R, mod

.A2A_C_A_L0:
    and R_l, R, #7
    lsr R, R, #3 
    add A, A, R_l 
    and A, A, mod
    and A_l, A, #7 
    ldrh A_l, [C_A, A_l, lsl #1] 
    lsr mod, mod, #3
    lsr A, A, #3
    add A, A_l, A
    and A, A, mod
    ClearLoad A_l
    subs n, n, #1
    bne .A2A_C_A_L0

    strh R, [Rptr]
    ClearStore R
    strh A, [Aptr]

    ClearDeInit

    pop {r4 - r7, pc}

.unreq C_A 
.unreq Aptr 
.unreq Rptr 
.unreq A 
.unreq R 
.unreq rrr 
.unreq R_l 
.unreq GAMMA 
.unreq A_l
.unreq mod
.unreq n
.endfunc

.global SecBitAddBitSubConsAdd
.func SecBitAddBitSubConsAdd, SecBitAddBitSubConsAdd
.type SecBitAddBitSubConsAdd, %function
@ .req r0, scratch reg in SecAnd
@ .req r1, scratch reg in SecAnd
z0 .req r2
z1 .req r3
t0 .req r4
u0 .req r4
w0 .req r5
t1 .req r6
u1 .req r6
w1 .req r7
x .req r8
j .req r8
y .req r9
z .req r10
l .req r11
SecBitAddBitSubConsAdd:

    push {r4 - r11, lr}

    mov x, r0
    mov y, r1
    mov z, r2  

    ClearInit r0

@SecBitAdd:

    @ j = 1
    ldr t0, [x, #0]
    ldr w0, [x, #4]
    eor z0, t0, w0
    str z0, [z, #0]

    ClearAll r0, z0
    
    ldr t1, [x, #16]
    ldr w1, [x, #20]
    eor z1, t1, w1
    str z1, [z, #16] 

    SecAnd w0, t0, w1, t1

    @ j = 2
    str w1, [z, #20]
    ldr t1, [z, #16] 
    ldr w1, [x, #24]
    eor z1, t1, w1
    str z1, [z, #16]

    ClearAll r0, z1

    str w0, [z, #4]
    ldr t0, [z, #0]
    ldr w0, [x, #8]
    eor z0, t0, w0
    str z0, [z, #0]
    
    SecAnd w0, t0, w1, t1

    @ j = 3
    ldr z0, [z, #4]
    eor w0, z0, w0
    str w0, [z, #4]
    ldr t0, [z, #0]
    ldr w0, [x, #12]
    eor z0, t0, w0
    str z0, [z, #0]

    ClearAll r0, z0

    ldr z1, [z, #20]
    eor w1, z1, w1
    str w1, [z, #20]
    ldr t1, [z, #16]
    ldr w1, [x, #28]
    eor z1, t1, w1
    str z1, [z, #16]

    SecAnd w0, t0, w1, t1

    ldr t1, [z, #20]
    eor z1, t1, w1
    str z1, [z, #20]

    ClearAll r0, z1

    ldr t0, [z, #4]
    eor z0, t0, w0
    str z0, [z, #4]

    SecAnd w0, t0, w1, t1

    str w0, [z, #8]
    mov z0, #0
    str z0, [z, #12]

    ClearAll r0, w0

    str w1, [z, #24]
    str z0, [z, #28] 

@SecBitSub:

    add j, y, #12
    add l, z, #12
    sub y, y, #4
    sub z, z, #4

.SecBitSubL1: 

    ldr w0, [y, #4]!
    ClearLoad r0 
    ldr w1, [y, #16] 

.SecBitSubL2: 

    ldr u0, [z, #4] 
    eor z0, u0, w0
    str z0, [z, #4]!
    
    ClearAllEor r0, z0 

    ldr u1, [z, #16] 
    eor z1, u1, w1
    str z1, [z, #16]
    mvn u1, u1

    SecAndBitSub w0, u0, w1, u1

    cmp z, l

    bne .SecBitSubL2

    sub z, z, #16
    cmp y, j

    bne .SecBitSubL1

@SecConsAdd: 

    ldr z0, [z, #16]
    ldr w0, [z, #12]
    eor z0, z0, w0
    mvn w0, w0
    str z0, [z, #16]
    str w0, [z, #12]

    ClearAll r0, z0

    ldr z1, [z, #32]
    ldr w1, [z, #28]
    eor z1, z1, w1
    str z1, [z, #32]

    ClearDeInit

    pop {r4 - r11, pc}

.unreq z0
.unreq z1
.unreq t0 
.unreq u0 
.unreq w0 
.unreq t1 
.unreq u1 
.unreq w1
.unreq x 
.unreq j 
.unreq y 
.unreq z 
.unreq l
.endfunc


.global B2A_Goubin
.func B2A_Goubin, B2A_Goubin
.type B2A_Goubin, %function
y .req r0
Bb .req r1
R .req r2
T .req r3
A .req lr
BPtr .req r4
RPtr .req r5
B2A_Goubin:

    push {r4 - r5, lr}

    mov BPtr, r0
    mov RPtr, r1

    ClearInit r0

    bl random_uint32

    ldr Bb, [BPtr, #0]

    usub16 T, Bb, y
    eor Bb, Bb, y
    eor T, T, Bb

    ClearLoad A

    ldr R, [RPtr, #0]

    eor A, Bb, R
    usub16 A, A, R
    and A, A, #536813567
    eor A, A, T

    str A, [BPtr, #0]

    eor R, R, y
    mov A, #262148
    usub16 R, R, A
    and R, R, #536813567

    ClearStore y

    str R, [RPtr, #0]

    ClearDeInit

    pop {r4 - r5, pc}

.unreq y 
.unreq Bb 
.unreq R 
.unreq T 
.unreq A 
.unreq BPtr 
.unreq RPtr 
.endfunc

.macro chi i

    ldr r6, [bc1Ptr, #(((4*\i)+16)%40)] @a_(i+2)
    ldr r5, [bc1Ptr, #(((4*\i)+8)%40)] @a_(i+1) 
    ldr r4, [bc1Ptr, #(((4*\i)))] @a_i
    bic r6, r6, r5
    eor r6, r6, r4
    ldr r4, [bc2Ptr, #(((4*\i)+16)%40)] @b_(i+2)
    and r5, r5, r4
    eor r6, r6, r5
    str r6, [st1Ptr, #(((4*\i)))] @a_i

    @ Clear ALU, r8 has random clear value
    bic r5, r8, r8, ror #16
    and r6, r8, r8, ror #16
    eor r5, r8, r8, ror #16
    str r5, [sp, #-4]

    @ldr r4, [bc2Ptr, #(((4*\i)+16)%40)] @b_(i+2) (already in there)
    ldr r6, [bc2Ptr, #(((4*\i)+8)%40)] @b_(i+1)
    ldr r5, [bc2Ptr, #(((4*\i)))] @b_i
    bic r4, r4, r6
    eor r4, r4, r5
    ldr r5, [bc1Ptr, #(((4*\i)+16)%40)] @a_(i+2)
    and r6, r6, r5
    eor r4, r4, r6
    str r4, [st2Ptr, #(((4*\i)))] @b_i

.endm 

.global sha3_chi_masked_nonlinear
.func sha3_chi_masked_nonlinear, sha3_chi_masked_nonlinear
.type sha3_chi_masked_nonlinear, %function
st1Ptr .req r0
st2Ptr .req r1
bc1Ptr .req r2
bc2Ptr .req r3
sha3_chi_masked_nonlinear:

    push {r4 - r8}

    ldr r7, =randclear @ random mask to clear ALU
    ldr r8, [r7]

    chi 0
    chi 1
    chi 2
    chi 3
    chi 4
    chi 5
    chi 6
    chi 7
    chi 8
    chi 9

    pop {r4 - r8}
    bx lr

.unreq st1Ptr
.unreq st2Ptr
.unreq bc1Ptr
.unreq bc2Ptr
.endfunc


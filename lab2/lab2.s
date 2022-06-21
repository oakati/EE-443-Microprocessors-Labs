/* Author: Omer A. Kati*/
.text
.equ LEDs, 0xFF200000
.global _start
_start:
 /* PUT YOUR CODE HERE*/
 /* Before stop turn on all LEDs to show that sorting is finished*/

LDR R0, =LEDs //address of the leds
LDR R7, =0x00000000
STR R7, [R0]

AGAIN:
LDR R1, DataNum // no of elements
LDR R2, =SortData // address of 1st element
ADD R3, R2, #4 // address of 2nd element


MYLOOP:
	LDR R4, [R2] // value of the 1st element
	LDR R5, [R3] // value of the 2nd element
	CMP R4, R5
	BGT GREATER
	ADD R2, R3, #0
	ADD R3, R2, #4
	SUB R1, #1
	CMP R1, #1
	BEQ FINISHED
	
B MYLOOP

GREATER:
	STR R4, [R3]
	STR R5, [R2]
B AGAIN

FINISHED:
	LDR R7, =0x000003ff
	STR R7, [R0]
	
 STOP: B STOP /*Branch unconditionally to STOP*/
DataNum: .word 10 /* Specify the number of data items. */
SortData: .word 0x7FFFFFF1, 0x7FFFFFFF, 0x00000000, 0x80000000, 0x80000001, 0xA1234332, 0x20141020, 0x6FFFFFFF, 0xA1234320, 0x20141F20 /* These are the data
items you need to sort 
/*SortData: .word 0x881517ed,0x2605845c,0xcfab2a6c,0xb3e0b201,0xdf629aad,0x37d6613a,0x5d27f9d2,0x5db77e06,0x29102850,0x3703e268,0xcfec1166,0xc2e1e44f,0xdb0153b9,0x06fbd6fe,0xa0bb35a4,0xd2d82ce3*/

.end

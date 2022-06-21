/* Author: Omer A. Kati*/

.text
.equ LEDs, 0xFF200000
.equ STACK_TOP, 0x20000800
.global _start
_start: 
LDR R13, =STACK_TOP 
LDR R1, DataNum1
LDR R2, =SortData1
BL SORTING
LDR R1, DataNum2
LDR R2, =SortData2
BL SORTING
LDR R1, DataNum3
LDR R2, =SortData3
BL SORTING

BL LEDFUNCTION

STOP: B STOP

LEDFUNCTION:
	STMFD SP!,{R0-R12}
	LDR R0, =LEDs
	LDR R1, =0x00000001
	STR R1, [R0]
	
	HERE1:
	BL DELAY
	MOV R1, R1, LSL #1
	STR R1, [R0]
	CMP R1, #128
	BEQ HERE2
	B HERE1
	
	HERE2:
	BL DELAY
	MOV R1, R1, LSR #1
	STR R1, [R0]
	CMP R1, #1
	BEQ HERE1
	B HERE2


SORTING:
	STMFD SP!, {R0-R12}
	LDR R0, =LEDs //address of the leds
	LDR R7, =0x00000000
	STR R7, [R0]
	
AGAIN:
	LDMFD SP!, {R0-R12}
	STMFD SP!, {R0-R12}
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
	LDMFD SP!, {R0-R12}
	MOV PC, LR
/* Sorting Subroutine */
/* PUT YOUR CODE HERE FOR SORTING ALGORITHM*/
/* DELAY SUBROUTINE Delay= R0 times ??? seconds */
/* Experiment with delay values */
/* Start with a value of R0 =0x200000 */
DELAY: 
	STMFD SP!,{R0-R12,LR}
	LDR R0, =0x00061A80
dowaitloop:
	SUBS R0,R0,#1
	BNE dowaitloop
	LDMFD SP!,{R0-R12,PC} /*Return from DELAY subroutine*/

/* Data used in the program */
.word STACK_TOP, LEDs

DataNum1: .word 10 /* Specify the number of data items. */
SortData1: .word 0x7FFFFFF1, 0x7FFFFFFF, 0x00000000, 0x80000000, 0x80000001, 0xA1234332, 0x20141020, 0x6FFFFFFF, 0xA1234320, 0x20141F20
/* These are the data items you need to sort */

DataNum2: .word 6 /* Specify the number of data items. */
SortData2: .word -1, 1023, -3029, 10023, -17, 2018
/* These are the data items you need to sort */

DataNum3: .word 4 /* Specify the number of data items. */
SortData3: .word 102301, -77, 1923,-23
/* These are the data items you need to sort */

.end

/*notes*/
At pc=00000088: Function clobbered register(s): r4 r5 r7Details...

Simulator requested a breakpoint.

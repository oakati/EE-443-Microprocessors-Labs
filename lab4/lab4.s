/* Author: Omer A. Kati*/

/*********************************************/
/* Initialize the exception vector table */
/*********************************************/
.section .vectors, "ax"
LDR PC, =_start /* Go to the beginning of the MAIN program. */
LDR PC, =SERVICE_UND /* Unimplemented instruction. */
LDR PC, =SERVICE_SVC /* Software interrupt. */
LDR PC, =SERVICE_ABT_DATA /* Failed data access. */
LDR PC, =SERVICE_ABT_INT /* Failed instruction access. */
.word 0 /* Null entry for address 0x014. */
LDR PC, =SERVICE_IRQ /* Hardware IRQ interrupt. */
LDR PC, =SERVICE_FIQ /* Hardware FIQ interrupt. */
.text
.equ LEDs, 0xFF200000
.equ SEV_SEGMENT1, 0xFF200020
.equ timer_status, 0xFF202000
.equ timer_control, 0xFF202004
.equ timer_startlow, 0xFF202008
.equ timer_starthigh, 0xFF20200C
.equ timer_snaplow, 0xFF202010
.equ timer_snaphigh, 0xFF202014
.equ STACK_TOP, 0x20000800
.equ starthigh, 0x05F5
.equ startlow, 0xE100

.global _start
_start:
 LDR R13, =STACK_TOP 
 MOV R1, #0b11010010
 MSR CPSR_c, R1 // change to IRQ mode with interrupts disabled
 LDR SP, =0xFFFFFFFC // set IRQ stack to top of A9 on-chip memory
 MOV R1, #0b11010011
 MSR CPSR_c, R1 // change to SVC mode with interrupts disabled
 LDR SP, =0x3FFFFFFC // set SVC stack to top of DDR3 memory
 /* enable IRQ interrupts in the processor */
 MOV R1, #0b01010011 // IRQ unmasked, MODE = SVC
 MSR CPSR_c, R1
 /* Configure the Generic Interrupt Controller (GIC) */
CONFIG_GIC:
/* configure the FPGA interval timer and KEYs interrupts */
LDR R0, =0xFFFED848 // ICDIPTRn: processor targets register
LDR R1, =0x00000101 // set targets to cpu0
STR R1, [R0]
LDR R0, =0xFFFED108 // ICDISERn: set enable register
LDR R1, =0x00000300 // set interrupt enable
STR R1, [R0]
/* configure the GIC CPU interface */
LDR R0, =0xFFFEC100 // base address of CPU interface
/* Set Interrupt Priority Mask Register (ICCPMR) */
LDR R1, =0xFFFF // enable interrupts of all priorities levels
STR R1, [R0, #0x04] // ICCPMR
/* Set the enable bit in the CPU Interface Control Register (ICCICR). This bit allows
* interrupts to be forwarded to the CPU(s) */
MOV R1, #1
STR R1, [R0, #0x00] // ICCICR
/* Set the enable bit in the Distributor Control Register (ICDDCR). This bit allows
* the distributor to forward interrupts to the CPU interface(s) */
LDR R0, =0xFFFED000
STR R1, [R0, #0x00] // ICDDCR
/* Make your other initializations here. Such as resetting the two digit 7-segments*/
LDR R0, =0xFF200020
MOV R1, #0
STR R1, [R0, #0x00]

LDR R0, =0xFF200030
MOV R1, #0
STR R1, [R0, #0x00]
/* Reset counter value and start interval timer and its interrupt bit*/
MOV R8, #0

LDR R0, =0xFF20200C
LDR R1, =starthigh
STRH R1, [R0]
LDR R1, =startlow
STRH R1, [R0,#-4]
LDRH R1, =0x0007
STRH R1, [R0,#-8]

BL LEDFUNCTION

/* Put your walking turned-on LED code here */
LEDFUNCTION:
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
	
DELAY: 
	STMFD SP!,{R0,R1}
	LDR R0, =0x030D4000
	/*LDR R0, =0x0000030D4*/
dowaitloop:
	SUBS R0,R0,#1
	BNE dowaitloop
	LDMFD SP!,{R0,R1}
	MOV PC, LR
STOP: B STOP
/*****************************************************************************
* Define the IRQ exception handler
******************************************************************************/
SERVICE_IRQ:
PUSH {R0-R7, LR} // save registers
/* get the interrupt ID from the GIC */
LDR R4, =0xFFFEC100 // GIC CPU interface base address
LDR R5, [R4, #0x0C] // read the ICCIAR
HPS_TIMER_CHECK:
CMP R5, #199 // check for HPS timer interrupt
BNE INTERVAL_TIMER_CHECK
B EXIT_IRQ
INTERVAL_TIMER_CHECK:
CMP R5, #72 // check for FPGA timer interrupt
BNE KEYS_CHECK
BL TIMER_ISR
B EXIT_IRQ
TIMER_ISR:
LDR R0, =0xFF202000
MOV R1, #0x00
STRB R1, [R0]
ADD R8, #1
MOV PC, LR
KEYS_CHECK:
CMP R5, #73 // check for KEYs interrupt
UNEXPECTED:
BNE UNEXPECTED // if not recognized, stop here
EXIT_IRQ:
LDR R0, =0xFF200020

STH:
CMP R8, #0
BNE QWE
CMP R9, #10
MOVEQ R9, #0
QWE:

LDRB R1, =0b00111111
CMP R8, #0
STREQB R1, [R0, #0]
CMP R9, #0
STREQB R1, [R0, #1]

LDRB R1, =0b00000110
CMP R8, #1
STREQB R1, [R0, #0]
CMP R9, #1
STREQB R1, [R0, #1]

LDRB R1, =0b01011011
CMP R8, #2
STREQB R1, [R0, #0]
CMP R9, #2
STREQB R1, [R0, #1]

LDRB R1, =0b01001111
CMP R8, #3
STREQB R1, [R0, #0]
CMP R9, #3
STREQB R1, [R0, #1]

LDRB R1, =0b01100110
CMP R8, #4
STREQB R1, [R0, #0]
CMP R9, #4
STREQB R1, [R0, #1]

LDRB R1, =0b01101101
CMP R8, #5
STREQB R1, [R0, #0]
CMP R9, #5
STREQB R1, [R0, #1]

LDRB R1, =0b01111101
CMP R8, #6
STREQB R1, [R0, #0]
CMP R9, #6
STREQB R1, [R0, #1]

LDRB R1, =0b00000111
CMP R8, #7
STREQB R1, [R0, #0]
CMP R9, #7
STREQB R1, [R0, #1]

LDRB R1, =0b01111111
CMP R8, #8
STREQB R1, [R0, #0]
CMP R9, #8
STREQB R1, [R0, #1]

LDRB R1, =0b01101111
CMP R8, #9
STREQB R1, [R0, #0]
CMP R9, #9
STREQB R1, [R0, #1]

LDRB R1, =0b00111111
CMP R8, #10
SUBEQ R8, #10
ADDEQ R9, #1
BEQ STH

STR R5, [R4, #0x10] // Write to end-of-interrupt register (ICCEOIR)
POP {R0-R7, LR}
SUBS PC, LR, #4
SERVICE_FIQ:
SUBS PC, LR, #4 
SERVICE_SVC:
MOVS PC, LR 
SERVICE_UND:
MOVS PC, LR 
SERVICE_ABT_DATA:
SUBS PC, LR, #8 
SERVICE_ABT_INT:
SUBS PC, LR, #4 
.end

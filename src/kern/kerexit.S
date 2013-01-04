	.file	"kerexit.c"
	.text

	.align	2
	.global	kerentirq
	.type	kerentirq, %function
kerentirq:

	/*switch to system state*/
	msr  	cpsr, #0xdf

	/*push registers of active task onto stack*/
	stmfd	sp!, {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	
	/*store sp of active task in scratch register*/
	mov	r1, sp

	/*return to irq state*/
	msr  	cpsr, #0xd2

	/*grab link register (where the interrupt happened) and the spsr*/
	sub	r3, lr, #4
	mrs	r2, spsr
	
	/*go into to svc state*/
	msr  	cpsr, #0xd3

	/*load TD pointer from stack */
	ldr	r0, [sp, #4]

	/*store sp, spsr, pc of active task in TD*/
	str	r3, [r0, #8]
	str	r2, [r0, #4]
	str	r1, [r0, #0]

	add	sp, sp, #12
	
	/*This is a hack*/
	/*We get a pointer to .irqstruct by taking the pc. 
	It's set to current instruction + 8 so it works.
	Don't ask questions, don't change anything in the 
	next three lines unless you know what you're doing*/
	mov	r0, pc

	ldmfd	sp, {r4, r5, r6, r7, r8, r9, r10, fp, sp, pc}

.irqstruct: /*magic*/
	.word	7 /*very magic - don't change*/
	.word	0
.irqstructptr:
	.word	.irqstruct(GOTOFF)
	.size	kerentirq, .-kerentirq

	.align	2
	.global	kerent
	.type	kerent, %function
kerent:
	/*store request struct pointer on stack from temp register*/
	str	r0, [sp, #0]

	/*switch to system state*/
	msr  	cpsr, #0xdf

	/*push registers of active task onto stack*/
	stmfd	sp!, {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	
	/*store sp of active task in scratch register*/
	mov	r1, sp
	
	/*return to svc state*/
	msr  	cpsr, #0xd3

	/*load TD pointer from stack */
	ldr	r0, [sp, #4]

	/*store sp, spsr, pc of active task in TD*/
	str	lr, [r0, #8]
	mrs	r2, spsr
	str	r2, [r0, #4]
	str	r1, [r0, #0]

	/*point a0 to req struct*/
	ldr	r0, [sp, #0]

	add	sp, sp, #12

	ldmfd	sp, {r4, r5, r6, r7, r8, r9, r10, fp, sp, pc}
	.size	kerent, .-kerent


	.align	2
	.global	kerexit
	.type	kerexit, %function
kerexit:
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 1, uses_anonymous_args = 0

	/*save registers*/
	mov	ip, sp
	stmfd	sp!, {r4, r5, r6, r7, r8, r9, r10, fp, ip, lr, pc}

	/*extra memory space on stack*/
	sub	sp, sp, #12



	/*get user stack location in r1*/
	ldr	r1, [r0, #0]	
	/*install spsr of active task from TD*/
	ldr	r2, [r0, #4]
	msr	spsr, r2
	/*install the location to jump to into our link register from the stack*/
	ldr	lr, [r0, #8]

	/*store TD pointer on stack*/
	str	r0, [sp, #4]

	/*switch to system state*/
	msr  	cpsr, #0xdf

	/*get sp from the TD in r0*/
	mov	sp, r1

	/*pop registers of the active task from its stack*/
	ldmfd	sp!, {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}

	/*return to svc state*/
	msr  	cpsr, #0xd3
	
	
	/*install pc of active task and move spsr to cpsr using movs*/
	movs	pc, lr

	.size	kerexit, .-kerexit
	.ident	"GCC: (GNU) 4.0.2"

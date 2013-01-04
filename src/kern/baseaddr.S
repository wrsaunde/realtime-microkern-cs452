	.file	"baseaddr.c"
	.text
	.align	2
	.global	baseaddr
	.type	baseaddr, %function
baseaddr:
	@ args = 0, pretend = 0, frame = 4
	@ frame_needed = 1, uses_anonymous_args = 0
	mov	ip, sp
	stmfd	sp!, {sl, fp, ip, lr, pc}
	sub	fp, ip, #4
	sub	sp, sp, #4
	ldr	sl, .L4
.L3:
	add	sl, pc, sl
	sub	r3, pc, #32
	str	r3, [fp, #-20]
	ldr	r3, .L4+4
	ldr	r3, [sl, r3]
	mov	r2, r3
	ldr	r3, [fp, #-20]
	rsb	r3, r2, r3
	str	r3, [fp, #-20]
	ldr	r3, [fp, #-20]
	mov	r0, r3
	ldmfd	sp, {r3, sl, fp, sp, pc}
.L5:
	.align	2
.L4:
	.word	_GLOBAL_OFFSET_TABLE_-(.L3+8)
	.word	baseaddr(GOT)
	.size	baseaddr, .-baseaddr
	.ident	"GCC: (GNU) 4.0.2"

	.file	"syscall_entry.c"
	.text
	.align	2
	.global	syscall_entry
	.type	syscall_entry, %function
syscall_entry:
	@ args = 0, pretend = 0, frame = 4
	@ frame_needed = 1, uses_anonymous_args = 0
	mov	ip, sp
	stmfd	sp!, {fp, ip, lr, pc}
	sub	fp, ip, #4
	sub	sp, sp, #4
	str	r0, [fp, #-16]
	swi	0
	ldmfd	sp, {r3, fp, sp, pc}
	.size	syscall_entry, .-syscall_entry
	.ident	"GCC: (GNU) 4.0.2"

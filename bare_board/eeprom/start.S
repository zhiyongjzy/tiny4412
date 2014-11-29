.text
.global _start
_start:
	bl bss_clear
	bl init_call

bss_clear:
	push {lr}
	ldr r0, = __bss_start__
	ldr r1, = __bss_end__
	mov r2, #0
1:
	cmp r0, r1
	popeq {pc}
	str r2, [r0]
	add r0, #4
	b 1b

	pop {pc}

init_call:
	push {lr}
	ldr r4, =__initcall_start__
	ldr r5, =__initcall_end__
1:
	cmp r4, r5
	popeq {pc}
	ldr r0, [r4]
	mov lr, pc
	mov pc, r0
	add r4, #4
	b 1b

	pop {pc}

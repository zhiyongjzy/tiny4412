.text
.global _start
_start:
	//异常向量表
	b reset
	ldr pc, _undefined
	ldr pc, _do_swi
	ldr pc, _prefetch_abort
	ldr pc, _data_abort
	nop
	ldr pc, _do_irq
	ldr pc, _do_fiq

_undefined:
_prefetch_abort:
_data_abort:
_do_fiq:
	.word loop

_do_swi:
	.word do_swi
_do_irq:
	.word do_irq

vector_end:

reset:
	bl bss_clear
	bl init_call
	bl uboot_system_clock_init
	bl mem_ctrl_asm_init
	
	bl system_clock_init
	ldr sp, =0x45000000  //设置栈
	//打开mmu
	bl create_section_table
	bl mmu_on

	bl set_cpsr_int_on

	//复制异常向量表
	ldr r0, =0xffff0000
	ldr r1, =_start
	ldr r2, =(vector_end - _start)
	bl memcpy
	
	//设置系统的异常向量表在0xffff0000
	mrc p15,0,r0,c1,c0,0
	orr r0,r0,#(1 << 13)
	mcr p15,0,r0,c1,c0,0



	bl main


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

.global mmu_off
mmu_off:
	push {lr}
	MRC p15, 0, r0, c1, c0, 0
	bic r0, #1
	MCR p15, 0, r0, c1, c0, 0
	pop {pc}

.global mmu_on
mmu_on:
	push {lr}
	MRC p15, 0, r0, c1, c0, 0
	orr r0, #1
	MCR p15, 0, r0, c1, c0, 0
	pop {pc}

.global set_domain_all_manager
set_domain_all_manager:
	push {lr}
	mvn r0, #0
	MCR p15, 0, r0, c3, c0, 0
	pop {pc}

.global set_ttb   //设置mmu页表首地址
set_ttb:
	push {lr}
	MCR p15, 0, r0, c2, c0, 0
	pop {pc}

.global set_cpsr_int_on
set_cpsr_int_on:
	//打开cpsr中的中断开关
	mrs r0, cpsr
	bic r0, #(1 << 7)
	msr cpsr, r0

	
do_swi:
	push {lr}

	//获取swi指令的内容，提取低24位作为do_swi的参数
	sub r0, lr, #4
	ldr r0, [r0]
	bic r0, #(0xff << 24)

	mov lr, pc
	ldr pc, =swi_handler

	pop {pc}


do_irq:
	//各模式下，栈指针都lr都是独立的，所以需要重新设置栈指针
	ldr sp, =0x55000000

	//修正lr寄存器,根据ARMv7手册P1152
	sub lr, #4

	//中断是突然打断原有程序的，所以原油程序的工作状态需要保护
	//只需要保护r-r3。因为按照ATPCS规范，函数在调用完毕后，r4-r12应该保持不变，不需要压栈
	push {r0-r3, lr}

	mov lr, pc
	ldr pc, =irq_handler

	//原模式的cpsr现在自动保存在irq下的spsr中，现在把它还原。尖角号表示spsr.irq还原到当前模式的cpsr中
	ldmfd sp!, {r0-r3, pc}^

.global send_swi_instruction
send_swi_instruction:
	push {lr}
	swi #22
	pop {pc}

loop:
	b loop


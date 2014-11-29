.text
.global _start
_start:
	ldr sp,=0x02027800
	bl main


#include "config.h"

#define TTB 0x46000000 //页表首地址

enum {
	CACHEABLE	= 1,
	UNCACHEABLE	= 0,
};

#define SECTION_FLAG 0b10
#define TEX_C_B(tex, c, b) ( ((tex) & 7) << 12 | (c) << 3 | (b) << 2 )

//填充一个页表项
static void create_section_desc(u32 pa, u32 mva, u32 cache_flag)
{
	u32 desc = 0; //页表项

	//创建页表项的内容
	if (cache_flag) {
		desc = (pa & ~0xfffff) | TEX_C_B(0, 1, 1) | SECTION_FLAG;
	} else {
		desc = (pa & ~0xfffff) | TEX_C_B(0, 0, 0) | SECTION_FLAG;
	}

	//填入页表
	*(volatile u32*)((TTB & ~0x3fff) | (mva >> 20 << 2)) = desc;
}

void create_section_table()
{
	//映射内存
	size_t i = 0;  
	for (i = 0x40000000; i < 0x60000000; i += 1024 * 1024) {
		create_section_desc(i, i, CACHEABLE);
		//create_section_desc(i, i, UNCACHEABLE);
	}

	//映射SFR
	for (i = 0x00000000; i < 0x40000000; i += 1024 * 1024) {
		create_section_desc(i, i, UNCACHEABLE);
	}

	//异常向量表存放位置
	create_section_desc(0x50000000, 0xffff0000, UNCACHEABLE);

	set_domain_all_manager();
	set_ttb(TTB);
}

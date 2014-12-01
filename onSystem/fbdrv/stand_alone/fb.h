#pragma once

#include <mach/regs-clock.h>
//#include <mach/regs-gpio.h> //S5P_VA_GPIO1　//fixed

#define CLK_GATE_IP_LCD0 (*((volatile u32 *)EXYNOS4_CLKGATE_IP_LCD0))

#define GPIO_BASE S5P_VA_GPIO1
#define GPF0CON  (*(volatile u32*)(GPIO_BASE + 0x0180))
#define GPF1CON  (*(volatile u32*)(GPIO_BASE + 0x01a0))
#define GPF2CON  (*(volatile u32*)(GPIO_BASE + 0x01c0))
#define GPF3CON  (*(volatile u32*)(GPIO_BASE + 0x01e0))


#define GPF0DRV  (*(volatile u32*)(GPIO_BASE + 0x018c))
#define GPF1DRV  (*(volatile u32*)(GPIO_BASE + 0x01ac))
#define GPF2DRV  (*(volatile u32*)(GPIO_BASE + 0x01cc))
#define GPF3DRV  (*(volatile u32*)(GPIO_BASE + 0x01ec))

#define GPF0PUD  (*(volatile u32*)(GPIO_BASE + 0x0188))
#define GPF1PUD  (*(volatile u32*)(GPIO_BASE + 0x01a8))
#define GPF2PUD  (*(volatile u32*)(GPIO_BASE + 0x01c8))
#define GPF3PUD  (*(volatile u32*)(GPIO_BASE + 0x01e8))

#define CLK_BASE fb_pri->clk_reg
#define CLK_SRC_LCD0  	(*(volatile u32*)(fb_pri->clk_reg + 0xc234))
#define CLK_DIV_LCD    	(*(volatile u32*)(fb_pri->clk_reg + 0xc534))

#define LCDBLK_CFG     	(*(volatile u32*)(fb_pri->lcdblkcfg_reg))



#define FIMD_BASE fb_pri->fb_reg
#define VIDCON0           	(*(volatile u32*)(FIMD_BASE + 0x0000))
#define VIDCON1           	(*(volatile u32*)(FIMD_BASE + 0x0004))

#define VIDTCON0         	(*(volatile u32*)(FIMD_BASE + 0x0010))
#define VIDTCON1         	(*(volatile u32*)(FIMD_BASE + 0x0014))
#define VIDTCON2         	(*(volatile u32*)(FIMD_BASE + 0x0018))


#define WINCON0         	(*(volatile u32*)(FIMD_BASE + 0x0020))
#define WINCON1       	 	(*(volatile u32*)(FIMD_BASE + 0x0024))
#define WINCON2         	(*(volatile u32*)(FIMD_BASE + 0x0028))
#define WINCON3         	(*(volatile u32*)(FIMD_BASE + 0x002c))
#define WINCON4         	(*(volatile u32*)(FIMD_BASE + 0x0030))

#define SHADOWCON        	(*(volatile u32*)(FIMD_BASE + 0x0034))
//window0
#define VIDOSD0A         	(*(volatile u32*)(FIMD_BASE + 0x0040))
#define VIDOSD0B        	(*(volatile u32*)(FIMD_BASE + 0x0044))
#define VIDOSD0C         	(*(volatile u32*)(FIMD_BASE + 0x0048))

#define VIDW00ADD0B0        (*(volatile u32*)(FIMD_BASE + 0x00a0))
#define VIDW00ADD1B0        (*(volatile u32*)(FIMD_BASE + 0x00d0))
#define VIDW00ADD2          (*(volatile u32*)(FIMD_BASE + 0x0100))

//window1
#define VIDOSD1A         	(*(volatile u32*)(FIMD_BASE + 0x0050))
#define VIDOSD1B        	(*(volatile u32*)(FIMD_BASE + 0x0054))
#define VIDOSD1C         	(*(volatile u32*)(FIMD_BASE + 0x0058))
#define VIDOSD1D        	(*(volatile u32*)(FIMD_BASE + 0x005c))

#define VIDW01ADD0B0        (*(volatile u32*)(FIMD_BASE + 0x00a8))
#define VIDW01ADD1B0        (*(volatile u32*)(FIMD_BASE + 0x00d8))
#define VIDW01ADD2          (*(volatile u32*)(FIMD_BASE + 0x0104))

//window2
#define VIDOSD2A         	(*(volatile u32*)(FIMD_BASE + 0x0060))
#define VIDOSD2B        	(*(volatile u32*)(FIMD_BASE + 0x0064))
#define VIDOSD2C         	(*(volatile u32*)(FIMD_BASE + 0x0068))
#define VIDOSD2D        	(*(volatile u32*)(FIMD_BASE + 0x006c))

#define VIDW02ADD0B0        (*(volatile u32*)(FIMD_BASE + 0x00b0))
#define VIDW02ADD1B0        (*(volatile u32*)(FIMD_BASE + 0x00e0))
#define VIDW02ADD2          (*(volatile u32*)(FIMD_BASE + 0x0108))

//颜色表


#define BPPPAL8	0b0011
#define BPP565 0b0101
#define BPP888 0b1011
enum {
	MPLL_CLK = 800000000,
	LCD_CLK = 33300000, //像素时钟，每个周期传输一个像素的数据,lcd屏手册P14

	//屏幕分辨率
	ROW	= 800,
	COL	= 480,

	//时序极性, 0-normal, 1-inverted
	IVDEN	= 0,
	IVSYNC	= 1,
	IHSYNC	= 1,
	IVCLK	= 1, //0-下降沿，1-上升沿

	//时序
	VSPW	= 9,
	VBPD	= 12,
	VFPD	= 21,

	HSPW	= 19,
	HBPD	= 25,
	HFPD	= 209,


	bpp = BPP888,
	pixel_size = 4,

	word_swap = 1,
	halfword_swap = 0,
	byte_swap = 0,
	bit_swap = 0,

};

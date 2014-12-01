#pragma once


#define FIMD_BASE fb_pri->reg
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

//中断
#define VIDINTCON0          (*(volatile u32*)(FIMD_BASE + 0x0130))
#define VIDINTCON1          (*(volatile u32*)(FIMD_BASE + 0x0134))





#define BPPPAL8	0b0011
#define BPP565 0b0101
#define BPP888 0b1011
enum {
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

static int alloc_framebuffer_space(void);
static void fill_fb_info(void);
static inline void lcd_off(void);
static inline void lcd_on(void);
static int hardware_init(void);
static int fb_setcolreg(unsigned regno, unsigned red, unsigned green, unsigned blue, unsigned transp, struct fb_info *info);
static int pan_display(struct fb_var_screeninfo *var, struct fb_info *info);
static irqreturn_t irq_handler(int irqno, void *arg);

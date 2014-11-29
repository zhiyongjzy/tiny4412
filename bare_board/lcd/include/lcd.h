#pragma once

#define GPF0CON  (*(volatile u32*)0x11400180)
#define GPF1CON  (*(volatile u32*)0x114001a0)
#define GPF2CON  (*(volatile u32*)0x114001c0)
#define GPF3CON  (*(volatile u32*)0x114001e0)

#define GPF0DRV  (*(volatile u32*)0x1140018c)
#define GPF1DRV  (*(volatile u32*)0x114001ac)
#define GPF2DRV  (*(volatile u32*)0x114001cc)
#define GPF3DRV  (*(volatile u32*)0x114001ec)

#define GPF0PUD  (*(volatile u32*)0x11400188)
#define GPF1PUD  (*(volatile u32*)0x114001a8)
#define GPF2PUD  (*(volatile u32*)0x114001c8)
#define GPF3PUD  (*(volatile u32*)0x114001e8)

#define CLK_SRC_LCD0  	(*(volatile u32*)0x1003c234)
#define CLK_DIV_LCD    	(*(volatile u32*)0x1003c534)
#define LCDBLK_CFG     	(*(volatile u32*)0x10010210)

#define FIMD_BASE 0x11c00000
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
#define WPALCON_H      			(*(volatile u32*)(FIMD_BASE + 0x019c))
#define WPALCON_L      			(*(volatile u32*)(FIMD_BASE + 0x01a0))
#define WIN0PALETTERAM(n)      	(*(volatile u32*)(FIMD_BASE + 0x2400 + n*4))
#define WIN1PALETTERAM(n)     	(*(volatile u32*)(FIMD_BASE + 0x2800 + n*4))
#define WIN2PALETTERAM(n)      	(*(volatile u32*)(FIMD_BASE + 0x2c00 + n*4))
#define WIN3PALETTERAM(n)      	(*(volatile u32*)(FIMD_BASE + 0x3000 + n*4))
#define WIN4PALETTERAM(n)      	(*(volatile u32*)(FIMD_BASE + 0x3400 + n*4))




enum {
	MPLL_CLK = 800000000,
	VCLK = 33300000, //像素时钟，每个周期传输一个像素的数据,lcd屏手册P14

	//屏幕分辨率
	ROW	= 800,
	COL	= 480,

	//时序极性, 0-normal, 1-inverted
	IVDEN	= 0,
	IVSYNC	= 1,
	IHSYNC	= 1,
	IVCLK	= 0, //0-下降沿，1-上升沿

	//时序
	VSPW	= 9,
	VBPD	= 12,
	VFPD	= 21,

	HSPW	= 19,
	HBPD	= 25,
	HFPD	= 209,
};

struct win_info {
	size_t addr;

#define BPPPAL8	0b0011
#define BPP565 0b0101
#define BPP888 0b1011   // 实际上是32位图片(24位颜色,但显存中每个像素会空出8位)
	u8 bpp;
	u8 pixel_size;

	u8 word_swap;
	u8 halfword_swap;
	u8 byte_swap;
	u8 bit_swap;

	u8 alpha;

	void (*draw_pixel)(struct win_info *info, size_t x, size_t y, u32 color);
	void (*change_osd)(struct win_info *info, size_t x_start, size_t y_start, size_t x_end, size_t y_end);
};


static void draw_pixel_32bit(struct win_info *info, size_t x, size_t y, u32 color);
static void draw_pixel_16bit(struct win_info *info, size_t x, size_t y, u32 color);
static void draw_pixel_8bit(struct win_info *info, size_t x, size_t y, u32 color);
static void draw_rect(struct win_info *info, size_t x_start, size_t y_start ,size_t x_end, size_t y_end, u32 color);
static void clean_scr(struct win_info *info, u32 color);

static void change_osd_win2(struct win_info *info, size_t x_start, size_t y_start, size_t x_end, size_t y_end);
static void test_osd(struct win_info *info, size_t xlen, size_t ylen, size_t xstep, size_t ystep);

static void pan_display(struct win_info *info, size_t new_addr);
static void double_buf_flow_rect(struct win_info *info, size_t xlen, size_t ylen, size_t xstep, size_t ystep);

static void draw_font(struct win_info *info, u8 ascii, size_t x, size_t y, u32 color);
static void uart_font(struct win_info *info);
static void scroll_bmp(struct win_info *info,size_t height, size_t ystep);

static inline void delay(size_t delay)
{
	while(delay--){}
}
#include "config.h"
#include "lcd.h"
#include "font.h"

extern const struct font_desc font_vga_8x16;
extern const struct font_desc font_sun_12x22;

static struct win_info win[] = {
	[0] = {
		.addr = 0x41000000,
		.bpp = BPP888,
		.pixel_size = 4,

		.word_swap = 1,
		.halfword_swap = 0,
		.byte_swap = 0,
		.bit_swap = 0,
		.draw_pixel = draw_pixel_32bit,
	},
	[1] = {
		.addr = 0x42000000,
		.bpp = BPP565,
		.pixel_size = 2,

		.word_swap = 0,
		.halfword_swap = 1,
		.byte_swap = 0,
		.bit_swap = 0,
		.alpha = 8,
		.draw_pixel = draw_pixel_16bit,
	},
	[2] = {
		.addr = 0x43000000,
		.bpp = BPPPAL8,//带颜色表
		.pixel_size = 1,

		.word_swap = 0,
		.halfword_swap = 0,
		.byte_swap = 1,
		.bit_swap = 0,
		.draw_pixel = draw_pixel_8bit,
		.change_osd = change_osd_win2, 
	},
};

static inline void lcd_off()
{
	WINCON0 &= ~1;
	WINCON1 &= ~1;
	WINCON2 &= ~1;
	WINCON3 &= ~1;
	WINCON4 &= ~1;

	VIDCON0 &= ~0b1; // 关闭lcd控制器
}

static inline void lcd_on()
{
	VIDCON0 |= 0b11;

	WINCON0 |= 1;
	// WINCON1 |= 1;
	 WINCON2 |= 1;
}


void test()
{
	printk("====================\n");
	printk("========test========\n");
//	draw_rect(win + 1, 0, 0, 800, 480, 0xffffff);

//往win2显存里面全部写0 取第0个颜色
	// draw_rect(win + 2, 0, 0, 800, 480, 0);

	// test_osd(win + 2, 50, 50, 2, 2);
	// double_buf_flow_rect(win + 0, 50, 50, 1, 1);
	clean_scr(win + 0, 0xff);
	// draw_font(win + 0, 'b', 200, 200, 0xff0000);
	// clean_scr(win + 0, 0xff);
	uart_font(win + 0);
	//scroll_bmp(win + 0, 1000, 1);
	

#if 0
	while (1) {

		VIDOSD1C = 0 << 12 | 0 << 16 | 0 << 20;	//ALPHA

		delay(0xffffff);

		VIDOSD1C = 15 << 12 | 15 << 16 | 15 << 20;	//ALPHA

		delay(0xffffff);
	}
#endif

	
#if 0
	while (1) {
		draw_rect(win + 0, 0, 0, 800, 480, 0xffff);
		delay(0xfffff);
		draw_rect(win + 0, 0, 0, 800, 480, 0xff00);
		delay(0xfffff);
	}
#endif
}
static void lcd_init()
{
	//1 初始化lcd的GPIO管脚
	GPF0CON = 0x22222222;
	GPF1CON = 0x22222222;
	GPF2CON = 0x22222222;
	GPF3CON = (GPF3CON & ~0xfffff) | 0x22222;

	GPF0PUD = 0x0000ffff;
	GPF1PUD = 0x0000ffff;
	GPF2PUD = 0x0000ffff;
	GPF3PUD = 0x0000ffff;

	//MAX drive strength­­­­­­­­­//
	GPF0DRV = 0x0000ffff;
	GPF1DRV = 0x0000ffff;
	GPF2DRV = 0x0000ffff;
	GPF3DRV = 0x0000ffff;

	//2 配置输入时钟sclk, 手册P457右上角图
	CLK_SRC_LCD0 = (CLK_SRC_LCD0 & ~0xf) | 0b0110; //选择MPLL输出做时钟源
	CLK_DIV_LCD &= ~0xf; //选择FIMD的输入时钟源分频值 不分频

	//3 配置lcd系统时钟由FIMD来使用, LCDBLK_CFG[1], 手册P884
	LCDBLK_CFG = LCDBLK_CFG | (1 << 1); //选1旁路FIMD

	//4 关闭lcd控制器，必须先关闭lcd控制器才能做初始化设置
	lcd_off();

	//==============初始化fimd==========
	/* VIDCON0: 全局总控寄存器
	 * [1:0]: lcd控制器开关
	 * [5]: vclk的开关是否由[1]来控制
	 * [13:6]: 分频值， 输入时钟sclk / (该分频值+1) = VCLK
	 * [16]: 如果改变clkval，什么时候生效，无所谓
	 * [17]: 不反转
	 * [18]: RGB并行 0
	 * [28:26]: 输出格式是RGB interface
	 */

	VIDCON0 &= 0; //先清零
	VIDCON0 = VIDCON0 | (1 << 5) | ((MPLL_CLK / VCLK - 1) << 6) | (1 << 16) | (0 << 17) | (0 << 18) | (0b000 << 26) ;

	/* VIDCON1： 设置时序极性，与fimd时序极性相同的为normal，相反的为inverted
	 */
	VIDCON1 = IVDEN << 4 | IVSYNC << 5 | IHSYNC << 6 | IVCLK << 7;

	//设置时序
	VIDTCON0 = ( VSPW & 0xff ) | ( VFPD & 0xff ) << 8 | ( VBPD & 0xff ) << 16 ;
	VIDTCON1 = ( HSPW & 0xff ) | ( HFPD & 0xff ) << 8 | ( HBPD & 0xff ) << 16 ;

	//分辨率
	VIDTCON2 = (COL - 1) << 11 | (ROW - 1);

	SHADOWCON |= 0b11111;

	//========================================== window 0 =============================
	/* WIN0CON: window0的主控制寄存器
	 * [0]: 开关
	 * [5:2]: 色深
	 * [9:8]: dma burst，设最大值  "00"
	 * [18:15]: 相邻像素数据的排列顺序，RGB565格式见手册P1779
	 */
	WINCON0 = win[0].bpp << 2 | win[0].word_swap << 15 | win[0].halfword_swap << 16 | win[0].byte_swap << 17 | win[0].bit_swap << 18;
	//window显示范围的左上角坐标
	VIDOSD0A = 0;
	//window显示范围的右下角坐标
	VIDOSD0B = (ROW - 1) << 11 | (COL - 1);
	//显示范围的所需字数 OSDSIZE
	VIDOSD0C = ROW * COL * win[0].pixel_size / 4;

	//显存首末地址，关于显存终止地址公式的由来，见手册P1813
	VIDW00ADD0B0 = win[0].addr;
	VIDW00ADD1B0 = win[0].addr + ROW * COL * win[0].pixel_size;
	VIDW00ADD2 = ROW * win[0].pixel_size;

	//========================================== window 1 ===========================
	WINCON1 = win[1].bpp << 2 | win[1].word_swap << 15 | win[1].halfword_swap << 16 | win[1].byte_swap << 17 | win[1].bit_swap << 18;
	//alpha总控
	WINCON1 = WINCON1 | 0 << 6 | 0 << 1;    // plain blending Using ALPHA0_R/G/B values
	
	VIDOSD1A = 0;    //window显示范围的左上角坐标

	VIDOSD1B = (ROW - 1) << 11 | (COL - 1); 	//window显示范围的右下角坐标

	VIDOSD1D = ROW * COL * win[1].pixel_size / 4;  	//显示范围的所需字数  OSDSIZE

	VIDOSD1C = win[1].alpha << 12 | win[1].alpha << 16 | win[1].alpha << 20;	//ALPHA

	//显存首末地址，关于显存终止地址公式的由来，见手册P1813
	VIDW01ADD0B0 = win[1].addr;
	VIDW01ADD1B0 = win[1].addr + ROW * COL * win[1].pixel_size;
	VIDW01ADD2 = ROW * win[1].pixel_size;

	//========================================== window 2 ========================
	WINCON2 = win[2].bpp << 2 | win[2].word_swap << 15 | win[2].halfword_swap << 16 | win[2].byte_swap << 17 | win[2].bit_swap << 18;
	WINCON2 = WINCON2 | 0 << 6 | 0 << 1;    // plain blending Using ALPHA0_R/G/B values
	
	VIDOSD2A = 0;    //window显示范围的左上角坐标

	VIDOSD2B = (ROW) << 11 | (COL); 	//window显示范围的右下角坐标

	VIDOSD2D = (ROW) * (COL) * win[2].pixel_size / 4;  	//显示范围的所需字数  OSDSIZE

	VIDOSD2C = 0xf << 12 | 0xf << 16 | 0xf << 20;	//ALPHA0

	//显存首末地址，关于显存终止地址公式的由来，见手册P1813
	VIDW02ADD0B0 = win[2].addr;
	VIDW02ADD1B0 = win[2].addr + (ROW) * (COL) * win[2].pixel_size;
	VIDW02ADD2 = (ROW) * win[2].pixel_size;

	//update platte
	WPALCON_L |= 1 << 9 | 1 << 6;    //WPAL2 = 101    24位颜色表  888
	WPALCON_H = (WPALCON_H & ~(0b11 << 9)) | 0b10 << 9;

	WIN2PALETTERAM(0) = 0xffff;  //win2 颜色表的0个颜色
	WIN2PALETTERAM(1) = 0xff0000;
	WPALCON_L &= ~(1 << 9);  //normal mode


	//开启lcd控制器
	lcd_on();

}
module_init(lcd_init);


static void draw_pixel_32bit(struct win_info *info, size_t x, size_t y, u32 color)
{
	*((u32*)info->addr + y * ROW + x) = color;
}
static void draw_pixel_16bit(struct win_info *info, size_t x, size_t y, u32 color)
{
	*((u16*)info->addr + y * ROW + x) = color;
}
static void draw_pixel_8bit(struct win_info *info, size_t x, size_t y, u32 color)
{
	*((u8*)info->addr + y * ROW + x) = color;
}
static void draw_rect(struct win_info *info, size_t x_start, size_t y_start ,size_t x_end, size_t y_end, u32 color)
{
	size_t x, y;
	for (y = y_start; y < y_end; ++y) {
		for (x = x_start; x < x_end; ++x) {
			info->draw_pixel(info, x, y, color);
		}
	}	
}
static void clean_scr(struct win_info *info, u32 color)
{
	draw_rect(info, 0, 0, 800, 480, color);
}

static void change_osd_win2(struct win_info *info, size_t x_start, size_t y_start, size_t x_end, size_t y_end)
{
	VIDOSD2A = x_start << 11 | y_start;    //window显示范围的左上角坐标
	VIDOSD2B = x_end << 11 | y_end; 	//window显示范围的右下角坐标
	VIDOSD2D = (x_end - x_start) * (y_start - y_end) * info->pixel_size / 4;  	//显示范围的所需字数  OSDSIZE
}
static void test_osd(struct win_info *info, size_t xlen, size_t ylen, size_t xstep, size_t ystep)
{
	ssize_t x = 0;
	ssize_t y = 0;
	ssize_t xstep_var;
	ssize_t ystep_var;
	for (xstep_var = xstep, ystep_var = 0; ; x += xstep_var, y += ystep_var) {
		if ((x + xlen >= ROW - 1) && (y == 0)) {
			xstep_var = 0;
			ystep_var = ystep;
			x = ROW - 1 - xlen;
		} else if ((x == ROW - 1 - xlen) && (y + ylen >= COL -1)) {
			xstep_var = -xstep;
			ystep_var = 0;
			y = COL - 1 - ylen;
		} else if ((x <= 0) && (y == COL - 1 - ylen)) {
			xstep_var = 0;
			ystep_var = -ystep;
			x = 0;
		} else if ((x == 0) && (y <= 0)) {
			xstep_var = xstep;
			ystep_var = 0;
			y = 0;
		}
		info->change_osd(info, x, y, x + xlen, y + ylen);
		delay(0xffff);
	}
}
 //切换显存首地址
static void pan_display(struct win_info *info, size_t new_addr)
{
	//在帧周期中，处于active时，不能切换显存。此时切换显存，那么画面中上半部分来自老显存，下半部分取自新显存，画面无疑是撕裂的。所以切换显存时必须避开帧周期是正在传输行数据的时候
	while (((VIDCON1 >> 13) & 3) == 0b10) {}

	switch (info - win) {
	case 0:
		VIDW00ADD0B0 = new_addr;
		VIDW00ADD1B0 = new_addr + ROW * COL * win[0].pixel_size;
		break;
	case 1:
		VIDW01ADD0B0 = new_addr;
		VIDW01ADD1B0 = new_addr + ROW * COL * win[1].pixel_size;
		break;
	case 2:
		VIDW02ADD0B0 = new_addr;
		VIDW02ADD1B0 = new_addr + ROW * COL * win[2].pixel_size;
		break;
	};
}
static void double_buf_flow_rect(struct win_info *info, size_t xlen, size_t ylen, size_t xstep, size_t ystep)
{	
	size_t front_buf = info->addr;
	size_t back_buf = info->addr + ROW * COL * info->pixel_size;
	size_t swap_buf = 0;  //该变量用于交换两个缓冲地址
	size_t back_color = 0xffff;
	size_t front_color = 0xff0000;

	//清屏第一块显存
	clean_scr(info, back_color);
	//清屏第二块显存
	info->addr = back_buf;
	clean_scr(info, back_color);

	ssize_t x, y;
	ssize_t xstep_var = xstep, ystep_var = 0;
	ssize_t x_prev = 0, y_prev = 0;
	for (x = 0, y = 0; ; x += xstep_var, y += ystep_var) {
		//画矩形
		draw_rect(info, x, y, x + xlen, y + ylen, front_color);

		swap_buf = front_buf;
		front_buf = back_buf;
		back_buf = swap_buf;
		pan_display(info, front_buf);

		//清除上一帧矩形为背景色
		info->addr = back_buf;
		draw_rect(win, x_prev, y_prev, x_prev + xlen, y_prev + ylen, back_color);

		//记住上一帧矩形坐标
		x_prev = x;
		y_prev = y;

		// 屏幕四个角的处理
		if ((x + xlen >= ROW - 1) && (y == 0)) {
			xstep_var = 0;
			ystep_var = ystep;
			x = ROW - 1 - xlen;
		} else if ((x == ROW - 1 - xlen) && (y + ylen >= COL -1)) {
			xstep_var = -xstep;
			ystep_var = 0;
			y = COL - 1 - ylen;
		} else if ((x <= 0) && (y == COL - 1 - ylen)) {
			xstep_var = 0;
			ystep_var = -ystep;
			x = 0;
		} else if ((x == 0) && (y <= 0)) {
			xstep_var = xstep;
			ystep_var = 0;
			y = 0;
		}

	}
}

static void draw_font(struct win_info *info, u8 ascii, size_t x, size_t y, u32 color)
{
#if defined(FONT_8x16)
	const struct font_desc *font = &font_vga_8x16;
	size_t i, j;
	u8 *p = (u8 *)font->data + ascii * font->height;
	for (j = 0;j < font->height; j++) {
		for (i = 0; i < font->width; i++) {
			if ((p[j] & ((1 << 7) >> i))) {
				info->draw_pixel(info, i + x, j + y, color);	
			}
		}
	}
#elif defined(FONT_12x22)
	const struct font_desc *font = &font_sun_12x22;
	size_t i, j;

	u16 *p = (u16 *)font->data + ascii * font->height;
	u16 data;
	for (j = 0;j < font->height; j++) {
		data = htons(p[j]);
		for (i = 0; i < font->width; i++) {
			if ((data & ((1 << 15) >> i))) {
				info->draw_pixel(info, i + x, j + y, color);	
			}
		}
	}

#endif
}
static void uart_font(struct win_info *info)
{
#if defined(FONT_8x16)
	const struct font_desc *font = &font_vga_8x16;
#elif defined(FONT_12x22)
	const struct font_desc *font = &font_sun_12x22;
#endif draw_rect(info, 0, 0, 800, 480, 0xff00);  // 进来清屏
	u16 i, j;
	while (1) {
		for (j = 0; j < COL - font->height; j += font->height) {
			for (i = 0; i < ROW - font->width; i += font->width) {
				u8 ch = uart_recv();
				print_ch(ch);
				if (ch == '\r' || ch == '\n') {//如果串口传过来的是换行符
					i = -font->width; //因为内层for循环会加8
					j += font->height;
					continue;
				}
				draw_font(info, ch, i, j, 0xff);
			}
		}
	}
}

//显示一张bmp,要求剥去文件头,只留像素数据,并且色深格式和像素宽度与对应window完全符合,高度允许超过显存范围,如果超出屏幕纵向分辨率,则自动滚动
static void scroll_bmp(struct win_info *info, size_t height, size_t ystep)
{
	ssize_t y = 0;
	for (y = 1; ; y += ystep) {
		if ((y >= height - 1 - COL) || (y <= 0)) {
			ystep = -ystep;
		}
		pan_display(info, info->addr + y * info->pixel_size * ROW);
		// printk("yyyy %d\n", y);
		delay(0xffff);
	}
}


#pragma once

#include <linux/fb.h>

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

#ifdef _DEBUG
#define  DBG_PRINTF(fmt, args...) \
		printf("[DEBUG][%s:%d:%s]"fmt, __FILE__, __LINE__, __FUNCTION__, ##args)
#else
#define  DBG_PRINTF(fmt, args...)
#endif

#define RGB888(r, g, b) ( ((r) & 0xff) << 16 | ((g) & 0xff) << 8 | ((b) & 0xff))


struct fb_info {
	struct fb_var_screeninfo var;
	struct fb_fix_screeninfo fix;
	size_t x, y; //屏幕分辨率
	size_t pixel_size; 
	u8 *pfb;
	void (*draw_dot)(struct fb_info *info, size_t x, size_t y, u32 color);
	void (*draw_line)(struct fb_info *info, size_t x_start, size_t y_start, size_t x_end, size_t y_end, u32 color);
	void (*draw_rect)(struct fb_info *info, size_t x_start, size_t y_start, size_t x_end, size_t y_end, u32 color);
	int (*draw_bmp)(struct fb_info *info, const char *bmpfile_path);
};
void fb_init(struct fb_info *info);
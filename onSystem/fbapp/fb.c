#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <arpa/inet.h>

#include "fb.h"
#include "bmp.h"

static inline void draw_dot_32bit(struct fb_info *info, size_t x, size_t y, u32 color)
{
	u32 real_color = (color >> 16 & 0xff) >> (8 - info->var.red.length) << info->var.red.offset | (color >> 8 & 0xff) >> (8 - info->var.green.length) << info->var.green.offset
		 | (color & 0xff) >> (8 - info->var.blue.length) << info->var.blue.offset
		 | (color >> 24 & 0xff) >> (8 - info->var.transp.length) << info->var.transp.offset;
	// *(((u32 *)(info->pfb)) + x + info->x * y) = real_color;
	*(((u32 *)(info->pfb)) + x + info->x * y) = color;
}
static inline void draw_dot_16bit(struct fb_info *info, size_t x, size_t y, u32 color)
{
	u32 real_color = (color >> 16 & 0xff) >> (8 - info->var.red.length) << info->var.red.offset | (color >> 8 & 0xff) >> (8 - info->var.green.length) << info->var.green.offset
		 | (color & 0xff) >> (8 - info->var.blue.length) << info->var.blue.offset
		 | (color >> 24 & 0xff) >> (8 - info->var.transp.length) << info->var.transp.offset;

	*((u16 *)(info->pfb) + x + info->x * y) = real_color;
	// *(info->pfb + x + info->x * y) = color;
}
static inline void draw_dot_8bit(struct fb_info *info, size_t x, size_t y, u32 color)
{
	u32 real_color = (color >> 16 & 0xff) >> (8 - info->var.red.length) << info->var.red.offset | (color >> 8 & 0xff) >> (8 - info->var.green.length) << info->var.green.offset
		 | (color & 0xff) >> (8 - info->var.blue.length) << info->var.blue.offset
		 | (color >> 24 & 0xff) >> (8 - info->var.transp.length) << info->var.transp.offset;

	*((u8 *)(info->pfb) + x + info->x * y) = real_color;
	// *(info->pfb + x + info->x * y) = color;
}
static void draw_line(struct fb_info *info, size_t x_start, size_t y_start, size_t x_end, size_t y_end, u32 color)
{

}
static void draw_rect(struct fb_info *info, size_t x_start, size_t y_start, size_t x_end, size_t y_end, u32 color)
{

	int i, j;
	for (i = x_start; i < x_end; i++)
		for (j = y_start; j < y_end; j++)
			info->draw_dot(info, i, j, color);

}
static int draw_bmp(struct fb_info *info, const char *bmpfile_path)
{
	int fd_pic = open(bmpfile_path, O_RDONLY);
	if (fd_pic < 0) {
		perror("open");
		return -2;
	}
	struct stat stat_buf = {0};
	int ret = fstat(fd_pic, &stat_buf);
	if (ret < 0) {
		perror("fstat");
		goto err_fstat;
	}
	u8 *pic_buf = (u8 *)mmap(NULL, stat_buf.st_size, PROT_READ, MAP_SHARED, fd_pic, 0);
	if (pic_buf == MAP_FAILED) {
		perror("mmap");
		goto err_mmap;
	}
	struct bmp_file *bmp_file = (struct bmp_file *)pic_buf;
	struct bmp_info *bmp_info = (struct bmp_info *)(pic_buf + sizeof(struct bmp_file));

	DBG_PRINTF("bmp_file->offset %u\n", bmp_file->offset);
	DBG_PRINTF("bmp_info->width %u\n", bmp_info->width);
	DBG_PRINTF("bmp_info->height %u\n", bmp_info->height);
	DBG_PRINTF("bmp_info->count %u\n", bmp_info->count);

	u32 *p_pic_data = (u32 *)(pic_buf + bmp_file->offset);
	int i, j, k = 0;
	for (j = bmp_info->height - 1; j >= 0; j--) {
		for (i = 0; i < bmp_info->width; i++, k++) { 
			usleep(10000);
			draw_dot_32bit(info, i, j, *(p_pic_data + k) >> 8);
		}
	}

	return 0;
err_fstat:
	return ret;
err_mmap:
	return -1;
}

void fb_init(struct fb_info *info)
{
	switch (info->pixel_size) {
	case 1:
		info->draw_dot = draw_dot_8bit;
		break;
	case 2:
		info->draw_dot = draw_dot_16bit;
		break;
	case 4:
		info->draw_dot = draw_dot_32bit;
		break;
	default:
		fprintf(stderr, "func fb_init switch case");
		break;
	};
	info->draw_line = draw_line;
	info->draw_rect = draw_rect;
	info->draw_bmp = draw_bmp;
}
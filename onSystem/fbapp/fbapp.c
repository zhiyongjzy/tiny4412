#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include <linux/fb.h>

#include "fb.h"

int main(int argc, char const *argv[])
{
	struct fb_info fb;
	int fd = open("/dev/fb0", O_RDWR);
	if (fd < 0) {
		perror("open");
		goto err_open;
	}

	ioctl(fd, FBIOGET_VSCREENINFO, &fb.var);
	ioctl(fd, FBIOGET_FSCREENINFO, &fb.fix);

	fb.pixel_size = fb.var.bits_per_pixel / 8;
	fb.x = fb.fix.line_length / fb.pixel_size;
	fb.y = fb.var.yres;
	
	fb.pfb = (u8 *)mmap(NULL, fb.fix.smem_len, PROT_WRITE, MAP_SHARED, fd, 0);
	if (fb.pfb == MAP_FAILED) {
		perror("mmap");
		goto err_mmap;
	}

	DBG_PRINTF("fb.var.bits_per_pixel  = %u\n", fb.var.bits_per_pixel);
	DBG_PRINTF("fb.x = %lu\n", fb.x);
	DBG_PRINTF("fb.y = %lu\n", fb.y);

	fb_init(&fb);
/*
	//=============================draw dot===========================
	int i, j;
	for (i = 0; i < fb.x; i++)
		for (j = 0; j < fb.y; j++)
		fb.draw_dot(&fb, i, j, 0x000000ff);
*/
	//=============================draw rect===========================
	 // fb.draw_rect(&fb, 0, 0, 800, 480, RGB888(255, 0, 0));

	//=============================draw bmp===========================
	fb.draw_bmp(&fb, argv[1]);


	sleep(1);

	signed char step = 1;
	for (fb.var.yoffset = 1; ; fb.var.yoffset += step) {
		if ((fb.var.yoffset >= fb.var.yres - 1) || (fb.var.yoffset <= 0)) {
			step = -1 * step;
		}
		usleep(10000);
		ioctl(fd, FBIOPAN_DISPLAY, &fb.var);
		// printf("=============  step %d\n", step);
	}




	pause();
	munmap(fb.pfb, fb.fix.smem_len);
	close(fd);

	return 0;
err_open:
	return -1;
err_mmap:
	return -2;
}

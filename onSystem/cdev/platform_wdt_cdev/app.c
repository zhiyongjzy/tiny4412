#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


int main(int argc, char const *argv[])
{
	int fd;
	if ((fd = open("/dev/ldm", O_RDWR)) < 0) {
		perror("open");
		return -1;
	}
	int a;
	if (*argv[1] == '1') {
		a = 1000;
	} else if (*argv[1] == '0') {
		a = 1001;
	}
	ioctl(fd, a);

	close(fd);

	return 0;
}
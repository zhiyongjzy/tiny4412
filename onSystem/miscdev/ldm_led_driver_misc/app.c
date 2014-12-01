#include <stdio.h>
#include <unistd.h>
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
	char write_buf = atoi(argv[1]);
	int ret;
	ret = write(fd, &write_buf, 1);
	printf("write return value %d\n", ret);

	char read_buf = 0;
	ret = read(fd, &read_buf, 1);
	printf("read from buf %d\nread return value %d\n", read_buf, ret);

	return 0;
}
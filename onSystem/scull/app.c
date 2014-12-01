#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


int main(int argc, char const *argv[])
{
	int fd;
	if ((fd = open(argv[1], O_RDWR)) < 0) {
		perror("open");
		return -1;
	}

	int ret = 0;
	char buf[1024] = "hello world";
	write(fd, buf, sizeof(buf));


	close(fd);

	return 0;
}
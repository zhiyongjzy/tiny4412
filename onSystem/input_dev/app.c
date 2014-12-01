#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <linux/input.h>

struct input_event event;

int main(int argc, char const *argv[])
{
	int fd;
	if ((fd = open(argv[1], O_RDWR)) < 0) {
		perror("open");
		return -1;
	}

	int ret = 0;
	while (read(fd, &event, sizeof(event))) {
		switch (event.code) {
		case KEY_L :
			if (event.value) {
				printf("key l is pressed.\n");
			} else {
				printf("key l is released.\n");
			}
			break;
		case KEY_S :
			if (event.value) {
				printf("key s is pressed.\n");
			} else {
				printf("key s is released.\n");
			}
			break;
		case KEY_P :
			if (event.value) {
				printf("key p is pressed.\n");
			} else {
				printf("key p is released.\n");
			}
			break;
		case KEY_ENTER :
			if (event.value) {
				printf("key enter is pressed.\n");
			} else {
				printf("key enter is released.\n");
			}
			break;
		}
	}


	close(fd);

	return 0;
}
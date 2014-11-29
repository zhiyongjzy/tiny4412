#include <stdio.h>
#include "bmp.h"


int main(int argc, char const *argv[])
{

	FILE *fp = fopen(argv[1], "r");
	if (fp == NULL) {
		perror("fopen");
		return -1;
	}
	FILE *fp_new = fopen(argv[2], "w");
	if (fp_new == NULL) {
		perror("fopen new");
		return -2;
	}

	int buf_tmp[1024];
	fread(buf_tmp, sizeof(buf_tmp), 1, fp);
	struct bmp_file *bfile = buf_tmp;
	fseek(fp, bfile->offset, SEEK_SET);

	int buf;
	while (fread(&buf, sizeof(buf), 1, fp)) {
		buf >>= 8;
		fwrite(&buf, sizeof(buf), 1, fp_new);
	}


	fclose(fp);
	fclose(fp_new);

	return 0;


}

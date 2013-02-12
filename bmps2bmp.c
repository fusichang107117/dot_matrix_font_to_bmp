#include <stdio.h>
#include <stdlib.h>
#include "dot_matrix_font_to_bmp.h"
#include "debug_log.h"

#ifndef	MAXLINE
#define	MAXLINE		1024
#endif

int read_and_alloc_one_bmp(FILE *fp, bmp_file_t *ptrbmp);
void free_bmp(bmp_file_t *ptrbmp);
int output_bmp(FILE *fp, bmp_file_t *ptrbmp);

int read_and_alloc_one_bmp(FILE *fp, bmp_file_t *ptrbmp)
{
	size_t ret;

	ret = fread(&ptrbmp->bmp_h, sizeof(struct bmp_file_header), 1, fp);
	if (ret < 0)
		return -1;
	ret = fread(&ptrbmp->dib_h, sizeof(struct dib_header), 1, fp);
	if (ret < 0)
		return -1;

	ptrbmp->pdata = malloc(ptrbmp->dib_h.image_size);
	ret = fread(ptrbmp->pdata, 1, ptrbmp->dib_h.image_size, fp);
	if (ret < 0)
		return -1;
	return 0;
}

void free_bmp(bmp_file_t *ptrbmp)
{
	if (ptrbmp) {
		if (ptrbmp->pdata) {
			free(ptrbmp->pdata);
			ptrbmp->pdata = NULL;
		}
		free(ptrbmp);
		ptrbmp = NULL;
	}
}

int output_bmp(FILE *fp, bmp_file_t *ptrbmp)
{
	int ret;

	ret = fwrite(&ptrbmp->bmp_h, 
		     sizeof(struct bmp_file_header),
		     1,
		     fp);
	if (ret < 0) {
		debug_print("fwrite fail");
		return -1;
	}

	ret = fwrite(&ptrbmp->dib_h, 
		     sizeof(struct dib_header),
		     1,
		     fp);
	if (ret < 0) {
		debug_print("fwrite fail");
		return -1;
	}

	ret = fwrite(ptrbmp->pdata, 
		     1, 
		     ptrbmp->dib_h.image_size, 
		     fp);
	if (ret < 0) {
		debug_print("fwrite fail");
		return -1;
	}
	return 0;
}

int main(int argc, char **argv)
{
	bmp_file_t bmp_all;
	bmp_file_t *ptmp_bmp1;
	bmp_file_t *ptmp_bmp2;
	uint8_t buf[MAXLINE];
	char ch;
	size_t length;
	int ret;

	ptmp_bmp1 = malloc(sizeof(bmp_file_t));
	memset(ptmp_bmp1, 0, sizeof(bmp_file_t));
	ret = read_and_alloc_one_bmp(stdin, ptmp_bmp1);
	if (ret < 0) {
		ret = -1;
		goto release1;
	}

	ptmp_bmp2 = malloc(sizeof(bmp_file_t));
	memset(ptmp_bmp2, 0, sizeof(bmp_file_t));
	ret = read_and_alloc_one_bmp(stdin, ptmp_bmp2);
	if (ret < 0) {
		ret = output_bmp(stdout, ptmp_bmp1);
		goto release2;
	}

	memset(&bmp_all, 0, sizeof(bmp_all));
	set_header(&bmp_all, 
		   ptmp_bmp1->dib_h.width + ptmp_bmp2->dib_h.width,
		   ptmp_bmp1->dib_h.height,
		   ptmp_bmp1->dib_h.bits_per_pix);
	bmp_all.pdata = malloc(bmp_all.dib_h.image_size);
	bmp_h_combin(ptmp_bmp1, ptmp_bmp2, &bmp_all);
	ret = output_bmp(stdout, &bmp_all);
	if (ret < 0) {
		debug_print("output_bmp() fail");
		goto release3;
	}

	if ((ch = fgetc(stdin)) == 'B') {
		ungetc(ch, stdin);
		ret = 1;
	} else {
		ret = 0;
		goto release3;
	}
	while ((length = fread(buf, 1, MAXLINE, stdin)) > 0) {
		if (fwrite(buf, 1, length, stdout) < length)
			debug_print("fwrite() return %d", ret);
	}

release3:
	free(bmp_all.pdata);
release2:
	free_bmp(ptmp_bmp2);
release1:
	free_bmp(ptmp_bmp1);
	return ret;
}

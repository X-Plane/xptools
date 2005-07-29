#include <stdio.h>
#include <errno.h>

int main(int argc, char ** argv)
{
	FILE * src;
	int x_total, y_total, x_cuts, y_cuts, x_sub, y_sub;
	FILE ** dst;
	int x, y;
	if (argc != 6)
	{
		fprintf(stderr, "Usage: split_image <width><height><xcuts><y cuts><file>\n");
		return 1;
	}

	x_total = atoi(argv[1]);	
	y_total = atoi(argv[2]);	
	x_cuts = atoi(argv[3]);	
	y_cuts = atoi(argv[4]);	
	x_sub = x_total / x_cuts;
	y_sub = y_total / y_cuts;
	if (x_sub * x_cuts < x_total) ++x_sub;
	if (y_sub * y_cuts < y_total) ++y_sub;
	printf("Will cut the %d bt %d image %s into %d by %d pieces of %d/%d.\n", x_total, y_total, argv[5], x_cuts, y_cuts, x_sub, y_sub);
	src = fopen(argv[5], "rb");
	if (!src)
	{
		fprintf(stderr,"Could not open %s: %d\n", argv[5], errno);
		return 1;
	}
	dst = (FILE**) malloc(x_cuts*y_cuts*sizeof(FILE*));
	if (dst==NULL)
	{
		fprintf(stderr,"Out of memory.\n");
		return 1;		
	}
	for (x = 0; x < x_cuts; ++x)
	for (y = 0; y < y_cuts; ++y)
	{
		char	name_buf[256];
		sprintf(name_buf,"%s_%d_%d",argv[5], x, y);
		dst[x+y*x_cuts] = fopen(name_buf,"wb");
		if (dst[x+y*x_cuts] == NULL)
		{
			fprintf(stderr,"Could not open %s: %d\n", name_buf, errno);
			return 1;
		}
	}

	
	for (y = 0; y < y_total; ++y)
	for (x = 0; x < x_total; ++x)
	{
		int xf = x / x_sub;
		int yf = y / y_sub;
		char b = fgetc(src);
		fputc(b, dst[xf+yf*x_cuts]);
	}
	
	for (x = 0; x < x_cuts; ++x)
	for (y = 0; y < y_cuts; ++y)
		fclose(dst[x+y*x_cuts]);

	fclose(src);			
}

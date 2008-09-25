#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <zlib.h>
#include <png.h>
#include <ft2build.h>
#include <freetype/ftglyph.h>

struct gl_glyph
{
	int		size;
	int		bwidth;
	int		bheight;
	long long	charcode;
	int		dl_width;
	int		dl_rows;
	float		advance;
	float		y_align;
	float		left_align;
	float		tex_x;
	float		tex_y;
};

__inline int next_p2(int a)
{
	int rval = 2;
	while(rval < a)
		rval <<= 1;
	return rval;
}

void process_file(char* filename, int min_height, int max_height)
{
	FT_Library			ftLib;
	FT_Face				ftFace;
	int					i, j, bwidth, bheight;
	long long			num_glyphs	= 0;
	long long			charcode	= 0;
	FT_UInt				gindex		= 0;
	FT_Glyph			glyph		= NULL;
	FT_BitmapGlyph		bitmap_glyph;
	FT_Bitmap			bitmap;
	unsigned char*		glyphdata	= NULL;
	int					fontfile	= -1;
	int					texfile		= -1;
	char				texfilename[2048];
	unsigned short		dpi_x		= 96;
	unsigned short		dpi_y		= 96;
	unsigned short		height = 0;
	unsigned long long	offset = 0;
	unsigned long long	backup = 0;
	struct gl_glyph		glyphstruct;
	unsigned long long	k = 0;
	unsigned char*		source = NULL;
	unsigned char*		dest = NULL;
	unsigned long long	s_len = 0;
	unsigned long long	d_len = 0;
	unsigned long		d_l	= 0;

	if (min_height < 1 || min_height > 99 || max_height < 1 || max_height > 99 || min_height > max_height)
	{
		printf("out of range.\n");
		return;
	}

	memset(texfilename, 0, 2048);

	if (!filename) return;
	printf("processing \"%s\".\n", filename);

	fontfile = open(filename, O_RDONLY);
	if (fontfile < 0)
	{
		perror("fontfile error");
		return;
	}
	close(fontfile);
	sprintf(texfilename, "%s", filename);
	sprintf(texfilename, "%s%d_%d.laft", strtok(texfilename, "."), min_height, max_height);
	texfile = open(texfilename, O_RDWR|O_CREAT|O_TRUNC, 0644);

	if (texfile < 0)
	{
		perror("texfile error");
		return;
	}

	if (FT_Init_FreeType(&ftLib))
	{
		FT_Done_FreeType(ftLib);
		return;
	}

	if (FT_New_Face(ftLib, filename, 0, &ftFace))
	{
		FT_Done_Face(ftFace);
		FT_Done_FreeType(ftLib);
		return;
	}
	num_glyphs = ftFace->num_glyphs;
	if (!num_glyphs)
	{
		FT_Done_Face(ftFace);
		FT_Done_FreeType(ftLib);
		return;
	}
	write(texfile, &min_height, sizeof(int));
	write(texfile, &max_height, sizeof(int));

	for (height = min_height; height < (max_height+1); height++)
	{
		backup = lseek(texfile, 0, SEEK_CUR);
		write(texfile, &offset, sizeof(unsigned long long));
		write(texfile, &num_glyphs, sizeof(long long));

		printf("processing fontsize %.2d     ", height);
		if (FT_Set_Char_Size(ftFace, 0, (height << 6), dpi_x, dpi_y))
		{
			FT_Done_Face(ftFace);
			FT_Done_FreeType(ftLib);
			return;
		}

		charcode = FT_Get_First_Char(ftFace, &gindex);
		while (gindex)
		{
			FT_Load_Glyph(ftFace, gindex, FT_LOAD_DEFAULT);
			FT_Get_Glyph(ftFace->glyph, &glyph);
			FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, 0, 1);
			bitmap_glyph = (FT_BitmapGlyph)glyph;
			bitmap = bitmap_glyph->bitmap;

			bwidth = next_p2(bitmap.width);
			bheight = next_p2(bitmap.rows);

			glyphdata = (unsigned char*)malloc(2*bwidth*bheight);

			if (glyphdata)
			for(j = 0; j < bheight; j++)
			{
				for(i = 0; i < bwidth; i++)
				{
					glyphdata[2*(i+j*bwidth)] = 255;
					glyphdata[2*(i+j*bwidth)+1] = (i >= bitmap.width || j >= bitmap.rows) ? 0 : bitmap.buffer[i + bitmap.width*j];
				}
			}

			glyphstruct.size = glyphdata?2*bwidth*bheight:0;
			glyphstruct.advance = (float)(ftFace->glyph->advance.x >> 6);
			glyphstruct.dl_width = bitmap.width;
			glyphstruct.dl_rows = bitmap.rows;
			glyphstruct.y_align = (float)(bitmap_glyph->top - bitmap.rows);
			glyphstruct.left_align = (float)bitmap_glyph->left;
			glyphstruct.tex_x =  bitmap.width / (float)bwidth;
			glyphstruct.tex_y = bitmap.rows / (float)bheight;
			glyphstruct.bwidth = bwidth;
			glyphstruct.bheight = bheight;
			glyphstruct.charcode = charcode;

			write(texfile, &glyphstruct.size, sizeof(int));
			write(texfile, &glyphstruct.advance, sizeof(float));
			write(texfile, &glyphstruct.dl_width, sizeof(int));
			write(texfile, &glyphstruct.dl_rows, sizeof(int));
			write(texfile, &glyphstruct.y_align, sizeof(float));
			write(texfile, &glyphstruct.left_align, sizeof(float));
			write(texfile, &glyphstruct.tex_x, sizeof(float));
			write(texfile, &glyphstruct.tex_y, sizeof(float));
			write(texfile, &glyphstruct.bwidth, sizeof(int));
			write(texfile, &glyphstruct.bheight, sizeof(int));
			write(texfile, &glyphstruct.charcode, sizeof(long long));

			if (glyphdata)
				write(texfile, glyphdata, glyphstruct.size);

			if (glyphdata)
			{
				free(glyphdata);
				glyphdata = NULL;
			}
			FT_Done_Glyph(glyph);
			k++;
			charcode = FT_Get_Next_Char(ftFace, charcode, &gindex);
		}

		printf("(%lld glyphs)\n", k);
		offset = lseek(texfile, 0, SEEK_CUR);
		lseek(texfile, backup, SEEK_SET);
		write(texfile, &offset, sizeof(unsigned long long));
		write(texfile, &k, sizeof(long long));
		lseek(texfile, offset, SEEK_SET);
		k = 0;
	}
	FT_Done_Face(ftFace);
	FT_Done_FreeType(ftLib);
	lseek(texfile, 0, SEEK_SET);
	s_len = offset;
	d_len = compressBound(s_len);
	d_l = d_len;
	unsigned long long cB = d_len;
	source = (unsigned char*)malloc(s_len);
	dest = (unsigned char*)malloc(d_len);
	if (source && dest)
	{
		printf("compressing file...\n");
		k = read(texfile, source, s_len);
		if (k == s_len)
		{
			if (compress2(dest, &d_l, source, s_len, 9) == Z_OK)
			{
				ftruncate(texfile, 0);
				lseek(texfile, 0, SEEK_SET);
				write(texfile, &cB, sizeof(unsigned long long));
				k = write(texfile, dest, d_l);
				if (k == d_l)
					printf("finished. successfully created \"%s\".\n", texfilename);
			}
		}

	}
	close(texfile);
	return;
}

int main(int argc, char* argv[])
{
	if (argc == 4)
	{
		process_file(argv[1], atoi(argv[2]), atoi(argv[3]));
		return EXIT_SUCCESS;
	}
	else
	{
		printf("specify a fontfile and fontsize range.\n");
		return EXIT_SUCCESS;
	}

	return 0;
}

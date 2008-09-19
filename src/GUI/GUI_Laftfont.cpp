#include <fcntl.h>
#include <wchar.h>
#include <stdarg.h>
#include <png.h>
#include <zlib.h>
#define GL_GLEXT_PROTOTYPES
#define GLX_GLXEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>
#include <GL/glx.h>
#include <GL/glxext.h>

#include "GUI_Laftfont.h"

GUI_Laftfont::GUI_Laftfont(unsigned short maxCodePoints)
{
	maxCP = maxCodePoints;
	listsFont = 0;
	glyph_m = NULL;
	return;
}

GUI_Laftfont::~GUI_Laftfont(void)
{
//	caused segfault on linux (fc8 x86_64 with mesa software renderer)
//	glDeleteLists(listsFont, maxCP);
	if (glyph_m) delete[] glyph_m;
	return;
}

void GUI_Laftfont::initLists(std::string &filename, unsigned short height)
{
	int			k		= 0;
	long long		num_glyphs	= 0;
	unsigned int*		textures	= NULL;
	unsigned char*		glyphdata	= NULL;
	struct gl_glyph		glyphstruct;
	int			texfile		= -1;
	unsigned long long	offset		= 0;
	unsigned char*		source		= NULL;
	unsigned char*		dest		= NULL;
	unsigned long long	s_len		= 0;
	unsigned long long	d_len		= 0;
	unsigned long long	o		= 0;
	unsigned long		d_l		= 0;

	if (filename.empty()) return;

	#ifdef WIN32
		texfile = open(filename.c_str(), O_RDONLY|O_BINARY, 0400);
	#else
		texfile = open(filename.c_str(), O_RDONLY, 0400);
	#endif
	if (texfile < 0)
	{
		return;
	}

	read(texfile, &d_len, sizeof(unsigned long long));
	d_l = (unsigned long)d_len;
	s_len = lseek(texfile, 0, SEEK_END) - sizeof(unsigned long long);
	lseek(texfile, sizeof(unsigned long long), SEEK_SET);

	dest = new unsigned char[(unsigned int)d_len];
	source = new unsigned char[(unsigned int)s_len];

	if (!dest || !source)
	{
		close(texfile);
		return;
	}

	read(texfile, source, (unsigned int)s_len);

	if (uncompress(dest, &d_l, source, (unsigned long)s_len) != Z_OK)
	{
		close(texfile);
		delete[] source;
		delete[] dest;
		return;
	}
	close(texfile);
	delete[] source;

	memcpy(&min_height, dest+o, sizeof(int));
	o += sizeof(int);
	memcpy(&max_height, dest+o, sizeof(int));
	o += sizeof(int);
	memcpy(&offset, dest+o, sizeof(unsigned long long));
	o += sizeof(unsigned long long);
	memcpy(&num_glyphs, dest+o, sizeof(long long));
	o += sizeof(long long);

	if ((height > max_height) || (height < min_height))
	{
		delete[] dest;
		return;
	}

	listsFont = glGenLists(maxCP);
	if (listsFont == 0)
	{
		delete[] dest;
		return;
	}

	for (int i = min_height; i <= max_height; i++)
	{
		if (i == height) break;
		o = offset;
		memcpy(&offset, dest+o, sizeof(unsigned long long));
		o += sizeof(unsigned long long);
		memcpy(&num_glyphs, dest+o, sizeof(long long));
		o += sizeof(long long);
	}

	glyph_m = new glyph_metric[maxCP];
	if (!glyph_m)
	{
		delete[] dest;
		return;
	}

	memset(glyph_m, 0, maxCP*sizeof(glyph_metric));

	textures = new unsigned int[(unsigned int)num_glyphs];
	if (!textures)
	{
		delete[] dest;
		return;
	}

    glEnable(GL_TEXTURE_2D);

	glGenTextures((GLsizei)num_glyphs, textures);

	for (k = 0; k < num_glyphs; k++)
	{
		memcpy(&glyphstruct.size, dest+o, sizeof(int));
		o += sizeof(int);
		memcpy(&glyphstruct.advance, dest+o, sizeof(float));
		o += sizeof(float);
		memcpy(&glyphstruct.dl_width, dest+o, sizeof(int));
		o += sizeof(int);
		memcpy(&glyphstruct.dl_rows, dest+o, sizeof(int));
		o += sizeof(int);
		memcpy(&glyphstruct.y_align, dest+o, sizeof(float));
		o += sizeof(float);
		memcpy(&glyphstruct.left_align, dest+o, sizeof(float));
		o += sizeof(float);
		memcpy(&glyphstruct.tex_x, dest+o, sizeof(float));
		o += sizeof(float);
		memcpy(&glyphstruct.tex_y, dest+o, sizeof(float));
		o += sizeof(float);
		memcpy(&glyphstruct.bwidth, dest+o, sizeof(int));
		o += sizeof(int);
		memcpy(&glyphstruct.bheight, dest+o, sizeof(int));
		o += sizeof(int);
		memcpy(&glyphstruct.charcode, dest+o, sizeof(long long));
		o += sizeof(long long);

		glyphdata = new unsigned char[glyphstruct.size];
		if (!glyphdata)
		{
			delete[] dest;
			return;
		}

		memcpy(glyphdata, dest+o, glyphstruct.size);
		o += glyphstruct.size;

		if ((glyphstruct.charcode >= maxCP) || (glyphstruct.charcode < 0))
		{
			delete[] glyphdata;
			glyphdata = NULL;
			continue;
		}

		glyph_m[glyphstruct.charcode].height = glyphstruct.dl_rows;
		glyph_m[glyphstruct.charcode].width = (int)glyphstruct.advance;

		glBindTexture(GL_TEXTURE_2D, textures[k]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, glyphstruct.bwidth, glyphstruct.bheight, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, glyphdata);

		glNewList((GLuint)(listsFont + glyphstruct.charcode), GL_COMPILE);
			glBindTexture(GL_TEXTURE_2D, textures[k]);
			glTranslatef(glyphstruct.left_align, glyphstruct.y_align, 0.0f);
			glBegin(GL_TRIANGLE_STRIP);
				glTexCoord2f(glyphstruct.tex_x, glyphstruct.tex_y);	glVertex2i(glyphstruct.dl_width, 0);
				glTexCoord2f(glyphstruct.tex_x, 0.0f);			glVertex2i(glyphstruct.dl_width, glyphstruct.dl_rows);
				glTexCoord2f(0.0f, glyphstruct.tex_y);			glVertex2i(0, 0);
				glTexCoord2f(0.0f, 0.0f);				glVertex2i(0, glyphstruct.dl_rows);
			glEnd();
			glTranslatef(glyphstruct.advance-glyphstruct.left_align, -glyphstruct.y_align, 0.0f);
		glEndList();

		delete[] glyphdata;
		glyphdata = NULL;
	}
	delete[] textures;
	textures = NULL;
	delete[] dest;
    glDisable(GL_TEXTURE_2D);
	return;
}

void GUI_Laftfont::printgl(int x, int y, int z, const wchar_t *fmt, ...)
{
	wchar_t			text[2048];
	va_list			ap;
	size_t			str_len;

	if ((!listsFont) || (!fmt)) return;

	va_start(ap, fmt);
	#ifdef WIN32
		_vsnwprintf(text, 2048, fmt, ap);
	#else
		vswprintf(text, 2048, fmt, ap);
	#endif
	va_end(ap);
	str_len = wcslen(text);

	// should be in modelview mode
	glPushMatrix();
	glLoadIdentity();
	glEnable(GL_TEXTURE_2D);
	glListBase(listsFont);
	glTranslatef((float)x, (float)y, (float)z);
	glScalef(1.0f, -1.0f, 1.0f);
	#ifdef WIN32
		glCallLists((GLsizei)(str_len), GL_UNSIGNED_SHORT, text);
	#else
		glCallLists((GLsizei)(str_len), GL_UNSIGNED_INT, text);
	#endif
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();

	return;
}

void GUI_Laftfont::printgl(int x, int y, int z, std::wstring& text)
{
	printgl(x, y, z, L"%ls", text.c_str());
	return;
}

void GUI_Laftfont::printgl(int x, int y, int z, std::string& text)
{
    std::wstring t(text.begin(), text.end());
	printgl(x, y, z, L"%ls", t.c_str());
	return;
}

void GUI_Laftfont::printgl(int x, int y, int z, const char *fmt, ...)
{
	char			text[2048];
	va_list			ap;
	size_t			str_len;

	if ((!listsFont) || (!fmt)) return;

	va_start(ap, fmt);
	snprintf(text, 2048, fmt, ap);
	va_end(ap);
	str_len = strlen(text);

	// should be in modelview mode
	glPushMatrix();
	glLoadIdentity();
	glEnable(GL_TEXTURE_2D);
	glListBase(listsFont);
	glTranslatef((float)x, (float)y, (float)z);
	glScalef(1.0f, -1.0f, 1.0f);
	glCallLists((GLsizei)(str_len), GL_BYTE, text);
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();
	return;
}

int GUI_Laftfont::textHeight(std::wstring& text)
{
	int h = 0;

	if (glyph_m)
	for (std::wstring::iterator i = text.begin(); i != text.end(); i++)
	{
		if ((*i > maxCP) || (*i < 0)) continue;
		h = (glyph_m[*i].height > h)?glyph_m[*i].height:h;
	}

	return h;
}

int GUI_Laftfont::textHeight(std::string& text)
{
	std::wstring t(text.begin(), text.end());
	return textHeight(t);
}

int GUI_Laftfont::textWidth(unsigned short codepoint)
{
	int w = 0;

	if (glyph_m)
	{
		if (codepoint > maxCP) return 0;
		w = glyph_m[codepoint].width;
	}
	return w;
}

int GUI_Laftfont::textWidth(std::wstring& text)
{
	int w = 0;

	if (glyph_m)
	for (std::wstring::iterator i = text.begin(); i != text.end(); i++)
	{
		if ((*i > maxCP) || (*i < 0)) continue;
		w += glyph_m[*i].width;
	}
	return w;
}

int GUI_Laftfont::textWidth(std::string& text)
{
	std::wstring t(text.begin(), text.end());
	return textWidth(t);
}


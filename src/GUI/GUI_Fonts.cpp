/*
 * Copyright (c) 2007, Laminar Research.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include "GUI_Fonts.h"
#include "GUI_GraphState.h"
#include "GUI_Resources.h"
#include <math.h>
#include "AssertUtils.h"
#include "MathUtils.h"
#include "TexUtils.h"
#include "BitmapUtils.h"

#if APL
	#include <OpenGL/gl.h>
#else
	#include <GL/gl.h>
#endif

#include "GUI_Fonts.h"
#include "GUI_Unicode.h"

#include <ft2build.h>
#include FT_FREETYPE_H

// This provides a gamma correction factor that makes fonts "darker".  Basically smaller
// numbers make darker fonts, and 1.0f gives you exactly what the "real font" looks like.
// Small fonts can look a bit stronger with a gamma of around 0.6 - for larger fonts, use
// normal 1.0
#define	FONT_GAMMA			0.6

#define X_SCALE 1.0
#define Y_SCALE 1.0

// These pre-defines control font creation and don't really need to be altered.
#define FM_DEFAULT_FACE_INDEX		 0
#define FM_DEFAULT_TEX_WIDTH	  1024
#define FM_DEVICE_RES_H			    72
#define FM_DEVICE_RES_V			    72
#define FM_PIX_PADDING				 4

struct	OGL_char_info {

	int		pixel_bounds[4];	// Integer pixel coords in the bitmap where we stored this guy!  Stored in absolute pixels, not ratio
								// because we may rescale the tex on the fly.

	float	dx;					// How much do we move our char position to the left and up from the base line location to draw.
	float	dy;					// (Obvious being the lower left corner of the bitmap on the baseline and left at the cursor pos.)
								// This means that we form a rect with the lower left corner on the baseline left side and move it
								// by dx, dy.

	float	advance_x;			// How much do we move to the right after this char.  The char's width, not the same as bitmap width.

	FT_Error	status;

};

inline int FIXED1616(float f) { return f * 65536.0f; }

typedef	hash_map<UTF32, OGL_char_info>	OGL_char_map;

class	TT_font_info {
public:

						TT_font_info();
	void				clear(const string& inPath, float inSize);
	float				require_char(UTF32 inChar, float s);					// returns advance-dist
	void				sync_tex(void);
	float				draw_char(UTF32 inChar, float x, float y, float s);		// returns advance-dist!

	// Sort this out!
	GLuint				tex_ref;

//private:

	int					tex_height;
	int					tex_dirty;
	int					cur_row_x;			// Coords where we will insert the next item.
	int					cur_row_y;
	int					cur_row_max;		// tallest bitmap jammed in the current row
	float				line_height;		// total distance from one baseline to the next
	float				line_descent;		// distance from the baseline to the bottom of all drawing
	float				line_ascent;		// distance from the baseline to the top of all drawing

	OGL_char_map		char_map;
	unsigned char *		image_mem;

	FT_Face				face;
	GUI_Resource		file;
};

static	FT_Library			sLibrary = NULL;

inline int get_pow2(int n)
{
	int r = 1;
	while(r < n) r <<= 1;
	return r;
}

static void		ProcessBitmapSection(
					unsigned int					inSrcWidth,
					unsigned int					inSrcHeight,
					unsigned char*					inSrcData)
{
	unsigned char kTable[256];
	for(int n = 0; n < 256; ++n)
	{
		float r = (float) n / 255.0f;
		kTable[n] = powf(r,FONT_GAMMA) * 255.0f;
	}
	long n = inSrcWidth * inSrcHeight;
	while(n--)
	{
		*inSrcData = kTable[*inSrcData];
		++inSrcData;
	}

}
static void		CopyBitmapSection(
					unsigned int					inSrcWidth,
					unsigned int					inSrcHeight,
					const unsigned char*			inSrcData,
					unsigned int					inDstLeft,
					unsigned int					inDstTop,
					unsigned int					inDstWidth,
					unsigned char*					inDstData)
{
	long    dst_nr = inDstWidth - inSrcWidth;
	inDstData += (inDstTop * inDstWidth + inDstLeft);

	while (inSrcHeight--)
	{
		long ctr = inSrcWidth;
		while (ctr--)
		{
			*inDstData++ = *inSrcData++;
		}
		inDstData += dst_nr;
	}
}

TT_font_info::TT_font_info()
{
	tex_ref=0;
	image_mem = NULL;
	tex_height = 0;
	tex_dirty = 0;
	file = NULL;
	face = NULL;
}

void TT_font_info::clear(const string& inPath, float inSize)
{
	FT_Error err;

	if(sLibrary == NULL)
	{
		err = FT_Init_FreeType(&sLibrary);
		Assert(err == 0);
	}

	if(tex_ref != 0)	glDeleteTextures(1,&tex_ref);
	tex_ref = 0;

	tex_height = 0;
	tex_dirty = 0;
	char_map.clear();

	if(image_mem) free(image_mem);
	image_mem = NULL;

	if(face != NULL)	FT_Done_Face(face);
	face = NULL;

	if(file)	GUI_UnloadResource(file);
	file = NULL;

	cur_row_x = cur_row_y = cur_row_max = 0;

	file = GUI_LoadResource(inPath.c_str());
	DebugAssert(file);

	err = FT_New_Memory_Face(sLibrary, (const FT_Byte*)GUI_GetResourceBegin(file), GUI_GetResourceEnd(file)-GUI_GetResourceBegin(file), FM_DEFAULT_FACE_INDEX, &face);
	DebugAssert(err==0);

	err = FT_Set_Char_Size(face,0,inSize * 64.0f, FM_DEVICE_RES_H, FM_DEVICE_RES_V);
	DebugAssert(err==0);

	FT_Matrix scaler;
	scaler.xx = FIXED1616(X_SCALE);	scaler.xy = FIXED1616(0.0f);
	scaler.yx = FIXED1616(0.0f);	scaler.yy = FIXED1616(Y_SCALE);
	FT_Set_Transform(face, &scaler, NULL);

	line_height = (float) inSize * (float) face->height / (float) face->units_per_EM;
	line_descent = (float) inSize * (float) -face->descender / (float) face->units_per_EM;
	line_ascent = (float) inSize * (float) face->ascender / (float) face->units_per_EM;

}

float TT_font_info::require_char(UTF32 inChar, float s)
{
	if(inChar == '\t') return 0.0f;

	OGL_char_map::iterator i = char_map.find(inChar);
	if (i != char_map.end())	return i->second.advance_x * s;

	FT_Error err;

	err = FT_Load_Char(face, inChar, FT_LOAD_RENDER|FT_LOAD_TARGET_LIGHT);
	if (err != 0)
	{
		OGL_char_info	info;
		info.pixel_bounds[0] = 0;
		info.pixel_bounds[1] = 0;
		info.pixel_bounds[2] = 0;
		info.pixel_bounds[3] = 0;
		info.dx = 0.0f;
		info.dy = 0.0f;
		info.advance_x = 0.0f;
		info.status = err;
		char_map[inChar] = info;
		return 0.0f;
	}

	FT_GlyphSlot	glyph = face->glyph;

	DebugAssert(!(glyph->metrics.width % 64));
	DebugAssert(!(glyph->metrics.height % 64));
	int width = glyph->bitmap.width;
	int height = glyph->bitmap.rows;

	if (cur_row_x + width > FM_DEFAULT_TEX_WIDTH)
	{
		cur_row_y += cur_row_max;
		cur_row_x = 0;
		cur_row_max = 0;
	}

	cur_row_max = intmax2(cur_row_max,height);

	if (cur_row_y + cur_row_max > tex_height)
	{
		int desired_height = get_pow2(tex_height + cur_row_max);
		if(tex_height == 0)
		{
			DebugAssert(image_mem == NULL);
			tex_height = desired_height;
			image_mem = (unsigned char*) malloc(FM_DEFAULT_TEX_WIDTH * tex_height);
			memset(image_mem,0,FM_DEFAULT_TEX_WIDTH * tex_height);
		}
		else
		{
			int old_height = tex_height;
			unsigned char * old_mem = image_mem;
			DebugAssert(image_mem != NULL);
			tex_height = desired_height;
			image_mem = (unsigned char*) malloc(FM_DEFAULT_TEX_WIDTH * tex_height);
			memset(image_mem + FM_DEFAULT_TEX_WIDTH * old_height,0,FM_DEFAULT_TEX_WIDTH * (tex_height - old_height));
			memcpy(image_mem, old_mem, FM_DEFAULT_TEX_WIDTH * old_height);
			free(old_mem);
		}
	}

	OGL_char_info	info;
	info.pixel_bounds[0] = cur_row_x;
	info.pixel_bounds[1] = cur_row_y;
	info.pixel_bounds[2] = cur_row_x + width;
	info.pixel_bounds[3] = cur_row_y + height;
	info.dx			= (float) glyph->metrics.horiBearingX * X_SCALE / 64.0f;
	info.dy			= (float) (glyph->metrics.horiBearingY - glyph->metrics.height) * Y_SCALE / 64.0f;
	info.advance_x	= (float) glyph->metrics.horiAdvance * X_SCALE / 64.0f;
	info.status = 0;
	char_map[inChar] = info;

	ProcessBitmapSection(width,height,glyph->bitmap.buffer);
	CopyBitmapSection(width, height, glyph->bitmap.buffer,
							cur_row_x, cur_row_y, FM_DEFAULT_TEX_WIDTH, image_mem);
	tex_dirty = 1;
	cur_row_x += width;
	return info.advance_x * s;
}

void TT_font_info::sync_tex(void)
{
	if (tex_dirty)
	{
		tex_dirty = 0;

		ImageInfo info;
		info.data = image_mem;
		info.width = FM_DEFAULT_TEX_WIDTH;
		info.height = tex_height;
		info.pad = 0;
		info.channels = 1;
		if(tex_ref==0)
			glGenTextures(1,&tex_ref);		
		bool ok = LoadTextureFromImage(info, tex_ref, 0, NULL,NULL,NULL,NULL);
		DebugAssert(ok);
	}
}

float TT_font_info::draw_char(UTF32 inChar, float x, float y, float s)
{
	if(inChar == '\t') return 0.0f;

	OGL_char_map::iterator i = char_map.find(inChar);
	if(i==char_map.end()) return 0.0f;

	OGL_char_info * info = &i->second;
	if(info->status != 0) return 0.0f;

	float	scale_tex_x = 1.0f / (float) FM_DEFAULT_TEX_WIDTH;
	float	scale_tex_y = 1.0f / (float) tex_height;

	float s1 = info->pixel_bounds[0] * scale_tex_x;
	float t1 = info->pixel_bounds[1] * scale_tex_y;
	float s2 = info->pixel_bounds[2] * scale_tex_x;
	float t2 = info->pixel_bounds[3] * scale_tex_y;

	// Floor?  Yeah...turns out if you draw a quad with nearest-neighbor on Radeon X1000 hw (and a bunch of others), you don't get perfect
	// mapping if you aren't pixel-aligned.  So make sure we are!
	float x1 = floorf(x + info->dx);
	float x2 = x1 + s * (float) (info->pixel_bounds[2] - info->pixel_bounds[0]);
	float y1 = floorf(y + info->dy);
	float y2 = y1 + s * (float) (info->pixel_bounds[3] - info->pixel_bounds[1]);

	// Splat the character down where it belongs now
	glTexCoord2f(s1,t2);	glVertex2f(x1, y1);		// NOT AN ERROR!!!  BEN SAY: TTF renders upside down (that is, Y=0 is the top)
	glTexCoord2f(s1,t1);	glVertex2f(x1, y2);		// So flip on the fly or else everything is upside down.
	glTexCoord2f(s2,t1);	glVertex2f(x2, y2);
	glTexCoord2f(s2,t2);	glVertex2f(x2, y1);

	return info->advance_x;
}

#define tt_dim font_Max
typedef int tt_t;

static TT_font_info*	tt_font [tt_dim]={0};
static const float		tt_sizes[tt_dim]={12.0, 10.0};																// Fonts are both 10.5, but if they weren't from the same family, we might need to compensate!
static const char *		tt_names[tt_dim]={"sans.ttf", "sans.ttf" };

inline void TT_reset_font(tt_t c){
	if(c==tt_dim){
		for(int i=0;i<tt_dim;i++)TT_reset_font((tt_t)i);
		return;}
	if(	tt_font [c]){
		tt_font [c]->clear(
		tt_names[c],
		tt_sizes[c]);}}

inline void TT_establish_font(tt_t c){
	if(	tt_font[c]==NULL){
		tt_font[c]=new TT_font_info();
		TT_reset_font(c);}}

float GUI_MeasureRange(int inFontID, const char * inStart, const char * inEnd)
{
	if(inStart == inEnd) return 0.0f;
	TT_establish_font(inFontID);
	float str_width = 0;
	const UTF8 * p = (const UTF8 *) inStart;
	const UTF8 * e = (const UTF8 *) inEnd;
	while(p < e){
		UTF32 c = UTF8_decode(p);
		str_width += tt_font[inFontID]->require_char(c,1.0f);
		p=UTF8_next(p, (const UTF8 *)inEnd);
	}
	return str_width;
}

int GUI_FitForward(int inFontID, const char* inStart, const char* inEnd,float width)
{
	if(inStart == inEnd) return 0;
	TT_establish_font(inFontID);
	float so_far = 0.0;
	const UTF8 * p = (const UTF8 *) inStart;
	const UTF8 * e = (const UTF8 *) inEnd;
	while(p < e)
	{
		UTF32 c = UTF8_decode(p);
		so_far += tt_font[inFontID]->require_char(c,1.0f);
		if(so_far > width)
			break;
		p=UTF8_next(p, (const UTF8 *)inEnd);
	}
	return (const char *) p - inStart;
}

int GUI_FitReverse(int inFontID, const char* inStart, const char* inEnd,float width)
{
	if(inStart == inEnd) return 0;
	TT_establish_font(inFontID);
	float so_far = 0.0;
	const UTF8 * s = (const UTF8 *) inStart;
	const UTF8 * e = (const UTF8 *) inEnd;
	if(s== e) return 0;
	--e;
	e = UTF8_align(e);
	int ct = 0;
	while(e >= s)
	{
		UTF32 c = UTF8_decode(e);
		so_far += tt_font[inFontID]->require_char(c,1.0f);
		if(so_far > width)
			break;
		e = UTF8_prev(e);
		++ct;
	}
	return ct;
}

float	GUI_GetLineHeight(int inFontID)
{
	TT_establish_font(inFontID);
	return tt_font[inFontID]->line_height;
}

float	GUI_GetLineDescent(int inFontID)
{
	TT_establish_font(inFontID);
	return tt_font[inFontID]->line_descent;
}

float	GUI_GetLineAscent(int inFontID)
{
	TT_establish_font(inFontID);
	return tt_font[inFontID]->line_ascent;
}

void	GUI_TruncateText(
				string&							ioText,
				int								inFontID,
				float							inSpace)
{
	if (ioText.empty()) return;

	int chars = GUI_FitForward(inFontID, &*ioText.begin(), &*ioText.end(), inSpace);
	if (chars == ioText.length()) return;
	if (chars < 0) { ioText.clear(); return; }
	ioText.erase(chars);
	if (ioText.length() > 0)	ioText[ioText.length()-1] = '.';
	if (ioText.length() > 1)	ioText[ioText.length()-2] = '.';
	if (ioText.length() > 2)	ioText[ioText.length()-3] = '.';

}

void	GUI_FontDraw(
				GUI_GraphState *				inState,
				int 							inFontID,
				const float						color[4],	//	4-part color, featuring alpha.
				float							inX,
				float							inY,
				const char *					inString,
				int								inAlign)
{
	TT_establish_font(inFontID);
	TT_font_info * f = tt_font[inFontID];
	GUI_FontDrawScaled(inState,inFontID,color,
		inX,
		inY - f->line_descent,
		inX,
		inY - f->line_descent  + f->line_height,
		inString, inString + strlen(inString),
		inAlign);
}


void	GUI_FontDrawScaled(
				GUI_GraphState *				inState,
				int 							inFontID,
				const float						color[4],	//	4-part color, featuring alpha.
				float							inLeft,
				float							inBottom,
				float							inRight,
				float							inTop,
				const char *					inStart,
				const char *					inEnd,
				int								inAlign)
{
	float l, b, scale;

	TT_establish_font(inFontID);
	TT_font_info * inFont = tt_font[inFontID];
	inState->SetState(0,1,0,  1, 1, 0, 0);
	inState->BindTex(inFont->tex_ref, 0);

	// Determine how much to scale the font to make it right the height
	scale = (inTop - inBottom) / inFont->line_height;

	// Why do we always measure?  Measuring forces the chars to be established.  That way, when we "sync" the font,
	// all chars have a slot allocated.  Without this, we would have to resync the text per char, which would be
	// a big thrash for OpenGL.

	float width = GUI_MeasureRange(inFontID, inStart, inEnd) * (inTop - inBottom) / inFont->line_height;

	if (inAlign == align_Left)
	{
		// Left align is the simple case - just start on the left.  We don't need inRIght and we don't pre-measure.
		l = inLeft;
	}
	else
	{
		// Justification cases.  First we measure the string and see how it compares to the box.  Set up left as needed.
		float slop;
		// Slop is the amount of room we have horizontally between how much we need
		// and how much space they've given up. A negative number means we'll go out
		// of our bounds.  This is a dummy and not used ifi we're left aligned.
		slop = inRight - inLeft - width;
		if(inAlign == align_Right)
			l = inLeft + slop;
		else
			// NOTE: Due to a possible driver bug, if slop is an even number (which has a remainder
			// when divded by 2 in an ALIGN_CENTER) the font gets skewed by 1 pixel. So we hack
			// around it by rounding down and accepting the 0.5pixel error.
			l = inLeft + floorf((slop / 2.0));
	}

	b = floorf(inBottom + inFont->line_descent * scale);
	inFont->sync_tex();
	// Set the color and let's start drawing
	glColor4fv(color);
	glBegin(GL_QUADS);

	for(const UTF8 * c = (const UTF8 *) inStart; c < (const UTF8 *) inEnd; c=UTF8_next(c, (const UTF8 *)inEnd))
	{
		float w = inFont->draw_char(UTF8_decode(c),l,b, scale);
		l += w;
	}
	glEnd();
}

#include "FontMgr.h"

#if IBM
#include <windows.h>
#include <GL/gl.h>
#else
#include <gl.h>
#endif

/*
	BEN SEZ: Mac has 3 ways to get true type fonts:
		A single true type font: .ttf file - this works in the lib and works on Win/Lin.
		Suitcase file is a resource/datafork file with MANY fonts inside - both .ttf and otherwise!
		.dfont is a datafork-only version of a suitcase.
		
		This lib will open suitcases but will NOT open .dfont files.
		
		Use "fondu" (http://fondu.sourceforge.net/) to extract .ttfs from suitcase or .dfont files.
*/

/*
 * Debugging stuff
 */
#if _DEBUG
#include <assert.h>
#define OGL_ERROR(expression) expression; if(glGetError()){assert(false);}
#else
#define OGL_ERROR(expression) expression;
#endif


/*
 * Font Manager constants
 */
#define FM_FONT_SIZE_INTERVAL		10
#define FM_DEFAULT_FACE_INDEX		 0
#define FM_DEFAULT_TEX_WIDTH	  1024
#define FM_BASE_ASCII_RANGE		    32
#define FM_PEAK_ASCII_RANGE		   254
#define FM_DEVICE_RES_H			    96
#define FM_DEVICE_RES_V			    96
#define FM_PIX_PADDING				 4

/*
 * This struct stores the information for a given instance of a font.
 * The metrics of each character in the font are stored in arrays in
 * the struct for easy access from the font class. Clients are given
 * an opaque pointer to the struct that stores the info for their class
 * so we use that for draw operations.
 */
struct	FontInfo_t {
	int		nRefCnt;
	int		tex_id;				// OpenGL texture ID for bitmap
	int		tex_width;			// Stores the texture width in px
	int 	tex_height;			// Stores the texture height in px
	int		tex_font_size;		// Stores the texture font size in px (NOT NECESSARILY WHAT'S DRAWN)
	float	line_height;		// Stores the line height of the font in px
	float	line_descent;		// Stores dist from bottom of the line to the baseline
	float	line_ascent;		// Stores dist from baseline to  tallest letter, ish
	int		char_left_x[256];	// Enter the start pixel location of the character
	int		char_right_x[256];	// Enter the end pixel location of the character
	int		char_bot_y[256];	// Enter the bottom pixel location of the character
	int		char_top_y[256];	// Enter the top pixel location of the character
	int		dx[256];			// Horizontal glyph adjustment
	int		dy[256];			// Vertical glyph adjustment
	int		advance[256];		// Distance between the right side of this glyph and the beginning of the next.
								// NOTE: This is NOT kerning!
};

 /*
 * Function pointer initialization
 *
 * These are necessary because sometimes users need to abstract OpenGL
 * to do their own texture management. The ones below are the defaults, but
 * users can always pass in their own as we do in X-Plane.
 */
static void BindTexture			(int in1,	int in2)	{glBindTexture(in1, in2);}
static void GenerateTextures	(int in1,	int* in2)	{glGenTextures(in1, (GLuint*)in2);}

/*
 * Helper function to figure out which textured font size
 * would be the closest to what the client requested so that
 * we can reuse textures.
 *
 * require_exact is a flag ... the client may not want rounding.
 * If the client has a lot of fonts and zooms or scales them, 
 * rounding is a non-issue.  But for a main UI font at fixed resolution
 * exact resolution looks a lot better.
 *
 */
static int FindNearestFontSize(int inSize, bool require_exact)
{
	// If we want an exact size, well, nothing to do!
	if (require_exact) return inSize;
	
	// What this does is it finds the nearest font size
	// that is an interval of 10. It rounds up by adding half of
	// the interval we're looking for and then it basically subtracts
	// out a value to bring it to the nearest interval. The end result 
	// is a round and truncate.
	int rounded = (inSize + (FM_FONT_SIZE_INTERVAL/2));
	int nearest = rounded - (rounded % FM_FONT_SIZE_INTERVAL);

	// Enfore at least the smallest interval
	if(nearest < FM_FONT_SIZE_INTERVAL)
		nearest = FM_FONT_SIZE_INTERVAL;

	return nearest;
}

/************************
	FontMgr Class Implementation
	*******************************/
FontMgr::FontMgr()
{
	// Setup our function pointers to point to the default OpenGL functions
	mFunctions.BindTexture			= BindTexture;
	mFunctions.GenerateTextures		= GenerateTextures;

	// Start an instance of the FreeType Library
	assert(!FT_Init_FreeType(&mLibrary));
}

void	FontMgr::InstallCallbacks(
				FontFuncs*						inFuncs)
{
	// Setup our function pointers to point to user defined functions
	// Anything they don't define, we'll set to our defaults.
	if(inFuncs->BindTexture)
		mFunctions.BindTexture		= inFuncs->BindTexture;
	else
		mFunctions.BindTexture		= BindTexture;
	if(inFuncs->GenerateTextures)
		mFunctions.GenerateTextures = inFuncs->GenerateTextures;
	else
		mFunctions.GenerateTextures = GenerateTextures;
}

FontMgr::~FontMgr()
{
	// Nuke all of the fonts we have loaded
	while(!mTexMap.empty())
		UnloadFont((*(mTexMap.begin())).second);

	// Shutdown freetype now
	assert(!FT_Done_FreeType(mLibrary));
}

FontHandle FontMgr::LoadFont(const char* inFontPath, unsigned int inSizePx, bool require_exact)
{
	// First see if we've used this font before
	if(mTexMap.count(inFontPath))
	{
		// If we have, now let's see if we've already loaded a size that's "close enough"
		// for the user. Right now we only do font sizes in intervals of 10 and it'll round
		// to the nearest 10th. So if the user asks for a 13px and we have a 10px, he'll get
		// the 10px. If he doesn't have a 10px, he'll make his own.
		for(TextureMap_t::iterator it = mTexMap.begin(); it != mTexMap.end(); ++it)
		{
			if((*it).second->tex_font_size == (FindNearestFontSize(inSizePx, require_exact)) &&
			   strcmp((*it).first.c_str(), inFontPath) == 0)
			{
				// Increment our reference count
				(*it).second->nRefCnt++;
				return (*it).second;
			}
		}
	}

	// We need to make them a texture but first we have to round their size to a 10px interval
	inSizePx = FindNearestFontSize(inSizePx, require_exact);

	FT_Face			face;
	FontHandle		info = new FontInfo_t;
	FT_GlyphSlot	glyph;

	assert(!FT_New_Face(mLibrary, inFontPath, FM_DEFAULT_FACE_INDEX, &face));
	assert(!FT_Set_Char_Size(face, 0, inSizePx * 64, FM_DEVICE_RES_H, FM_DEVICE_RES_V));

	CalcTexSize(&face, &info->tex_height, &info->tex_width);
	// Ben sez: use nearest neighbor for exact-size fonts...pixel accurate!  Use linear for scaled fonts....less artifacts when we scale.
	info->tex_id = CreateTexture(info->tex_width, info->tex_height, require_exact ? GL_NEAREST : GL_LINEAR);
	info->tex_font_size = inSizePx;

	float maxHeight = 0;
	float x_off	  = 0, y_off	= 0;

	for(int n = FM_BASE_ASCII_RANGE; n <= FM_PEAK_ASCII_RANGE; ++n)
	{
		// If there's no glyph for this font, we have nothing to do
		if(FT_Load_Char(face, n, FT_LOAD_RENDER))
			continue;

		// Merely a shortcut from typing this out everytime
		glyph = face->glyph;

		float width  = glyph->metrics.width  / 64.0;
		float height = glyph->metrics.height / 64.0;

		assert(!(glyph->metrics.width % 64));
		assert(!(glyph->metrics.height % 64));

		if(height > maxHeight)
			maxHeight = height;

		if(x_off + width + FM_PIX_PADDING > info->tex_width)
		{
			x_off = 0;
			if(y_off + maxHeight > info->tex_height)
				y_off = 0;
			else
				y_off += (maxHeight+FM_PIX_PADDING);
		}

		// Set all of the coordinates of the glyph
		info->char_left_x[n]	= x_off;
		info->char_right_x[n]	= x_off + width;
		info->char_top_y[n]		= y_off + height;
		info->char_bot_y[n]		= y_off;
		info->dx[n]				= glyph->metrics.horiBearingX;
		info->dy[n]				= glyph->metrics.horiBearingY - glyph->metrics.height;
		info->advance[n]		= glyph->metrics.horiAdvance;

		// Set the unpacking alignment
		glPushClientAttrib(GL_CLIENT_PIXEL_STORE_BIT);
        glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		// Copy the glyph bitmap into OpenGL memory.
		OGL_ERROR(glTexSubImage2D(GL_TEXTURE_2D, 0, x_off, y_off, width, height,
						GL_ALPHA, GL_UNSIGNED_BYTE, glyph->bitmap.buffer))

		// Return our states back to the way they were
		glPopClientAttrib();
		
		if(x_off + width + FM_PIX_PADDING<= info->tex_width)
			x_off += width + FM_PIX_PADDING;
		else
		{
			if(y_off + maxHeight < info->tex_height)
				y_off += (maxHeight+FM_PIX_PADDING);
			else
				y_off = maxHeight;

			x_off = width + FM_PIX_PADDING;
		}
	}

	// By setting the lineheight, we can determine how tall we can stretch
	// our fonts later on give a certain height constraint.
	// NOTE: We use the max height because the FT function to determine this
	// is very unreliable.
//	info->line_height = maxHeight;
	info->line_height = (float) inSizePx * (float) face->height / (float) face->units_per_EM;
	info->line_descent = (float) inSizePx * (float) -face->descender / (float) face->units_per_EM;
	info->line_ascent = (float) inSizePx * (float) face->ascender / (float) face->units_per_EM;
	// Set the reference count
	info->nRefCnt = 1;

	mTexMap.insert(TextureMap_t::value_type(inFontPath, info));
	return info;
}

void FontMgr::UnloadFont(FontHandle inFont)
{	
	if(!inFont)
		return;

	// Decrement our reference count.
	// If we're the last to use this font
	// then we cleanup the resources.
	if(--(inFont->nRefCnt) > 0)
		return;

	// Delete the texture from OpenGL memory
	OGL_ERROR(glDeleteTextures(1, (GLuint*)&(inFont->tex_id)))

	// Remove it from our map
	for(TextureMap_t::iterator it = mTexMap.begin(); it != mTexMap.end(); ++it)
	{
		if(inFont == (*it).second)
		{
			mTexMap.erase(it);
			break;
		}
	}

	// Delete the struct's memory
	delete inFont;
}

int FontMgr::CreateTexture(unsigned int inWidth, unsigned int inHeight, int inFiltering)
{
	int textureId;

	// Create the texture number that we'll be tied to
	mFunctions.GenerateTextures(1, &textureId);

	// Now we bind to it
	mFunctions.BindTexture(GL_TEXTURE_2D, textureId);

	unsigned char* textureData = new unsigned char[inHeight * inWidth];

	// We have to 0 out the memory or we'll get artifacts when the glyphs are cut
	memset(textureData, 0, inHeight * inWidth);

	// Create some texture memory in OpenGL for this font
	OGL_ERROR(glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, inWidth,
					inHeight, 0, GL_ALPHA, GL_UNSIGNED_BYTE, textureData))

	//This is important or else we'll only see a white screen!
	OGL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, inFiltering))
	OGL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, inFiltering))

	delete[] textureData;

	return textureId;
}

void FontMgr::CalcTexSize(
				FT_Face*						inFace,
				int*							outHeight,
				int*							outWidth)
{
	if(!outHeight || !outWidth)
		return;

	int curRowWidth = 0, maxHeight = 0, nRows = 1;

	// We iterate through every letter that we care about
	// and record the tallest letter and the width of all
	// letters combined as well as the widest letter.
	for(int n = FM_BASE_ASCII_RANGE; n <= FM_PEAK_ASCII_RANGE; ++n)
	{
		if(FT_Load_Char(*inFace, n, FT_LOAD_RENDER))
			continue;

		// If the bitmap doesn't exist, we have nothing to do
		if(!(*inFace)->glyph->bitmap.rows || !(*inFace)->glyph->bitmap.width)
			continue;

		int width  = (*inFace)->glyph->metrics.width / 64.0;
		int height = (*inFace)->glyph->metrics.height / 64.0;

		// If any of the sizes are 0, we have nothing to do
		if(!(*inFace)->glyph->metrics.width || !(*inFace)->glyph->metrics.height)
			continue;

		if(height > maxHeight)
			maxHeight = height;

		// Add this to our sum with some room for padding as well
		curRowWidth += width + FM_PIX_PADDING;

		// If it doesn't fit, we add a new row and put it there
		if(curRowWidth > FM_DEFAULT_TEX_WIDTH)
		{
			nRows++;
			curRowWidth = width + FM_PIX_PADDING;
		}
	}

	// We can assume our width is 1024px right now. This can be inefficient
	// but for our purposes, I doubt it'll be a problem because our textures
	// are fairly cheap resource-wise.
	*outWidth = FM_DEFAULT_TEX_WIDTH;

	// Our height is how many rows we need times the height of each row
	*outHeight = nRows * (maxHeight+FM_PIX_PADDING);

	// Now round our height up to the nearest power of 2
	int roundedHeight = 0;
	for(int n = 0; n < 11; ++n)
	{
		if(roundedHeight > *outHeight)
			break;

		roundedHeight = 1L << n;
	}

	*outHeight = roundedHeight;
}

void FontMgr::DisplayTexture(
				FontHandle						inFont)
{
	if(!inFont)
		return;
	
	mFunctions.BindTexture(GL_TEXTURE_2D, inFont->tex_id);
	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_QUADS);
		glTexCoord2f(0.0, 1.0);		glVertex2f(0, 0);
		glTexCoord2f(0.0, 0.0);		glVertex2f(0, inFont->tex_height);
		glTexCoord2f(1.0, 0.0);		glVertex2f(inFont->tex_width, inFont->tex_height);
		glTexCoord2f(1.0, 1.0);		glVertex2f(inFont->tex_width, 0);
	glEnd();
}

void	FontMgr::DrawString(	
				FontHandle						inFont,
				float							color[4],	//	4-part color, featuring alpha.
				float							inX,
				float							inY,
				const char *					inString)
{	
	if (!inFont || !inString) return;
	DrawRange(
			inFont, color, 
			inX, 
			inY - inFont->line_descent, 
			inX, 								// Ben sez: "inRight" of box is NOT used in left-align case.  OKAY to pass anything here.
			inY - inFont->line_descent + inFont->line_height, 
			inString, inString + strlen(inString), 
			ALIGN_LEFT);
}


void FontMgr::DrawRange(	
			FontHandle						inFont,
			float							color[4],	//	4-part color, featuring alpha.
			float							inLeft,
			float							inBottom,
			float							inRight,
			float							inTop,
			const char *					inStart,
			const char *					inEnd,
			FontAlign_t						fontAlign)
{
	if(!inFont || !inStart)
		return;
	float l, b, r, t, scale;

	mFunctions.BindTexture(GL_TEXTURE_2D, inFont->tex_id);

	// Determine how much to scale the font to make it right the height
	scale = (inTop - inBottom) / inFont->line_height;

	if (fontAlign == ALIGN_LEFT)
	{
		// Left align is the simple case - just start on the left.  We don't need inRIght and we don't pre-measure.
		l = inLeft;
	} 
	else 
	{
		// Justification cases.  First we measure the string and see how it compares to the box.  Set up left as needed.
		float width, slop;
		width = MeasureRange(inFont, inTop - inBottom, inStart, inEnd);
		// Slop is the amount of room we have horizontally between how much we need
		// and how much space they've given up. A negative number means we'll go out
		// of our bounds.  This is a dummy and not used ifi we're left aligned.
		slop = (inRight - inLeft) - width;
		if(fontAlign == ALIGN_RIGHT)
			l = inLeft + slop;
		else
			l = inLeft + (slop / 2.0);
	}

	b = inBottom + inFont->line_descent * scale;

	// Set the color and let's start drawing
	glColor4fv(color);
	glBegin(GL_QUADS);

	for (const char * p = inStart; p < inEnd; ++p)
	{
		float s1 = (inFont->char_left_x[*p] - (FM_PIX_PADDING / 2)) / (float)inFont->tex_width;
		float s2 = (inFont->char_right_x[*p] + (FM_PIX_PADDING / 2)) / (float)inFont->tex_width;
		float t1 = (inFont->char_bot_y[*p] - (FM_PIX_PADDING / 2)) / (float)inFont->tex_height;
		float t2 = (inFont->char_top_y[*p] + (FM_PIX_PADDING / 2)) / (float)inFont->tex_height;
		// Our top is the bottom plus the height of the Glyph and adjusted for scale;
		t = inBottom + (inFont->char_top_y[*p] - inFont->char_bot_y[*p]) * scale;
		// Our right side is the left plus the width of the Glyph and adjusted for scale
		r =  l + (inFont->char_right_x[*p] - inFont->char_left_x[*p]) * scale;

		// Now we adjust the positioning of the letter in both the X and Y directions
		// remembering to scale the adjustment properly
		l += ((inFont->dx[*p] / 64.0) - (FM_PIX_PADDING / 2)) * scale;
		r += ((inFont->dx[*p] / 64.0) + (FM_PIX_PADDING / 2)) * scale;
		b += ((inFont->dy[*p] / 64.0) - (FM_PIX_PADDING / 2)) * scale;
		t += ((inFont->dy[*p] / 64.0) + (FM_PIX_PADDING / 2)) * scale;

		// Splat the character down where it belongs now
		glTexCoord2f(s1,t2);	glVertex2f(l, b);
		glTexCoord2f(s1,t1);	glVertex2f(l, t);
		glTexCoord2f(s2,t1);	glVertex2f(r, t);
		glTexCoord2f(s2,t2);	glVertex2f(r, b);

		// Now we have to undo our dx for the next time around
		l -= ((inFont->dx[*p] / 64.0) - (FM_PIX_PADDING / 2)) * scale;
		r -= ((inFont->dx[*p] / 64.0) + (FM_PIX_PADDING / 2)) * scale;
		b -= ((inFont->dy[*p] / 64.0) - (FM_PIX_PADDING / 2)) * scale;
		t -= ((inFont->dy[*p] / 64.0) + (FM_PIX_PADDING / 2)) * scale;

		// Now we set our l position for the next time...advance already
		// includes the width of the font to "advance" it for next time.
		l += ((inFont->advance[*p] / 64.0) * scale);
	}
	glEnd();
}

float FontMgr::MeasureString(
				FontHandle						inFont,
				float							inHeight,
				const char *					inString)
{
	return MeasureRange(inFont, inHeight, inString, inString + strlen(inString));
}

float FontMgr::MeasureRange(
				FontHandle						inFont,
				float							inHeight,
				const char *					inStart,
				const char *					inEnd)
{
	float scale = inHeight / inFont->line_height;
	float width = 0;

	for (const char * p = inStart; p < inEnd; ++p)
	{
		width += ((inFont->advance[*p] / 64.0) * scale) ;
	}

	return width;
}

int		FontMgr::FitForward(
				FontHandle						inFont,
				float							inHeight,
				const char *					inCharStart,
				const char *					inCharEnd,
				float							inSpace)
{
	float so_far = 0.0;
	float scale = inHeight / inFont->line_height;	
	int total = 0;
	for (const char * c = inCharStart; c < inCharEnd; ++c)
	{
		if (so_far > inSpace) return total;
		++total;
		so_far += ((inFont->advance[*c] / 64.0) * scale) ;
	}
	return total;
}
	
int		FontMgr::FitReverse(
				FontHandle						inFont,
				float							inHeight,
				const char *					inCharStart,
				const char *					inCharEnd,
				float							inSpace)
{
	float so_far = 0.0;
	float scale = inHeight / inFont->line_height;	
	int total = 0;
	for (const char * c = inCharEnd-1; c >= inCharStart; --c)
	{
		if (so_far > inSpace) return total;
		++total;
		so_far += ((inFont->advance[*c] / 64.0) * scale) ;
	}
	return total;
}


float FontMgr::GetLineHeight(
				FontHandle						inFont,
				unsigned int					inFontSizePx)
{
	if(!inFont)
		return 0;
	else
		return ((float) inFontSizePx / (float) inFont->tex_font_size) * inFont->line_height;
}

float	FontMgr::GetLineDescent(
				FontHandle						inFont,
				unsigned int					inFontSizePx)
{
	if(!inFont)
		return 0;
	else
		return ((float) inFontSizePx / (float) inFont->tex_font_size) * inFont->line_descent;
}

float	FontMgr::GetLineAscent(
				FontHandle						inFont,
				unsigned int					inFontSizePx)
{
	if(!inFont)
		return 0;
	else
		return ((float) inFontSizePx / (float) inFont->tex_font_size) * inFont->line_ascent;
}



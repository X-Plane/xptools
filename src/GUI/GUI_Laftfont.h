#ifndef GUI_LAFTFONT_H
#define GUI_LAFTFONT_H

#include <string>

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

struct glyph_metric
{
	int	width;
	int	height;
};

class GUI_Laftfont
{
	public:
		GUI_Laftfont(unsigned short maxCodePoints = 256);
		virtual ~GUI_Laftfont(void);

		void	initLists(std::string &filename, unsigned short height);
		void	printgl(int x, int y, int z, std::wstring &text);
		void	printgl(int x, int y, int z, const wchar_t *fmt, ...);
		int	textHeight(std::wstring &text);
		int textHeight(std::string& text);
		int	textWidth(std::wstring &text);
		int	textWidth(std::string &text);
		int	textWidth(unsigned short codepoint);
		void printgl(int x, int y, int z, std::string& text);
		void printgl(int x, int y, int z, const char *fmt, ...);
	private:
		unsigned int	listsFont;
		unsigned short	maxCP;
		int		min_height;
		int		max_height;
		glyph_metric	*glyph_m;
};

#endif // GUI_LAFTFONT_H

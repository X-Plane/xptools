/*
 * Copyright (c) 2004, Laminar Research.
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
#include "OE_TexMgr.h"
#include "XUtils.h"
#include "TexUtils.h"
#include "OE_Notify.h"
#include "OE_Msgs.h"

struct	TexInfo_t {
	int		width;
	int		height;
	string	fileName;
	GLenum	id;
};

map<string, TexInfo_t >		gDayTextures;
map<string, TexInfo_t >		gNightTextures;

GLenum		FindTexture(const string& inName, bool inNight, int * outWidth, int * outHeight)
{
	if (inName.empty()) return 0;

	string 	name(inName);
	StripPathCP(name);
	StringToUpper(name);
	if (inNight)
	{
		map<string, TexInfo_t >::iterator i = gNightTextures.find(name);
		if (i == gNightTextures.end()) return 0;
		if (outWidth) *outWidth = i->second.width;
		if (outHeight) *outHeight = i->second.height;
		return i->second.id;
	} else {
		map<string, TexInfo_t >::iterator i = gDayTextures.find(name);
		if (i == gDayTextures.end()) return 0;
		if (outWidth) *outWidth = i->second.width;
		if (outHeight) *outHeight = i->second.height;
		return i->second.id;
	}
}

void		AccumTexture(const string& inFileName)
{
	int 	w, h;
	static	GLenum	gCounter = 1000;
	bool	lit = HasExtNoCase(inFileName, "LIT.bmp") || HasExtNoCase(inFileName, "LIT.png");
	map<string, TexInfo_t >&	texDB = lit ? gNightTextures : gDayTextures;
	string	shortName = inFileName;
	if (!HasExtNoCase(inFileName, ".bmp") && !HasExtNoCase(inFileName, ".png"))
		return;

	if (lit)
		shortName = shortName.substr(0, shortName.length() - 7);
	else
		shortName = shortName.substr(0, shortName.length() - 4);

	StripPathCP(shortName);

	StringToUpper(shortName);

	for (map<string, TexInfo_t >::iterator i = texDB.begin();	 i != texDB.end(); ++i)
	{
		if (i->second.fileName == inFileName)
		{
			LoadTextureFromFile(inFileName.c_str(), i->second.id, (!lit ? tex_MagentaAlpha : 0) + tex_Linear + tex_Mipmap, NULL, NULL, NULL, NULL);
			OE_Notifiable::Notify(catagory_Texture, msg_TexLoaded, NULL);
			return;
		}
	}

	GLenum	texID = gCounter++;
	if (LoadTextureFromFile(inFileName.c_str(), texID, (!lit ? tex_MagentaAlpha : 0) + tex_Linear + tex_Mipmap, &w, &h, NULL, NULL))
	{
		TexInfo_t	ti;
		ti.width = w;
		ti.height = h;
		ti.id = texID;
		ti.fileName = inFileName;
		texDB.insert(map<string, TexInfo_t >::value_type(
				shortName, ti));
		OE_Notifiable::Notify(catagory_Texture, msg_TexLoaded, NULL);
	}
}

void		ReloadTexture(const string& inName)
{
	string	name(inName);
	StringToUpper(name);
	map<string, TexInfo_t >::iterator i;
	i = gDayTextures.find(name);
	if (i != gDayTextures.end())
		LoadTextureFromFile(i->second.fileName.c_str(), i->second.id, tex_MagentaAlpha + tex_Linear + tex_Mipmap, &i->second.width, &i->second.height, NULL, NULL);

	i = gNightTextures.find(name);
	if (i != gNightTextures.end())
		LoadTextureFromFile(i->second.fileName.c_str(), i->second.id, tex_Linear + tex_Mipmap, &i->second.width, &i->second.height, NULL, NULL);

	OE_Notifiable::Notify(catagory_Texture, msg_TexLoaded, NULL);
}


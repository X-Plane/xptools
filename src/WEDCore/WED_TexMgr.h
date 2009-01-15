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

#ifndef WED_TexMgr_H
#define WED_TexMgr_H

#include "ITexMgr.h"

class WED_TexMgr : public virtual ITexMgr {
public:

						WED_TexMgr(const string& package);

	virtual	TexRef		LookupTexture(const char * path, bool is_absolute, int flags);

	virtual	int			GetTexID(TexRef ref);
	virtual	void		GetTexInfo(
								TexRef	ref,
								int *	vis_x,
								int *	vis_y,
								int *	act_x,
								int *	act_y,
								int *	org_x,
								int *	org_y);

private:

	struct	TexInfo {
		int			tex_id;
		int			vis_x;
		int			vis_y;
		int			act_x;
		int			act_y;
		int			org_x;
		int			org_y;
	};

	typedef map<string,TexInfo *>	TexMap;

	TexMap mTexes;

	string	mPackage;

	TexInfo *	LoadTexture(const char * path, bool is_absolute, int flags);

};

#endif /* WED_TexMgr_H */

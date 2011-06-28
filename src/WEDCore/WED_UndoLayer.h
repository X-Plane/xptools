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

#ifndef WED_UNDOLAYER_H
#define WED_UNDOLAYER_H

class	WED_Archive;
class	WED_Buffer;
class	WED_FastBuffer;
class	WED_FastBufferGroup;
class	WED_Persistent;

#define 	UNDO_DISCARD	((WED_UndoLayer *) -1)


class	WED_UndoLayer {
public:

				WED_UndoLayer(WED_Archive * inArchive, const string& inName, const char * file, int line);
				~WED_UndoLayer(void);

		void 	ObjectCreated(WED_Persistent * inObject);
		void	ObjectChanged(WED_Persistent * inObject, int change_kind);
		void	ObjectDestroyed(WED_Persistent * inObject);

		void	Execute(void);

		string	GetName(void) const { return mName; }
		const char * GetFile(void) const { return mFile; }
		int		GetLine(void) const { return mLine; }
		bool	Empty(void) const { return mObjects.empty(); }

		int		GetChangeMask(void) { return mChangeMask; }

private:

	enum LayerOp {
			op_Created,
			op_Changed,
			op_Destroyed
	};

	struct ObjInfo {
		LayerOp				op;
		int					id;
		const char *		the_class;
		WED_FastBuffer *	buffer;
	};

	typedef hash_map<int, ObjInfo>		ObjInfoMap;

	ObjInfoMap				mObjects;
	WED_Archive *			mArchive;
	string					mName;
	const char *			mFile;
	int						mLine;
	int						mChangeMask;
	WED_FastBufferGroup *	mStorage;

	// Things we do not allow
	WED_UndoLayer();
	WED_UndoLayer(const WED_UndoLayer&);
	WED_UndoLayer& operator=(const WED_UndoLayer&);

};

#endif /* WED_UNDOLAYER_H */

/*
 * Copyright (c) 2011, mroe.
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

#ifndef WED_NWLINKADAPTER_H
#define WED_NWLINKADAPTER_H

#include "GUI_Timer.h"
#include "GUI_Listener.h"

class WED_Server;
class WED_Archive;
class WED_Persistent;

class	WED_NWLinkAdapter :public GUI_Listener ,public GUI_Timer  {
public:

				WED_NWLinkAdapter(WED_Server * inServer,WED_Archive * inArchive);
	virtual	~WED_NWLinkAdapter();

				void 	ObjectCreated(WED_Persistent * inObject);
				void	ObjectChanged(WED_Persistent * inObject, int chgkind);
				void	ObjectDestroyed(WED_Persistent * inObject);
				void    DoReadData();
				void    DoSendData();

				void   	TimerFired();

				void	ReceiveMessage(	GUI_Broadcaster * inSrc,intptr_t inMsg,intptr_t inParam);


private:
				//ToDo:mroe privat for now,should merged with the other changeflags
				// found in WED_Archive.h ,WED_Thing.h
				enum {
					wed_Change_Any_Ex   = 32,
				};

				bool			 mTimerIsStarted;

				set<int>		 mDelList;
	map<WED_Persistent *,int>	 mObjCache;

			WED_Archive *		 mArchive;
			WED_Server * 		 mServer;
};

#endif // WED_NWLINKADAPTER_H

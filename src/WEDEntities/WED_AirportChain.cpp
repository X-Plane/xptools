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

#include "WED_AirportChain.h"
#include "WED_AirportNode.h"
#include "IODefs.h"
#include "SQLutils.h"
#include "WED_EnumSystem.h"
#include "WED_Errors.h"
#include "AptDefs.h"

DEFINE_PERSISTENT(WED_AirportChain)

WED_AirportChain::WED_AirportChain(WED_Archive * a, int i) : WED_GISChain(a,i),
	closed(0),
	lines(this,"Line Attributes","","","Line Attributes"),
	lights(this,"Light Attributes","","","Light Attributes")
{
}

WED_AirportChain::~WED_AirportChain()
{
}

void WED_AirportChain::CopyFrom(const WED_AirportChain * rhs)
{
	WED_GISChain::CopyFrom(rhs);
	StateChanged();
	closed = rhs->closed;
}

void	WED_AirportChain::SetClosed(int closure)
{
	if (closed != closure)
	{
		StateChanged();
		closed = closure;
	}
}
/*
IGISPoint *	WED_AirportChain::SplitSide   (int n)
{
	int c = GetNumSides();
	
	if (n > c) return NULL;
	
	Bezier2		b;
	Segment2	s;
	
	if (GetSide(n, s, b))
	{
		WED_AirportNode * node = WED_AirportNode::CreateTyped(GetArchive());
		
		Bezier2	b1, b2;
		b.partition(b1,b2,0.5);
		
		node->SetLocation(b2.p1);
		node->SetControlHandleHi(b2.c1);
		
		node->SetParent(this, n+1);		
		return node;

	} else {

		WED_AirportNode * node = WED_AirportNode::CreateTyped(GetArchive());
		
		node->SetLocation(s.midpoint(0.5));
		node->SetControlHandleHi(s.midpoint(0.5));
		
		node->SetParent(this, n+1);
		return node;
	}
	
}
*/
bool	 WED_AirportChain::IsClosed	(void	) const
{
	return closed;
}


void 			WED_AirportChain::ReadFrom(IOReader * reader)
{
	WED_GISChain::ReadFrom(reader);
	reader->ReadInt(closed);
}

void 			WED_AirportChain::WriteTo(IOWriter * writer)
{
	WED_GISChain::WriteTo(writer);
	writer->WriteInt(closed);
}

void			WED_AirportChain::FromDB(sqlite3 * db, const map<int,int>& mapping)
{
	WED_GISChain::FromDB(db, mapping);
	
	sql_command	cmd(db,"SELECT closed FROM WED_airportchains WHERE id=@i;","@i");
	
	sql_row1<int>						key(GetID());
	sql_row1<int>						cl;
	
	int err = cmd.simple_exec(key, cl);
	if (err != SQLITE_DONE)	WED_ThrowPrintf("%s (%d)",sqlite3_errmsg(db),err);
	
	closed = cl.a; 	
}

void			WED_AirportChain::ToDB(sqlite3 * db)
{
	WED_GISChain::ToDB(db);

	int err;
	sql_command	write_me(db,"INSERT OR REPLACE INTO WED_airportchains values(@id,@closed);","@id,@closed");
	sql_row2 <int,int>bindings(GetID(),closed);
	err = write_me.simple_exec(bindings);
	if(err != SQLITE_DONE)		WED_ThrowPrintf("%s (%d)",sqlite3_errmsg(db),err);
}


void	WED_AirportChain::Import(const AptMarking_t& x, void (* print_func)(void *, const char *, ...), void * ref)
{
	SetName(x.name);
}

void	WED_AirportChain::Export(		 AptMarking_t& x) const
{
	GetName(x.name);
}

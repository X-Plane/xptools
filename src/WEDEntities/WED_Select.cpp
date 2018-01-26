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

#include "WED_Select.h"
#include "IODefs.h"
#include "WED_XMLWriter.h"
#include "WED_Errors.h"
#include "WED_Messages.h"

DEFINE_PERSISTENT(WED_Select)

WED_Select::WED_Select(WED_Archive * parent, int id) :
	WED_Thing(parent, id)
{
}

WED_Select::~WED_Select()
{
}

void WED_Select::CopyFrom(const WED_Select * rhs)
{
	DebugAssert(!"We should not be copying selection objects.");
	WED_Thing::CopyFrom(rhs);
	StateChanged();
	mSelected = rhs->mSelected;
}

bool 			WED_Select::ReadFrom(IOReader * reader)
{
	bool r = WED_Thing::ReadFrom(reader);
	int n,id;
	reader->ReadInt(n);
	mSelected.clear();
	while(n--)
	{
		reader->ReadInt(id);
		mSelected.insert(id);
	}
	return r;
}

void 			WED_Select::WriteTo(IOWriter * writer)
{
	WED_Thing::WriteTo(writer);
	writer->WriteInt(mSelected.size());
	for (set<int>::iterator i = mSelected.begin(); i != mSelected.end(); ++i)
		writer->WriteInt(*i);

}

void		WED_Select::AddExtraXML(WED_XMLElement * obj)
{
	WED_XMLElement * selection = obj->add_sub_element("selection");
	for(set<int>::iterator i = mSelected.begin(); i != mSelected.end(); ++i)
	{
		WED_XMLElement * sel = selection->add_sub_element("sel");
		sel->add_attr_int("id",*i);
	}
}

void		WED_Select::StartElement(
								WED_XMLReader * reader,
								const XML_Char *	name,
								const XML_Char **	atts)
{
	if(strcasecmp(name,"sel")==0)
	{
		const XML_Char * id = get_att("id",atts);
		if(!id)
			reader->FailWithError("No id");
		else
			mSelected.insert(atoi(id));
	}
	else if(strcasecmp(name,"selection")==0)
	{
		mSelected.clear();
	}
	else
		WED_Thing::StartElement(reader,name,atts);
}

void		WED_Select::EndElement(void) { }
void		WED_Select::PopHandler(void) { }


#pragma mark -

bool		WED_Select::IsSelected(ISelectable * iwho) const
{
	return mSelected.count(iwho->GetSelectionID()) > 0;
}

void		WED_Select::Select(ISelectable * iwho)
{
	int id = iwho->GetSelectionID();

	if (mSelected.size() != 1 || mSelected.count(id) == 0)
	{
		StateChanged(wed_Change_Selection);
		mSelected.clear();
		mSelected.insert(id);
	}
}

void		WED_Select::Clear(void)
{
	if (!mSelected.empty())
	{
		StateChanged(wed_Change_Selection);
		mSelected.clear();
	}
}

void		WED_Select::Toggle(ISelectable * iwho)
{
	int id = iwho->GetSelectionID();

	StateChanged(wed_Change_Selection);
	if (mSelected.count(id) > 0)
		mSelected.erase(id);
	else
		mSelected.insert(id);
}

void		WED_Select::Insert(ISelectable * iwho)
{
	int id = iwho->GetSelectionID();

	if (mSelected.count(id) == 0)
	{
		StateChanged(wed_Change_Selection);
		mSelected.insert(id);
	}
}

void WED_Select::Insert(const set<ISelectable*>& sel)
{
	Insert(sel.begin(), sel.end());
}

void WED_Select::Insert(const set<ISelectable*>::const_iterator& begin, const set<ISelectable*>::const_iterator& end)
{
	for (set<ISelectable*>::const_iterator itr = begin; itr != end; ++itr)
	{
		Insert(*itr);
	}
}

void WED_Select::Insert(const vector<ISelectable*>& sel)
{
	Insert(sel.begin(), sel.end());
}

void WED_Select::Insert(const vector<ISelectable*>::const_iterator& begin, const vector<ISelectable*>::const_iterator& end)
{
	for (vector<ISelectable*>::const_iterator itr = begin; itr != end; ++itr)
	{
		Insert(*itr);
	}
}

void		WED_Select::Erase(ISelectable * iwho)
{
	int id = iwho->GetSelectionID();

	if (mSelected.count(id) > 0)
	{
		StateChanged(wed_Change_Selection);
		mSelected.erase(id);
	}
}

int				WED_Select::GetSelectionCount(void) const
{
	return mSelected.size();
}

void			WED_Select::GetSelectionSet(set<ISelectable *>& sel) const
{
	sel.clear();
	for (set<int>::const_iterator i = mSelected.begin(); i != mSelected.end(); ++i)
		sel.insert(FetchPeer(*i));
}

void			WED_Select::GetSelectionVector(vector<ISelectable *>& sel) const
{
	sel.clear();
	if (mSelected.empty()) return;
	sel.reserve(mSelected.size());
	for (set<int>::const_iterator i = mSelected.begin(); i != mSelected.end(); ++i)
		sel.push_back(FetchPeer(*i));
}

ISelectable *		WED_Select::GetNthSelection(int n) const
{
	DebugAssert(n >= 0 && n < mSelected.size());
	if (n < 0) return NULL;
	set<int>::const_iterator i = mSelected.begin();
	while (n > 0 && i != mSelected.end())
	{
		--n;
		++i;
	}
	if (i == mSelected.end()) return NULL;
	return FetchPeer(*i);
}

int			WED_Select::IterateSelectionOr(int (* func)(ISelectable * who, void * ref), void * ref) const
{
	int n = 0;
	for (set<int>::const_iterator i = mSelected.begin(); i != mSelected.end(); ++i)
		if ((n=func(FetchPeer(*i), ref)) != 0)
			return n;
	return 0;
}

int			WED_Select::IterateSelectionAnd(int (* func)(ISelectable * who, void * ref), void * ref) const
{
	int n = 0;
	for (set<int>::const_iterator i = mSelected.begin(); i != mSelected.end(); ++i)
		if ((n=func(FetchPeer(*i), ref)) == 0)
			return n;
	return 1;
}

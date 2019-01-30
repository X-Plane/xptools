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

#include "WED_Thing.h"
#include "IODefs.h"
#include "WED_Errors.h"
#include "WED_XMLWriter.h"
#include <algorithm>

WED_Thing::WED_Thing(WED_Archive * parent, int id) :
	WED_Persistent(parent, id),
	type(this),
	name(this,PROP_Name("Name", XML_Name("hierarchy","name")),"unnamed entity")
{
	parent_id = 0;
	sources = 0;
}

WED_Thing::~WED_Thing()
{
}

void WED_Thing::CopyFrom(const WED_Thing * rhs)
{
	StateChanged();

	int nn = rhs->CountChildren();
	for (int n = 0; n < nn; ++n)
	{
		WED_Thing * child = rhs->GetNthChild(n);
		WED_Thing * new_child = dynamic_cast<WED_Thing *>(child->Clone());
		new_child->SetParent(this, n);
	}
	
	viewer_id.clear();	      	// I am a clone.  No one is REALLY watching me.

	sources = 0;
	for(auto s : rhs->child_id)
		if(s < 0)
		{
			child_id.push_back(s);
			sources++;
		}
													// But I am YET ANOTHER observer of my sources...
	for(int n = 0; n < sources; ++n)						// go register with my parent now!
	{
		WED_Thing * the_src = GetNthSource(n);
		the_src->AddViewer(GetID());
	}

	int pc = rhs->CountProperties();
	for (int p = 0; p < pc; ++p)
	{
		PropertyInfo_t i;
		PropertyVal_t v;
		rhs->GetNthPropertyInfo(p, i);
		if(i.can_edit && !i.synthetic)
		{
			rhs->GetNthProperty(p, v);
			this->SetNthProperty(p, v);
		}
	}
}

bool 			WED_Thing::ReadFrom(IOReader * reader)
{
	int ct;
	reader->ReadInt(parent_id);

	// Children and Sources
	reader->ReadInt(ct);
	child_id.resize(ct);
	for (int n = 0; n < ct; ++n)
	{
		int cid;
		reader->ReadInt(cid);
		child_id.push_back(cid);
		if(cid < 0) sources ++;
	}

	// Viewers
	viewer_id.clear();
	reader->ReadInt(ct);
	for(int n = 0; n < ct; ++n)
	{
		int vid;
		reader->ReadInt(vid);
		viewer_id.insert(vid);
//		if(find(viewer_id.begin(), viewer_id.end(), vid) == viewer_id.end())   // MMnote: Is this really needed ? We wrote that stuff ourselves, how can it be wrong ?
//			viewer_id.push_back(vid);
	}

	ReadPropsFrom(reader);
	return false;
}

void 			WED_Thing::WriteTo(IOWriter * writer)
{
	writer->WriteInt(parent_id);

	// Children and sources
	writer->WriteInt(child_id.size());
	for (int n = 0; n < child_id.size(); ++n)
		writer->WriteInt(child_id[n]);

	// Viewers
	writer->WriteInt(viewer_id.size());
	for(auto vid : viewer_id)
		writer->WriteInt(vid);

	WritePropsTo(writer);
}

void			WED_Thing::ToXML(WED_XMLElement * parent)
{
	WED_XMLElement * obj = parent->add_sub_element("object");
	
	obj->add_attr_c_str("class",this->GetClass());
	obj->add_attr_int("id",GetID());
	obj->add_attr_int("parent_id",parent_id);
	
	if(viewer_id.size())
	{
		WED_XMLElement * vwr = obj->add_sub_element("viewers");
		for(auto v : viewer_id)
		{
			WED_XMLElement * vi = vwr->add_sub_element("viewer");
			vi->add_attr_int("id",v);
		}
	}
//	if(child_id.size())  // would be nice to skip this, too. But wed-O-maker won't like that for now.
	{
		WED_XMLElement * chld = obj->add_sub_element("children");
		for(int n = 0; n < child_id.size(); ++n)
		{
			WED_XMLElement * c = chld->add_sub_element("child_src");
			c->add_attr_int("id",child_id[n]);
		}
	}
	WED_PropertyHelper::PropsToXML(obj);
	this->AddExtraXML(obj);
}

void	WED_Thing::FromXML(WED_XMLReader * reader, const XML_Char ** atts)
{
	const XML_Char ** a = atts;
	reader->PushHandler(this);
	const char * pid = get_att("parent_id",atts);
	if(!pid) reader->FailWithError("No parent ID");
	parent_id = atoi(pid);
	child_id.clear();
//	source_id.clear();
   sources = 0;
}

void		WED_Thing::StartElement(
								WED_XMLReader * reader,
								const XML_Char *	name,
								const XML_Char **	atts)
{
	if(strcasecmp(name,"viewer")==0)
	{
		const char * id = get_att("id",atts);
		if(!id)
			reader->FailWithError("no id");
		viewer_id.insert(atoi(id));
//		if(find(viewer_id.begin(), viewer_id.end(), atoi(id)) == viewer_id.end())   // MMnote: Is this really needed ? Granted, someone could have messed with the XML
//			viewer_id.push_back(atoi(id));
	} 
	else if(strcasecmp(name,"source")==0)
	{
		const char * id = get_att("id",atts);
		if(!id)
			reader->FailWithError("no id");
		child_id.push_back(-atoi(id));
		sources++;
	} 
	else if(strcasecmp(name,"child") == 0)
	{
		const char * id = get_att("id",atts);
		if(!id)
			reader->FailWithError("no id");
		child_id.push_back(atoi(id));
	}
	else if(strcasecmp(name,"child_src") == 0)
	{
		const char * id = get_att("id",atts);
		if(!id)
			reader->FailWithError("no id");
		child_id.push_back(atoi(id));
		if(atoi(id)<0) sources++;
	}
	else
		WED_PropertyHelper::StartElement(reader,name,atts);
}

void		WED_Thing::EndElement(void)
{
}

void		WED_Thing::PopHandler(void)
{
}

void	WED_Thing::PostChangeNotify(void)
{
}

int					WED_Thing::CountChildren(void) const
{
   // MMnote: do we want to discount sources ?
   // Really depends on how we want to use Sources ... if they're supoposed to be true fake children, include them
   
	return child_id.size() - sources;
}

WED_Thing *		WED_Thing::GetNthChild(int n) const
{
	if (child_id.empty())     // prevent SIGSEGV
		return NULL;
	else
	{	// MMnote: do we need to skip over the sources we may have amongst the children ?
		// Really depends on how we want to use Sources ... if they're supoposed to be true fake children, don't skip sources
		if(sources)
		{
			for(auto c : child_id)
			{
				if(c < 0) continue;
				n--;
				if (n < 0)
					return STATIC_CAST(WED_Thing,FetchPeer(c));
			}
			DebugAssert(0);  // couldn't find enough real children !
		}
		else	// cast path - don't scan all over for sources
			return STATIC_CAST(WED_Thing,FetchPeer(child_id[n]));
	}
}

WED_Thing *		WED_Thing::GetNamedChild(const string& s) const
{
	int c = CountChildren();
	for (int n = 0; n < c; ++n)
	{
		WED_Thing * t = GetNthChild(n);
		if (t)
		{
			string n;
			t->GetName(n);
			if (s==n)
				return t;
		}
	}
	return NULL;
}

int					WED_Thing::CountSources(void) const
{
	return sources;
}

WED_Thing *			WED_Thing::GetNthSource(int n) const
{
	DebugAssert(n < sources);
	for(auto s : child_id)
	{
		if(s >= 0) continue;
		n--;
		if(n < 0) 
			return STATIC_CAST(WED_Thing,FetchPeer(-s));
	}
	DebugAssert(n < 0);
	return NULL;
}

int					WED_Thing::CountViewers(void) const
{
	return viewer_id.size();
}

void WED_Thing::GetAllViewers(set<WED_Thing *>& out_viewers) const
{
	out_viewers.clear();
	for(auto i : viewer_id)
	{
		WED_Thing * v = STATIC_CAST(WED_Thing, FetchPeer(i));
		DebugAssert(v);
		if(v)
			out_viewers.insert(v);
	}
}


void	WED_Thing::GetName(string& n) const
{
	n = name.value;
}

void	WED_Thing::SetName(const string& n)
{
	name = n;
}

WED_Thing *		WED_Thing::GetParent(void) const
{
	return STATIC_CAST(WED_Thing,FetchPeer(parent_id));
}

void				WED_Thing::SetParent(WED_Thing * parent, int nth)
{
	StateChanged(wed_Change_Topology);
	WED_Thing * old_parent = STATIC_CAST(WED_Thing, FetchPeer(parent_id));
	if (old_parent) old_parent->RemoveChild(GetID());
	parent_id = parent ? parent->GetID() : 0;
	if (parent) parent->AddChild(GetID(),nth);
}

void				WED_Thing::AddSource(WED_Thing * src, int nth)
{
	DebugAssert(nth >= 0);
	DebugAssert(nth <= sources);
	StateChanged(wed_Change_Topology);
	child_id.insert(child_id.begin()+nth,-src->GetID());
	sources++;
	if(src->viewer_id.count(GetID())==0)
//	if(find(src->viewer_id.begin(), src->viewer_id.end(), GetID()) == src->viewer_id.end())
		src->AddViewer(GetID());
}

void				WED_Thing::RemoveSource(WED_Thing * src)
{
	auto k = find(child_id.begin(), child_id.end(), -src->GetID());
	DebugAssert(k != child_id.end());
	DebugAssert(src->viewer_id.count(GetID()) > 0);
//	DebugAssert(find(src->viewer_id.begin(), src->viewer_id.end(), GetID()) != src->viewer_id.end());
	StateChanged(wed_Change_Topology);

	while(k != child_id.end())
	{
		child_id.erase(k);
		sources--;
		k = find(child_id.begin(), child_id.end(), -src->GetID());
	}
	src->RemoveViewer(GetID());
}

void	WED_Thing::ReplaceSource(WED_Thing * old, WED_Thing * rep)
{
	int old_id = old->GetID();
	int new_id = rep->GetID();
	DebugAssert(old->viewer_id.count(GetID()) > 0);
	old->RemoveViewer(GetID());
	
	StateChanged();
	int subs =0;
	for(vector<int>::iterator s = child_id.begin(); s != child_id.end(); ++s)
	if(*s == -old_id)
	{
		++subs;
		*s = -new_id;
	}
	DebugAssert(subs > 0);
	if(rep->viewer_id.count(GetID()) == 0)
		rep->AddViewer(GetID());
}


int			WED_Thing::GetMyPosition(void) const
{
	WED_Thing * parent = STATIC_CAST(WED_Thing, FetchPeer(parent_id));
	if (!parent) return 0;
	auto i = find(parent->child_id.begin(), parent->child_id.end(), this->GetID());
	return distance(parent->child_id.begin(), i);
}

void				WED_Thing::AddChild(int id, int n)
{
	StateChanged(wed_Change_Topology);
	DebugAssert(n >= 0);
	DebugAssert(n <= child_id.size());
	auto i = find(child_id.begin(),child_id.end(),id);
	DebugAssert(i == child_id.end());
	child_id.insert(child_id.begin()+n,id);
}

void				WED_Thing::RemoveChild(int id)
{
	StateChanged(wed_Change_Topology);
	auto i = find(child_id.begin(),child_id.end(),id);
	DebugAssert(i != child_id.end());
	child_id.erase(i);
}

void		WED_Thing::AddViewer(int id)
{
	StateChanged(wed_Change_Topology);
	DebugAssert(viewer_id.count(id) == 0);
	viewer_id.insert(id);
}
	
void		WED_Thing::RemoveViewer(int id)
{
	StateChanged(wed_Change_Topology);
	DebugAssert(viewer_id.count(id) != 0);
	viewer_id.erase(id);
}


void		WED_Thing::PropEditCallback(int before)
{
	if (before)
		StateChanged(wed_Change_Properties);
}

int					WED_Thing::CountSubs(void)
{
	return CountChildren();
}

IPropertyObject *	WED_Thing::GetNthSub(int n)
{
	return GetNthChild(n);
}


int				WED_Thing::Array_Count (void )
{
	return CountChildren();
}

IBase *		WED_Thing::Array_GetNth(int n)
{
	return GetNthChild(n);
}

IBase *		WED_Thing::Directory_Find(const char * name)
{
	return GetNamedChild(name);
}

void	WED_Thing::__StartOperation(const char * op_name, const char * inFile, int inLine)
{
	__StartCommand(op_name, inFile, inLine);
}

void	WED_Thing::CommitOperation(void)
{
	CommitCommand();
}

void	WED_Thing::AbortOperation(void)
{
	AbortCommand();
}


void	WED_Thing::Validate(void)
{
	if(parent_id != 0)
	{
		WED_Thing * p = SAFE_CAST(WED_Thing,FetchPeer(parent_id));
		DebugAssert(p);
		auto me = find(p->child_id.begin(),p->child_id.end(), GetID());
		DebugAssert(me != p->child_id.end());     // parent pointing to thing no listing us as child
	}
	
	int found_src = 0;
	for(auto c : child_id)
	{
		WED_Thing * cc = SAFE_CAST(WED_Thing,FetchPeer(abs(c)));
		DebugAssert(cc);
		if(c >= 0)
			DebugAssert(cc->parent_id == GetID()); // child pointing at thing not listing us as parent
		else
		{
			found_src++;
			auto it = find(cc->viewer_id.begin(), cc->viewer_id.end(), GetID());
			DebugAssert(it != cc->viewer_id.end()); // source pointing at thing not listing us as viewer
		}
	}
	DebugAssert(found_src == sources);
	
	for(set<int>::iterator v = viewer_id.begin(); v != viewer_id.end(); ++v)
	{
		WED_Thing * vv = SAFE_CAST(WED_Thing,FetchPeer(*v));
		DebugAssert(vv);
		auto me = find(vv->child_id.begin(), vv->child_id.end(), -GetID());
		DebugAssert(me != vv->child_id.end());    // viewer pointing to thing not listing us as source
	}
}

#pragma mark -

WED_TypeField::WED_TypeField(WED_Thing * t) : WED_PropertyItem(t, "Class")
{
}


void		WED_TypeField::GetPropertyInfo(PropertyInfo_t& info)
{
	info.can_delete = false;
	info.can_edit = 0;
	info.prop_kind = prop_String;
	info.prop_name = "Class";
	info.synthetic = 0;
}

void		WED_TypeField::GetPropertyDict(PropertyDict_t& dict)
{
}

void		WED_TypeField::GetPropertyDictItem(int e, string& item)
{
}

void		WED_TypeField::GetProperty(PropertyVal_t& v) const
{
	v.prop_kind = prop_String;
	v.string_val = dynamic_cast<WED_Thing*>(mParent)->HumanReadableType();
}

void		WED_TypeField::SetProperty(const PropertyVal_t& val, WED_PropertyHelper * parent)
{
}

void 		WED_TypeField::ReadFrom(IOReader * reader)
{
}

void 		WED_TypeField::WriteTo(IOWriter * writer)
{
}

void		WED_TypeField::ToXML(WED_XMLElement * parent)
{
}

bool		WED_TypeField::WantsAttribute(const char * ele, const char * att_name, const char * att_value)
{
	return false;
}

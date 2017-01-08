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
	name(this,"Name", XML_Name("hierarchy","name"),"unnamed entity")
{
	parent_id = 0;
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
	
	viewer_id.clear();		// I am a clone.  No one is REALLY watching me.
	
	source_id = rhs->source_id;
	nn = CountSources();						// But I am YET ANOTHER observer of my sources...
	for(int n = 0; n < nn; ++n)						// go register with my parent now!
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

	// Children
	reader->ReadInt(ct);
	child_id.resize(ct);
	for (int n = 0; n < ct; ++n)
		reader->ReadInt(child_id[n]);

	// Viewers
	viewer_id.clear();
	reader->ReadInt(ct);
	for(int n = 0; n < ct; ++n)
	{
		int vid;
		reader->ReadInt(vid);
		viewer_id.insert(vid);
	}

	// Sources
	reader->ReadInt(ct);
	source_id.resize(ct);
	for(int n = 0; n < ct; ++n)
		reader->ReadInt(source_id[n]);

	ReadPropsFrom(reader);
	return false;
}

void 			WED_Thing::WriteTo(IOWriter * writer)
{
	int n;
	writer->WriteInt(parent_id);

	// Children
	writer->WriteInt(child_id.size());
	for (int n = 0; n < child_id.size(); ++n)
		writer->WriteInt(child_id[n]);

	// Viewers
	writer->WriteInt(viewer_id.size());
	for(set<int>::iterator vid = viewer_id.begin(); vid != viewer_id.end(); ++vid)
		writer->WriteInt(*vid);

	//Sources
	writer->WriteInt(source_id.size());
	for(int n = 0; n < source_id.size(); ++n)
		writer->WriteInt(source_id[n]);

	WritePropsTo(writer);
}

void			WED_Thing::ToXML(WED_XMLElement * parent)
{
	WED_XMLElement * obj = parent->add_sub_element("object");
	
	obj->add_attr_c_str("class",this->GetClass());
	obj->add_attr_int("id",GetID());
	obj->add_attr_int("parent_id",parent_id);
	
	WED_XMLElement * src = obj->add_sub_element("sources");
	for(int n = 0; n < source_id.size(); ++n)
	{
		WED_XMLElement * s = src->add_sub_element("source");
		s->add_attr_int("id",source_id[n]);
	}

	WED_XMLElement * vwr = obj->add_sub_element("viewers");
	for(set<int>::iterator v = viewer_id.begin(); v != viewer_id.end(); ++v)
	{
		WED_XMLElement * vi = vwr->add_sub_element("viewer");
		vi->add_attr_int("id",*v);
	}

	WED_XMLElement * chld = obj->add_sub_element("children");
	for(int n = 0; n < child_id.size(); ++n)
	{
		WED_XMLElement * c = chld->add_sub_element("child");
		c->add_attr_int("id",child_id[n]);
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
	source_id.clear();
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
	} 
	else if(strcasecmp(name,"source")==0)
	{
		const char * id = get_att("id",atts);
		if(!id)
			reader->FailWithError("no id");
		source_id.push_back(atoi(id));
	} 
	else if(strcasecmp(name,"child") == 0)
	{
		const char * id = get_att("id",atts);
		if(!id)
			reader->FailWithError("no id");
		child_id.push_back(atoi(id));
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
	return child_id.size();
}

WED_Thing *		WED_Thing::GetNthChild(int n) const
{
	if (child_id.empty())     // prevent SIGSEGV
		return NULL;
	else
		return STATIC_CAST(WED_Thing,FetchPeer(child_id[n]));
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
	return source_id.size();
}

WED_Thing *			WED_Thing::GetNthSource(int n) const
{
	return STATIC_CAST(WED_Thing,FetchPeer(source_id[n]));
}

int					WED_Thing::CountViewers(void) const
{
	return viewer_id.size();
}

void WED_Thing::GetAllViewers(set<WED_Thing *>& out_viewers) const
{
	out_viewers.clear();
	for(set<int>::iterator i = viewer_id.begin(); i != viewer_id.end(); ++i)
	{
		WED_Thing * v = STATIC_CAST(WED_Thing, FetchPeer(*i));
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
	DebugAssert(nth <= source_id.size());
	StateChanged(wed_Change_Topology);
	source_id.insert(source_id.begin()+nth,src->GetID());
	if(src->viewer_id.count(GetID())==0)
		src->AddViewer(GetID());
}

void				WED_Thing::RemoveSource(WED_Thing * src)
{
	vector<int>::iterator k = find(source_id.begin(), source_id.end(), src->GetID());
	DebugAssert(k != source_id.end());
	DebugAssert(src->viewer_id.count(GetID()) > 0);
	StateChanged(wed_Change_Topology);

	while(k != source_id.end())
	{
		source_id.erase(k);
		k = find(source_id.begin(), source_id.end(), src->GetID());
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
	for(vector<int>::iterator s = source_id.begin(); s != source_id.end(); ++s)
	if(*s == old_id)
	{
		++subs;
		*s = new_id;
	}
	DebugAssert(subs > 0);
	if(rep->viewer_id.count(GetID()) == 0)
		rep->AddViewer(GetID());
}


int			WED_Thing::GetMyPosition(void) const
{
	WED_Thing * parent = STATIC_CAST(WED_Thing, FetchPeer(parent_id));
	if (!parent) return 0;
	int n = 0;
	vector<int>::iterator i = find(parent->child_id.begin(), parent->child_id.end(), this->GetID());
	return distance(parent->child_id.begin(), i);
}

void				WED_Thing::AddChild(int id, int n)
{
	StateChanged(wed_Change_Topology);
	DebugAssert(n >= 0);
	DebugAssert(n <= child_id.size());
	vector<int>::iterator i = find(child_id.begin(),child_id.end(),id);
	DebugAssert(i == child_id.end());
	child_id.insert(child_id.begin()+n,id);
}

void				WED_Thing::RemoveChild(int id)
{
	StateChanged(wed_Change_Topology);
	vector<int>::iterator i = find(child_id.begin(),child_id.end(),id);
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
		vector<int>::iterator me = find(p->child_id.begin(),p->child_id.end(), GetID());
		DebugAssert(me != p->child_id.end());
	}
	
	for(vector<int>::iterator c = child_id.begin(); c != child_id.end(); ++c)
	{
		WED_Thing * cc = SAFE_CAST(WED_Thing,FetchPeer(*c));
		DebugAssert(cc);
		DebugAssert(cc->parent_id == GetID());
	}
	
	for(vector<int>::iterator s = source_id.begin(); s != source_id.end(); ++s)
	{
		WED_Thing * ss = SAFE_CAST(WED_Thing,FetchPeer(*s));
		DebugAssert(ss);
		DebugAssert(ss->viewer_id.count(GetID()) > 0);
	}
	
	for(set<int>::iterator v = viewer_id.begin(); v != viewer_id.end(); ++v)
	{
		WED_Thing * vv = SAFE_CAST(WED_Thing,FetchPeer(*v));
		DebugAssert(vv);
		vector<int>::iterator me = find(vv->source_id.begin(), vv->source_id.end(), GetID());
		DebugAssert(me != vv->source_id.end());
	}
}

#pragma mark -

WED_TypeField::WED_TypeField(WED_Thing * t) : WED_PropertyItem(t, "Class", XML_Name("","")), val(t)
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
	v.string_val = val->HumanReadableType();
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

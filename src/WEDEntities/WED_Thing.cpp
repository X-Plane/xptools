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
#include "SQLUtils.h"
#include "WED_Errors.h"
#include "WED_XMLWriter.h"
#include <algorithm>

WED_Thing::WED_Thing(WED_Archive * parent, int id) :
	WED_Persistent(parent, id),
	name(this,"Name",SQL_Name("WED_things", "name"),XML_Name("hierarchy","name"),"unnamed entity")
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
		PropertyVal_t v;
		rhs->GetNthProperty(p, v);
		this->SetNthProperty(p, v);
	}
}

void 			WED_Thing::ReadFrom(IOReader * reader)
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

void			WED_Thing::FromDB(sqlite3 * db, const map<int,int>& mapping)
{
	child_id.clear();
	viewer_id.clear();
	source_id.clear();

	sql_row1<int>						key(GetID());
	
	// Read in parent
	sql_command	cmd(db,"SELECT parent FROM WED_things WHERE id=@i;","@i");
	sql_row1<int>						me;
	int err = cmd.simple_exec(key, me);
	if (err != SQLITE_DONE)	WED_ThrowPrintf("Unable to complete thing query: %d (%s)",err, sqlite3_errmsg(db));
	parent_id = me.a;

	// Read in Children
	sql_command kids(db, "SELECT id FROM WED_things WHERE parent=@id ORDER BY seq;","@id");
	sql_row1<int>	kid;
	kids.set_params(key);
	kids.begin();
	while ((err = kids.get_row(kid)) == SQLITE_ROW)
	{
		child_id.push_back(kid.a);
	}
	if(err != SQLITE_DONE)		WED_ThrowPrintf("Unable to complete thing query on kids: %d (%s)",err, sqlite3_errmsg(db));

	// Read in viewers
	sql_command viewers(db,"SELECT viewer FROM WED_thing_viewers WHERE source=@id;","@id");
	sql_row1<int>	viewer;
	viewers.set_params(key);
	viewers.begin();
	while((err = viewers.get_row(viewer)) == SQLITE_ROW)
	{
		viewer_id.insert(viewer.a);
	}
	if(err != SQLITE_DONE)		WED_ThrowPrintf("Unable to complete thing query on viewers: %d (%s)",err, sqlite3_errmsg(db));

	// Read in sources.  Note that sources are ORDERED because we might make a point sequence from our sources. 
	// Viewers are not ordered - they only exist so that a point can notify its observing line that it is, like, dead or something.
	sql_command sources(db,"SELECT source FROM WED_thing_viewers WHERE viewer=@id ORDER BY seq;","@id");
	sql_row1<int>	source;
	sources.set_params(key);
	sources.begin();
	while((err = sources.get_row(source)) == SQLITE_ROW)
	{
		source_id.push_back(source.a);
	}
	if(err != SQLITE_DONE)		WED_ThrowPrintf("Unable to complete thing query on sources: %d (%s)",err, sqlite3_errmsg(db));	
	
	// Read in properties
	char where_crud[100];
	sprintf(where_crud,"id=%d",GetID());
	PropsFromDB(db,where_crud,mapping);
}

void			WED_Thing::ToDB(sqlite3 * db)
{
	int err;

	int persistent_class_id;

	{
		const char * my_class = this->GetClass();

		sql_command	find_my_class(db,"SELECT id FROM WED_classes WHERE name=@name;","@name");
		sql_row1<string>	class_key(my_class);

		find_my_class.set_params(class_key);
		sql_row1<int>	found_id;
		if (find_my_class.get_row(found_id) == SQLITE_ROW)
		{
			persistent_class_id = found_id.a;
		}
		else
		{
			sql_command	find_highest_id(db,"SELECT MAX(id) FROM WED_classes;",NULL);
			sql_row1<int>	highest_key;
			err = find_highest_id.simple_exec(sql_row0(), highest_key);
			if(err != SQLITE_DONE)		WED_ThrowPrintf("UNable to update thing info: %d (%s)",err, sqlite3_errmsg(db));

			persistent_class_id = highest_key.a + 1;

			sql_command record_new_class(db,"INSERT INTO WED_classes VALUES(@id,@name);","@id,@name");
			sql_row2<int,string> new_class_info(persistent_class_id,my_class);
			err = record_new_class.simple_exec(new_class_info);
			if(err != SQLITE_DONE)		WED_ThrowPrintf("UNable to update thing info: %d (%s)",err, sqlite3_errmsg(db));
		}
	}
	
	// Write our properties and parent.  Children array is not written - it is inferred by a backward query.
	sql_command write_me(db,"INSERT OR REPLACE INTO WED_things VALUES(@id,@parent,@seq,@name,@class_id);","@id,@parent,@seq,@name,@class_id");
	sql_row5<int,int,int,string,int>	bindings(
												GetID(),
												parent_id,
												GetMyPosition(),
												name.value,
												persistent_class_id);

	err =  write_me.simple_exec(bindings);
	if(err != SQLITE_DONE)		WED_ThrowPrintf("UNable to update thing info: %d (%s)",err, sqlite3_errmsg(db));

	// We have to clear out old viewers that we might have had!  INSERT OR REPLACE only replaces if we have a sane primary key.
	// Since the viewer->source table is really a bunch of tuples (with ordering), we must nuke everything or we'll have stale points.
	char cmd_buf[1024];
	sprintf(cmd_buf,"DELETE FROM WED_thing_viewers WHERE viewer=%d;",GetID());
	sql_command clear_viewers(db,cmd_buf,NULL);
	err = clear_viewers.simple_exec();
	if (err != SQLITE_DONE)	WED_ThrowPrintf("%s (%d)",sqlite3_errmsg(db),err);

	// Write out our sources in order. 
	sql_command write_src(db,"INSERT OR REPLACE INTO WED_thing_viewers VALUES(@v, @s,@n);","@v,@s,@n");
	for(int n = 0; n < source_id.size(); ++n)
	{
		sql_row3<int,int,int> one_tuple(GetID(), source_id[n], n);
		err = write_src.simple_exec(one_tuple);
		if(err != SQLITE_DONE)		WED_ThrowPrintf("Unable to update thing source info info: %d (%s)",err, sqlite3_errmsg(db));		
	}

	char id_str[20];
	sprintf(id_str,"%d",GetID());
	PropsToDB(db,"id",id_str, "WED_things");
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

int					WED_Thing::CountChildren(void) const
{
	return child_id.size();
}

WED_Thing *		WED_Thing::GetNthChild(int n) const
{
	return SAFE_CAST(WED_Thing,FetchPeer(child_id[n]));
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
	return SAFE_CAST(WED_Thing,FetchPeer(source_id[n]));
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
		WED_Thing * v = SAFE_CAST(WED_Thing, FetchPeer(*i));
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
	return SAFE_CAST(WED_Thing,FetchPeer(parent_id));
}

void				WED_Thing::SetParent(WED_Thing * parent, int nth)
{
	StateChanged(wed_Change_Topology);
	WED_Thing * old_parent = SAFE_CAST(WED_Thing, FetchPeer(parent_id));
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
	WED_Thing * parent = SAFE_CAST(WED_Thing, FetchPeer(parent_id));
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
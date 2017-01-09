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

#include "WED_KeyObjects.h"
#include "IODefs.h"
#include "SQLUtils.h"
#include "WED_Errors.h"
#include "WED_XMLWriter.h"

DEFINE_PERSISTENT(WED_KeyObjects)

WED_KeyObjects::WED_KeyObjects(WED_Archive * a, int id) : WED_Thing(a,id)
{
}

WED_KeyObjects::~WED_KeyObjects()
{
}

void WED_KeyObjects::CopyFrom(const WED_KeyObjects * rhs)
{
	DebugAssert(!"We should not be copying the key object hash table.");
	WED_Thing::CopyFrom(rhs);
	StateChanged();
	choices = rhs->choices;		// Copy ptrs, not dupe objs!
}

IBase *	WED_KeyObjects::Directory_Find(const char * name)
{
	map<string,int>::iterator i = choices.find(name);
	if (i != choices.end())
		return FetchPeer(i->second);
	return WED_Thing::Directory_Find(name);
}


void WED_KeyObjects::Directory_Edit(const char * name, IBase * who)
{
	string n(name);
	WED_Persistent * target = SAFE_CAST(WED_Persistent,who);
	if (target == NULL && who != NULL)
		AssertPrintf("Can't set a non-persistent to %s\n", name);

	int id = target ? target->GetID() : 0;

	map<string,int>::iterator i = choices.find(n);
	if (i != choices.end() && i->second == id) return;

	StateChanged();
	choices[n] = id;
}

bool 			WED_KeyObjects::ReadFrom(IOReader * reader)
{
	bool r = WED_Thing::ReadFrom(reader);
	choices.clear();
	int n;
	reader->ReadInt(n);
	while (n--)
	{
		int b;
		int id;
		reader->ReadInt(b);
		vector<char> buf(b);
		reader->ReadBulk(&*buf.begin(), b, false);
		string key(buf.begin(),buf.end());
		reader->ReadInt(id);
		choices[key] = id;
	}
	return r;
}

void 			WED_KeyObjects::WriteTo(IOWriter * writer)
{
	WED_Thing::WriteTo(writer);
	writer->WriteInt(choices.size());
	for (map<string,int>::iterator it = choices.begin(); it != choices.end(); ++it)
	{
		writer->WriteInt(it->first.size());
		writer->WriteBulk(it->first.c_str(),it->first.size(),false);
		writer->WriteInt(it->second);
	}
}

void			WED_KeyObjects::FromDB(sqlite3 * db, const map<int,int>& mapping)
{
	WED_Thing::FromDB(db, mapping);
	choices.clear();

	int err;
	sql_command cmd(db,"SELECT key,value FROM WED_key_objects WHERE id=@i;","@i");

	sql_row1<int>			key(GetID());
	sql_row2<string,int>	pair;

	cmd.set_params(key);
	cmd.begin();
	while((err = cmd.get_row(pair)) == SQLITE_ROW)
	{
		choices[pair.a] = pair.b;
	}
	if (err != SQLITE_DONE) WED_ThrowPrintf("%s (%d)",sqlite3_errmsg(db),err);
}

void			WED_KeyObjects::ToDB(sqlite3 * db)
{
	WED_Thing::ToDB(db);
	int err;

	{
		sql_command clear_it(db,"DELETE FROM WED_key_objects WHERE id=@i;","@i");
		sql_row1<int>	key(GetID());

		err = clear_it.simple_exec(key);
		if (err != SQLITE_DONE) WED_ThrowPrintf("%s (%d)",sqlite3_errmsg(db),err);
	}
	{
		sql_command add_pair(db,"INSERT INTO WED_key_objects(id,key,value) VALUES(@i,@k,@v);","@i,@k,@v");

		for (map<string,int>::iterator i = choices.begin(); i != choices.end(); ++i)
		{
			sql_row3<int, string, int>	binding(GetID(),i->first,i->second);
			err = add_pair.simple_exec(binding);
			if (err != SQLITE_DONE) WED_ThrowPrintf("%s (%d)",sqlite3_errmsg(db),err);
		}
	}
}

void			WED_KeyObjects::AddExtraXML(WED_XMLElement * obj)
{
	WED_XMLElement * xml = obj->add_sub_element("keys");
	for(map<string,int>::iterator i = choices.begin(); i != choices.end(); ++i)
	{
		WED_XMLElement * c = xml->add_sub_element("key");
		c->add_attr_stl_str("name",i->first);
		c->add_attr_int("id",i->second);
	}
}


void		WED_KeyObjects::StartElement(
								WED_XMLReader * reader,
								const XML_Char *	name,
								const XML_Char **	atts)
{
	if(strcasecmp(name,"keys")==0)
	{
		choices.clear();
	}
	else if(strcasecmp(name,"key")==0)
	{
		const XML_Char * k = get_att("name",atts);
		const XML_Char * v = get_att("id",atts);
		if(k && v)
		{
			choices[k] = atoi(v);
		}
		else 
			reader->FailWithError("bad key");
	}
	else
		WED_Thing::StartElement(reader,name,atts);
}
void		WED_KeyObjects::EndElement(void)
{
}

void		WED_KeyObjects::PopHandler(void)
{
}

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

#include "XGrinderShell.h"
#include "XGrinderApp.h"
#include "MemFileUtils.h"

#include <string>
#include <vector>

using std::string;
using std::vector;

struct conversion_info {
	string					cmd_string;
	string					input_extension;
	string					output_extension;
};

struct flag_item_info {
	string					item_name;			// text of menu item
	string					token;				// token to substitute
	string					flag;				// empty for dividers
	int						enabled;			// 1 if enabled, 0 if not.
	int						radio;				// enforce mutually exclusive behavoir
};

struct flag_menu_info {
	xmenu					menu;
	string					title;
	vector<flag_item_info>	items;
};

static vector<flag_menu_info>			flag_menus;
static vector<conversion_info*>			conversions;
static xmenu							conversion_menu;
static map<string,conversion_info *>	selected_conversions;

static bool file_cb(const char * fileName, bool isDir, unsigned long long modTime, void * ref);
static void	sync_menu_checks();
static void sub_str(string& io_str, const string& key, const string& rep);

static void sub_str(string& io_str, const string& key, const string& rep)
{
	string::size_type p = 0;
	while ((p = io_str.find(key,p)) != io_str.npos)
	{
		io_str.replace(p,key.size(),rep);
	}
}


static void	sync_menu_checks()
{
	for (int n = 0; n < conversions.size(); ++n)
	if (conversions[n] != NULL)
		XWin::CheckMenuItem(conversion_menu, n, selected_conversions[conversions[n]->input_extension]==conversions[n]);

	for (vector<flag_menu_info>::iterator m = flag_menus.begin(); m != flag_menus.end(); ++m)
	for (int i = 0; i < m->items.size(); ++i)
	if (!m->items[i].item_name.empty())
		XWin::CheckMenuItem(m->menu, i, m->items[i].enabled);		
}

static bool file_cb(const char * fileName, bool isDir, unsigned long long modTime, void * ref)
{
	if (!isDir && strcmp(fileName+strlen(fileName)-4,".cmd")==0)
	{
		string path="config/";
		path += fileName;
		MFMemFile * fi = MemFile_Open(path.c_str());
		if (fi)
		{
			MFScanner	s;
			MFS_init(&s, fi);
			while(!MFS_done(&s))
			{
				if (MFS_string_match(&s, "CMD", 0))
				{
					conversion_info * info = new conversion_info;
					MFS_string(&s,&info->input_extension);
					MFS_string(&s,&info->output_extension);
					MFS_string_eol(&s,&info->cmd_string);
					conversions.push_back(info);
					selected_conversions[info->input_extension] = info;
				} 
				else if (MFS_string_match(&s,"OPTIONS", 0))
				{
					flag_menus.push_back(flag_menu_info());
					flag_menus.back().menu = NULL;
					MFS_string_eol(&s,&flag_menus.back().title);
				}
				else if (MFS_string_match(&s,"DIV",1))
				{
					flag_menus.back().items.push_back(flag_item_info());
					flag_menus.back().items.back().enabled = 0;
					flag_menus.back().items.back().radio = 0;
				}
				else if (MFS_string_match(&s,"CHECK",0))
				{
					flag_menus.back().items.push_back(flag_item_info());
					flag_menus.back().items.back().radio = 0;
					MFS_string(&s,&flag_menus.back().items.back().token);
					flag_menus.back().items.back().enabled = MFS_int(&s);
					MFS_string(&s,&flag_menus.back().items.back().flag);					
					MFS_string_eol(&s,&flag_menus.back().items.back().item_name);					
				}
				else if (MFS_string_match(&s,"RADIO",0))
				{
					flag_menus.back().items.push_back(flag_item_info());
					flag_menus.back().items.back().radio = 1;				
					MFS_string(&s,&flag_menus.back().items.back().token);
					flag_menus.back().items.back().enabled = MFS_int(&s);
					MFS_string(&s,&flag_menus.back().items.back().flag);					
					MFS_string_eol(&s,&flag_menus.back().items.back().item_name);					
				}
				else
					MFS_string_eol(&s, NULL);
			}
			MemFile_Close(fi);
		}
	}
	return true;
}

static void spool_job(const char * cmd_line)
{
	FILE * log = fopen("log.txt", "a");
	fprintf(log,"%s\n",cmd_line);
	XGrinder_ShowMessage("%s",cmd_line);
	FILE * pipe = popen(cmd_line, "r");
	while(!feof(pipe))
	{
		char buf[1000];
		int count = fread(buf,1,sizeof(buf),pipe);
		if(count == -1)
		{
			fprintf(log,"Error: %d\n", errno);
			break;
		}
		fwrite(buf,1,count,log);
	}
	pclose(pipe);
	fclose(log);
}



void	XGrindFiles(const vector<string>& files)
{
	for (vector<string>::const_iterator i = files.begin(); i != files.end(); ++i)
	{
		grind_file(i->c_str());
	}
}

void	grind_file(const char * inFileName)
{
	string fname(inFileName);
	string::size_type p = fname.rfind('.');
	if (p != fname.npos)
	{
		string suffix(fname.substr(p));
		string root(fname.substr(0,p));
		
		if (selected_conversions.find(suffix) == selected_conversions.end())
			XGrinder_ShowMessage("Unable to convert file '%s' - no converter for %s files.",inFileName, suffix.c_str());
		else {
			conversion_info * c = selected_conversions[suffix];
			string newname = root + c->output_extension;
//			XGrinder_ShowMessage("Will use: %s with %s and %s", c->cmd_string.c_str(), fname.c_str(), newname.c_str());			
			map<string,string>	sub_flags;
			for(vector<flag_menu_info>::iterator m = flag_menus.begin(); m != flag_menus.end(); ++m)
			for(vector<flag_item_info>::iterator i = m->items.begin(); i != m->items.end(); ++i)
			if(!i->item_name.empty())
			if(i->enabled)
			{
				if (sub_flags.count(i->token) > 0)
					sub_flags[i->token] += " ";					
				sub_flags[i->token] += i->flag;
			}
			string cmd_line = c->cmd_string;
			sub_str(cmd_line,"INFILE", fname);
			sub_str(cmd_line,"OUTFILE", newname);
			for(map<string,string>::iterator p = sub_flags.begin(); p != sub_flags.end(); ++p)
				sub_str(cmd_line,p->first,p->second);
			
			spool_job(cmd_line.c_str());
		}
	} else
		XGrinder_ShowMessage("Unable to convert file '%s' - no extension.",inFileName);
}

int	XGrinderMenuPick(xmenu menu, int item)
{
	if(menu==conversion_menu)
	{
		if(conversions[item] != NULL)
		{
			selected_conversions[conversions[item]->input_extension.c_str()] = conversions[item];
			sync_menu_checks();
			return 1;
		}
	} else
	for (vector<flag_menu_info>::iterator m = flag_menus.begin(); m != flag_menus.end(); ++m)
	if (m->menu == menu)
	{
		if (m->items[item].radio)
		{
			int n;
			for (n = item-1; n >= 0; --n)
			{
				if (m->items[n].item_name.empty()) break;
				m->items[n].enabled = 0;
			}
			for (n = item+1; n < m->items.size(); ++n)
			{
				if (m->items[n].item_name.empty()) break;
				m->items[n].enabled = 0;
			}			
		}
		m->items[item].enabled = true;
		sync_menu_checks();
		return 1;
	}
	return 0;
}

void	XGrindInit(string& t)
{
	// load ini files
	MF_GetDirectoryBulk("config", file_cb, NULL);
	
	// sort conversions
	
	// insert nulls at ext change
	for (vector<conversion_info *>::iterator c = conversions.begin(); c != conversions.end(); ++c)
	{
		vector<conversion_info *>::iterator n = c;
		++n;
		if (n != conversions.end() && *c != NULL && *n != NULL && (*n)->input_extension != (*c)->input_extension)
			c = conversions.insert(n, NULL);
	}
	
	// build conversion menu
	const char ** items = new const char *[conversions.size()+1];
	items[conversions.size()] = 0;	
	for (int n = 0; n < conversions.size(); ++n)
	{
		char buf[256];
		if (conversions[n] == NULL)
			strcpy(buf,"-");
		else
			sprintf(buf,"%s to %s",
				conversions[n]->input_extension.c_str(),
				conversions[n]->output_extension.c_str());
		char * p = new char[strlen(buf)+1];
		items[n] = p;
		strcpy(p,buf);
	}
	conversion_menu = XGrinder_AddMenu("Convert",items);
	for (int n = 0; n < conversions.size(); ++n)
	{
		delete [] items[n];
	}
	delete [] items;

	// build flags menus
	for(vector<flag_menu_info>::iterator m = flag_menus.begin(); m != flag_menus.end(); ++m)
	{
		const char ** items = new const char *[m->items.size()+1];
		items[m->items.size()] = 0;
		for (int n = 0; n < m->items.size(); ++n)
		if (m->items[n].item_name.empty())
			items[n] = "-";
		else
			items[n] = m->items[n].item_name.c_str();
		
		m->menu = XGrinder_AddMenu(m->title.c_str(), items);		
		delete [] items;
	}
	
	sync_menu_checks();
}


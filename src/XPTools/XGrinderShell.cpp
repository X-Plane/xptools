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

/*
	META TOKENS: INFILE OUTFILE

	CMD	<in_ext> <out_ext> <cmd-prompt-string>
	OPTIONS <title of menu for this tool>
	DIV
	CHECK <token> <enabled> <flag> <menu item name>
	RADIO <token> <enabled> <flag> <menu item name>

*/

#include "XGrinderShell.h"
#include "XGrinderApp.h"
#include "MemFileUtils.h"
#include "PlatformUtils.h"
#include <string>
#include <vector>
#include <sys/stat.h>

#if IBM
#define popen _popen
#define pclose _pclose
#endif

using std::string;
using std::vector;

struct conversion_info {
	string					cmd_string;
	string					input_extension;
	string					output_extension;
	string					tool_name;
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

static char * g_me = NULL;

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

#if IBM
bool IsConsoleApp(const char * path)
{
	FILE * fi = fopen(path,"rb");
	if(!fi) return false;
	IMAGE_DOS_HEADER h;
	if(fread(&h,1,sizeof(h),fi) != sizeof(h)) goto bail;
	if(h.e_magic != IMAGE_DOS_SIGNATURE) goto bail;
	if(fseek(fi,h.e_lfanew,SEEK_SET)==-1) goto bail;
	IMAGE_NT_HEADERS nt;
	if(fread(&nt,1,sizeof(nt),fi) != sizeof(nt)) goto bail;
	if(nt.Signature != IMAGE_NT_SIGNATURE) goto bail;
	fclose(fi);
	return nt.OptionalHeader.Subsystem == IMAGE_SUBSYSTEM_WINDOWS_CUI ||
		nt.OptionalHeader.Subsystem == IMAGE_SUBSYSTEM_POSIX_CUI;
bail:
	fclose(fi);
	return false;
}
#endif

static bool file_cb(const char * fileName, bool isDir, unsigned long long modTime, void * ref)
{
	if(isDir) return true;
	if(strstr(fileName,".icns")) return true;
	if(g_me && strcmp(fileName, g_me)==0) return true;
	if(fileName[0] == '.') return true;
	char pipe_buf[1024];
	sprintf(pipe_buf,"%s/%s",ref,fileName);
	struct stat ss;
	if(stat(pipe_buf,&ss) != 0) return true;
#if IBM
	if((ss.st_mode & S_IEXEC) == 0) return true;
#else
	if((ss.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH)) == 0) return true;
#endif
	if((ss.st_mode & S_IFMT) != S_IFREG) return true;
#if IBM
	if(!IsConsoleApp(pipe_buf)) return true;
#endif
	sprintf(pipe_buf,"\"%s/%s\" --auto_config", ref,fileName);
	FILE * fi = popen(pipe_buf, "r");
	if (fi)
	{
		while(!feof(fi))
		{
			char	line[2048];
			char	s1[512];
			char	s2[512];
			char	s3[512];
			int		en;

			if(!fgets(line,sizeof(line),fi)) break;
//			printf("%s\n",line);
			if(sscanf(line,"CMD %s %s %[^\r\n]",s1,s2,s3)==3)
			{
				conversion_info * info = new conversion_info;
				info->input_extension	= s1;
				info->output_extension	= s2;
				info->cmd_string		= s3;
				info->tool_name			= fileName;
				conversions.push_back(info);
				selected_conversions[info->input_extension] = info;
			}
			else if (sscanf(line,"OPTIONS %[^\r\n]",s1)==1)
			{
				flag_menus.push_back(flag_menu_info());
				flag_menus.back().menu = NULL;
				flag_menus.back().title = s1;
			}
			else if (strncmp(line,"DIV",3)==0)
			{
				flag_menus.back().items.push_back(flag_item_info());
				flag_menus.back().items.back().enabled = 0;
				flag_menus.back().items.back().radio = 0;
			}
			else if (sscanf(line,"CHECK %s %d %s %[^\r\n]",s1,&en,s2,s3)==4)
			{
				flag_menus.back().items.push_back(flag_item_info());
				flag_menus.back().items.back().radio = 0;
				flag_menus.back().items.back().token = s1;
				flag_menus.back().items.back().enabled = en;
				flag_menus.back().items.back().flag = s2;
				flag_menus.back().items.back().item_name = s3;
			}
			else if (sscanf(line,"RADIO %s %d %s %[^\r\n]",s1,&en,s2,s3)==4)
			{
				flag_menus.back().items.push_back(flag_item_info());
				flag_menus.back().items.back().radio = 1;
				flag_menus.back().items.back().token = s1;
				flag_menus.back().items.back().enabled = en;
				flag_menus.back().items.back().flag = s2;
				flag_menus.back().items.back().item_name = s3;
			}
		}
		pclose(fi);
	}
	return true;
}

static void spool_job(const char * cmd_line)
{
	FILE * log = fopen("log.txt", "a");
	if(log == NULL) log = stdout;
	fprintf(log,"%s\n",cmd_line);
	XGrinder_ShowMessage("%s",cmd_line);
	string quoted(cmd_line);
#if IBM
	quoted = "\"" + quoted + "\"";
#endif
	FILE * pipe = popen(quoted.c_str(), "r");
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
	if(log != stdout)
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
			{
				if(i->enabled)
				{
					if (sub_flags.count(i->token) > 0)
						sub_flags[i->token] += " ";
					sub_flags[i->token] += i->flag;
				} else if (!i->radio) {
					sub_flags[i->token] = "";
				}
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
			m->items[item].enabled = true;
		} else
			m->items[item].enabled = !m->items[item].enabled;
		sync_menu_checks();
		return 1;
	}
	return 0;
}

void	XGrindInit(string& t)
{
/*	char base[2048];
	char resp[2048];
	CFURLRef	res_url = CFBundleCopyResourcesDirectoryURL(CFBundleGetMainBundle());
	CFURLRef	main_url = CFBundleCopyBundleURL(CFBundleGetMainBundle());
	CFStringRef	res_path = CFURLCopyFileSystemPath(res_url, kCFURLPOSIXPathStyle);
	CFStringRef	main_path = CFURLCopyFileSystemPath(main_url, kCFURLPOSIXPathStyle);
	CFStringGetCString(res_path,resp,sizeof(resp),kCFStringEncodingMacRoman);
	CFStringGetCString(main_path,base,sizeof(base),kCFStringEncodingMacRoman);
	CFRelease(res_url);
	CFRelease(main_url);
	CFRelease(res_path);
	CFRelease(main_path);
	strcat(base,"/");
	strcat(base,resp);
	MF_GetDirectoryBulk(base, file_cb, base);

//	file_cb("DSFTool");
//	file_cb("DDSTool");
//	file_cb("ObjConverter");
*/
	char	base[2048];
	GetApplicationPath(base,sizeof(base));

	char * last_sep = base;
	char * p = base;
	while(*p)
	{
		if(*p == '/' || *p == '\\') last_sep = p;
		++p;
	}
	*last_sep++=0;
	g_me=last_sep;

	MF_GetDirectoryBulk(base, file_cb, base);

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
			sprintf(buf,"%s to %s (%s)",
				conversions[n]->input_extension.c_str(),
				conversions[n]->output_extension.c_str(),
				conversions[n]->tool_name.c_str());
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


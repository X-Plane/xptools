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

#include "GUI_Prefs.h"
#include "PlatformUtils.h"
#include "MemFileUtils.h"

#define DEBUG_PREFS 0
#if APL
	#include <Carbon/Carbon.h>
#endif
#if IBM
	#include <shlobj.h>
#endif
#if LIN
    #include <pwd.h>
#endif

typedef map<string,string>				GUI_PrefSection_t;
typedef map<string,GUI_PrefSection_t>	GUI_Prefs_t;

static	GUI_Prefs_t		sPrefs;

void	dequote(string& s)
{
	for(string::size_type n = 0; n < s.length(); ++n)
	{
		if (s[n] == '\\')
			s.erase(n,1);
	}
}

void	enquote(string& s)
{
	for(string::size_type n = 0; n < s.length(); ++n)
	{
		if (s[n] == ' ' ||
			s[n] == '\\' ||
			s[n] == '\r' ||
			s[n] == '\n' ||
			s[n] == '\t' ||
			s[n] == '=')
		{
			s.insert(n,"\\");
			++n;
		}
	}	
}


bool			GUI_GetPrefsDir(string& path)
{
	#if APL
			FSRef	ref;
		OSErr	err = FSFindFolder(kOnAppropriateDisk,kPreferencesFolderType,TRUE, &ref);
		if (err != noErr) return false;
		
		char	buf[1024];
		err = FSRefMakePath(&ref, (UInt8*) buf, sizeof(buf));
		if (err != noErr) return false;
		path = buf;
		return true;
	#endif
	#if IBM
		char buf[MAX_PATH];
		HRESULT res = SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, buf);
		if (!SUCCEEDED(res))
			return false;
		path = buf;
		return true;
	#endif 
	#if LIN
        passwd *pw = getpwuid(getuid());
        if (pw)
        {
            path = pw->pw_dir;
            return true;
        }
        else path = ".";
        return false;
	#endif
}

inline bool	is_eol(const char p) { return p == '\r' || p == '\n'; }
inline bool	is_spc(const char p) { return p == '\t' || p == ' '; }
inline void	skip_space(const char *&p, const char * e) { while(p<e && is_spc(*p)) ++p; }
inline void	skip_eol(const char *&p, const char * e) { while(p<e && is_eol(*p)) ++p; }
		
void			GUI_Prefs_Read(const char *app_name)
{
	sPrefs.clear();
	string pref_dir;
	if (!GUI_GetPrefsDir(pref_dir)) return;
	pref_dir += DIR_STR;
    #if LIN
    pref_dir += ".";
    pref_dir +=  app_name;
    #else
	pref_dir += app_name;
    #endif
	pref_dir += ".prefs";
	MFMemFile* f = MemFile_Open(pref_dir.c_str());
	GUI_PrefSection_t * cur=NULL;
	if(f)
	{
		const char * p = MemFile_GetBegin(f);
		const char * e = MemFile_GetEnd(f);
		while(p<e)
		{
			skip_space(p,e);
			if(p<e && *p=='[')
			{
				++p;
				const char * cs = p;
				while(p<e && !is_eol(*p) && *p != ']')
					++p;
				string cur_name(cs,p);
				cur=&sPrefs[cur_name];
			}
			else if(p<e && *p != '\r' && *p != '\n')
			{
				const char * ks = p;
				while(p<e && !is_spc(*p) && !is_eol(*p) && *p != '=')
				{
					if (*p=='\\')	++p;
					if(p<e)			++p;
				}
				
				const char * ke = p;
				skip_space(p,e);
				if(p<e && *p=='=')
				{
					++p;
					skip_space(p,e);
					if(p<e)
					{
						const char * vs = p;
						while(p<e && !is_spc(*p) && !is_eol(*p) && *p != '=')
						{
							if (*p=='\\')	++p;
							if(p<e)			++p;
						}
						const char * ve = p;
						if(cur)
						{
							string key(ks,ke);
							string val(vs,ve);
							dequote(key);
							dequote(val);
							(*cur)[key] = val;
						}
					}
				}				
			}
			skip_eol(p,e);
		}
		MemFile_Close(f);
	}
	
	#if DEBUG_PREFS
	for(GUI_Prefs_t::iterator s = sPrefs.begin(); s != sPrefs.end(); ++s)
	{
		printf("[%s]" CRLF, s->first.c_str());
		for(GUI_PrefSection_t::iterator p = s->second.begin(); p != s->second.end(); ++p)
			printf("'%s'='%s'" CRLF, p->first.c_str(), p->second.c_str());
	}
	#endif
}

void			GUI_Prefs_Write(const char * app_name)
{
	string pref_dir;
	if (!GUI_GetPrefsDir(pref_dir)) { DoUserAlert("Warning: preferences file could not be written - preferences directory not found."); return; }
	pref_dir += DIR_STR;
    #if LIN
    pref_dir += ".";
    pref_dir +=  app_name;
    #else
	pref_dir += app_name;
    #endif
	pref_dir += ".prefs";

	FILE * fi = fopen(pref_dir.c_str(), "w");
	if (fi == NULL) { DoUserAlert("Warning: preferences file could not be written - could not write file."); return; }
	for(GUI_Prefs_t::iterator s = sPrefs.begin(); s != sPrefs.end(); ++s)
	{
		fprintf(fi,"[%s]" CRLF, s->first.c_str());
		for(GUI_PrefSection_t::iterator p = s->second.begin(); p != s->second.end(); ++p)
		{
			string k(p->first), v(p->second);
			enquote(k);
			enquote(v);
			fprintf(fi,"%s=%s" CRLF, k.c_str(), v.c_str());
		}
	}
	fclose(fi);
}

void			GUI_EnumSection(const char * section, void (* cb)(const char * key, const char * value, void * ref), void * ref)
{
	GUI_Prefs_t::iterator sec=sPrefs.find(string(section));
	if (sec != sPrefs.end())
	for(GUI_PrefSection_t::iterator pref = sec->second.begin(); pref != sec->second.end(); ++pref)
	{
		cb(pref->first.c_str(),pref->second.c_str(),ref);
	}
}

const char *	GUI_GetPrefString(const char * section, const char * key, const char * def)
{
	GUI_Prefs_t::iterator sec=sPrefs.find(string(section));
	if(sec==sPrefs.end()) return def;
	GUI_PrefSection_t::iterator pref=sec->second.find(key);
	if (pref==sec->second.end()) return def;
	return pref->second.c_str();
}

void			GUI_SetPrefString(const char * section, const char * key, const char * value)
{
	sPrefs[section][key] = value;
}

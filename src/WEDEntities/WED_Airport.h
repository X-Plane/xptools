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

#ifndef WED_AIRPORT_H
#define WED_AIRPORT_H

#include "WED_GISComposite.h"
#include <map>

struct	AptInfo_t;

class	WED_Airport : public WED_GISComposite {

DECLARE_PERSISTENT(WED_Airport)

public:

	void		GetICAO(string& icao) const;
	int			GetAirportType(void) const;
	int			GetSceneryID(void) const;

	void		SetSceneryID(int new_id);
	void		SetAirportType(int airport_type);
	void		SetElevation(double elev);
	void		SetHasATC(int has_atc);
	void		SetICAO(const string& icao);

	void		Import(const AptInfo_t& info, void (* print_func)(void *, const char *, ...), void * ref);
	void		Export(		 AptInfo_t& info) const;

	// IPropertyObject
	int			FindProperty(const char * in_prop) const;
	int			CountProperties(void) const;
	void		GetNthPropertyInfo(int n, PropertyInfo_t& info) const;

	void		GetNthPropertyDict(int n, PropertyDict_t& dict) const;
	void		GetNthPropertyDictItem(int n, int e, string& item) const;
	
	void		GetNthProperty(int n, PropertyVal_t& val) const;
	void		SetNthProperty(int n, const PropertyVal_t& val);
	
	//WED_Persistant, for Undo/Redo
	virtual	void 			ReadFrom(IOReader * reader);
	virtual	void 			WriteTo(IOWriter * writer);
	/*virtual void			FromDB(sqlite3 * db, const map<int,int>& mapping);
	virtual void			ToDB(sqlite3 * db);*/

	virtual void			AddExtraXML(WED_XMLElement * obj);
	
	//IOperation
	virtual void		StartElement(
								WED_XMLReader * reader,
								const XML_Char *	name,
								const XML_Char **	atts);
	/*virtual	void		EndElement(void);
	virtual	void		PopHandler(void);*/

	virtual const char *	HumanReadableType(void) const { return "Airport"; }

private:

	WED_PropIntEnum				airport_type;
	WED_PropDoubleTextMeters	elevation;
	WED_PropBoolText			has_atc;
	WED_PropStringText			icao;
	WED_PropIntText				scenery_id;
	
	//A hashmap of meta data. Due to the way it is stored in XML
	//Keys are not allowed to contain commas
	std::map<string,string>		meta_data_hashmap;
	typedef std::pair<string,string> meta_data_entry;
};


#endif /* WED_AIRPORT_H */

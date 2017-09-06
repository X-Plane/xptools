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

#ifndef WED_THING_H
#define WED_THING_H

/*
	WED_Thing - THEORY OF OPERATION

	WED_Thing is the base persistent object for WorldEditor.  Quite literally EVERYTHIGN is a WED_Thing.

	WED_Things have a few tricks:

		- They are property helpers, and thus meet the property objecct interface for generalized editing.
		- They are persistent.
		- They are proeprty helpers, so all subclasses can easily embed "properties".
		- They use the property helper's streaming code.  Thus you don't have to worry about undo for prop-helper-type code.
		- They provide nesting via a parent-child relationship to other WED persistent objs, managing undo safely.
		- All things have names (at least until I decide that this is silly).

 */

#include "WED_Persistent.h"
#include "WED_PropertyHelper.h"
#include "IArray.h"
#include "IDirectory.h"
#include "IOperation.h"

enum {
	wed_Change_Selection = 2,
	wed_Change_Topology = 4,
	wed_Change_Properties = 8
};

class	WED_Thing;

class WED_TypeField : public WED_PropertyItem {
public:

	WED_TypeField(WED_Thing * parent);

	// From WED_PropertyItem - just to get our type
	virtual void		GetPropertyInfo(PropertyInfo_t& info);
	virtual	void		GetPropertyDict(PropertyDict_t& dict);
	virtual	void		GetPropertyDictItem(int e, string& item);
	virtual void		GetProperty(PropertyVal_t& val) const;
	virtual void		SetProperty(const PropertyVal_t& val, WED_PropertyHelper * parent);
	virtual	void 		ReadFrom(IOReader * reader);
	virtual	void 		WriteTo(IOWriter * writer);
	virtual	void		ToXML(WED_XMLElement * parent);
	virtual	bool		WantsAttribute(const char * ele, const char * att_name, const char * att_value);	

	WED_Thing * val;

};

class	WED_Thing :	public			WED_Persistent, 
					public			WED_PropertyHelper, 
					public virtual	IArray, 
					public virtual	IDirectory, 
					public virtual	IOperation {

DECLARE_INTERMEDIATE(WED_Thing)

public:

			int					CountChildren(void) const;
			WED_Thing *			GetNthChild(int n) const;
			WED_Thing *			GetNamedChild(const string& s) const;
			WED_Thing *			GetParent(void) const;
			void				SetParent(WED_Thing * parent, int nth);
			int					GetMyPosition(void) const;

			int					CountSources(void) const;
			WED_Thing *			GetNthSource(int n) const;
			int					CountViewers(void) const;
			void				GetAllViewers(set<WED_Thing *>& out_viewers) const;
			void				AddSource(WED_Thing * src, int nth);
			void				RemoveSource(WED_Thing * src);
			void				ReplaceSource(WED_Thing * old, WED_Thing * rep);

			void				GetName(string& name) const;
			void				SetName(const string& name);

	// WED_Persistent
	virtual	bool 			ReadFrom(IOReader * reader);
	virtual	void 			WriteTo(IOWriter * writer);
	virtual	void			ToXML(WED_XMLElement * parent);
	virtual	void			FromXML(WED_XMLReader * reader, const XML_Char ** atts);
	virtual	void			PostChangeNotify(void);
	virtual	void			Validate(void);

	// This is a template method - sub-classes of things that have to add MORE XML than they would get via the property system and the thing
	// itself override this method.  This way their extra XML is _inside_ the toplevel obj.
	virtual	void			AddExtraXML(WED_XMLElement * obj) { }
	
	// One more template method: all WED things must have a human-readable type!
	virtual const char *	HumanReadableType(void) const=0;
	

	// From WED_PropertyHelper...
	virtual	void			PropEditCallback(int before);
	virtual	int					CountSubs(void);
	virtual	IPropertyObject *	GetNthSub(int n);
	
	// IArray
	virtual	int				Array_Count (void );
	virtual IBase *			Array_GetNth(int n);

	// IDirectory
	virtual	IBase *		Directory_Find(const char * name);

	// IOperation
	virtual		void			__StartOperation(const char * op_name, const char * inFile, int inLine);
	virtual		void			CommitOperation(void);
	virtual		void			AbortOperation(void);
	
	virtual void		StartElement(
								WED_XMLReader * reader,
								const XML_Char *	name,
								const XML_Char **	atts);
	virtual	void		EndElement(void);
	virtual	void		PopHandler(void);

protected:

	virtual		void			AddChild(int id, int n);
	virtual		void			RemoveChild(int id);
	virtual		void			AddViewer(int id);
	virtual		void			RemoveViewer(int id);

private:

	int				parent_id;
	vector<int>		child_id;
	
	vector<int>		source_id;				// These are MY sources!  I am watching them.
	set<int>		viewer_id;				// These are MY vieweres!  They are watching me.

	WED_TypeField				type;
	WED_PropStringText			name;
	
friend class WED_TaxiRoute;
friend class WED_AirportSign;

};


#endif /* WED_THING_H */

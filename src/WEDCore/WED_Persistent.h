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

#ifndef WED_PERSISTENT
#define WED_PERSISTENT

#include "IBase.h"
#include "WED_Archive.h"
#include "AssertUtils.h"
#include "ISelection.h"
#include "WED_XMLReader.h"

class	IOReader;
class	IOWriter;
class	WED_XMLElement;

/*
	WED_Persistent.h - THEORY OF OPERATION

	WED_Persistent is the base for an object that participates in the file format via tables and the undo system.

	Persistent classes have class ID, and the objects have object IDs within an archive.  Clients must define:

	1. The casting supported - function-based casting is used here.
	2. Streaming methods via IODef reader/writers for the undo system.
	3. Persistence methods via sqlite for the file format.

	CONSTRUCTORS AND DESTRUCTORS

	All persistent objs are made via new/delete, but these are wrapped, as is the ctor.  We have only access via:

	(non-virtual ctors)
	CLASS::CreateTyped - creates a new object of type CLASS, returns a CLASS *.
	CLASS::Create - creates a new object of type CLASS, returns a WED_Persitent *.
	(virtual ctors)
	WED_Persistent::CreateByClass(class) - creates an object of type "class", returns a WED_Persistent *

	To delete an object, use o->Delete();


*/

/*
 * Persistent object header macros.  All persistent objects must have this
 * stuff - use the declare macro in the class and the define macro in the
 * translation unit.  It makes:
 *
 * - A static string with the class name.
 * - A virtual function to get runtime class.
 * - A virtual function to safely cast.
 * - A Static constructor.
 * - A convenient registration function for that static constuctor.
 * - Private constructor/destructors (to force proper dynamic creation.
 *
 *
 * MACROS
 *
 * In the header, inside the class, use:
 *
 *	DECLARE_PERSISTENT(CLASS)
 *
 * to set up the boiler plate.  In the CPP you must use these:
 *
 *	DEFINE_PERSISTENT(CLASS)
 *	IMPLEMENTS_INTERFACE(CLASS)					Use this to implement an abstract interface.
 *	INHERITS_FROM(CLASS)						Use this to inherit all of the behavior of one base class.
 *	BASE_CASE									Use this if you do not inherit from any class.
 *	END_CASTING									Use this to close off the casting control block.
 *
 */

//Because the sClass of WED_Persistent is actually a pointer to a static string
//we can communicate the difference between the data type and expected use with this
//typedef
typedef const char* sClass_t;

#define DECLARE_PERSISTENT(__Class)		 							\
public: 															\
	static WED_Persistent * Create(									\
								WED_Archive *	parent, int id);	\
	static __Class * CreateTyped(									\
								WED_Archive *	parent);			\
	virtual const char * 	GetClass(void) const;					\
	virtual WED_Persistent*	Clone(void) const;						\
			void			CopyFrom(const __Class * rhs);			\
	static	const char *	sClass;									\
protected:															\
	__Class(WED_Archive * parent);									\
	__Class(WED_Archive * parent, int inID);						\
	virtual ~__Class();

#define DECLARE_INTERMEDIATE(__Class)		 						\
protected:															\
	__Class(WED_Archive * parent);									\
	__Class(WED_Archive * parent, int inID);						\
	virtual ~__Class();												\
	void CopyFrom(const __Class * rhs);


#define DEFINE_PERSISTENT(__Class)								\
																\
WED_Persistent * __Class::Create(								\
								WED_Archive * parent, int id)	\
{																\
	return new __Class(parent, id);								\
}																\
																\
WED_Persistent*	__Class::Clone(void) const						\
{																\
	__Class * new_obj = new __Class(							\
			GetArchive(),GetArchive()->NewID());				\
	new_obj->PostCtor();										\
	new_obj->CopyFrom(this);									\
	return new_obj;												\
}																\
																\
__Class * __Class::CreateTyped(									\
								WED_Archive * parent)			\
{																\
	__Class * r = new __Class(parent, parent->NewID());			\
	r->PostCtor();												\
	return r;													\
}																\
																\
void __Class##_Register(void); 									\
void __Class##_Register(void) 									\
{																\
	WED_Persistent::Register(__Class::sClass, __Class::Create);	\
}																\
																\
const char * __Class::GetClass(void) const						\
{																\
	return sClass;												\
}																\
																\
const char * __Class::sClass = #__Class;

#define TRIVIAL_COPY(__Class, __Base)							\
void __Class::CopyFrom(const __Class * rhs)						\
{																\
	__Base::CopyFrom(rhs);										\
}

#define StartCommand(x) __StartCommand(x,__FILE__,__LINE__)

class	WED_Persistent : public virtual ISelectable {
public:

	typedef WED_Persistent * (* CTOR_f)(WED_Archive *, int);

	// Persistent creation-destruction:
	static	void			Register(const char * id, CTOR_f ctor);
	static	WED_Persistent *CreateByClass(const char * id, WED_Archive * parent, int in_ID);

							WED_Persistent(WED_Archive * parent);
							WED_Persistent(WED_Archive * parent, int inID);

	// Destructor is hidden - you must call Delete.  This both enforces dynamic allocation
	// and guarantees that we can fully handle destruction BEFORE we lose our typing info!
			 void			Delete(void);

	// Convenience routine for finding your peers:
	WED_Persistent *		FetchPeer(int inID) const;

	// Convenience routines for undo...
	void					__StartCommand(const string& inName, const char * inFile, int inLine);		// pass-throughs
	void					CommitCommand(void);
	void					AbortCommand(void);

	// This method is called by the derived-class whenever its internals are changed
	// by an editing operation.  It is the convenient way to signal "we better start
	// recording persistent stuff".
	//
	//IMPORTANT! Despite "Changed" being past tense, StateChanged MUST BE CALLED BEFORE changes to the object!
			void 			StateChanged(int change_kind = wed_Change_Any);

	// Methods provided by the base: all persistent objs must have an archive and
	// GUID - this is non-negotiable!

			WED_Archive *	GetArchive(void) const 				{ return mArchive;	}
			int				GetID(void) const					{ return mID;		}

	// IO methods to read and write state of class to a data holder.  ReadFrom
	// does NOT call StateChanged!  Subclasses must provide.  Readers return true
	// if they need a post-change notification - see below.
	virtual WED_Persistent*	Clone(void) const=0;
	virtual	bool 			ReadFrom(IOReader * reader)=0;
	virtual	void 			WriteTo(IOWriter * writer)=0;
	virtual	void			ToXML(WED_XMLElement * parent)=0;
	virtual	void			FromXML(WED_XMLReader * reader, const XML_Char ** atts)=0;

	// If you return true from read, this is called on you AFTER the entire archive is
	// processed. Computing that requires all peers/parents/children to be fully
	// re-instantiated goes here.
	virtual	void			PostChangeNotify(void)=0;

	virtual void			Validate(void) { }

	virtual const char *	GetClass(void) const=0;

	// These are for the archive's use..
			void			SetDirty(int dirty);
			int				GetDirty(void) const;

	// ISelectable
	virtual		int			GetSelectionID(void) const { return mID; }

protected:

	// Dtor protected to prevent instantiation.
	virtual 				~WED_Persistent();

			void			PostCtor(void);

private:

		int				mID;
		WED_Archive *	mArchive;
		int				mDirty;

		WED_Persistent();	// No ctor without archive!

		// wait on copy ctor for now
			WED_Persistent(const WED_Persistent& rhs);
			WED_Persistent& operator=(const WED_Persistent& rhs);

};

#endif /* WED_PERSISTENT */

#ifndef WED_PERSISTENT
#define WED_PERSISTENT

#include "WED_Archive.h"
#include "AssertUtils.h"

struct	sqlite3;
class	IOReader;
class	IOWriter;

/*
	WED_Persistent.h - THEORY OF OPERATION

	WED_Persistent is the base for an object that participates in the file format via tables and the undo system.
	
	Persistent classes have class ID, and the objects have object IDs within an archive.  Clients must define:
	
	1. The casting supported - function-based casting is used here.
	2. Streaming methods via IODef reader/writers for the undo system.
	3. Persistence methods via sqlite for the file format.
	
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
 
#define DECLARE_PERSISTENT(__Class)		 							\
public: 															\
	static WED_Persistent * Create(									\
								WED_Archive *	parent,				\
								int				inID);				\
	static __Class * CreateTyped(									\
								WED_Archive *	parent,				\
								int				inID);				\
	static void				Register(void);							\
	virtual const char * 	GetClass(void) const;					\
	virtual void * 			SafeCast(const char * class_id);		\
protected:															\
	__Class(WED_Archive * parent);									\
	__Class(WED_Archive * parent, int inID);						\
	virtual ~__Class();



#define DEFINE_PERSISTENT(__Class)								\
																\
WED_Persistent * __Class::Create(								\
								WED_Archive * parent,			\
								int inID)						\
{																\
	return new __Class(parent, inID);							\
}																\
																\
__Class * __Class::CreateTyped(									\
								WED_Archive * parent,			\
								int inID)						\
{																\
	return new __Class(parent, inID);							\
}																\
																\
void __Class::Register(void) 									\
{																\
	WED_Persistent::Register(#__Class, Create);					\
}																\
																\
const char * __Class::GetClass(void) const						\
{																\
	return #__Class;											\
}																\
																\
void * __Class::SafeCast(const char * class_id)					\
{																\
	if (!strcmp(class_id, #__Class))							\
		return this;

#define IMPLEMENTS_INTERFACE(CLASS)								\
	if (!strcmp(class_id, #CLASS))								\
		return (CLASS*)this;

#define INHERITS_FROM(CLASS)									\
	return CLASS::SafeCast(class_id);

#define BASE_CASE												\
										return NULL; 
	
#define END_CASTING												\
}


// Safe cast macro based on safe-cast mechanism
#define SAFE_CAST(__Class, __Var) reinterpret_cast<__Class *>((__Var)->SafeCast(#__Class));



class	WED_Persistent {
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

	// This method is called by the derived-class whenever its internals are changed
	// by an editing operation.  It is the convenient way to signal "we better start
	// recording persistent stuff".
			void 			StateChanged(void);
	
	// Methods provided by the base: all persistent objs must have an archive and
	// GUID - this is non-negotiable!
	
			WED_Archive *	GetArchive(void) const 				{ return mArchive;	}
			int				GetID(void) const					{ return mID;		}
	
	// IO methods to read and write state of class to a data holder.  ReadFrom
	// does NOT call StateChanged!  Subclasses must provide.
	virtual	void 			ReadFrom(IOReader * reader)=0;
	virtual	void 			WriteTo(IOWriter * writer)=0;
	virtual void			FromDB(sqlite3 * db)=0;
	virtual void			ToDB(sqlite3 * db)=0;
	
	virtual const char *	GetClass(void) const=0;
	
	virtual void *			SafeCast(const char * class_id)=0;

	// These are for the archive's use..
			void			SetDirty(int dirty);
			int				GetDirty(void) const;

protected:

	// Dtor protected to prevent instantiation.
	virtual 				~WED_Persistent();

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

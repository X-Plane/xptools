#ifndef WED_PERSISTENT
#define WED_PERSISTENT

#include "WED_GUID.h"
#include "WED_Archive.h"
#include "AssertUtils.h"

class	IOReader;
class	IOWriter;

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
 */
#define DECLARE_PERSISTENT(__Class)		 							\
public: 															\
	static const char * class_ID;									\
	static WED_Persistent * Create(									\
								WED_Archive * parent,				\
								const WED_GUID& inGUID);			\
	static __Class * CreateTyped(									\
								WED_Archive * parent,				\
								const WED_GUID& inGUID);			\
	static void				Register(void);							\
	virtual const char * 	GetClass(void) const;					\
	virtual void * 			SafeCast(const char * class_id);		\
private:															\
	__Class(WED_Archive * parent);									\
	__Class(WED_Archive * parent, const WED_GUID& inGUID);			\
	virtual ~__Class();



#define DEFINE_PERSISTENT(__Class)								\
																\
const char * __Class::class_ID = #__Class;						\
																\
WED_Persistent * __Class::Create(								\
								WED_Archive * parent,			\
								const WED_GUID& inGUID)			\
{																\
	return new __Class(parent, inGUID);							\
}																\
																\
__Class * __Class::CreateTyped(									\
								WED_Archive * parent,			\
								const WED_GUID& inGUID)			\
{																\
	return new __Class(parent, inGUID);							\
}																\
																\
void __Class::Register(void) 									\
{																\
	WED_Persistent::Register(class_ID, Create);					\
}																\
																\
const char * __Class::GetClass(void) const						\
{																\
	return __Class::class_ID;									\
}																\
																\
void * __Class::SafeCast(const char * class_id)					\
{																\
	if (!strcmp(class_id, class_ID)) 	return this; 			\
										return NULL; 			\
}


// Safe cast macro based on safe-cast mechanism
#define SAFE_CAST(__Class, __Var) reinterpret_cast<__Class *>((__Var)->SafeCast(__Class::class_ID));



class	WED_Persistent {
public:

	typedef WED_Persistent * (* CTOR_f)(WED_Archive *, const WED_GUID&);

	// Persistent creation-destruction:
	static	void			Register(const char * id, CTOR_f ctor);
	static	WED_Persistent *CreateByClass(const char * id, WED_Archive * parent, const WED_GUID& inGUID);

							WED_Persistent(WED_Archive * parent);
							WED_Persistent(WED_Archive * parent, const WED_GUID& inGUID);

	// Destructor is hidden - you must call Delete.  This both enforces dynamic allocation
	// and guarantees that we can fully handle destruction BEFORE we lose our typing info!
			 void			Delete(void);
			 
	// Convenience routine for finding your peers:
	WED_Persistent *		Fetch(const WED_GUID& inGUID) const;

	// This method is called by the derived-class whenever its internals are changed
	// by an editing operation.  It is the convenient way to signal "we better start
	// recording persistent stuff".
			void 			StateChanged(void);
	
	// Methods provided by the base: all persistent objs must have an archive and
	// GUID - this is non-negotiable!
	
			WED_Archive *	GetArchive(void) const 				{ return mArchive; }
			void			GetGUID(WED_GUID& outGUID) const 	{ outGUID = mGUID; }
			const WED_GUID& GetGUID(void) const 				{ return mGUID; }
	
	// IO methods to read and write state of class to a data holder.  ReadFrom
	// does NOT call StateChanged!  Subclasses must provide.
	virtual	void 			ReadFrom(IOReader * reader)=0;
	virtual	void 			WriteTo(IOWriter * writer)=0;
	virtual const char *	GetClass(void) const=0;
	
	virtual void *			SafeCast(const char * class_id)=0;
	
protected:

	// Dtor protected to prevent instantiation.
	virtual 				~WED_Persistent();

private:

		WED_GUID		mGUID;
		WED_Archive *	mArchive;

		WED_Persistent();	// No ctor without archive!

		// wait on copy ctor for now
			WED_Persistent(const WED_Persistent& rhs);
			WED_Persistent& operator=(const WED_Persistent& rhs);

};			

template <class T>
class	WED_PersistentPtr {
	WED_Archive *		owner;
	WED_GUID			guid;
public:

	WED_PersistentPtr(T * target) : owner(target ? target->GetArchive() : NULL), guid(target ? target->GetGUID() : NULL_GUID) { }
	explicit WED_PersistentPtr(WED_Archive * iowner, T * target) : owner(iowner), guid(target ? target->GetGUID() : NULL_GUID) { }
	WED_PersistentPtr(WED_Archive * iowner, const WED_GUID& inGUID) : owner(iowner), guid(inGUID) { }
	WED_PersistentPtr(const WED_PersistentPtr& rhs) : owner(rhs.owner), guid(rhs.guid) { }
	
	WED_PersistentPtr& operator=(const WED_PersistentPtr& rhs) { Assert(owner == rhs.owner); owner = rhs.owner; guid = rhs.guid; return *this; }
	WED_PersistentPtr& operator=(T * target) { if (target == NULL) guid = NULL_GUID; else { Assert(owner == target->GetArchive()); guid = target->GetGUID(); } return *this; }
	WED_PersistentPtr& operator=(const WED_GUID& inGUID) { guid = inGUID; return *this; }
	
	bool operator==(const WED_PersistentPtr& rhs) const { return owner == rhs.owner && guid == rhs.guid; }
	bool operator==(const T * rhs) const { if (rhs) return owner == rhs->GetArchive() && guid == rhs->GetGUID(); else return guid == NULL_GUID; }

	bool operator!=(const WED_PersistentPtr& rhs) const { return owner != rhs.owner || guid != rhs.guid; }
	bool operator!=(const T * rhs) const { if (rhs) return owner != rhs->GetArchive() || guid != rhs->GetGUID(); else return guid != NULL_GUID; }

	bool operator< (const WED_PersistentPtr& rhs) const { if (owner == rhs.owner) return guid < rhs.guid; return owner < rhs.owner; }
	bool operator< (const T * rhs) const { if (rhs) { if (owner == rhs->GetArchive()) return guid < rhs->GetGUID(); return owner < rhs->GetArchive(); } return guid < NULL_GUID; }

	void ReadFrom(IOReader * reader) { guid.ReadFrom(reader); }
	void WriteTo(IOWriter * writer) { guid.WriteTo(writer); }

	T& operator * (void) const { if (owner == NULL) return NULL; WED_Persistent * obj = owner->Fetch(guid); if (obj) return *SAFE_CAST(T, obj); return NULL; }
	T * operator -> (void) const { if (owner == NULL) return NULL; WED_Persistent * obj = owner->Fetch(guid); if (obj) return SAFE_CAST(T, obj); return NULL; }
	operator T * () const { if (owner == NULL) return NULL; WED_Persistent * obj = owner->Fetch(guid); if (obj) return SAFE_CAST(T, obj); return NULL; }

private:

	WED_PersistentPtr();

};
#endif /* WED_PERSISTENT */

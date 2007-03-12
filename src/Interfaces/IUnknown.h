#ifndef IUNKNOWN_H
#define IUNKNOWN_H

/*
	IUnknown - THEORY OF OPERATION

	WED uses interfaces, as described by Don Box's "Essential COM" chapter 1.  Here's the main ideas:
	
	- Interfaces are abstract classes containing only pure virtual methods.
	- They do not contain destructors, virtual or otherwise.
	- They do not contain any memory management info (this is different than Don's ref-counted
	  interfaces).
	- They all derive from IUnknown, a generic facility for safely casting interfaces.
	- Class IDs are strings, e.g. "IUnknown" is theh interface class_id for IUnkown.
	- Inheritance is NOT with virtual base classes.  Because interfaces contain only pure virtual
	- functions, multiply inheriting an interface is NOT harmful in any way.
	
	(Why is it done like this?  Well, the goal is to make DLL-safe interfaces.  This means:
	
	- No RTTI/dynamic casts, since the RTTI data structures require linkeage of typeids.  Not terribly DLL-safe.
	- Simplest possible memory layout.  Virtual destructor can tweak the vtable.
	- No member functions - means the class is just a vtable ptr really.
	- Technically you'd need to provide allocation hooks in the interface since operator new/delete is not DLL-safe
	  but for us this must be addressed by infrastructure for plugin factories.
	
	WAIT - Ben says:
			If memory management is NOT a goal, better to make IUnknown virtual - that way we never have ambiguous 
			base downcast issues!  Just a time saver.
	
*/

class	IUnknown {
public:

	virtual void *			QueryInterface(const char * class_id)=0;

};

// Safe cast macro based on safe-cast mechanism

inline void * __SAFE_CAST(IUnknown * interface, const char * class_id)
{
	// We use an inline function to avoid double-evaluating our interface var, which might
	// come from an expensive expression in the macro!
	if (interface) return interface->QueryInterface(class_id);
	return NULL;
}

#define SAFE_CAST(__Class,__Var) reinterpret_cast<__Class *>(__SAFE_CAST(__Var,#__Class))

#endif

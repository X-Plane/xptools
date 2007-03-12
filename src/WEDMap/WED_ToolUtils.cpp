#include "WED_ToolUtils.h"
#include "ISelection.h"
#include "WED_Thing.h"
#include "IResolver.h"
#include <list>

using std::list;

// FIRST thing is root, last is WHO
static void GetLineage(WED_Thing * who, list<WED_Thing *>& parents)
{
	parents.clear();
	while(who)
	{
		parents.push_front(who);
		who = who->GetParent();
	}
}

static WED_Thing *	FindCommonParent(WED_Thing * a, WED_Thing * b)
{
	if (a == NULL) return b;
	if (b == NULL) return a;
	list<WED_Thing *> a_lin, b_lin;
	GetLineage(a,a_lin);
	GetLineage(b,b_lin);
	
	WED_Thing * best = NULL;
	
	list<WED_Thing *>::iterator a_it = a_lin.begin(), b_it = b_lin.begin();
	while (a_it != a_lin.end() && b_it != b_lin.end() && *a_it == *b_it)
	{
		best = *a_it;
		++a_it, ++b_it;
	}

	return best;
}



WED_Thing *	WED_FindParent(ISelection * isel, WED_Thing * require_this, WED_Thing * backup_choice)
{
	vector<IUnknown *> sel;
	isel->GetSelectionVector(sel);
	
	WED_Thing * common_parent = NULL;
	
	if (sel.empty()) return backup_choice;
	
	for (vector<IUnknown *>::iterator iter = sel.begin(); iter != sel.end(); ++iter)
	{
		WED_Thing * obj = SAFE_CAST(WED_Thing, *iter);
		if (obj)
		{
			WED_Thing * new_parent = FindCommonParent(obj, common_parent);
			DebugAssert(new_parent);
			common_parent = new_parent;
		}
	}

	DebugAssert(common_parent);

	if (require_this)
	if (FindCommonParent(common_parent, require_this) != require_this)
		return require_this;
		
	return common_parent;	
}


WED_Thing *		WED_GetCurrentAirport(IResolver * resolver)
{
	return SAFE_CAST(WED_Thing,resolver->Resolver_Find("choices.airport"));
}

ISelection *	WED_GetSelect(IResolver * resolver)
{
	return SAFE_CAST(ISelection,resolver->Resolver_Find("selection"));
}


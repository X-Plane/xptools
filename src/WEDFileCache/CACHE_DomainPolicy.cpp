#include "CACHE_DomainPolicy.h"
#include "AssertUtils.h"
#if LIN
#include <limits.h>
#endif

#undef INFINITE

const int INFINITE = INT_MAX; //Don't worry, by 2038 an AI will have instantly rewritten WED in Haskell during a code interview.
const int MINUTE = 60;
const int HOUR = MINUTE * 60;
const int DAY  = HOUR * 24;
const int WEEK = DAY * 7;

const CACHE_domain_policy k_domain_policies[] = {

	// age,          client,  server cooldown
	{ (MINUTE * 10), (MINUTE), (MINUTE) }, //none
	{ (DAY)        , (MINUTE), (MINUTE) }, //metadata_csv
	{ (MINUTE * 10), (MINUTE), (MINUTE) }, //airports_json
	{ (MINUTE * 10), (MINUTE), (MINUTE) }, //airports_versions_json
	{ (INFINITE),    (MINUTE), (MINUTE) }  //scenery_pack
};

CACHE_domain_policy GetDomainPolicy(CACHE_domain domain)
{
	DebugAssert(domain >= cache_domain_none && domain < cache_domain_end);
	return k_domain_policies[domain];
}
#include "CACHE_DomainPolicy.h"
#include "AssertUtils.h"

#undef INFINITE

const int INFINITE = -1;
const int MINUTE = 60;
const int HOUR = MINUTE * 60;
const int DAY  = HOUR * 24;
const int WEEK = DAY * 7;

const CACHE_domain_policy k_domain_policies[] = {
#if DEV
	//  buffer size age       client,    server cooldown
	{ 0,       0,            0,        0        }, //none
	{ 1100000, (MINUTE * 2), (MINUTE), (MINUTE) }, //metadata_csv
	{ 9000000, (MINUTE * 2), (MINUTE), (MINUTE) }, //airports_json
	{ 4000,    (MINUTE * 2), (MINUTE), (MINUTE) }, //airports_versions_json
	{ 6000,    (INFINITE),   (MINUTE), (MINUTE) }  //scenery_pack
#else
	// buffer size age       client,    server cooldown
	{ 0,       (WEEK * 2), (MINUTE), (MINUTE * 15) }, //none
	{ 1100000, (WEEK)    , (MINUTE), (MINUTE * 15) }, //metadata_csv
	{ 9000000, (INFINITE), (MINUTE), (MINUTE * 15) }, //airports_json
	{ 4000,    (WEEK * 2), (MINUTE), (MINUTE * 15) }, //airports_versions_json
	{ 6000,    (WEEK * 2), (MINUTE), (MINUTE * 15) }  //scenery_pack
#endif
};

CACHE_domain_policy GetDomainPolicy(CACHE_domain domain)
{
	DebugAssert(domain >= CACHE_domain::cache_domain_none && domain < CACHE_domain::cache_domain_end);
	return k_domain_policies[domain];
}
/* 
 * Copyright (c) 2016, Laminar Research.
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

#ifndef CACHE_DOMAINPOLICY_H
#define CACHE_DOMAINPOLICY_H

enum CACHE_domain
{
	cache_domain_none,
	//--Ordered by the stages of GatewayImport
	cache_domain_metadata_csv,
	cache_domain_airports_json,
	cache_domain_airport_versions_json,
	cache_domain_scenery_pack,
	//----------------------------------------
	cache_domain_osm_tile,
#if DEV
	cache_domain_debug,
#endif
	cache_domain_end //do not use!
};

struct CACHE_domain_policy
{
	//int                 cache_domain_pol_max_KB_on_disk;
	int                 cache_domain_pol_max_seconds_on_disk;
	int                 cache_domain_pol_min_client_cool_down_snds;
	int                 cache_domain_pol_min_server_cool_down_snds;
};

CACHE_domain_policy GetDomainPolicy(CACHE_domain domain);
#endif

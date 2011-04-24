/*
 * Copyright (c) 2008, Laminar Research.
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

/*

	BUILD:	gcc osm2shape.c -lz -lexpat -lshp -o osm2shape

	OSM2Shape converts ways to shape file arcs.  (An arc in a shape file is really a
	string of connected line segments.)  Attributes on the ways are put into a
	DBF table with the shape file.

	Requirements:
		expat
		shapelib
		zlib


	TODO:
		Provide filtering for way sub-segments that are out of crop box.
		Provide filtering for ways with no attributes.
		Provide filtering for attributes that we don't care about.

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>
#include <expat.h>
#include <shapefil.h>
#include <stdarg.h>

enum {
	MODE_WAYS = 0,
	MODE_POLYGONS = 1
};

static int	MAX_FIELD_LEN = 64;

static const char * ok_tags = NULL;
static int drop_empty = 0;
static int exp_mode = MODE_WAYS;

static SHPHandle sfile = NULL;
static DBFHandle dfile = NULL;

static int node_count = 0;			// Total counts
static int way_count = 0;
static int rel_count = 0;

static int max_node_refs = 0;
static int max_way_refs = 0;
static int max_rel_refs = 0;

static int node_free = 0;			// Used memory as we read ... entities and references
static int way_free = 0;
static int rel_free = 0;
static int node_ref_free = 0;
static int way_ref_free = 0;
static int rel_ref_free = 0;

static int nodes_now = 0;			// Tracking high water marks for how much stuff we ahve
static int ways_now = 0;
static int rels_now = 0;
static int last_node_id=-1;			// Counting info to track out-of-order XML files.
static int last_way_id=-1;
static int last_rel_id=-1;
static int must_sort_nodes;
static int must_sort_ways;
static int must_sort_rels;




static int is_way = 0;
static int is_rel = 0;
static int is_node = 0;

static int tot_f=0,tot_v=0,tot_a=0;

typedef struct tag_att_info {
	const char *			name;
	int						len;
	int						dbf_id;
	struct tag_att_info *	next;
} att_info_t;

static att_info_t *		g_atts = NULL;
static int				g_num_atts = 0;
static int				osm_id;

typedef struct {
	int id;
	char ** atts;
	int flag;
} entity_t;

typedef struct {
	entity_t	ent;
	double lon;
	double lat;
} node_info_t;

typedef struct {
	entity_t	ent;
	union {
		int *			node_ids;
		node_info_t **	node_ptrs;
	} nodes;
	int			node_count;
} way_info_t;

typedef struct rel_info_tag {
	entity_t	ent;
	union {
		int *			node_ids;
		node_info_t **	node_ptrs;
	} nodes;
	char **				node_roles;
	union {
		int *			way_ids;
		way_info_t **	way_ptrs;		
	} ways;
	char **				way_roles;
	union {
		int *					rel_ids;
		struct rel_info_tag **	rel_ptrs;
	} rels;
	char **				rel_roles;
	
	char *		rel_type;
	int			has_anything;		// This tracks whether we have any attributes other than type...even ones we dropped.  (The problem 
									// is that if we filter enough attributes, we think our outer relation is blank and we start promoting things.)	
	int			node_count;
	int			way_count;
	int			rel_count;
} rel_info_t;

static node_info_t * g_nodes = NULL;
static way_info_t * g_ways = NULL;
static rel_info_t * g_rels = NULL;

static int * g_temp_node_refs = NULL;
static char ** g_temp_node_role = NULL;
static int * g_temp_way_refs = NULL;
static char ** g_temp_way_role = NULL;
static int * g_temp_rel_refs = NULL;
static char ** g_temp_rel_role = NULL;

void die(const char * fmt, ...)
{
	va_list va;
	va_start(va,fmt);
	vprintf(fmt, va);
	exit(1);
}


int ent_compare(const void * a, const void * b)
{
	const entity_t * aa = (entity_t *) a;
	const entity_t * bb = (entity_t *) b;
	return aa->id - bb->id;
}

node_info_t * find_node(int id)
{
	entity_t key;
	key.id = id;
	return(node_info_t *) bsearch(&key, g_nodes, node_count, sizeof(node_info_t), ent_compare);
}

way_info_t * find_way(int id)
{
	entity_t key;
	key.id = id;
	return(way_info_t *) bsearch(&key, g_ways, way_count, sizeof(way_info_t), ent_compare);
}

rel_info_t * find_rel(int id)
{
	entity_t key;
	key.id = id;
	return(rel_info_t *) bsearch(&key, g_rels, rel_count, sizeof(rel_info_t), ent_compare);
}

static const char * type_list[] = { "node", "way", "relation", 0 };
enum { type_node = 0, type_way, type_relation };

void convert_way_ptrs(int idx)
{
	int i, id;
	way_info_t * w = g_ways+idx;
	for(i = 0; i < w->node_count; ++i)
	{
		w->nodes.node_ptrs[i] = find_node(id=w->nodes.node_ids[i]);
//		if(w->nodes.node_ptrs[i] == NULL) die("Way %d references missing node %d.\n", w->ent.id, id);
	}
}

void convert_rel_ptrs(int idx)
{
	int i,id;
	rel_info_t * r = g_rels+idx;
	for(i = 0; i < r->node_count; ++i)
	{
		r->nodes.node_ptrs[i] = find_node(id=r->nodes.node_ids[i]);
//		if(r->nodes.node_ptrs[i] == NULL) die("Rel %d references missing node %d.\n", r->ent.id, id);
	}
	for(i = 0; i < r->way_count; ++i)
	{
		r->ways.way_ptrs[i] = find_way(id=r->ways.way_ids[i]);
		if(r->ways.way_ptrs[i] == NULL)
		{
			if(r->way_roles[i]) free(r->way_roles[i]);
	
			--r->way_count;
			if(i != r->way_count)
			{		
				r->ways.way_ptrs[i] = r->ways.way_ptrs[r->way_count];
				r->way_roles[i] = r->way_roles[r->way_count];
			}
			--i;
		}
	}
	for(i = 0; i < r->rel_count; ++i)
	{
		r->rels.rel_ptrs[i] = find_rel(id=r->rels.rel_ids[i]);
//		if(r->rels.rel_ptrs[i] == NULL) die("Rel %d references missing rel %d.\n", r->ent.id, id);
	}	
}

int equal_keys(const entity_t * a, const entity_t * b)
{
	int i;
	if(!a->atts || !b->atts) return a->atts == b->atts;
	for(i = 0; i < g_num_atts; ++i)
	{
		if(!a->atts[i] && !b->atts[i]) continue;
		if(!a->atts[i] || !b->atts[i]) return 0;

		if(strcmp(a->atts[i],b->atts[i]) != 0) return 0;
	}
	return 1;	
}

void migrate_atts(entity_t * dst, entity_t * src)
{
	int a;

	// No src at all?  Bail
	if(src->atts == NULL) return;	
	
	// No dest?  Take the src as a whole.
	if(dst->atts == NULL)
	{
		dst->atts = src->atts;
		src->atts = NULL;
	}
	else for(a = 0; a < g_num_atts; ++a)
	if(src->atts[a])
	{
		if(dst->atts[a])
		{
			// We have a src and a dest.  Zap equal att, if we have it.
			if(strcmp(src->atts[a],dst->atts[a]) == 0)
			{
				free(src->atts[a]);
				src->atts[a] = NULL;
			}
		}
		else
		{
			// Src but no dest - move it over.
			dst->atts[a] = src->atts[a];
			src->atts[a] = NULL;
		}
	}
}

void swap_double(double * a, double * b)
{
	double t = *a;
	*a = *b;
	*b = t;
}

int is_inner(int r, int w)
{
	if(g_rels[r].way_roles[w] == NULL) return 0;
	return strcmp(g_rels[r].way_roles[w],"inner") == 0;
}

int is_multipolygon(int r)
{
	return (g_rels[r].rel_type && strcmp(g_rels[r].rel_type, "multipolygon")==0);
}

void kill_way(int r, int w)
{
	rel_info_t * rel = g_rels+r;
	--rel->way_count;
	
	if(rel->way_roles[w]) free(rel->way_roles[w]);
	
	if(w != rel->way_count)
	{		
		rel->ways.way_ptrs[w] = rel->ways.way_ptrs[rel->way_count];
		rel->way_roles[w] = rel->way_roles[rel->way_count];
	}
}

#pragma mark -
/********************************************************************************************************************************************
 * XML UTILS
 ********************************************************************************************************************************************

*/

int is_left_turn(double x1, double y1, double x2, double y2, double x3, double y3)
{
	double vx1 = x2 - x1;
	double vy1 = y2 - y1;
	double vx2 = x3 - x2;
	double vy2 = y3 - y2;
	
	return (
		-vy1 * vx2 + 
		 vx1 * vy2
		) > 0.0;
}

int is_ccw_poly(double * x, double * y, int c)
{
	if(c < 3) return 1;
	
	int highest, i, p, n;

	// First: scan to find any one vertex that is on the top of the polygon, for sure.  No one is higher!
	highest = 0;
	for (i=1; i < c; ++i)
		if(y[i] > y[highest])
			highest = i;

	// P = prev, N = next in our ring.
	p = (highest + c - 1) % c;
	n = (highest     + 1) % c;
	
	
	// All 3 are colinear in Y?  Means the top of the polygon is flat.  In that case case,
	// decreasing X is CCW.
	if(y[highest] == y[p] && y[highest] == y[n])
	{
		return x[n] < x[highest];
	}
	
	// Not colinear - since middle pt is highest.  Means we can use orientation test.
	return is_left_turn(x[p],y[p],x[highest],y[highest],x[n],y[n]);
	
}

void reverse_poly(double * x, double * y, int c)
{	
	int i = 0, j = c-1;
	while(i < j)
	{
		swap_double(x+i,x+j);
		swap_double(y+i,y+j);
		++i;
		--j;
	}
}



static void do_help(void)
{
	fprintf(stderr,"Usage: osm2shape [-d] [-p] [-w] [-t<tag list>] [-l<max length>] <osm file> <shape file>\n");
	exit(1);
}

static int * copy_int(const int * src, int count)
{
	int * dst = (int *) malloc(sizeof(int) * count);
	memcpy(dst, src, count * sizeof(int));
	return dst;
}


static char * strdup_null(const char * s)
{
	if(!s) return NULL;
	if(!s[0]) return NULL;
	return strdup(s);
}

static char ** copy_str(char ** src, int count)
{
	int n;
	char ** dst = (char **) malloc(sizeof(char *) * count);
	for(n = 0; n < count; ++n)
	dst[n] = strdup_null(src[n]);
	return dst;
}

static int fetch_att_i(const char * key, const char ** att, int def)
{
	while(*att)
	{
		if(strcmp(*att++, key)==0)
		{
			return atoi(*att);
		}
		++att;
	}
	return def;
}

static double fetch_att_d(const char * key, const char ** att, double def)
{
	while(*att)
	{
		if(strcmp(*att++, key)==0)
		{
			return atof(*att);
		}
		++att;
	}
	return def;
}

static const char * fetch_att_s(const char * key, const char ** att)
{
	while(*att)
	{
		if(strcmp(*att++, key)==0)
		{
			return *att;
		}
		++att;
	}
	return NULL;
}

static int match_str_attr(const char * key, const char * values[], const XML_Char ** atts)
{
	while(*atts)
	{
		if(strcmp(*atts++,key)==0)
		{
			int n = 0;
			while(values[n])
			{
				if(strcmp(*atts,values[n])==0) return n;
				++n;
			}
			return -1;
		}
		++atts;
	}
	return -1;
}

static int ok_attr(const char * attr, const char * list)
{
	if(list == NULL) return 1;
	int al = strlen(attr);
	const char * l = list;
	if(al == 0) return 0;
	while(l)
	{
		const char * sub = strstr(l,attr);
		if(!sub) return 0;						// not found
		if(sub[al] == 0 || sub[al] == ',')
			return 1;
		l = sub+al;


	}
	return 0;

}

static void accum_attribute(const char * key, int len)
{
	if(!ok_attr(key,ok_tags)) return;
	att_info_t * i;
	att_info_t * na;
	for(i = g_atts; i; i = i->next)
	{
		if(strcmp(key,i->name)==0)
		{
			if(len > i->len) i->len = len;
			return;
		}
	}
	na = (att_info_t *) malloc(sizeof(att_info_t));
	na->name = strdup(key);
	na->len = len;
	na->dbf_id = 0;
	na->next = g_atts;
	g_atts = na;
	++g_num_atts;
}



static XML_Parser   g_parser=NULL;
static gzFile		g_file=NULL;
static char			g_buf[1024*1024*16];

void xml_die(const char * fmt, ...)
{
	va_list va;
	va_start(va,fmt);
	vprintf(fmt, va);

	printf("Line: %d, Col: %d (Byte %d)\n",
		XML_GetCurrentLineNumber(g_parser),
		XML_GetCurrentColumnNumber(g_parser),
		XML_GetCurrentByteIndex(g_parser));

	exit(1);
}



void run_file(const char * fname,XML_StartElementHandler start,XML_EndElementHandler end)
{
	if(g_parser == NULL)
		g_parser = XML_ParserCreate(NULL);
	else
		XML_ParserReset(g_parser, NULL);

	XML_SetElementHandler(g_parser, start,end);

	if(g_file == NULL)
		g_file = gzopen(fname,"rb");
	else
		gzrewind(g_file);
	if(g_file == NULL) die("Could not open file: %s\n", fname);

	while(!gzeof(g_file))
	{
		int len = gzread(g_file,g_buf,sizeof(g_buf));
		XML_Parse(g_parser, g_buf, len, 0);
	}
	XML_Parse(g_parser, g_buf, 0, 1);
}


#pragma mark -

/********************************************************************************************************************************************
 * COUNTING PASS
 ********************************************************************************************************************************************

	We are going to find out how much of everything we have: all attributes (with culling), all nodes/ways/rels, and how many references the
	biggest way-node-rel has.  That last bit of information will be used to allocate temp buffers during the data read-in.

*/

void StartElementHandler_Count(void *userData,
					const XML_Char *name,
					const XML_Char **atts)
{
	int id;
	
	if(strcmp(name,"node") == 0)
	{
		is_node=1;
		++node_count;
		id = fetch_att_i("id", atts,-1);
		if(id==-1) xml_die("node has no id.\n");
		if(id == last_node_id) xml_die("error: dupe node id %d\n", id);
		if(id < last_node_id)
			must_sort_nodes=1;
		last_node_id = id;		
	}
	else if(strcmp(name,"way") == 0)
	{
		is_way=1;
		++way_count;
		nodes_now = 0;
		id = fetch_att_i("id", atts,-1);
		if(id==-1) xml_die("way has no id.\n");
		if(id == last_way_id) xml_die("error: dupe way id %d\n", id);
		if(id < last_way_id)
			must_sort_ways=1;
		last_way_id = id;		
	}
	else if(strcmp(name,"relation") == 0)
	{
		is_rel=1;
		++rel_count;
		nodes_now = 0;
		ways_now = 0;
		rels_now = 0;
		id = fetch_att_i("id", atts,-1);
		if(id==-1) xml_die("rel has no id.\n");
		if(id == last_rel_id) xml_die("error: dupe rel id %d\n", id);
		if(id < last_rel_id)
			must_sort_rels=1;
		last_rel_id = id;		
	}
	else if(strcmp(name,"tag") == 0)
	{
		const char * v, * k;
		if(!is_way && !is_node && !is_rel)
		{
			const char ** a = atts;
			fprintf(stderr,"ERROR: tag found outside a way or node!\n");
			while(*a)
			{
				fprintf(stderr,"   %s=",*a++);
				fprintf(stderr,"%s\n",*a++);
			}
			exit(1);
		}

		v = fetch_att_s("v",atts);
		k = fetch_att_s("k",atts);
		if(v == NULL || k == NULL) xml_die("ERROR: tag missing v and k attributes.\n");

		accum_attribute(k,strlen(v));
	}
	else if(strcmp(name,"nd") == 0)
	{
		if(!is_way)	xml_die("nd ref found outside a way.\n");
		++nodes_now;
	}
	else if(strcmp(name,"member") == 0)
	{
		if(!is_rel) xml_die("member ref found outside a relation.\n");
		int t = match_str_attr("type", type_list, atts);
		switch(t) {
		case type_node: ++nodes_now;	break;
		case type_way: ++ways_now;		break;
		case type_relation: ++rels_now;		break;
		default: xml_die("Unknown member type in relation.");
		}
	}	
}



void EndElementHandler_Count(void *userData,
				      const XML_Char *name)
{
	if(strcmp(name,"node") == 0)
	{
		is_node=0;
	}
	else if(strcmp(name,"way") == 0)
	{
		is_way=0;
		if(max_node_refs < nodes_now) max_node_refs = nodes_now;
	}
	else if(strcmp(name,"relation") == 0)
	{
		is_rel=0;
		if(max_node_refs < nodes_now) max_node_refs = nodes_now;
		if(max_way_refs < ways_now) max_way_refs = ways_now;
		if(max_rel_refs < rels_now) max_rel_refs = rels_now;
	}

}

/********************************************************************************************************************************************
 * DATA READ PASS
 ********************************************************************************************************************************************

*/



void StartElementHandler_ReadData(void *userData,
					const XML_Char *name,
					const XML_Char **atts)
{
	if(strcmp(name,"node") == 0)
	{
		g_nodes[node_free].ent.id = fetch_att_i("id", atts,-1);
		g_nodes[node_free].lat = fetch_att_d("lat", atts,9999.0);
		g_nodes[node_free].lon = fetch_att_d("lon", atts,9999.0);
		if(g_nodes[node_free].lat == 9999.9) xml_die("Node is missing lat.");
		if(g_nodes[node_free].lon == 9999.9) xml_die("Node is missing lon.");
		is_node = 1;
	}
	else if(strcmp(name,"way") == 0)
	{
		node_ref_free = 0;
		g_ways[way_free].ent.id = fetch_att_i("id", atts,-1);
		is_way = 1;
	}
	else if(strcmp(name,"relation") == 0)
	{
		node_ref_free = way_ref_free = rel_ref_free = 0;
		g_rels[rel_free].ent.id = fetch_att_i("id", atts,-1);
		is_rel = 1;
	}
	else if(is_way && strcmp(name,"nd")==0)
	{
		g_temp_node_refs[node_ref_free++] = fetch_att_i("ref", atts, -1);
	}
	else if (is_rel && strcmp(name,"member")==0)
	{
		int t = match_str_attr("type", type_list, atts);
		switch(t) {
		case type_node: 
			g_temp_node_refs[node_ref_free  ] = fetch_att_i("ref",atts,-1);	
			g_temp_node_role[node_ref_free++] = strdup_null(fetch_att_s("role",atts));
			break;
		case type_way:
			g_temp_way_refs[way_ref_free  ] = fetch_att_i("ref",atts,-1);	
			g_temp_way_role[way_ref_free++] = strdup_null(fetch_att_s("role",atts));
			break;
		case type_relation:
			g_temp_rel_refs[rel_ref_free  ] = fetch_att_i("ref",atts,-1);	
			g_temp_rel_role[rel_ref_free++] = strdup_null(fetch_att_s("role",atts));
			break;
		}		
	}
	else if(strcmp(name,"tag")==0)
	{	
		att_info_t * a;
		const char * k = fetch_att_s("k",atts);
		int idx = 0;
		
		if(is_rel && strcmp(k,"type") == 0)
			g_rels[rel_free].rel_type = strdup(fetch_att_s("v", atts));
		else
		{
			entity_t * me = NULL;
			if(is_node) me = &g_nodes[node_free].ent;
			if(is_way) me = &g_ways[way_free].ent;
			if(is_rel) me = &g_rels[rel_free].ent;
			
			if(is_rel && me)
				g_rels[rel_free].has_anything = 1;
			
			if(me)
			for(a = g_atts; a; a = a->next, ++idx)
			if(strcmp(a->name,k)==0)
			{
				if(me->atts == NULL)
				{
					me->atts = (char **) malloc(sizeof(char *) * g_num_atts);
					memset(me->atts, 0, sizeof(char *) * g_num_atts);
				}
			
				me->atts[idx] = strdup(fetch_att_s("v",atts));
				break;
			}
		}
	}
}

void EndElementHandler_ReadData(void *userData,
				      const XML_Char *name)
{
	if(is_node && strcmp(name,"node")==0)
	{
		is_node = 0;
		++node_free;
	}
	if(is_way && strcmp(name,"way")==0)
	{
		g_ways[way_free].node_count = node_ref_free;
		if(node_ref_free > 0)
			g_ways[way_free].nodes.node_ids = copy_int(g_temp_node_refs, node_ref_free);
		is_way = 0;
		++way_free;
		
	}
	if(strcmp(name,"relation") == 0)
	{
		g_rels[rel_free].node_count = node_ref_free;
		g_rels[rel_free].way_count = way_ref_free;
		g_rels[rel_free].rel_count = rel_ref_free;

		if(node_ref_free > 0)
		{
			g_rels[rel_free].nodes.node_ids = copy_int(g_temp_node_refs, node_ref_free);
			g_rels[rel_free].node_roles = copy_str(g_temp_node_role, node_ref_free);
		}
		if(way_ref_free > 0)
		{
			g_rels[rel_free].ways.way_ids = copy_int(g_temp_way_refs, way_ref_free);
			g_rels[rel_free].way_roles = copy_str(g_temp_way_role, way_ref_free);
		}
		if(rel_ref_free > 0)
		{
			g_rels[rel_free].rels.rel_ids = copy_int(g_temp_rel_refs, rel_ref_free);
			g_rels[rel_free].rel_roles = copy_str(g_temp_rel_role, rel_ref_free);
		}

		is_rel = 0;
		++rel_free;
	}
}

#pragma mark -

/********************************************************************************************************************************************
 * EXPORT CODE
 ********************************************************************************************************************************************

*/

int write_ent_attributes(DBFHandle dbf, int id, entity_t * ent)
{
	int r = 0;
	att_info_t * a;
	int c = 0;
	if(ent->atts)
	for(a = g_atts; a; a = a->next, ++c)
	{
		if(ent->atts[c])
		{
			if(strlen(ent->atts[c]) > MAX_FIELD_LEN)
				ent->atts[MAX_FIELD_LEN] = 0;
		
			DBFWriteStringAttribute(dbf, id, a->dbf_id, ent->atts[c]);
			++r;
		}
		else 
		{
			// null
		}
	}
//	if(!ent->atts && g_atts)
//		DBFWriteNULLAttribute(dbf, id, g_atts->dbf_id);
	char ent_string[10];
	sprintf(ent_string,"%d", ent->id);
	DBFWriteStringAttribute(dbf, id, osm_id, ent_string);
	return r;
}

int main(int argc, const char * argv[])
{
	att_info_t * a;
//	gzFile * fi;
	int id;
	
	++argv;
	--argc;

	while(argc > 0)
	{
		if(argv[0][0] == '-')
		{
			switch(argv[0][1]) {
			case 't':	ok_tags = (*argv)+2;	break;
			case 'l':	MAX_FIELD_LEN = atoi(*argv+2);		break;
			case 'd':	drop_empty = 1;			break;
			case 'p':	exp_mode = MODE_POLYGONS;	break;
			case 'w':	exp_mode = MODE_WAYS;	break;
			default:
				do_help();
			}
			++argv;
			--argc;

		}
		else
			break;
	}

	printf("Converting %s to %s\n", argv[0], argv[1]);

	if(argc != 2)
		do_help();

	printf("Useful tags: %s, Drop empty: %s.  Max field len: %d.\n", ok_tags ? ok_tags : "none", drop_empty ? "yes" : "no", MAX_FIELD_LEN);

	/* PASS 1 - Count all nodes and ways, so we can allocate memory. */

	run_file(argv[0], StartElementHandler_Count, EndElementHandler_Count);

	printf("%d nodes, %d ways %d rels.\n", node_count, way_count, rel_count);
	printf("%d node refs, %d way refs, %d rel refs.\n", max_node_refs, max_way_refs, max_rel_refs);

	g_nodes = (node_info_t *) malloc(node_count * sizeof(node_info_t));
	g_ways = (way_info_t *) malloc(way_count * sizeof(way_info_t));
	g_rels = (rel_info_t *) malloc(rel_count * sizeof(rel_info_t));
	
	memset(g_nodes,0,node_count * sizeof(node_info_t));
	memset(g_ways,0,way_count * sizeof(way_info_t));
	memset(g_rels,0,rel_count * sizeof(rel_info_t));

	g_temp_node_refs = (int *) malloc(max_node_refs * sizeof(int));
	g_temp_way_refs = (int *) malloc(max_way_refs * sizeof(int));
	g_temp_rel_refs = (int *) malloc(max_rel_refs * sizeof(int));

	g_temp_node_role = (char **) malloc(max_node_refs * sizeof(char *));
	g_temp_way_role = (char **) malloc(max_way_refs * sizeof(char *));
	g_temp_rel_role = (char **) malloc(max_rel_refs * sizeof(char *));

	/* PASS 2 - Read data. */

	run_file(argv[0], StartElementHandler_ReadData, EndElementHandler_ReadData);

	if(node_free != node_count)	die("Accounting error: mismatch nodes.\n");
	if(way_free != way_count)	die("Accounting error: mismatch ways.\n");
	if(rel_free != rel_count)	die("Accounting error: mismatch rels.\n");

	if(must_sort_nodes)
	{
		printf("Nodes were out of order - sorting.\n");
		qsort(g_nodes, node_count, sizeof(node_info_t), ent_compare);
	}
	if(must_sort_ways)
	{
		printf("ways were out of order - sorting.\n");
		qsort(g_ways, way_count, sizeof(way_info_t), ent_compare);
	}
	if(must_sort_rels)
	{
		printf("rels were out of order - sorting.\n");
		qsort(g_rels, rel_count, sizeof(rel_info_t), ent_compare);
	}



	for(a = g_atts; a; a = a->next)
	{
		printf("     %s (%d)\n", a->name,a->len);
		if(a->len > MAX_FIELD_LEN) a->len = MAX_FIELD_LEN;
	}

	printf("Converting ptrs.\n");
	for(id = 0; id < way_count; ++id)
		convert_way_ptrs(id);
	for(id = 0; id < rel_count; ++id)
		convert_rel_ptrs(id);

	/* PASS 3 - Read and process all ways. */

	printf("Writing shape file.\n");

	sfile = SHPCreate(argv[1], (exp_mode == MODE_WAYS ? SHPT_ARC : SHPT_POLYGON));
	dfile = DBFCreate(argv[1]);
	if(sfile == NULL)
	{
		fprintf(stderr,"Could not create shape file %s.\n", argv[1]);
		exit(1);
	}
	if (dfile == NULL)
	{
		fprintf(stderr, "Could not create dbf file for %s.\n", argv[1]);
		exit(1);
	}
	for(a=g_atts;a;a=a->next)
	{
		a->dbf_id = DBFAddField(dfile, a->name, FTString, a->len, 0);
	}
	osm_id = DBFAddField(dfile,"osm_id",FTString,8,0);

	if(exp_mode == MODE_WAYS)
	for(id = 0; id < way_count; ++id)
	{
		int i, count = 0;
		double * x, * y;
		if(g_ways[id].ent.atts == NULL && drop_empty) continue;
	
		x = (double *) malloc(sizeof(double) * g_ways[id].node_count);
		y = (double *) malloc(sizeof(double) * g_ways[id].node_count);
		
		for(i = 0; i < g_ways[id].node_count; ++i)
		{
			node_info_t * n = g_ways[id].nodes.node_ptrs[i];
			if(n)
			{
				x[count] = n->lon;
				y[count] = n->lat;
				++count;
			}
		}
		
		if(count)
		{
			int sid;
			SHPObject * obj = SHPCreateSimpleObject(SHPT_ARC, count, x, y, NULL);
			++tot_f;
			tot_v += count;
			sid = SHPWriteObject(sfile, -1, obj);
			SHPDestroyObject(obj);

			tot_a += write_ent_attributes(dfile, sid, &g_ways[id].ent);
		}
		
		free(x);
		free(y);
	}

	if(exp_mode == MODE_POLYGONS)
	{
		// Zero: detect topology errors in multipolygons
		for(id = 0; id < rel_count; ++id)
		if(is_multipolygon(id))
		{
			rel_info_t * rel = g_rels+id;		
			int w,n;
			int ok = 1;
			int total_nodes = 0;
			
			// Ben says: this is NOT quite the greatest scheme.  The problem is that we DEMAND no X shapes...because we don't 
			// know how the hell to resolve them.  Unfortunately you sometimes DO get them.  We end up nuking the X ways.
			
			while(1)
			{
				// Our count for connectivity has to be RECURSIVE.  The reason: we can have a way with a valid link on one side
				// (e.g. our start matches someone else's end exactly, for a count of 2) but our way's end is hanging.  We nuke
				// the way, but our neighbor is considered link.  He links to us and we are DEAD!  So we need to recount with
				// us gone to determine that our friend is gone.
				//
				// A lot of the time this is going to chain react heavily and nuke a LOT more stuff.  But we might recover, e.g. if
				// our outer is sane and our inner is just goo.
				int did_nuke = 0;
				
				for(w=0;w<rel->way_count;++w)
				for(n=0;n<rel->ways.way_ptrs[w]->node_count; ++n, ++total_nodes)
					rel->ways.way_ptrs[w]->nodes.node_ptrs[n]->ent.flag = 0;

				for(w=0;w<rel->way_count;++w)
				if(rel->ways.way_ptrs[w]->nodes.node_ptrs[0] != rel->ways.way_ptrs[w]->nodes.node_ptrs[rel->ways.way_ptrs[w]->node_count-1])
				{
					rel->ways.way_ptrs[w]->nodes.node_ptrs[0]->ent.flag++;
					rel->ways.way_ptrs[w]->nodes.node_ptrs[rel->ways.way_ptrs[w]->node_count-1]->ent.flag++;
				}
					
				for(w=0;w<rel->way_count;++w)
				if(rel->ways.way_ptrs[w]->nodes.node_ptrs[0] != rel->ways.way_ptrs[w]->nodes.node_ptrs[rel->ways.way_ptrs[w]->node_count-1])				
				{
					if(rel->ways.way_ptrs[w]->nodes.node_ptrs[0]->ent.flag != 2)
					{
						printf("     Node %d has %d links in way %d\n", 
								rel->ways.way_ptrs[w]->nodes.node_ptrs[0]->ent.id,
								rel->ways.way_ptrs[w]->nodes.node_ptrs[0]->ent.flag,
								rel->ways.way_ptrs[w]->ent.id);
						ok = 0;
					}
					if(rel->ways.way_ptrs[w]->nodes.node_ptrs[rel->ways.way_ptrs[w]->node_count-1]->ent.flag != 2)
					{
						printf("     Node %d has %d links in way %d\n", 
								rel->ways.way_ptrs[w]->nodes.node_ptrs[rel->ways.way_ptrs[w]->node_count-1]->ent.id,
								rel->ways.way_ptrs[w]->nodes.node_ptrs[rel->ways.way_ptrs[w]->node_count-1]->ent.flag,
								rel->ways.way_ptrs[w]->ent.id);
						ok = 0;
					}
					
					if(rel->ways.way_ptrs[w]->nodes.node_ptrs[0]->ent.flag != 2 ||
					   rel->ways.way_ptrs[w]->nodes.node_ptrs[rel->ways.way_ptrs[w]->node_count-1]->ent.flag != 2)
					{
						kill_way(id,w);
						did_nuke = 1;
						break;
					}
				}
				if(!did_nuke)
					break;
			}
			
			if(!ok)
			{
				printf("Relation %d has a topology link error. (%d nodes, %d ways\n", rel->ent.id, total_nodes, rel->way_count);
			}
			
			n=0;
			for(w=0;w<rel->way_count;++w)
			if(!is_inner(id,w)) 
				++n;

			if(n == 0)
			{
				printf("(Relation %d is entirely removed, as it is unsalvageable.\n",rel->ent.id);
				free(rel->rel_type);
				rel->rel_type = strdup("<broken>");
			}
		}			
		
		// First: migrate all non-inner multipolygon attributes out to the outer member.
		// Hrm -- there isn't really a good heuristic for this.  Try doing this if we have ONLY a type=multiplygon tag?
		for(id = 0; id < rel_count; ++id)
		if(is_multipolygon(id))
		if(!g_rels[id].has_anything)
		{
			int w;
			for(w = 0; w < g_rels[id].way_count; ++w)
			if(!is_inner(id,w))			
				migrate_atts(&g_rels[id].ent,&g_rels[id].ways.way_ptrs[w]->ent);
		}

		// Second: emit all multipolygons.
		for(id = 0; id < rel_count; ++id)
		if(is_multipolygon(id))
		if(g_rels[id].ent.atts != NULL || !drop_empty)
		{
			printf("Exporting relation: %d\n", g_rels[id].ent.id);
			SHPObject * obj;
			int sid;
			double * x, * y;
			rel_info_t * rel = g_rels+id;
			int * offsets;
			int w, ww;
			int total = 0;
			int cur_r = 0;
			int cur_p = 0;
			for(w = 0; w < rel->way_count; ++w)
			{
				total += rel->ways.way_ptrs[w]->node_count;
				rel->ways.way_ptrs[w]->ent.flag = 0;
			}
							
			offsets = (int *) malloc(sizeof(int) * rel->way_count);		// Max rings: no more than one ring per way
			x = (double *) malloc(sizeof(double) * total);				
			y = (double *) malloc(sizeof(double) * total);
			
			for(w = 0; w < rel->way_count; ++w)
			if(rel->ways.way_ptrs[w]->ent.flag == 0)
			{
				int n;
				way_info_t * main_way = rel->ways.way_ptrs[w];
				int main_inner = is_inner(id, w);
				
				// Push our main way onto the ring list.  This includes an offset!
				// Always push it forward.

				node_info_t * want_stop = main_way->nodes.node_ptrs[0];
				node_info_t * want_link = main_way->nodes.node_ptrs[main_way->node_count-1];
				main_way->ent.flag = 1;
				
//				printf("Exporting way %d\n", main_way->ent.id);
				
				offsets[cur_p] = cur_r;
				for(n = 0; n < main_way->node_count; ++n)
				{
					x[cur_r] = main_way->nodes.node_ptrs[n]->lon;
					y[cur_r] = main_way->nodes.node_ptrs[n]->lat;
					++cur_r;
				}

				// Now for all the next ways, go searching for the closing parts of the ring.
				// We have to search the ENTIRE list, not just the LATER part of our list, because
				// the linkeage in a set of ways sometimes goes backward.
				for(ww = 0; ww < rel->way_count; ++ww)
				if(main_inner == is_inner(id,ww))			// ONLY look at nodes with the same type.  Where the outer and inner touch each other, this helps us keep from freaking out.
				{
					// Stop condition - we link up.
					if(want_link == want_stop)	
						break;
						
					if(rel->ways.way_ptrs[ww]->ent.flag == 0)
					{
						way_info_t * link_way = rel->ways.way_ptrs[ww];
						int is_inner_link = is_inner(id,ww);

						// First, check for a forward link, e.g. this way does not need to be reversed to link up.
						if(link_way->nodes.node_ptrs[0] == want_link)
						{
//							printf("Exporting way %d\n", link_way->ent.id);
							if(is_inner_link != main_inner) 
								die("inner-outer conflict.");
							link_way->ent.flag = 1;
							for(n = 1; n < link_way->node_count; ++n)
							{
								x[cur_r] = link_way->nodes.node_ptrs[n]->lon;
								y[cur_r] = link_way->nodes.node_ptrs[n]->lat;
								++cur_r;
							}
							want_link = link_way->nodes.node_ptrs[link_way->node_count-1];
							ww = -1;
						}
						
						// Second, check for opposite direction linkage.						
						else if (link_way->nodes.node_ptrs[link_way->node_count-1] == want_link)
						{
//							printf("Exporting way %d\n", link_way->ent.id);
							if(is_inner_link != main_inner) 
								die("inner-outer conflict.");
							link_way->ent.flag = 1;
							for(n = link_way->node_count-2; n >= 0; --n)
							{
								x[cur_r] = link_way->nodes.node_ptrs[n]->lon;
								y[cur_r] = link_way->nodes.node_ptrs[n]->lat;
								++cur_r;
							}
							want_link = link_way->nodes.node_ptrs[0];
							ww = -1;
						}
					}
				}
				
				if(want_link != want_stop)
				{
					int wc, nc;
					printf("ERROR with rel: %d (type=%s)\n", rel->ent.id, rel->rel_type ? rel->rel_type : "<none>");
					printf("WAYS:\n");
					for(wc=0;wc < rel->way_count; ++wc)
					{
						printf("   Way %d: %d (%s) used = %d\n", wc, rel->ways.way_ptrs[wc]->ent.id, rel->way_roles[wc] ? rel->way_roles[wc] : "<null>", rel->ways.way_ptrs[wc]->ent.flag);
						printf("     contains %d nodes.\n", rel->ways.way_ptrs[wc]->node_count); 
						for(nc = 0; nc < rel->ways.way_ptrs[wc]->node_count; ++nc)
							printf("          %d: %d\n", nc, rel->ways.way_ptrs[wc]->nodes.node_ptrs[nc]->ent.id);
					}
					printf("We were trying to connect to %d or stop at %d.  But no remaining ways do that.\n",
						want_link->ent.id, want_stop->ent.id);
					die("error connecting multipolygon.\n");
				}
								
				if(is_ccw_poly(x+offsets[cur_p],y+offsets[cur_p],cur_r - offsets[cur_p]))
				{
					if(!main_inner)
						reverse_poly(x+offsets[cur_p],y+offsets[cur_p],cur_r - offsets[cur_p]);
				} else {
					if(main_inner)
						reverse_poly(x+offsets[cur_p],y+offsets[cur_p],cur_r - offsets[cur_p]);
				}
	
				++cur_p;
			}
			
			
			obj = SHPCreateObject(SHPT_POLYGON, -1, cur_p, offsets, NULL, cur_r, x, y, NULL, NULL);
			++tot_f;
			tot_v += cur_r;
			sid = SHPWriteObject(sfile, -1, obj);
			SHPDestroyObject(obj);

			tot_a += write_ent_attributes(dfile, sid, &g_rels[id].ent);
			
			free(offsets);
			free(x);
			free(y);

		}
		
		// Third: polygon iteration.  This takes any simple "area" ways.  Since we have moved polygons,
		// any of the ways that had multipolygon attributes have hoooopefully had their attributes stripped off.
	
		for(id = 0; id < way_count; ++id)
		{
			int i, count = 0;
			double * x, * y;
			if(g_ways[id].ent.atts == NULL && drop_empty) continue;
		
			if(g_ways[id].node_count < 3) continue;
			
			if(g_ways[id].nodes.node_ptrs[0] != g_ways[id].nodes.node_ptrs[g_ways[id].node_count-1]) continue;
		
			x = (double *) malloc(sizeof(double) * g_ways[id].node_count);
			y = (double *) malloc(sizeof(double) * g_ways[id].node_count);
			
			for(i = 0; i < g_ways[id].node_count; ++i)
			{
				node_info_t * n = g_ways[id].nodes.node_ptrs[i];
				if(n)
				{
					x[count] = n->lon;
					y[count] = n->lat;
					++count;
				}
			}
			
			if(count)
			{
				int sid;
				if(is_ccw_poly(x,y,count))
					reverse_poly(x,y,count);
				SHPObject * obj = SHPCreateSimpleObject(SHPT_POLYGON, count, x, y, NULL);
				++tot_f;
				tot_v += count;
				sid = SHPWriteObject(sfile, -1, obj);
				SHPDestroyObject(obj);

				tot_a += write_ent_attributes(dfile, sid, &g_ways[id].ent);
			}
			
			free(x);
			free(y);

			
		}
	}
	
	SHPClose(sfile);
	DBFClose(dfile);

	printf("Wrote: %d features, %d vertices, %d attributes.\n",tot_f,tot_v,tot_a);
	return 0;
}
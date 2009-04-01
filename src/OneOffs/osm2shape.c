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
#include "xmlparse/xmlparse.h"
#include "shapefile/shapefil.h"

static int	MAX_FIELD_LEN = 64;

static char buf[1024*1024*16];

static const char * ok_tags = NULL;
static int drop_empty = 0;

static SHPHandle sfile = NULL;
static DBFHandle dfile = NULL;

static int node_count = 0;
static int way_count = 0;
static int node_free = 0;
static int way_free = 0;
static int vert_free = 0;
static int vert_drop = 0;
static int last_id = -1;
static int in_order = 1;
static int is_way = 0;
static int is_rel = 0;
static int is_node = 0;
static int way_id = 0;

static int tot_f=0,tot_v=0,tot_a=0;

typedef struct tag_att_info {
	const char *			name;
	int						len;
	int						dbf_id;
	char *					temp_val;
	struct tag_att_info *	next;
} att_info_t;

static att_info_t *		g_atts = NULL;

typedef struct {
	int id;
	double lon;
	double lat;
} node_info_t;

static node_info_t * g_nodes = NULL;
static int * g_ways = NULL;

static double * g_x = NULL, * g_y = NULL;

static void do_help(void)
{
	fprintf(stderr,"Usage: osm2shape [-t<tag list>] [-d] [-l<max length>] <osm file> <shape file>\n");
	exit(1);
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
	na->temp_val = NULL;
	na->next = g_atts;
	g_atts = na;
}

int node_compare(const void * a, const void * b)
{
	const node_info_t * aa = (node_info_t *) a;
	const node_info_t * bb = (node_info_t *) b;
	return aa->id - bb->id;
}

void StartElementHandler_Count(void *userData,
					const XML_Char *name,
					const XML_Char **atts)
{
	if(strcmp(name,"node") == 0)
		++node_count;
	if(strcmp(name,"way") == 0)
		++way_count;
}

void EndElementHandler_Count(void *userData,
				      const XML_Char *name)
{
}


void StartElementHandler_ReadNodes(void *userData,
					const XML_Char *name,
					const XML_Char **atts)
{
	if(strcmp(name,"way") == 0)
	{
		way_id = fetch_att_i("id", atts,-1);
		is_way = 1;
	}
	if(strcmp(name,"relation") == 0)
	{
		is_rel = 1;
	}
	if(strcmp(name,"tag") == 0)
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
		if(is_way)
		{
			v = fetch_att_s("v",atts);
			k = fetch_att_s("k",atts);
			if(v == NULL || k == NULL)
			{
				fprintf(stderr,"ERROR: tag missing v and k attributes.\n");
				exit(1);
			}
			accum_attribute(k,strlen(v));
		}
	}
	else if (strcmp(name,"nd") == 0)
	{
		if(!is_way)
		{
			fprintf(stderr,"ERROR: nd found outside a way!\n");
			exit(1);
		}
		node_info_t key;
		key.id = fetch_att_i("ref", atts, -1);
		if(key.id < 0) 
		{
			fprintf(stderr,"No ref!");
			exit(1);
		}

		// Ben says: do not try to figure out the REAL vertex count here - our nodes are still unsorted, so we can't bsearch!
		g_ways[way_free]++;
	}
	if(strcmp(name,"node") == 0)
	{
		is_node = 1;
		int	id;
		double lon, lat;
		int found = 0;
		while(*atts)
		{
			if(strcmp(*atts,"id")==0)
			{
				++found;
				++atts;
				id = atoi(*atts++);
			}
			else if (strcmp(*atts,"lon")==0)
			{
				++found;
				++atts;
				lon = atof(*atts++);
			}
			else if(strcmp(*atts,"lat")==0)
			{
				++found;
				++atts;
				lat = atof(*atts++);
			}
			else
			{
				++atts;
				++atts;
			}
		}
		if(found != 3)
		{
			fprintf(stderr,"We found a node that doesn't have exactly one id, lon and lat.\n");
			exit(1);
		}

		if (id <= last_id)
			in_order = 0;
		last_id = id;
		
		g_nodes[node_free].lat = lat;
		g_nodes[node_free].lon = lon;
		g_nodes[node_free].id = id;
		++node_free;
	}
}

void EndElementHandler_ReadNodes(void *userData,
				      const XML_Char *name)
{
	if(is_way && strcmp(name,"way")==0)
	{
		is_way = 0;
		++way_free;
	}
	if(is_node && strcmp(name,"node")==0)
	{
		is_node = 0;
	}
	if(strcmp(name,"relation") == 0)
	{
		is_rel = 0;
	}	
}



void StartElementHandler_ReadWays(void *userData,
					const XML_Char *name,
					const XML_Char **atts)
{
	if(strcmp(name,"way")==0)
	{
		g_x = (double *) malloc(g_ways[way_free] * sizeof(double));
		g_y = (double *) malloc(g_ways[way_free] * sizeof(double));
		vert_free = 0;
		vert_drop = 0;
		is_way = 1;
		way_id = fetch_att_i("id", atts,-1);
	}
	if(is_way && strcmp(name,"nd")==0)
	{
		node_info_t key;
		key.id = fetch_att_i("ref", atts, -1);
		if(key.id < 0) 
		{
			fprintf(stderr,"No ref!");
			exit(1);
		}
		
		node_info_t * my_node = (node_info_t *) bsearch(&key, g_nodes, node_count, sizeof(node_info_t), node_compare);
		if(my_node != NULL)
		{		
			g_x[vert_free] = my_node->lon;
			g_y[vert_free] = my_node->lat;
			++vert_free;
		}
		else
		{
			++vert_drop;
			printf("WARNING: way %d references unknown node %d.\n", way_id, key.id);
		}
	}
	if(is_way && strcmp(name,"tag")==0)
	{
		att_info_t * a;		
		const char * k = fetch_att_s("k",atts);
		for(a = g_atts; a; a = a->next)
		if(strcmp(a->name,k)==0)
		{
			if(a->temp_val == NULL)
				a->temp_val = strdup(fetch_att_s("v",atts));
			break;
		}
	}
}

void EndElementHandler_ReadWays(void *userData,
				      const XML_Char *name)
{
	if(strcmp(name,"way")==0)
	{
		SHPObject * obj;
		int id;
		att_info_t * a;
		int has_any=0;
		for(a = g_atts; a; a = a->next)
		if(a->temp_val)
		{
			has_any=1;
			break;
		}
		
		if((vert_free+vert_drop) != g_ways[way_free])
		{
			fprintf(stderr,"Accounting error - vert count did not match.\n");
			exit(1);
		}
		
		if((vert_free > 1) && (has_any || !drop_empty))
		{
			obj = SHPCreateSimpleObject(SHPT_ARC, vert_free, g_x, g_y, NULL);
			++tot_f;
			tot_v += vert_free;
			id = SHPWriteObject(sfile, -1, obj);
			SHPDestroyObject(obj);
			
			for(a = g_atts; a; a = a->next)
			{
				if(a->temp_val)
				{
					if(strlen(a->temp_val) > a->len)
					{
						a->temp_val[a->len] = 0;
					}
					++tot_a;
					DBFWriteStringAttribute(dfile, id, a->dbf_id, a->temp_val);
					free(a->temp_val);
					a->temp_val = NULL;
				}
				else {
					// null
				}	
			}
			if(!has_any && g_atts)
				DBFWriteNULLAttribute(dfile, id, g_atts->dbf_id);
		} 
		else
		{
			for(a = g_atts; a; a = a->next)
			{
				if(a->temp_val)
					free(a->temp_val);
				a->temp_val = NULL;
			}
		}
		free(g_x);
		free(g_y);
		++way_free;
		is_way = 0;
	}
}


static void parse_stdio_file(gzFile * fi, XML_Parser parser)
{	
	gzrewind(fi);
	while(!gzeof(fi))
	{
		int len = gzread(fi,buf,sizeof(buf));
		XML_Parse(parser, buf, len, 0);
	}
	XML_Parse(parser, buf, 0, 1);
}

int main(int argc, const char * argv[])
{
	att_info_t * a;
	gzFile * fi;
	
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
			default:
				do_help();
			}
			++argv;
			--argc;
				
		}
		else
			break;
	}
	
	
	if(argc != 2)
		do_help();
		
	printf("Useful tags: %s, Drop empty: %s.  Max field len: %d.\n", ok_tags ? ok_tags : "none", drop_empty ? "yes" : "no", MAX_FIELD_LEN);
	 
	 fi = gzopen(argv[0],"rb");
	if(!fi) { fprintf(stderr, "Could not open %s.\n", argv[0]); exit(1); }

	/* PASS 1 - Count all nodes and ways. */	

	XML_Parser	parser = XML_ParserCreate(NULL);
	XML_SetElementHandler(parser, StartElementHandler_Count, EndElementHandler_Count);
	parse_stdio_file(fi,parser);
	XML_ParserFree(parser);
	
	printf("%d nodes, %d ways.\n", node_count, way_count);

	g_nodes = (node_info_t *) malloc(node_count * sizeof(node_info_t));
	g_ways = (int *) malloc(way_count * sizeof(int));
	memset(g_ways,0,way_count * sizeof(int));	

	/* PASS 2 - Count per-way nodes, read all node positions. */

	way_free = 0;
	parser = XML_ParserCreate(NULL);
	XML_SetElementHandler(parser, StartElementHandler_ReadNodes, EndElementHandler_ReadNodes);
	parse_stdio_file(fi,parser);
	XML_ParserFree(parser);

	if(node_free != node_count)
	{
		fprintf(stderr, "Accounting error: mismatch of nodes.\n");
		exit(1);
	}
	if(!in_order)
	{
		printf("Nodes were out of order - sorting.\n");
		qsort(g_nodes, node_count, sizeof(node_info_t), node_compare);
	}

	for(a = g_atts; a; a = a->next)
	{
		printf("     %s (%d)\n", a->name,a->len);
		if(a->len > MAX_FIELD_LEN) a->len = MAX_FIELD_LEN;
	}

	/* PASS 3 - Read and process all ways. */
	
	printf("Writing shape file.\n");
	
	sfile = SHPCreate(argv[1], SHPT_ARC);
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
	
	way_free = 0;
	parser = XML_ParserCreate(NULL);

	XML_SetElementHandler(parser, StartElementHandler_ReadWays, EndElementHandler_ReadWays);

	parse_stdio_file(fi,parser);
	XML_ParserFree(parser);
	
	SHPClose(sfile);
	DBFClose(dfile);
	
	printf("Wrote: %d features, %d vertices, %d attributes.\n",tot_f,tot_v,tot_a);
	gzclose(fi);
	return 0;
}
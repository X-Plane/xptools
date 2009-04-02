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
	Dependenceis: libz
	Compile: gcc osm_tile.c -lz -lexpat -o osm_tile
	Use: osm_tile <max open streams> <osm source file>
	
	osm_tile is a simple program to split an OSM planet XML file into 1x1 degree tiles.
	
	osm_tile does a "rough" tiling:
	
	- Any way that intersects or surrounds the tile is included.
	- Any node that is used in any way that is included is also included.
	
	The first limitation is to avoid doing true geometric intersections of each segment with 
	each tile.  The second limitation is to ensure that each included way is fully 
	geometrically available (again without processing per segment).
	
	The resulting "tile" needs to be cut to true tile boundaries (with way splitting) by
	another program.
	
	The down-side of this is that very large ways (like a coastline) can be included by
	every tile within the coastline.
	
	osm_tile will read an XML or g-zipped files, and outputs gzipped files.  The idea here
	is to keep total disk space down by never having the XML around as plain-text.
	
	ALGORITHM
	
	osm_tile works using tile-based bounding boxes, e.g. a bounding box describing the set
	of tiles a node or way must participate in.
	
	osm_tile uses a 5+ pass algorithm.
	
	1.	osm_tile scans the entire input file, finding the highest number node and way ID,
		for table allocation.
	2.	osm_tile reads all node positoins, building per-tile bounding boxes for each node.
	3.	osm_tile reads each way.  For each way, the way's boundidng box is taken as the
		union of the bounding boxes of all nodes.
	4.	osm_tile then clears each node's bounding box and re-reads the ways.  For each 
		node in the way, the way's bounding box is added to the nodes.  This step expands
		the node's bounding box to ensure that it will be included in a tile if it is
		part of a way included in the tile, even if the node is out-of-tile. 
		(Clients that want to do geometric cropping need a few nodes outside the tile.)
	5.	The export pass.  The export pass is actually recursive: it runs through the master
		file as many times as necessary.  For each pass through, it attempts to export each
		node or way to all of the participating tiles.
		
	The limitation on step 5 is the maximum number of open file descriptors (1024 on my
	Mac, but maybe higher on Linux); setting the highest possible ulimit will improve export
	time a lot.  For each pass, for each never-before-exported tile, a new output file is opened
	until we hit our upper limit.
	
	This sorting by tile export is done to avoid opening and closing files all the time - tests
	of the code with an RLU file descriptor cache showed very poor cache coherency.
	
	IMPLEMENTATION
	
	The table of bouding boxes is simply a malloced array by node ID.  This means there is wasted
	space - a hash table or sorted packed array could be use.
	
	Bounding boxes are stored as 32-bit ints:
	
	- The tile number is stored as: (lon + 181) + (lat + 90) * 362.
	- The 32-bits contains a lower left tile number and upper right tile number.
	
	The extra padding horizontally lets us mark tiles on the international date line
	that need to wrap around.
	
	The file scanner works by simply "windowing" a chunk of the file - when we get near the end, we 
	move it down.  The ratio of the total window size to the shift point affect efficiency greatly -
	if they are too similar we spend a ton of CPU time "scrolling".
	
	BUGS
	
	International date line is not yet fully supported - the bounding boxes contain slop, but the 
	"extra" is not yet included.	
	
	The XML parser is really dreadful - it's not an XML parser at all.
	
	POSSIBLE OPTIMIZATIONS

	Replace the parser with expat.  Actually expat is slower right now, probably since it has to do work on things
	we don't care about.  We should investigate whether expat can be programmed to run faster by giving it short-
	circuit instructions.
	
	It may at some point be necessary to replace the flat node and way tables with sorted tables.
	
*/


#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <zlib.h>
#include <stdarg.h>
#include <expat.h>

#define START_STRING	"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<osm version=\"0.5\" generator=\"JOSM\">\n"
#define END_STRING "</osm>\n"

// osm_tile will log all actions taken with one node and one way. 
// this provides a crude way to trace output (e.g. why is this node in my tile or why is this way not in my tile).
// def to -1 to not use.
#define CHECK_NODE_ID 343223997
#define CHECK_WAY_ID 30857863

/********************************************************************************************************************************************
 * BOUNDING BOX
 ********************************************************************************************************************************************/

// Upper 16 bits is south west corner, lower 16 is NE.
// Each 16 bit is X + Y * 362.  Lon is encoded in tiles, but -180 = 1!  Why?  Allow to be over the edge by one so that international
// dateline doesn't wrap/go neg.  When we collect the files we want for the international dateline, we need to check the "wrap buffer"
// on the other side.

int int_min(int a,int b) { return a < b ? a : b; }
int int_max(int a,int b) { return a > b ? a : b; }
int int_lim(int n, int min, int max) { if(n < min) return min; if (n > max) return max; return n; }
 
typedef unsigned long		bbox_t;

bbox_t make_bbox(int w, int s, int e, int n)
{
	unsigned int ne, sw;
	w += 181;
	e += 181;
	s += 90;
	n += 90;
	
	sw = s * 362 + w;
	ne = n * 362 + e;
	
	return (sw << 16) | (ne);
}

int decode_bbox(bbox_t b, int bounds[4])
{
	unsigned int sw, ne;
	if(b == 0xFFFFFFFF) return 1;
	if(!bounds) return 0;
	
	sw = (b & 0xFFFF0000) >> 16;
	ne = (b & 0x0000FFFF);
	
	bounds[0] = sw % 362;
	bounds[1] = sw / 362;
	bounds[2] = ne % 362;
	bounds[3] = ne / 362;
	
	bounds[0] -= 181;
	bounds[1] -=  90;
	bounds[2] -= 181;
	bounds[3] -=  90;
	
	return 0;
}

bbox_t bbox_union(bbox_t b1, bbox_t b2)
{
	int bounds1[4], bounds2[4];
	if(decode_bbox(b1, bounds1))
		return b2;
	if(decode_bbox(b2, bounds2))
		return b1;

	return make_bbox(
		int_min(bounds1[0], bounds2[0]),
		int_min(bounds1[1], bounds2[1]),
		int_max(bounds1[2], bounds2[2]),
		int_max(bounds1[3], bounds2[3]));
}

bbox_t bbox_for_ll(double lon, double lat)
{
	// pass in 1.5 - bbox should be: 1,1		(ceil-1,floor)	1	2
	// pass in 2  - bbox should be 1,2			(ceil-1,floor)	1	2
	return make_bbox(
					int_lim(ceil (lon)-1,-181,181),
					int_lim(ceil (lat)-1, -90, 90),
					int_lim(floor(lon)  ,-181,181),
					int_lim(floor(lat)  , -90, 90));
}

/********************************************************************************************************************************************
 * OUTPUT HELPERS
 ********************************************************************************************************************************************/


#define MAX_FILES_EVER	FD_SETSIZE
static int max_files_use = 16;

// Values: -1 = completed.  0 not open. > 0 = index+1 of file handle.
static int tile_file_status[360*180] = { 0 };
static int free_file_ptr = 0;
gzFile *	file_table[MAX_FILES_EVER] = { 0 };
static int not_enough_files = 0;

gzFile * fopen_cached(const char * fname, int hash)
{
	if(tile_file_status[hash] == -1) return NULL;
	if(tile_file_status[hash] > 0) return file_table[tile_file_status[hash]-1];
	
	if(free_file_ptr < max_files_use)
	{
		tile_file_status[hash] = free_file_ptr+1;
		gzFile * fi = gzopen(fname,"wb");
		gzprintf(fi,START_STRING);
		file_table[free_file_ptr++] = fi;
		return fi;
	}
	else
	{
		not_enough_files = 1;
		return NULL;
	}
}

void fclose_cache_all(void)
{
	int n;
	for(n = 0; n < (360*180); ++n)
	if(tile_file_status[n] > 0)
		tile_file_status[n] = -1;
		
	for(n=0;n<max_files_use;++n)
	{
		if(file_table[n])
		{
			gzprintf(file_table[n],END_STRING);		
			gzclose(file_table[n]);
		}
		file_table[n] = NULL;
	}
	free_file_ptr=0;
	not_enough_files=0;
}

int hash(int x, int y) { return (x + 180) + (y + 90) * 360; }

char * hash_fname(int x, int y, char * buf) { sprintf(buf,"%+03d%+04d.osm.gz",y,x); return buf; }
											
gzFile * print_to_bucket_ok(int x, int y)
{
	char buf[256];
	if(x < -180) x += 360;
	if (x >= 180) x -= 360;
	return fopen_cached(hash_fname(x,y,buf), hash(x,y));
}



void die(const char * fmt, ...)
{
	va_list va;
	va_start(va,fmt);
	vprintf(fmt, va);
	exit(1);
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

int get_int_attr(const char * key, const XML_Char ** atts,int d)
{
	while(*atts)
	{
		if(strcmp(*atts++,key)==0)	return atoi(*atts);
		++atts;
	}
	return d;
}

float get_float_attr(const char * key, const XML_Char ** atts,float d)
{
	while(*atts)
	{
		if(strcmp(*atts++,key)==0)	return atof(*atts);
		++atts;
	}
	return d;
}


/********************************************************************************************************************************************
 * GLOBALS
 ********************************************************************************************************************************************/

static bbox_t *		g_nodes = NULL;
static bbox_t *		g_ways = NULL;

static	int highest_w = 0;
static	int highest_n = 0;
static	int nw = 0, nn = 0;

static	int	cur_way_id = -1;

/********************************************************************************************************************************************
 * PASS 0
 ********************************************************************************************************************************************/

void StartElementHandler_Count(void *userData,
					const XML_Char *name,
					const XML_Char **atts)
{
	if(strcmp(name,"node")==0)
	{
		++nn;
		int nd = get_int_attr("id",atts,-1);
		if(nd == -1) xml_die("Node contains no index.\n");
		if(nd > highest_n) highest_n = nd;
	}	

	if(strcmp(name,"way")==0)
	{
		++nw;
		int wd = get_int_attr("id",atts,-1);
		if(wd == -1) xml_die("Way contains no index.\n");
		if(wd > highest_w) highest_w = wd;
	}	
}

void EndElementHandler_Count(void *userData,
				      const XML_Char *name)
{
}

/********************************************************************************************************************************************
 * PASS 1
 ********************************************************************************************************************************************/

void StartElementHandler_IndexNodes(void *userData,
					const XML_Char *name,
					const XML_Char **atts)
{
	if(strcmp(name,"node")==0)
	{
		int nd = get_int_attr("id",atts,-1);
		float lat = get_float_attr("lat",atts,-999.0);
		float lon = get_float_attr("lon",atts,-999.0);

		if(nd < 0 || nd > highest_n)	xml_die("Node has illegal or missing ID: %d\n", nd);
		if(lat == -999.0)				xml_die("Node %d missing latitude.\n", nd);
		if(lon == -999.0)				xml_die("Node %d missing longitude.\n", nd);
		
		g_nodes[nd] = bbox_for_ll(lon, lat);
			
		if(nd == CHECK_NODE_ID)
		{
			int bbox[4];
			printf("    Node ID: %d lon=%lf, lat=%lf, bbox=0x%08x\n", nd,lon,lat,g_nodes[nd]);
			if(!decode_bbox(g_nodes[nd],bbox))
				printf("    (bbox=%d,%d -> %d,%d)\n",bbox[0],bbox[1],bbox[2],bbox[3]);
			else
				printf("	(bbox is empty.)\n");
		}		
	}	
}

void EndElementHandler_IndexNodes(void *userData,
				      const XML_Char *name)
{
}

/********************************************************************************************************************************************
 * PASS 2
 ********************************************************************************************************************************************/

void StartElementHandler_IndexWays(void *userData,
					const XML_Char *name,
					const XML_Char **atts)
{
	if(strcmp(name,"way")==0)
	{
		if(cur_way_id != -1)	xml_die("Error: found a way inside way %d.\n",cur_way_id);
		cur_way_id = get_int_attr("id",atts,-1);
		if(cur_way_id < 0 || cur_way_id > highest_w)	xml_die("Way ID out of bounds: %d.\n",cur_way_id);
		
		if(cur_way_id == CHECK_WAY_ID)
			printf("Found way ID: %d\n", cur_way_id);
	}
	
	if(cur_way_id != -1 && strcmp(name,"nd")==0)
	{
		int nd_ref_id = get_int_attr("ref",atts,-1);
		if(nd_ref_id == -1) xml_die("Way %d has no ref.\n", cur_way_id);
		
		if(nd_ref_id < 0 || nd_ref_id > highest_n)
			fprintf(stderr,"ERROR: the file contains a way %d that references a node %d that we do not have.\n", cur_way_id, nd_ref_id);
		
		if(cur_way_id == CHECK_WAY_ID || nd_ref_id == CHECK_NODE_ID)
			printf("Adding node %d to way %d.  ", nd_ref_id,cur_way_id);
																
		g_ways[cur_way_id] = bbox_union(g_ways[cur_way_id], g_nodes[nd_ref_id]);				

		if(cur_way_id == CHECK_WAY_ID)
		{
			int bbox[4];
			if(!decode_bbox(g_ways[nw],bbox))
				printf("New bbox is: %d,%d -> %d,%d\n", bbox[0],bbox[1],bbox[2],bbox[3]);
			else
				printf("Empty.\n");
		}
	}
}

void EndElementHandler_IndexWays(void *userData,
				      const XML_Char *name)
{
	if(strcmp(name,"way")==0)
		cur_way_id = -1;
}


/********************************************************************************************************************************************
 * PASS 3
 ********************************************************************************************************************************************/

void StartElementHandler_ReindexNodes(void *userData,
					const XML_Char *name,
					const XML_Char **atts)
{
	if(strcmp(name,"way")==0)
	{
		if(cur_way_id != -1)	xml_die("Error: found a way inside way %d.\n",cur_way_id);
		cur_way_id = get_int_attr("id",atts,-1);
		if(cur_way_id < 0 || cur_way_id > highest_w)	xml_die("Way ID out of bounds: %d.\n",cur_way_id);
		
		if(cur_way_id == CHECK_WAY_ID)
			printf("Found way ID: %d\n", cur_way_id);
	}
	
	if(cur_way_id != -1 && strcmp(name,"nd")==0)
	{
		int nd_ref_id = get_int_attr("ref",atts,-1);
		if(nd_ref_id == -1) xml_die("Way %d has no ref.\n", cur_way_id);
				
		// Ben says: turns out the master OSM database sometimes has corruption - ways that reference deleted nodes.  If the node
		// doesn't show up, don't panic - but do not try to use it, as we could use junk data or seg fault.  Note that if the node ID
		// is IN table range but NOT in the file, the bbox is inited to empty, which is fine.
		if(nd_ref_id >= 0 && nd_ref_id <= highest_n)
			g_nodes[nd_ref_id] = bbox_union(g_ways[cur_way_id], g_nodes[nd_ref_id]);

		if(nd_ref_id == CHECK_NODE_ID || cur_way_id == CHECK_WAY_ID)
		{
			int bbox[4];
			printf("    Node ID: %d bbox=0x%08x, way ID: %d bbox=0x%08X\n", nd_ref_id,g_nodes[nd_ref_id], cur_way_id, g_ways[cur_way_id]);
			if(!decode_bbox(g_nodes[nd_ref_id],bbox))
				printf("    (bbox=%d,%d -> %d,%d)\n",bbox[0],bbox[1],bbox[2],bbox[3]);
			else
				printf("	(bbox is empty.)\n");
		}
	}
}

void EndElementHandler_ReindexNodes(void *userData,
				      const XML_Char *name)
{
	if(strcmp(name,"way")==0)
		cur_way_id = -1;
}


/********************************************************************************************************************************************
 * PASS 4
 ********************************************************************************************************************************************/

static int	tag_level = 0;
static int	output_level = 0;
static int	out_box[4];
static int	is_leaf = 0;

const char * indent_str(int n)
{
	const char * spaces = "                                                  ";
	int l = strlen(spaces);
	if(n > l) n = l;
	return spaces + l - n;
}

static void print_xml_encoded(gzFile * fi, const char * str)
{
	const char * r = str, * e;
	while(*r)
	{
		e=r;
		while(*e != 0 &&
			*e != '<' &&
			*e != '&' &&
			*e != '>' &&
			*e != '"' &&
			*e != '\'') ++e;
		
		if(r != e)
			gzwrite(fi,r,e-r);
		
		r = e;
		if(*r == 0) 
			break;
		if(*r == '<')	gzprintf(fi,"&lt;");
		if(*r == '&')	gzprintf(fi,"&amp;");
		if(*r == '>')	gzprintf(fi,"&gt;");
		if(*r == '"')	gzprintf(fi,"&quot;");
		if(*r == '\'')	gzprintf(fi,"&apos;");
		
		++r;
	}
}

static void print_one_tag(
					gzFile *			fi,
					const XML_Char *	name,
					const XML_Char **	atts,
					int					need_close,
					int					x,
					int					y,
					int					node_id,
					int					way_id)
{
	if(!fi)
		return;
	if(node_id == CHECK_NODE_ID)
		printf("Wrote node %d to %d,%d, bbox=%d,%d -> %d,%d.\n", node_id, x, y, out_box[0], out_box[1],out_box[2],out_box[3]);

	if(way_id == CHECK_WAY_ID)
		printf("Wrote way %d to %d,%d, bbox=%d,%d -> %d,%d.\n", way_id, x, y, out_box[0], out_box[1],out_box[2],out_box[3]);

	if(need_close)
	gzprintf(fi,">\n");
	
	gzprintf(fi,"%s<%s",indent_str(tag_level),name);
	if(*atts)
		gzprintf(fi," ");
		
	while(*atts)
	{
		gzprintf(fi,"%s=\"",*atts++);
		print_xml_encoded(fi,*atts++);		
		gzprintf(fi,"\" ");
	}			
}

void StartElementHandler_Output(void *userData,
					const XML_Char *name,
					const XML_Char **atts)
{
	int x,y;
	int want_print = output_level > 0;

	int need_to_close = is_leaf && output_level > 0;
	is_leaf = 1;
	
	if(!want_print && strcmp(name,"node")==0)
	{
		if(output_level > 0)	xml_die("Nesting error: tag %s is in an already output tag.\n",name);
		int node_id = get_int_attr("id",atts,-1);
		if(node_id < 0 || node_id > highest_n) xml_die("Bad node id: %d\n",node_id);
		
		++output_level;
		
		if(!decode_bbox(g_nodes[node_id],out_box))
		for(y = out_box[1]; y <= out_box[3]; ++y)
		for(x = out_box[0]; x <= out_box[2]; ++x)
			print_one_tag(
				print_to_bucket_ok(x,y),
				name,atts,need_to_close,
				x,y,node_id,-1);
	}
	else if(!want_print && strcmp(name,"way")==0)
	{
		if(output_level > 0)	xml_die("Nesting error: tag %s is in an already output tag.\n",name);
		int way_id = get_int_attr("id",atts,-1);
		if(way_id < 0 || way_id > highest_w) xml_die("Bad way id: %d\n", way_id);

		++output_level;
		
		if(!decode_bbox(g_ways[way_id],out_box))
		for(y = out_box[1]; y <= out_box[3]; ++y)
		for(x = out_box[0]; x <= out_box[2]; ++x)
			print_one_tag(
				print_to_bucket_ok(x,y),
				name,atts,need_to_close,
				x,y,-1,way_id);
	}
	else if (output_level > 0)
	{
		++output_level;
		for(y = out_box[1]; y <= out_box[3]; ++y)
		for(x = out_box[0]; x <= out_box[2]; ++x)
			print_one_tag(
				print_to_bucket_ok(x,y),
				name,atts,need_to_close,
				x,y,-1,-1);
	}
	++tag_level;
}

void EndElementHandler_Output(void *userData,
				      const XML_Char *name)
{
	int x, y;
	
	--tag_level;
	if(output_level > 0)
	{
		--output_level;
		for(y = out_box[1]; y <= out_box[3]; ++y)
		for(x = out_box[0]; x <= out_box[2]; ++x)
		{
			gzFile * fi = print_to_bucket_ok(x,y);
			if(fi)
			{
				if(is_leaf)
					gzprintf(fi,"/>\n");
				else
					gzprintf(fi,"%s</%s>\n",indent_str(tag_level),name);
			} 
		}
	}
	is_leaf=0;
}


/********************************************************************************************************************************************
 * MAIN!!!!
 ********************************************************************************************************************************************/

static void die_usage(void)
{
	fprintf(stderr,"osm_tile [-b<west>,<south>,<east>,<north>] [-m<max files>] <osm xml file>\n");
	exit(1);
}

int main(int argc, const char * argv[])
{
	int exp_bounds[4], x,y;
	const char ** fname = argv;
	if(argc < 1)	die_usage;

	++fname;
	--argc;
	
	while(argc && fname[0][0] == '-')
	{
		switch(fname[0][1]) {
		case 'm':
			max_files_use = atoi(*fname+2);
			printf("Will write %d files at a time.\n",max_files_use);
			++fname;
			--argc;
			break;
		case 'b':
			if(sscanf(*fname,"-b%d,%d,%d,%d",exp_bounds,exp_bounds+1,exp_bounds+2,exp_bounds+3) != 4)	die_usage();
			for(y= -90;y< 90;++y)
			for(x=-180;x<180;++x)
			if (x < exp_bounds[0] ||
				x > exp_bounds[2] ||
				y < exp_bounds[1] ||
				y > exp_bounds[3])
			{
				tile_file_status[hash(x,y)]=-1;
			}
			printf("Will restrict output to: %d,%d -> %d,%d\n",exp_bounds[0],exp_bounds[1],exp_bounds[2],exp_bounds[3]);
			++fname;
			--argc;
			break;
		default:
			die_usage();
		}
	}
	if(argc == 0)	die_usage();
	
	
	
	if(max_files_use > MAX_FILES_EVER)
	{
		max_files_use = MAX_FILES_EVER;
		printf("(I will use %d files - that is the API FD limit for this OS.\n", max_files_use);
	}
	
	/**************************************************************************************************************
	 * PASS 0 - COUNTING 
	 **************************************************************************************************************

	First we have to count every node and way in the file and find the highest IDs, so we can allocate spatial 
	indices. */
	
	printf("Counting nodes and ways.\n");
	run_file(*fname, StartElementHandler_Count,EndElementHandler_Count);
		
	printf("Highest node: %d.  Highest way: %d.\n", highest_n, highest_w);
	printf("Total nodes: %d.  Total ways: %d.\n", nn, nw);
	
	g_nodes = (bbox_t *) malloc((highest_n+1) * sizeof(bbox_t));
	g_ways  = (bbox_t *) malloc((highest_w+1) * sizeof(bbox_t));
	
	memset(g_nodes,0xFF,(highest_n+1) * sizeof(bbox_t));
	memset(g_ways ,0xFF,(highest_w+1) * sizeof(bbox_t));

	/**************************************************************************************************************
	 * PASS 1 - NODE INDEXING
	 **************************************************************************************************************
	 
	Read every node and get a bounding box for it. */

	printf("Building node spatial index.\n");

	run_file(*fname, StartElementHandler_IndexNodes,EndElementHandler_IndexNodes);

	/**************************************************************************************************************
	 * PASS 2 - WAY INDEXING
	 **************************************************************************************************************
	 
	 Read every way.  Build up a bounding box for a way that is the union of the bounding boxes for nodes. */

	printf("Building way spatial index.\n");
	run_file(*fname, StartElementHandler_IndexWays,EndElementHandler_IndexWays);

	/**************************************************************************************************************
	 * PASS 3 - NODE RE-INDEXING
	 **************************************************************************************************************
	 
	 Node re-indexing.  We actually want to include every node in every tile that has a way that intersects that tile
	 AND uses the node!  In other words, if a node is in tile B, but a way uses this node and is in tile A, we want 
	 this node in tile A too, so that we don't have any nodes that are "out of tile somewhere".  
	 
	 We care because when we _crop_ a way, we need to have one vertex OUTSIDE the node so that we can have a line to
	 chop in half. *
	 
	 Union all ways back INTO the nodes!  Note that the bbox is a _poor_ structure for this.  If we had two ways, 
	 say I-95 and I-80, both crossing the US, then the bbox for their junction in NYC would have a bbox of the 
	 entire united states.  But this is for the "clipper" to clean up later.  We do not zap the node's original box
	 because we want to include un-way-referenced nodes (e.g. a POI) in their original tile(s). */

	printf("Rebuilding node spatial index.\n");
	run_file(*fname, StartElementHandler_ReindexNodes,EndElementHandler_ReindexNodes);
	
	/**************************************************************************************************************
	 * PASS 4 - ACTUAL FILE OUTPUT.
	 **************************************************************************************************************
	 
	 Now we can finally output all nodes and ways, in a once-over copy pass. */

	if(CHECK_WAY_ID >= 0 && CHECK_WAY_ID <= highest_w)
		printf("way %d: 0x%08X\n", CHECK_WAY_ID, g_ways[CHECK_WAY_ID]);
	if(CHECK_NODE_ID >= 0 && CHECK_NODE_ID <= highest_n)
		printf("node %d: 0x%08X\n", CHECK_NODE_ID, g_nodes[CHECK_NODE_ID]);

	while(1)
	{
		printf("Exporting tiles, %d at a time.\n", max_files_use);

		not_enough_files=0;
		run_file(*fname,StartElementHandler_Output,EndElementHandler_Output);

		int done = not_enough_files == 0;
		
		fclose_cache_all();
		if(done) 
			break;
	}
	return 0;
}
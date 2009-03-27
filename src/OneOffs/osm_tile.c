/*
 * Copyright (c) 2004, Laminar Research.
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

	Compile: gcc osm_tile.c -lz -o osm_tile

	Use: osm_tile <max open streams> <osm source file>
	
*/


#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <zlib.h>


#define START_STRING	"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<osm version=\"0.5\" generator=\"JOSM\">\n"
#define END_STRING "</osm>\n"

/********************************************************************************************************************************************
 * BOUNDING BOX
 ********************************************************************************************************************************************/

// Upper 16 bits is south west corner, lower 16 is NE.
// Each 16 bit is X + Y * 362.  Lon is encoded in tiles, but -180 = 1!  Why?  Allow to be over the edge by one so that international
// dateline doesn't wrap/go neg.  When we collect the files we want for the international dateline, we need to check the "wrap buffer"
// on the other side.

int min(int a,int b) { return a < b ? a : b; }
int max(int a,int b) { return a > b ? a : b; }
 
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
		min(bounds1[0], bounds2[0]),
		min(bounds1[1], bounds2[1]),
		max(bounds1[2], bounds2[2]),
		max(bounds1[3], bounds2[3]));
}

bbox_t bbox_for_ll(double lon, double lat)
{
	// pass in 1.5 - bbox should be: 1,1		(ceil-1,floor)	1	2
	// pass in 2  - bbox should be 1,2			(ceil-1,floor)	1	2
	return make_bbox(
					ceil (lon)-1,
					ceil (lat)-1,
					floor(lon)  ,
					floor(lat)  );
}

/********************************************************************************************************************************************
 * BLOCK STREAM SCANNER
 ********************************************************************************************************************************************

This is a simple file scanner that maps a chunk of file into memory using stdio.  The idea is to burn some CPU by "scooting down" the block
in order to simplify file scanning.  We do this because we can't memmap the entire planet file on a 32-bit machine.
 
*/


#define BUF_SIZE (1024 * 1024 * 64)
#define SKIP_PT (1024 * 64)

typedef struct {
	gzFile *		fi;
	char *		buf_start;
	char *		buf_end;		/* Logical end of buffer */
	int			buf_size;
} file_scanner;

 
int scanner_init(file_scanner * scanner, const char * file_name);
char *	scanner_advance(file_scanner * scanner, char * my_ptr);
void scanner_kill(file_scanner * scanner);

int scanner_init(file_scanner * scanner, const char * file_name)
{
	scanner->fi = gzopen(file_name, "rb");
	if(!scanner->fi) return errno;
	scanner->buf_start = (char *) malloc(BUF_SIZE);
	scanner->buf_end = scanner->buf_start;
	scanner->buf_size = BUF_SIZE;
	scanner_advance(scanner, scanner->buf_start);
	return 0;
}

void scanner_reset(file_scanner * scanner)
{
	gzrewind(scanner->fi);
//	fseek(scanner->fi,0L, SEEK_SET);
	scanner->buf_end = scanner->buf_start;
	scanner_advance(scanner,scanner->buf_start);
}

/* Given a scanner and a pointer to a character within the buffer, this attemps to page the buffer forward without "losing" the ptr.
 * It returns the new ptr location.  If my_ptr and the returned value are the same, we cannot page forward any more! */
char *	scanner_advance(file_scanner * scanner, char * my_ptr)
{
	if(my_ptr > scanner->buf_start)
	{
		memmove(scanner->buf_start, my_ptr, scanner->buf_end - my_ptr);
		scanner->buf_end -= (my_ptr - scanner->buf_start);
		my_ptr = scanner->buf_start;
	}
	if(!gzeof(scanner->fi) && (scanner->buf_end - scanner->buf_start < scanner->buf_size))
	{
		int free_space = scanner->buf_size - (scanner->buf_end - scanner->buf_start);
		if(free_space > 0)
		{
			int read_len = gzread(scanner->fi, scanner->buf_end, free_space);
			if(read_len > 0)
				scanner->buf_end += read_len;
		}
	}
	return my_ptr;
}

void scanner_kill(file_scanner * scanner)
{
	gzclose(scanner->fi);
	free(scanner->buf_start);
}

/********************************************************************************************************************************************
 * XML TAG SCANNER
 ********************************************************************************************************************************************/

char * find_close_tag(const char * tag_s, const char * tag_e, char * p, char * lim)
{
	int len = tag_e - tag_s;
	if(len > 252)
	{
		fprintf(stderr,"We are searching for a huge tag name: %253s\n", tag_s);
		exit(1);
	}
	char buf[256];
	memcpy(buf+2,tag_s,tag_e-tag_s);
	buf[0] = '<';
	buf[1] = '/';
	buf[2 + len] = '>';
	buf[3 + len] = 0;
	
	while(p < (lim-len))
	{
		if(memcmp(p,buf,len+3) == 0) return p;
		++p;
	}
	return NULL;
}

const char * find_xml_attr(const char * attr_str, const char * tag_start, const char * tag_end)
{
	char buf[256];
	int len = strlen(attr_str);
	if(len > 253)
	{
		fprintf(stderr,"Attribute name too long: %s\n", attr_str);
		exit(1);
	}
	sprintf(buf,"%s=",attr_str);
	len = strlen(buf);
	const char * p = tag_start;	
	while(p < tag_end - len)
	{
		if(memcmp(p,buf,len) == 0)
			return p + len + 1;
		++p;
	}
	return NULL;
}

// pulls out an XML chunk by range.
// TODO: this is not safe - some of the scanners assume our tag will be 100% in our block!!

int	find_xml_tag(
		file_scanner *			scanner,
		char *					ptr,
		char **					out_tag_start,
		char **					out_tag_end,
		int						do_not_close)
{
	char * p = ptr;
	char * name_s;
	char * name_e;
	
	char * close_p;
	
	while(1)
	{
		if(scanner->buf_end - p < SKIP_PT)
			p = scanner_advance(scanner, p);
			
		while(p < scanner->buf_end && *p != '<') ++p;
		if(p == scanner->buf_end)
		{
			scanner_advance(scanner, scanner->buf_end);
			if(scanner->buf_end == scanner->buf_start) 
				return 0;
		} else {
			break;
		}
	}
	*out_tag_start = p;
	name_s = p+1;
	name_e = name_s;
	while(*name_e != ' ' && *name_e != '>' && *name_e != '/') ++name_e;
	
	close_p = name_e;
	while(*close_p != '>') ++close_p;
	if(close_p[-1] == '/' || close_p[-1] == '?' || do_not_close)
	{
		*out_tag_end = close_p+1;
	}
	else
	{
		*out_tag_end = find_close_tag(name_s,name_e,close_p+1, scanner->buf_end);
		if(*out_tag_end == NULL) return 0;
		*out_tag_end += (name_e - name_s) + 3;
	}
	return 1;	
}

// Return a property from its tag, e..g id="...."		
double get_xml_property_d(
					const char * tag_start,
					const char * tag_end,
					const char * tag_id)
{
	const char * v = find_xml_attr(tag_id, tag_start,tag_end);
	if(v == NULL) return 0;
	return atof(v);
}
					
int get_xml_property_i(
					const char * tag_start,
					const char * tag_end,
					const char * tag_id)
{
	const char * v = find_xml_attr(tag_id, tag_start,tag_end);
	if(v == NULL) return 0;
	return atoi(v);
}

char * get_xml_next_str(char * s, char * e, const char * pat)
{
	int len = strlen(pat);
	e -= len;
	while(s < e)
	{
		if(strncmp(s,pat,len) == 0) return s;
		++s;
	}
	return NULL;
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
											
void copy_tag_to_bucket(int x, int y, const char * s, const char * e)
{
	char buf[256];
	if(x < -180) x += 360;
	if (x >= 180) x -= 360;
	gzFile * fi = fopen_cached(hash_fname(x,y,buf), hash(x,y));
	if(fi)
	{
		gzprintf(fi,"  ");
		gzwrite(fi,s,e-s);
		gzprintf(fi,"\n");
	}
}
					
					
/********************************************************************************************************************************************
 * PLANET SCANNER
 ********************************************************************************************************************************************/

char * scan_osm_headers(file_scanner * scanner)
{
	char * tag_s;
	char * tag_e;

	if(!find_xml_tag(scanner, scanner->buf_start, &tag_s, &tag_e, 1))
	{
		fprintf(stderr,"Could not find XML tag.\n");
		exit(1);
	}
	if(strncmp(tag_s,"<?xml",5) != 0)
	{
		fprintf(stderr,"XML tag is incorrect.\n");
		exit(1);
	}
	if(!find_xml_tag(scanner, tag_e, &tag_s, &tag_e, 1))
	{
		fprintf(stderr,"Could not find OSM tag.\n");
		exit(1);
	}
	if(strncmp(tag_s,"<osm",4) != 0)
	{
		fprintf(stderr,"OSM tag is incorrect.\n");
		exit(1);
	}
	return tag_e;
}

/********************************************************************************************************************************************
 * MAIN!!!!
 ********************************************************************************************************************************************/

bbox_t *		g_nodes = NULL;
bbox_t *		g_ways = NULL;

int main(int argc, const char * argv[])
{
	char * tag_s;
	char * tag_e;
	char * p;
	
	int highest_w = 0;
	int highest_n = 0;
	int nw = 0, nn = 0;
	
	max_files_use = atoi(argv[1]);
	printf("Export %d files at a time.\n", max_files_use);
	if(max_files_use > MAX_FILES_EVER)
	{
		max_files_use = MAX_FILES_EVER;
		printf("(I will use %d files - that is the API FD limit for this OS.\n", max_files_use);
	}
	
	file_scanner scanner;
	if (scanner_init(&scanner, argv[2]) != 0)
	{
		fprintf(stderr,"Could not open %s.\n", argv[2]);
		exit(1);
	}

	/**************************************************************************************************************
	 * PASS 0 - COUNTING 
	 **************************************************************************************************************

	First we have to count every node and way in the file and find the highest IDs, so we can allocate spatial 
	indices. */
	

	printf("Counting nodes and ways.\n");

	p = scan_osm_headers(&scanner);
	
	while(find_xml_tag(&scanner, p, &tag_s, &tag_e, 0))
	{	
		if(strncmp(tag_s,"<node",5)==0)
		{
			++nn;
			int nd = get_xml_property_i(tag_s,tag_e,"id");
			if(nd > highest_n) highest_n = nd;			
		}
		if(strncmp(tag_s,"<way",4)==0)
		{
			++nw;
			int wd = get_xml_property_i(tag_s,tag_e,"id");
			if(wd > highest_w) highest_w = wd;
		}
		p = tag_e;
	}
	
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

	scanner_reset(&scanner);
	p = scan_osm_headers(&scanner);
	
	while(find_xml_tag(&scanner, p, &tag_s, &tag_e, 0))
	{	
		if(strncmp(tag_s,"<node",5)==0)
		{
			int nd = get_xml_property_i(tag_s,tag_e,"id");
			double lat = get_xml_property_d(tag_s,tag_e,"lat");
			double lon = get_xml_property_d(tag_s,tag_e,"lon");
			g_nodes[nd] = bbox_for_ll(lon, lat);
		}

		p = tag_e;
	}

	/**************************************************************************************************************
	 * PASS 2 - WAY INDEXING
	 **************************************************************************************************************
	 
	 Read every way.  Build up a bounding box for a way that is the union of the bounding boxes for nodes. */

	printf("Building way spatial index.\n");

	scanner_reset(&scanner);
	p = scan_osm_headers(&scanner);
	
	while(find_xml_tag(&scanner, p, &tag_s, &tag_e, 0))
	{	
		if(strncmp(tag_s,"<way",4)==0)
		{
			int nw = get_xml_property_i(tag_s,tag_e,"id");
			char * ndr = tag_s;
			while((ndr = get_xml_next_str(ndr, tag_e, "<nd ref=")) != NULL)
			{
				int nd_ref_id = atoi(ndr + 9);				
				g_ways[nw] = bbox_union(g_ways[nw], g_nodes[nd_ref_id]);				
				ndr = ndr+9;
			}			
		}

		p = tag_e;
	}
	
	/**************************************************************************************************************
	 * PASS 3 - NODE RE-INDEXING
	 **************************************************************************************************************
	 
	 Node re-indexing.  We actually want to include every node in every tile that has a way that intersects that tile
	 AND uses the node!  In other words, if a node is in tile B, but a way uses this node and is in tile A, we want 
	 this node in tile A too, so that we don't have any nodes that are "out of tile somewhere".  
	 
	 We care because when we _crop_ a way, we need to have one vertex OUTSIDE the node so that we can have a line to
	 chop in half. *
	 
	 So...reset all node bboxes and then union all ways back INTO the nodes!  Note that the bbox is a _poor_ structure
	 for this.  If we had two ways, say I-95 and I-80, both crossing the US, then the bbox for their junction in NYC
	 would have a bbox of the entire united states.  But this is for the "clipper" to clean up later. */

	printf("Rebuilding node spatial index.\n");
	memset(g_nodes,0xFF,(highest_n+1) * sizeof(bbox_t));
	
	scanner_reset(&scanner);
	p = scan_osm_headers(&scanner);
	
	while(find_xml_tag(&scanner, p, &tag_s, &tag_e, 0))
	{	
		if(strncmp(tag_s,"<way",4)==0)
		{
			int nw = get_xml_property_i(tag_s,tag_e,"id");
			char * ndr = tag_s;
			while((ndr = get_xml_next_str(ndr, tag_e, "<nd ref=")) != NULL)
			{
				int nd_ref_id = atoi(ndr + 9);				
				
				g_nodes[nd_ref_id] = bbox_union(g_ways[nw], g_nodes[nd_ref_id]);

				ndr = ndr+9;
			}			
		}

		p = tag_e;
	}

	/**************************************************************************************************************
	 * PASS 4 - ACTUAL FILE OUTPUT.
	 **************************************************************************************************************
	 
	 Now we can finally output all nodes and ways, in a once-over copy pass. */

	while(1)
	{
		printf("Exporting tiles, %d at a time.\n", max_files_use);

		scanner_reset(&scanner);
		p = scan_osm_headers(&scanner);
			
		while(find_xml_tag(&scanner, p, &tag_s, &tag_e, 0))
		{	
			int bbox[4], x, y;
			
			if(strncmp(tag_s,"<node",5)==0)
			{
				int nd = get_xml_property_i(tag_s,tag_e,"id");
				
				if(!decode_bbox(g_nodes[nd],bbox))
				{
					for(y = bbox[1]; y <= bbox[3]; ++y)
					for(x = bbox[0]; x <= bbox[2]; ++x)
						copy_tag_to_bucket(x,y,tag_s,tag_e);
				}			
			}

			if(strncmp(tag_s,"<way",4)==0)
			{
				int nw = get_xml_property_i(tag_s,tag_e,"id");

				if(!decode_bbox(g_ways[nw],bbox))
				{
					for(y = bbox[1]; y <= bbox[3]; ++y)
					for(x = bbox[0]; x <= bbox[2]; ++x)
						copy_tag_to_bucket(x,y,tag_s,tag_e);
				}			

			}

			p = tag_e;
		}
	
		int done = not_enough_files == 0;
		
		fclose_cache_all();
		if(done) 
			break;
	}
	scanner_kill(&scanner);
	
	return 0;
}
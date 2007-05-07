#include "uv_mapper.h"
#include <set>
#include <map>
#include <algorithm>
#include <vector>
#include <math.h>

using std::set;
using std::map;
using std::multimap;
using std::vector;
using std::min;
using std::max;

namespace cgd {
#include "CompGeomDefs2.h"
#include "CompGeomDefs3.h"
};

#include <ac_plugin.h>

#include "ac_utils.h"

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// COMMON UTILS
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------


// Note: the cross-product of two vectors is the sin of the angle times the length of the two vectors.  This is also the area formula for
// a parallelogram (base * side * sin theta, which is also base * height).  Divide by two to get the triangle.
// The problem is: the magnitude of the vector is the area, but the DIRECTION is the "sign", e.g. a CW/CCW test.  Of course a CW triangle is
// CCW from the OTHER SIDE.  So...we pass in a VECTOR that defines which way is "up".  By comparing the dir of the direction vector with our
// area vector, we can add sign info to our area, which allows us to get negative areas for CW triangles.  (This is of course necessary for a number
// of algorithms that add areas and exepct "inside out" triangles to be negative".
inline double	signed_area_3d(const cgd::Point3& p0,const cgd::Point3& p1,const cgd::Point3& p2, const cgd::Vector3 positive_dir)
{
	cgd::Vector3 v(p0,p1);
	cgd::Vector3 w(p0,p2);	
	cgd::Vector3	area = v.cross(w);	
	double coef = (positive_dir.dot(area) < 0.0) ? -0.5 : 0.5;
	return coef * sqrt(area.squared_length());
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// BARREL-UNWRAP UV MAPPING
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------

struct	uv_vertex_info_t {
	SVertex *						me;				// Underlying ac3d vertex
	set<uv_vertex_info_t *>			connected;		// Everyone connecting to me
	float							new_s;			// Newly computed S & T coords
	float							new_t;
	float							proj_height;	// Projected height for ordering.  Signed!
	uv_vertex_info_t *				my_source;		// One vertex that feeds me
	set<uv_vertex_info_t *>			my_targets;		// Vertex I will feed to
};

typedef map<Vertex*, uv_vertex_info_t*>		VertexMap;
typedef map<SVertex *, vector<SVertex *> >	BackMap;

// Add an entire object into our mesh map.
static void		map_object(ACObject * ob, VertexMap& map, BackMap& back);
// Add a single surface into our mesh map.
static void		map_surface(Surface * surf, VertexMap& map, BackMap& back);

// Given a projection plane and a quad on that plane, calculate the new s coord by projecting onto the plane and then interping
// from the corners.
static void		map_s_on_tri(VertexMap& map, const cgd::Plane3& proj_plane, const cgd::Point3 corners[3], const cgd::Point2 sts[3]);

// build up proj_height from a plane
static void		map_height_from_plane(VertexMap& map, const cgd::Plane3& proj_plane);
// Given heights and connections, calculate our best "dependency flow".
static void		calc_best_dependencies(VertexMap& map,const cgd::Plane3& proj_plane);
// calculate each T coordinate from our dependent. 
static void		map_t_from_heights(VertexMap& map, const cgd::Plane3& lateral_plane);

// Given our mesh map, normalize both ST to the range of 0..1 based on the extrema of new coords already present
static void		st_normalize(VertexMap& map);
// Throw out the mesh map.  Optionally apply ST back onto the mesh.
static void		cleanup(VertexMap& map, BackMap& back, int apply_st);


void		map_object(ACObject * ob, VertexMap& map, BackMap& back)
{
	List * l = ac_object_get_surfacelist(ob);
	for (List * i = l; i; i = i->next)
		map_surface((Surface *) i->data, map, back);
}

static void		map_surface(Surface * surf, VertexMap& map, BackMap& back)
{
	vector<uv_vertex_info_t *>	new_verts(surf->numvert, NULL);
	
	List * i;
	int n;
	for (n = 0, i = surf->vertlist; i; ++n, i = i->next)
	{
		SVertex * sv = (SVertex *) i->data;
		VertexMap::iterator found = map.find(sv->v);
		if (found == map.end())
		{
			new_verts[n] = new uv_vertex_info_t;
			new_verts[n]->me = sv;
			new_verts[n]->my_source = NULL;
			map.insert(VertexMap::value_type(new_verts[n]->me->v,new_verts[n]));
		}
		else
		{
			new_verts[n] = found->second;
			back[found->second->me].push_back(sv);
		}
	}

	if (new_verts.size() > 1)
	for (n = 0; n < new_verts.size(); ++n)
	{
		int n1 = (n+1) % new_verts.size();
		new_verts[n]->connected.insert(new_verts[n1]);
		new_verts[n1]->connected.insert(new_verts[n]);
	}
}

static void		map_s_on_tri(VertexMap& map, const cgd::Plane3& proj_plane, const cgd::Point3 corners[3], const cgd::Point2 sts[3])
{
	double area = signed_area_3d(corners[0],corners[1],corners[2], proj_plane.n);
	double r0,r1,r2;

	for(VertexMap::iterator i = map.begin(); i != map.end(); ++i)
	{
		cgd::Point3	on_p = proj_plane.projection(cgd::Point3(i->second->me->v->x,i->second->me->v->y,i->second->me->v->z));
		
		r0 = signed_area_3d(on_p,corners[1],corners[2],proj_plane.n) / area ;
		r1 = signed_area_3d(on_p,corners[2],corners[0],proj_plane.n) / area;
		r2 = signed_area_3d(on_p,corners[0],corners[1],proj_plane.n) / area;
		
		i->second->new_s =  r0 * sts[0].x +
							r1 * sts[1].x +
							r2 * sts[2].x;
	}
}

static void		map_height_from_plane(VertexMap& map, const cgd::Plane3& proj_plane)
{
	for (VertexMap::iterator i = map.begin(); i != map.end(); ++i)
		i->second->proj_height = proj_plane.distance_denormaled(cgd::Point3(i->second->me->v->x,i->second->me->v->y,i->second->me->v->z));
}

static void		calc_best_dependencies(VertexMap& map, const cgd::Plane3& proj_plane)
{
	VertexMap::iterator i;
	for (i = map.begin(); i != map.end(); ++i)
	{
		uv_vertex_info_t * me = i->second;
		for (set<uv_vertex_info_t *>::iterator fi = me->connected.begin(); fi != me->connected.end(); ++fi)
		{
			uv_vertex_info_t * f = *fi;
			
			// We want to determine whether the vertex "me" is the best feeder of the vertex "f", our friend.
			// It is if "me" is the closest vertex to F on the same side of the ref line that is below f.			
			
			// This first check makes sure that we are closer to the line than the guy we will feed!
			printf("Exploring: %f,%f,%f (%f) - can we feed %f,%f,%f (%f)\n",
				me->me->v->x,me->me->v->y,me->me->v->z,me->proj_height,
				f->me->v->x,f->me->v->y,f->me->v->z,f->proj_height);
				
			cgd::Vector3 feed_angle(cgd::Point3(me->me->v->x,me->me->v->y,me->me->v->z),cgd::Point3(f->me->v->x,f->me->v->y,f->me->v->z));
			feed_angle.normalize();
				
			if (fabs(feed_angle.dz) < 0.5)
			if (fabs(f->proj_height) > fabs(me->proj_height))
			if ((f->proj_height >= 0 && me->proj_height >= 0) ||
				(f->proj_height <= 0 && me->proj_height <= 0))
			{
				// If we don't have ANY source feeding F, we must be the best one.
				if (f->my_source == NULL)
					f->my_source = me;
				else {
					uv_vertex_info_t * other = f->my_source;
					
					cgd::Point3	m_p(me->me->v->x,me->me->v->y,me->me->v->z);
					cgd::Point3 f_p(f->me->v->x,f->me->v->y,f->me->v->z);
					cgd::Point3 o_p(other->me->v->x,other->me->v->y,other->me->v->z);

					printf("====FEED CHOICE====\n");
					printf("We are feeding: %f,%f,%f\n",f_p.x,f_p.y,f_p.z);
					printf("We are:         %f,%f,%f\n",m_p.x,m_p.y,m_p.z);
					printf("Other is:       %f,%f,%f\n",o_p.x,o_p.y,o_p.z);
					
					cgd::Vector3 to_me   (f_p, m_p);
					cgd::Vector3 to_other(f_p, o_p);
					
					printf("pre-proj vectors are: %f,%f,%f for me, %f,%f,%f for 'other' (proj vec = %f,%f,%f\n",to_me.dx,to_me.dy,to_me.dz,to_other.dx,to_other.dy,to_other.dz,
						proj_plane.n.dx,proj_plane.n.dy,proj_plane.n.dz);
					to_me = proj_plane.n.projection(to_me);
					to_other = proj_plane.n.projection(to_other);
					printf("post-proj vectors are: %f,%f,%f (%f)for me, %f,%f,%f (%f) for 'other'\n",to_me.dx,to_me.dy,to_me.dz,to_me.squared_length(),to_other.dx,to_other.dy,to_other.dz,
						to_other.squared_length());
					
					if (to_me.squared_length() < to_other.squared_length())
					
//					m_p = proj_plane.projection(m_p);
//					f_p = proj_plane.projection(f_p);
//					o_p = proj_plane.projection(o_p);
				
//					if (cgd::Vector3(m_p,f_p).squared_length() < 
//						cgd::Vector3(o_p,f_p).squared_length())
				
					// There is another vertex.  If we are closer to the line from the other one, we're farther
					// from f and a better match.					
//					if ((me->proj_height >= 0 && other->proj_height > me->proj_height) ||
//						(me->proj_height <  0 && other->proj_height < me->proj_height))
					{
						printf("   REPLACING WITH ME!\n");
						f->my_source = me;
					}						
				}
			}
		}
	}

	// Create back-links
	for (i = map.begin(); i != map.end(); ++i)
	if (i->second->my_source)
		i->second->my_source->my_targets.insert(i->second);
}

static void		map_t_from_heights(VertexMap& map, const cgd::Plane3& lateral_plane)
{
	printf("HEIGHT TRANSFER!!!!!\n");
	// NOTE: we think the dependency list should always be acyclic.  But we are NOT sure.  Use "visited" set to ASSERT this.

	typedef multimap<float, uv_vertex_info_t *>	q_type;

	set<	   uv_vertex_info_t *>		visited;
	set<	   uv_vertex_info_t *>		needed;
	q_type								todo;

	// Start off with all of the unfed vertices!
	for(VertexMap::iterator i = map.begin(); i != map.end(); ++i)
	{
		if (i->second->my_source == NULL)
			todo.insert(q_type::value_type(fabs(i->second->proj_height),i->second));
		needed.insert(i->second);
	}
	
	while (!todo.empty())
	{
		uv_vertex_info_t * v = todo.begin()->second;
		todo.erase(todo.begin());
		visited.insert(v);
		needed.erase(v);
		
		printf("Vertex %f,%f,%f is already at: %f\n",	v->me->v->x,v->me->v->y,v->me->v->z,v->proj_height);
		
		v->new_t = v->proj_height;
		
		cgd::Point3 me_p(v->me->v->x,v->me->v->y,v->me->v->z);
		printf("   Before proj: %f,%f,%f ",me_p.x,me_p.y,me_p.z);
		me_p = lateral_plane.projection(me_p);
		printf("   After proj: %f,%f,%f\n ",me_p.x,me_p.y,me_p.z);
		
		for (set<uv_vertex_info_t *>::iterator di = v->my_targets.begin(); di != v->my_targets.end(); ++di)
		{
			printf("      I feed:\n");
			uv_vertex_info_t * d = *di;			
			cgd::Point3 his_p(d->me->v->x,d->me->v->y,d->me->v->z);
			printf("      Before proj: %f,%f,%f ",his_p.x,his_p.y,his_p.z);
			his_p = lateral_plane.projection(his_p);
			printf("      after proj: %f,%f,%f\n ",his_p.x,his_p.y,his_p.z);
			if (visited.count(d) > 0) message_dialog("ERROR - multiply hit vertex.");
			
			cgd::Vector3	to_this_pt(me_p,his_p);	
			printf("      vector is: %f,%f,%f (%f)\n",to_this_pt.dx,to_this_pt.dy,to_this_pt.dz,sqrt(to_this_pt.squared_length()));
			
			if (d->proj_height >= 0)
				d->proj_height = v->proj_height + sqrt(to_this_pt.squared_length());
			else
				d->proj_height = v->proj_height - sqrt(to_this_pt.squared_length());
			
			printf("       so proj height is: %f\n",d->proj_height);
			
			todo.insert(q_type::value_type(fabs(d->proj_height), d));
		}		
	}	
	if (!needed.empty())
		message_dialog("ERROR - missed vertex!");
}

static void		st_normalize(VertexMap& map)
{
	float min_s =  9.9e9;
	float max_s = -9.9e9;
	float min_t =  9.9e9;
	float max_t = -9.9e9;
	
	for (VertexMap::iterator i = map.begin(); i != map.end(); ++i)
	{
		min_s = min(min_s, i->second->new_s);
		min_t = min(min_t, i->second->new_t);
		max_s = max(max_s, i->second->new_s);
		max_t = max(max_t, i->second->new_t);
	}
	
	max_s -= min_s;
	max_t -= min_t;
	
	for (VertexMap::iterator i = map.begin(); i != map.end(); ++i)
	{
		i->second->new_s = (i->second->new_s - min_s) / max_s;
		i->second->new_t = (i->second->new_t - min_t) / max_t;
	}
}

static void		cleanup(VertexMap& map, BackMap& back, int apply_st)
{
	if (apply_st)
	{
		for (VertexMap::iterator i = map.begin(); i != map.end(); ++i)
		{
			i->second->me->tx = i->second->new_s;
			i->second->me->ty = i->second->new_t;
		}

		for (BackMap::iterator b = back.begin(); b != back.end(); ++b)
		{
			for (vector<SVertex *>::iterator s = b->second.begin(); s != b->second.end(); ++s)
			{
				(*s)->tx = b->first->tx;
				(*s)->ty = b->first->ty;
			}
		}
	}


	for (VertexMap::iterator i = map.begin(); i != map.end(); ++i)
		delete i->second;
	map.clear();
}


/*
	ALGORITHM:
	
	1. we have a bunch of points and a line.
	
	2. Find the projection plane (longitude/altitude plane) by: 
		cross line vector with up vector - gives us normal to plane...through a point at the start of the plane gives us our plane.

	3. Find the projecting plane horizontally by crossing the normal vector of the up plane with our main line, giving us the horizontal plane.
	
	4. form the vertical projective box - simply take the proj line and push it up and down by one meter to form a box. 
	
	5. Form the lateral projective plane - cross normal of vertical with up vector to get this normal
	
	S COORDINATE CALCULATION
	
		For each point.
			project it onto the vertical projection plane		
			use bathymetric coordinates to derive a T coordinate from our 4 "corners"
		
		Find the S extrema
		For each point
			Rescale S based on those extrema
		
	T COORDINATE CALCULATION
	
		For each point
			Find its absolute height from the horizontal projection plane
			Determine whether it is an "up" or "down" point
			Its best contributor is its source with a smaller ABS height with the same sign
				(or none if no such point exists.)
				
		While there are points
			Find the point closest to the ground with no source
			
			Its T coordinate is its fabs height
			
			for each of the points that depends on it
				their height becomes our height plus pythag of our distance along the LATERAL cut!*
				cut them off from us
		
		
		Find S extrema
		for each pt
			Rescale S based on extrema
		
		* take vector from one to the other and project on lateral plane to get this
		
*/

void do_uv_map(void)
{
	cgd::Plane3	proj_lateral(cgd::Point3(0,0,0), cgd::Vector3(1,0,0));
	cgd::Plane3	proj_longitudinal(cgd::Point3(0,0,0), cgd::Vector3(0,0,1));
	cgd::Plane3	proj_vertical(cgd::Point3(0,0,0), cgd::Vector3(0,1,0));
	
	cgd::Point3	corners[3];
	corners[0] = cgd::Point3(0,1,-1);
	corners[1] = cgd::Point3(0,0,-1);
	corners[2] = cgd::Point3(0,0, 1);
	cgd::Point2	texes[3];
	texes[0] = cgd::Point2(0,1);
	texes[1] = cgd::Point2(0,0);
	texes[2] = cgd::Point2(1,0);

	vector<ACObject *>	objs;
	VertexMap			mapping;
	BackMap				backing;

	find_all_selected_objects_flat(objs);
	
	for(vector<ACObject *>::iterator i = objs.begin(); i != objs.end(); ++i)
		map_object(*i, mapping, backing);

	map_s_on_tri(mapping, proj_lateral, corners,texes);	

	map_height_from_plane(mapping, proj_vertical);
	calc_best_dependencies(mapping, proj_longitudinal);
	
	for (VertexMap::iterator i = mapping.begin(); i != mapping.end(); ++i)
	{
		printf("Vertex %f,%f,%f is fed by:", i->first->x,i->first->y,i->first->z);
		if (i->second->my_source == NULL)
			printf(" no one.\n");
		else
			printf(" vertex %f,%f,%f\n", i->second->my_source->me->v->x,i->second->my_source->me->v->y,i->second->my_source->me->v->z);
	}
	
	map_t_from_heights(mapping, proj_longitudinal);
	
	st_normalize(mapping);
	cleanup(mapping, backing, 1);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// MAP FROM OBJECT TO OBJECT
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------

struct	a_tri {
	cgd::Point3		xyz[3];
	cgd::Point2		st[3];
	cgd::Plane3		p;
};

static int pt_in_tri_3d(const a_tri& tri, const cgd::Point3& p)
{
	cgd::Vector3	s0(tri.xyz[0],tri.xyz[1]);
	cgd::Vector3	s1(tri.xyz[1],tri.xyz[2]);
	cgd::Vector3	s2(tri.xyz[2],tri.xyz[0]);
	
	cgd::Vector3	n0(tri.p.n.cross(s0));
	cgd::Vector3	n1(tri.p.n.cross(s1));
	cgd::Vector3	n2(tri.p.n.cross(s2));
	
	cgd::Plane3		pl0(tri.xyz[0],n0);
	cgd::Plane3		pl1(tri.xyz[1],n1);
	cgd::Plane3		pl2(tri.xyz[2],n2);
	
	return  pl0.on_normal_side(p) && 
			pl1.on_normal_side(p) && 
			pl2.on_normal_side(p);
}

static void	find_st_for_tri(const a_tri& tri, const cgd::Point3& p, cgd::Point2& st)
{
	double area_total = signed_area_3d(tri.xyz[0],tri.xyz[1],tri.xyz[2],tri.p.n);
	
	double	r0 = signed_area_3d(p,tri.xyz[1],tri.xyz[2],tri.p.n) / area_total;
	double	r1 = signed_area_3d(p,tri.xyz[2],tri.xyz[0],tri.p.n) / area_total;
	double	r2 = signed_area_3d(p,tri.xyz[0],tri.xyz[1],tri.p.n) / area_total;
	
	st.x = r0 * tri.st[0].x + r1 * tri.st[1].x + r2 * tri.st[2].x;
	st.y = r0 * tri.st[0].y + r1 * tri.st[1].y + r2 * tri.st[2].y;
}

static int	better_tri(const a_tri& tri, const cgd::Line3& l, double& best_dist, cgd::Point3& x_pt)
{
	cgd::Point3	our_pt;
	if (tri.p.intersect(l,our_pt))
	{
		double dist = cgd::Vector3(l.p, our_pt).squared_length();
		if (best_dist > dist)
		if (pt_in_tri_3d(tri,our_pt))
		{
			best_dist = dist;
			x_pt = our_pt;
			return 1;
		}
	}
	return 0;
}

static int tex_a_surface(Surface * surf, const vector<a_tri>& tris)
{	
	int missed = 0;
	for (List * p = surf->vertlist; p; p = p->next)
	{
		SVertex * sv = (SVertex *) p->data;
		
		double	best_dist = 9.9e9;
		const a_tri * best_tri = NULL;
		cgd::Point3	best_pt;
		cgd::Line3	shooter(cgd::Point3(sv->v->x,sv->v->y,sv->v->z),cgd::Vector3(sv->normal.x,sv->normal.y,sv->normal.z));
				
		for (vector<a_tri>::const_iterator it = tris.begin(); it != tris.end(); ++it)
		{		
			if (better_tri(*it, shooter, best_dist, best_pt))
				best_tri = &*it;
		}
		
		if (best_tri)
		{
			cgd::Point2	 st;
			find_st_for_tri(*best_tri, best_pt, st);
			sv->tx = st.x;
			sv->ty = st.y;			
		} else
			++missed;
	}
	return missed;
}

static int	tex_an_object(ACObject * ob, const vector<a_tri>& tris)
{
	int total = 0;
	List * l = ac_object_get_surfacelist(ob);
	for (List * i = l; i; i = i->next)
		total += tex_a_surface((Surface *) i->data, tris); 
	return total;
}

static void tri_map_from_obj(ACObject * ob, vector<a_tri>& tris)
{
	tris.clear();
	List * surf_tris = ac_object_get_triangle_surfaces(ob);
	
	for (List * t = surf_tris; t; t = t->next)
	{
		Surface * s = (Surface *) t->data;

		SVertex * sv1 = (SVertex *) s->vertlist->data;
		SVertex * sv2 = (SVertex *) s->vertlist->next->data;
		SVertex * sv3 = (SVertex *) s->vertlist->next->next->data;

		tris.push_back(a_tri());
		tris.back().xyz[0] = cgd::Point3(sv1->v->x,sv1->v->y,sv1->v->z);
		tris.back().xyz[1] = cgd::Point3(sv2->v->x,sv2->v->y,sv2->v->z);
		tris.back().xyz[2] = cgd::Point3(sv3->v->x,sv3->v->y,sv3->v->z);

		tris.back().st[0] = cgd::Point2(sv1->tx,sv1->ty);
		tris.back().st[1] = cgd::Point2(sv2->tx,sv2->ty);
		tris.back().st[2] = cgd::Point2(sv3->tx,sv3->ty);

		cgd::Vector3	n = cgd::Vector3(tris.back().xyz[0],tris.back().xyz[1]).cross(cgd::Vector3(tris.back().xyz[0],tris.back().xyz[2]));
		n.normalize();
		tris.back().p = cgd::Plane3(tris.back().xyz[0],n);
	}
}

static	vector<a_tri>	g_tris;

void do_uv_copy(void)
{
	vector<ACObject *> objs;

	find_all_selected_objects_flat(objs);
	if (objs.size() != 1) message_dialog("Please select one object.");
	else
	{
		g_tris.clear();
		tri_map_from_obj(objs[0], g_tris);
	}
}

void do_uv_paste(void)
{
	vector<ACObject *> objs;
	find_all_selected_objects_flat(objs);
	int total = 0;
	for (int n = 0; n < objs.size(); ++n)
	{
		total += tex_an_object(objs[n], g_tris);
	}
	if (total > 0) message_dialog("Unable to texture %d points.\n", total);
}

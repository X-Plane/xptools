/*
 *  XObjWriteEmbedded.cp
 *  SceneryTools
 *
 *  Created by bsupnik on 8/29/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#include "XObjWriteEmbedded.h"
#include "AssertUtils.h"
#include <math.h>
#include <algorithm>
#include "MathUtils.h"
#include "MatrixUtils.h"
#include <list>

using std::max;

struct sort_by_key {
	bool operator()(const XObjKey& lhs, const XObjKey& rhs) const {
		return lhs.key < rhs.key;
	}
};

struct cmp_by_key {
	bool operator()(const XObjKey& lhs, const XObjKey& rhs) const {
		return lhs.eq_key(rhs);
	}
};

struct cmp_by_val {
	bool operator()(const XObjKey& lhs, const XObjKey& rhs) const {
		return lhs.eq_val(rhs);
	}
};

struct cmp_by_all {
	bool operator()(const XObjKey& lhs, const XObjKey& rhs) const {
		return lhs.eq(rhs);
	}
};

static void dump(const vector<XObjKey>& d)
{
	for(int i = 0; i < d.size(); ++i)
		printf("%d:  %f -> %f,%f,%f\n", i, d[i].key, d[i].v[0], d[i].v[1], d[i].v[2]); 
}



//••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••
// BOUNDING SPHERES
//••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••
// This routine calculates a bounding sphere from point data.  The bounding sphere isn't the SMALLEST one containing all the points, but it is very close, and still calculates
// fast.  Since we use  bounding spheres to cull, having small bounding spheres is good.  (This bounding sphere is better than doing a pythag on the bounding cube.  In other words,
// this bounding sphere of points is smaller than the bounding sphere of the bounding cube of the points.)
//
// pts_start	pointer to the floating point data.  Pointer to the first X value!
// pts_count	number of distinct points.
// pts_stride	number of floats in each point.  XYZ must be first, so this must be at least 3.  This lets us skip normals and ST coordinates.
// out_min_xyz	these get filled out with the min and max X, Y and Z values in the entire data aset
// out_max_xyz
// out_sphere	This sphere gets filled out.  It is in the form x, y, z (center), radius
inline void	bounding_sphere(
				const float *	pts_start,
				int			pts_count,
				int			pts_stride,
				float			out_min_xyz[3],
				float			out_max_xyz[3],
				float			out_sphere[4])
{
	Assert(pts_count > 0);
	
	int		n;

	const float *	p;

	if(out_min_xyz)
	{
		out_min_xyz[0] = out_max_xyz[0] = pts_start[0];
		out_min_xyz[1] = out_max_xyz[1] = pts_start[1];
		out_min_xyz[2] = out_max_xyz[2] = pts_start[2];
	}
	p = pts_start;

	if(out_min_xyz)
	for(n = 0; n < pts_count; ++n, p += pts_stride)
	{
		out_min_xyz[0] = min(out_min_xyz[0],p[0]);
		out_min_xyz[1] = min(out_min_xyz[1],p[1]);
		out_min_xyz[2] = min(out_min_xyz[2],p[2]);

		out_max_xyz[0] = max(out_max_xyz[0],p[0]);
		out_max_xyz[1] = max(out_max_xyz[1],p[1]);
		out_max_xyz[2] = max(out_max_xyz[2],p[2]);
	}

	p = pts_start;

	out_sphere[0] = p[0];
	out_sphere[1] = p[1];
	out_sphere[2] = p[2];
	out_sphere[3] = 0.0f;	

	p += pts_stride;

	for(n = 1; n < pts_count; ++n, p += pts_stride)
	{
		float	dV[3] = {
						p[0] - out_sphere[0],
						p[1] - out_sphere[1],
						p[2] - out_sphere[2] };
		float	dist2 = pythag_sqr(dV[0],dV[1], dV[2]);

		if(dist2 > sqr(out_sphere[3]))
		{
			float dist = sqrt(dist2);
			out_sphere[3] = (out_sphere[3] + dist) * 0.5f;
			float scale = (dist - out_sphere[3]) / dist;
			out_sphere[0] += (dV[0] * scale);
			out_sphere[1] += (dV[1] * scale);
			out_sphere[2] += (dV[2] * scale);
		}
	}
}

inline void permute_vector(vector<float>& v)
{
	int n = v.size() / 3;
	int count = n / 2;
	while(count-- > 0)
	{
		int p1 = random() % n;
		int p2 = random() % n;
		swap(v[3*p1  ],v[3*p2  ]);
		swap(v[3*p1+1],v[3*p2+1]);
		swap(v[3*p1+2],v[3*p2+2]);
	}
}

// when we don't beat the heursitic, we're within 1% of the best I have by algorithm.
inline void bounding_sphere_random(const vector<float>& pts, int n, float sphere[4])
{
	if(pts.empty())
	{
		sphere[0] = sphere[1] = sphere[2] = sphere[3] = 0.0f;
		return;
	}
	vector<float> ozp(pts);
	bounding_sphere(&*pts.begin(),pts.size() / 3, 3, NULL, NULL, sphere);
	
	for(int seed = 0; seed < n; ++seed)
	{
		permute_vector(ozp);
		float ns[4];
		bounding_sphere(&*ozp.begin(), ozp.size() / 3, 3, NULL,NULL,ns);
		if(ns[3] < sphere[3])
		{
			sphere[0] = ns[0];
			sphere[1] = ns[1];
			sphere[2] = ns[2];
			sphere[3] = ns[3];
		}
	}
}

// when we don't beat the heursitic, we're within 1% of the best I have by algorithm.
void bounding_sphere_random(
				const float *	pts_start,
				int			pts_count,
				int			pts_stride,
				float			out_min_xyz[3],
				float			out_max_xyz[3],
				float			out_sphere[4])
{
	bounding_sphere(pts_start,pts_count,pts_stride,out_min_xyz,out_max_xyz,out_sphere);
	vector<float> p (3 * pts_count);
	for(int i = 0; i < pts_count; ++i)
	{
		p[i*3  ] = pts_start[i*pts_stride  ];
		p[i*3+1] = pts_start[i*pts_stride+1];
		p[i*3+2] = pts_start[i*pts_stride+2];
	}

	bounding_sphere_random(p, 1000, out_sphere);
	//printf("Got: %f, %f, %f, %f\n", out_sphere[0],out_sphere[1],out_sphere[2],out_sphere[3]);
}



// This routine takes two spheres.  Cur is the sphere we want to grow, and add is the sphere that we are adding to  it.
// cur will become bigger such that add is fully contained in cur.  We try to grow cur as little as possible.
// NOTE: a sphere with negative radius is treated as the "empty" sphere...so if cur has negative radius, then cur becomes
// add.  Both spheres are in the form x,y,z (center), r.
inline void grow_sphere(float cur[4], const float add[4])
{
	Assert	(add[3]>=0.0);			// Quick exit: new sphere is gone.
	if			(add[3]< 0.0)return;	// if the size is 0, we cannot add it.

	// Quick exit: old sphere is gone -- use new.
	if (cur[3] < 0.0) {
		cur[0] = add[0];
		cur[1] = add[1];
		cur[2] = add[2];
		cur[3] = add[3];
		return;
	}

	// Quick exit: new sphere is fully inside old (difference in radii is larger than the distance
	// from the old center to the new.  OR vice versa!
	if (sqr(cur[3] - add[3]) >= pythag_sqr(cur[0]-add[0] , cur[1]-add[1] , cur[2]-add[2]))
	{
		if (cur[3] >= add[3]) return;	// Old sphere is bigger - we win
		cur[0] = add[0]; 				// Use the new sphere and bail
		cur[1] = add[1];
		cur[2] = add[2];
		cur[3] = add[3];
	 	return;
	}

	// Vector from old to new
	double	to_new[3] = { add[0] - cur[0], add[1] - cur[1], add[2] - cur[2] };
	vec3_normalize(to_new);

	double	old_r[3] = { to_new[0] * -cur[3], to_new[1] * -cur[3], to_new[2] * -cur[3] };
	double	new_r[3] = { to_new[0] *  add[3], to_new[1] *  add[3], to_new[2] *  add[3] };

	double	p1[3] = { add[0] + new_r[0], add[1] + new_r[1], add[2] + new_r[2] };
	double	p2[3] = { cur[0] + old_r[0], cur[1] + old_r[1], cur[2] + old_r[2] };

	cur[0] = (p1[0]+p2[0])*0.5;
	cur[1] = (p1[1]+p2[1])*0.5;
	cur[2] = (p1[2]+p2[2])*0.5;
	cur[3] = pythag(p2[0]-p1[0],p2[1]-p1[1],p2[2]-p1[2])*0.5 + 0.1;	// add 0.1 to counter any roundoff error that would give us a dev-assert below! this will give us enough round-off error buffer to handle numbers along the lines of 800,000 m from the origin, or 2.4 million feet up
	// What the heck is this?!?  +0.1  Here's the problem: enough rounding happens that the "Exact" new large sphere that
	// merges the two inputs might not be big enough to really contain it..it might be 0.000000001 to small. :-(  So - we fudge and
	// grow the sphere by an extra 0.1 to hedge rounding error.  Also do a quick check to see if we're definitely going to fail
	// containment tests later... that would mean our fudge factor isn't large enough.
	Assert(sqr(cur[3] - add[3]) >= pythag_sqr(cur[0]-add[0] , cur[1]-add[1] , cur[2]-add[2]) && cur[3] >= add[3]);
}


//••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••
// ANIMATION DREDGING UTILS
//••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••

void	setIdentityMatrixf(GLfloat m[16])
{
	memset(m, 0, sizeof(GLfloat)*16);
	m[0] = m[5] = m[10] = m[15] = 1.0;
}

void
buildRotationf( GLfloat	m[16],
		     GLfloat angle, GLfloat x, GLfloat y, GLfloat z )
{
   GLfloat xx, yy, zz, xy, yz, zx, xs, ys, zs, one_c, s, c;
   GLboolean optimized;

   s = sin( angle * DEG2RAD );
   c = cos( angle * DEG2RAD );
   setIdentityMatrixf(m);
   optimized = GL_FALSE;

#define M(row,col)  m[col*4+row]

   if (x == 0.0f) {
      if (y == 0.0f) {
         if (z != 0.0f) {
            optimized = GL_TRUE;
            /* rotate only around z-axis */
            M(0,0) = c;
            M(1,1) = c;
            if (z < 0.0f) {
               M(0,1) = s;
               M(1,0) = -s;
            }
            else {
               M(0,1) = -s;
               M(1,0) = s;
            }
         }
      }
      else if (z == 0.0f) {
         optimized = GL_TRUE;
         /* rotate only around y-axis */
         M(0,0) = c;
         M(2,2) = c;
         if (y < 0.0f) {
            M(0,2) = -s;
            M(2,0) = s;
         }
         else {
            M(0,2) = s;
            M(2,0) = -s;
         }
      }
   }
   else if (y == 0.0f) {
      if (z == 0.0f) {
         optimized = GL_TRUE;
         /* rotate only around x-axis */
         M(1,1) = c;
         M(2,2) = c;
         if (x < 0.0f) {
            M(1,2) = s;
            M(2,1) = -s;
         }
         else {
            M(1,2) = -s;
            M(2,1) = s;
         }
      }
   }

   if (!optimized) {
      const GLfloat mag = sqrtf(x * x + y * y + z * z);

      if (mag <= 1.0e-4f) {
         /* no rotation, leave mat as-is */
         return;
      }

      x /= mag;
      y /= mag;
      z /= mag;


      /*
       *     Arbitrary axis rotation matrix.
       *
       *  This is composed of 5 matrices, Rz, Ry, T, Ry', Rz', multiplied
       *  like so:  Rz * Ry * T * Ry' * Rz'.  T is the final rotation
       *  (which is about the X-axis), and the two composite transforms
       *  Ry' * Rz' and Rz * Ry are (respectively) the rotations necessary
       *  from the arbitrary axis to the X-axis then back.  They are
       *  all elementary rotations.
       *
       *  Rz' is a rotation about the Z-axis, to bring the axis vector
       *  into the x-z plane.  Then Ry' is applied, rotating about the
       *  Y-axis to bring the axis vector parallel with the X-axis.  The
       *  rotation about the X-axis is then performed.  Ry and Rz are
       *  simply the respective inverse transforms to bring the arbitrary
       *  axis back to its original orientation.  The first transforms
       *  Rz' and Ry' are considered inverses, since the data from the
       *  arbitrary axis gives you info on how to get to it, not how
       *  to get away from it, and an inverse must be applied.
       *
       *  The basic calculation used is to recognize that the arbitrary
       *  axis vector (x, y, z), since it is of unit length, actually
       *  represents the sines and cosines of the angles to rotate the
       *  X-axis to the same orientation, with theta being the angle about
       *  Z and phi the angle about Y (in the order described above)
       *  as follows:
       *
       *  cos ( theta ) = x / sqrt ( 1 - z^2 )
       *  sin ( theta ) = y / sqrt ( 1 - z^2 )
       *
       *  cos ( phi ) = sqrt ( 1 - z^2 )
       *  sin ( phi ) = z
       *
       *  Note that cos ( phi ) can further be inserted to the above
       *  formulas:
       *
       *  cos ( theta ) = x / cos ( phi )
       *  sin ( theta ) = y / sin ( phi )
       *
       *  ...etc.  Because of those relations and the standard trigonometric
       *  relations, it is pssible to reduce the transforms down to what
       *  is used below.  It may be that any primary axis chosen will give the
       *  same results (modulo a sign convention) using thie method.
       *
       *  Particularly nice is to notice that all divisions that might
       *  have caused trouble when parallel to certain planes or
       *  axis go away with care paid to reducing the expressions.
       *  After checking, it does perform correctly under all cases, since
       *  in all the cases of division where the denominator would have
       *  been zero, the numerator would have been zero as well, giving
       *  the expected result.
       */

      xx = x * x;
      yy = y * y;
      zz = z * z;
      xy = x * y;
      yz = y * z;
      zx = z * x;
      xs = x * s;
      ys = y * s;
      zs = z * s;
      one_c = 1.0f - c;

      /* We already hold the identity-matrix so we can skip some statements */
      M(0,0) = (one_c * xx) + c;
      M(0,1) = (one_c * xy) - zs;
      M(0,2) = (one_c * zx) + ys;
/*    M(0,3) = 0.0f; */

      M(1,0) = (one_c * xy) + zs;
      M(1,1) = (one_c * yy) + c;
      M(1,2) = (one_c * yz) - xs;
/*    M(1,3) = 0.0f; */

      M(2,0) = (one_c * zx) - ys;
      M(2,1) = (one_c * yz) + xs;
      M(2,2) = (one_c * zz) + c;
/*    M(2,3) = 0.0f; */

/*
      M(3,0) = 0.0f;
      M(3,1) = 0.0f;
      M(3,2) = 0.0f;
      M(3,3) = 1.0f;
*/
   }
#undef M

}

inline void multMatricesf(GLfloat dst[16], const GLfloat a[16], const GLfloat b[16])
{
	dst[0 ] = b[0 ]*a[0] + b[1 ]*a[4] + b[2 ]*a[8 ] + b[3 ]*a[12];
	dst[1 ] = b[0 ]*a[1] + b[1 ]*a[5] + b[2 ]*a[9 ] + b[3 ]*a[13];
	dst[2 ] = b[0 ]*a[2] + b[1 ]*a[6] + b[2 ]*a[10] + b[3 ]*a[14];
	dst[3 ] = b[0 ]*a[3] + b[1 ]*a[7] + b[2 ]*a[11] + b[3 ]*a[15];
	dst[4 ] = b[4 ]*a[0] + b[5 ]*a[4] + b[6 ]*a[8 ] + b[7 ]*a[12];
	dst[5 ] = b[4 ]*a[1] + b[5 ]*a[5] + b[6 ]*a[9 ] + b[7 ]*a[13];
	dst[6 ] = b[4 ]*a[2] + b[5 ]*a[6] + b[6 ]*a[10] + b[7 ]*a[14];
	dst[7 ] = b[4 ]*a[3] + b[5 ]*a[7] + b[6 ]*a[11] + b[7 ]*a[15];
	dst[8 ] = b[8 ]*a[0] + b[9 ]*a[4] + b[10]*a[8 ] + b[11]*a[12];
	dst[9 ] = b[8 ]*a[1] + b[9 ]*a[5] + b[10]*a[9 ] + b[11]*a[13];
	dst[10] = b[8 ]*a[2] + b[9 ]*a[6] + b[10]*a[10] + b[11]*a[14];
	dst[11] = b[8 ]*a[3] + b[9 ]*a[7] + b[10]*a[11] + b[11]*a[15];
	dst[12] = b[12]*a[0] + b[13]*a[4] + b[14]*a[8 ] + b[15]*a[12];
	dst[13] = b[12]*a[1] + b[13]*a[5] + b[14]*a[9 ] + b[15]*a[13];
	dst[14] = b[12]*a[2] + b[13]*a[6] + b[14]*a[10] + b[15]*a[14];
	dst[15] = b[12]*a[3] + b[13]*a[7] + b[14]*a[11] + b[15]*a[15];
}

void applyRotationf( GLfloat m[16], GLfloat angle, GLfloat x, GLfloat y, GLfloat z )
{
	GLfloat	temp[16], rot[16];
	buildRotationf(rot, angle, x, y, z);
	multMatricesf(temp, m, rot);
	memcpy(m,temp,sizeof(temp));
}

extern	void applyTranslationf( GLfloat m[16], GLfloat x, GLfloat y, GLfloat z )
{
   m[12] = m[0] * x + m[4] * y + m[8]  * z + m[12];
   m[13] = m[1] * x + m[5] * y + m[9]  * z + m[13];
   m[14] = m[2] * x + m[6] * y + m[10] * z + m[14];
   m[15] = m[3] * x + m[7] * y + m[11] * z + m[15];
}

inline void  multMatrixVec3f(GLfloat dst[3], const GLfloat m[16], const GLfloat v[3])
{
	dst[0] = v[0] * m[0] + v[1] * m[4] + v[2] * m[8] + m[12];
	dst[1] = v[0] * m[1] + v[1] * m[5] + v[2] * m[9] + m[13];
	dst[2] = v[0] * m[2] + v[1] * m[6] + v[2] * m[10] + m[14];
}

inline void applyMatrixVec3f(GLfloat vec[3], const GLfloat m[16])
{
	GLfloat	t[3];
	multMatrixVec3f(t, m, vec);
	vec[0] = t[0];
	vec[1] = t[1];
	vec[2] = t[2];
}




struct anim_xform {
public:

	anim_xform(const float a[3], const vector<XObjKey>& kft) : key_table(kft), is_rotate(true)  { axis[0] = a[0]; axis[1] = a[1]; axis[2] = a[2]; clean_kft();  }
	anim_xform(					 const vector<XObjKey>& kft) : key_table(kft), is_rotate(false) { clean_kft(); }

	bool	is_rotate;
	float	axis[3];
	vector<XObjKey>		key_table;


	void clean_kft()
	{
		// For the purpose of simulating animations, duplicate OUTPUT values tell us nothing useful about the animatoin - the pose is the same.  But they fool
		// us into doing too many dupe steps. So cut the KFT down to its minimum...unsorted unique will simply remove adjacents.  For KFTs with clamping (four keys
		// for two points) this gives us a 2^N speedup where N is the nesting level - so that's 256x faster for an 8-level deep animation!
		key_table.erase(unique(key_table.begin(),key_table.end(), cmp_by_val()),key_table.end());
	}

	int	calc_steps() { 
		Assert(key_table.size() > 0);
		if(key_table.size() == 1) return 1;
		if(key_table.size() == 2 && key_table[0].key == key_table[1].key)	
			return 1;
			
		if(!is_rotate)
			return 2;
		
		float vmin, vmax;
		vmin = vmax = key_table[0].v[0];
		for(int i = 0; i < key_table.size(); ++i)
		{
			vmin = min(vmin,key_table[i].v[0]);
			vmax = min(vmax,key_table[i].v[0]);
		}		
		
		return intmax2(2, floorf((vmax - vmin) / 30.0f));
	}
	void	apply_anim(float m[16], int s, int steps)
	{
		Assert(s < steps);
		Assert(steps > 0);
		float r = 0.0f;
		if(steps > 1)
			r = (float) s / (float) (steps-1);
	
		float vmin[3], vmax[3];
		for(int c = 0; c < 3; ++c)
		{
			vmin[c] = vmax[c] = key_table[0].v[c];
			for(int i = 0; i < key_table.size(); ++i)
			{
				vmin[c] = min(vmin[c],key_table[i].v[c]);
				vmax[c] = min(vmax[c],key_table[i].v[c]);
			}		
		}
		
		if(is_rotate)
		{
			applyRotationf(m, 
				interp(0,vmin[0],1,vmax[0],r),					   
				axis[0], axis[1], axis[2]);
		}
		else
		{
			applyTranslationf(m,
					interp(0,vmin[0],1,vmax[0],r),
					interp(0,vmin[1],1,vmax[1],r),
					interp(0,vmin[2],1,vmax[2],r));
		}
	}
};

typedef vector<anim_xform>	anim_xform_list;

void grow_sphere_with_list(float cur[4], const float add[4], vector<anim_xform>& anim_stack)
{
	if(anim_stack.empty())
	{
		grow_sphere(cur,add);
		return;
	}
	int a;
	vector<int> steps(anim_stack.size(), 0);
	vector<int> stops;
	
	long long int ctr = 1;
	
	for(int a = 0; a < anim_stack.size(); ++a)
	{
		stops.push_back(anim_stack[a].calc_steps());
		ctr *= (long long int) stops.back();
	}
	
//	printf("Need to eval %lld steps for %zd nested animations.\n", ctr, anim_stack.size());
	
	while(1)
	{
		GLfloat m[16];
		setIdentityMatrixf(m);
		for(a = 0; a < anim_stack.size(); ++a)
			anim_stack[a].apply_anim(m,steps[a],stops[a]);
		
		GLfloat ta[4];
		multMatrixVec3f(ta,m,add);
		ta[3] = add[3];
		grow_sphere(cur,ta);
		//printf("Grow to: %f %f %f %f\n", cur[0],cur[1],cur[2],cur[3]);
		
		++steps[0];
		for(a = 1; a < anim_stack.size(); ++a)
		{
			int p = a-1;
			if(steps[p] == stops[p])
			{
				steps[p]=0;
				++steps[a];
			}
		}
		if(steps.back() == stops.back())
			break;
	}
}

//••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••
// OBJe structures
//••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••
// This MUST be kept in sync with the dest app.

struct match_lite {
	match_lite(const XObjCmd8& proto) : b(proto) { }
	bool operator()(XObjCmd8& a) const
	{
		if(a.cmd != b.cmd) return false;
		if(a.idx_count != b.idx_count) return false;
		if(a.name != b.name) return false;
		int tp = a.idx_count + 3;
		for(int i = 0; i < tp; ++i)
			if(a.params[i] != b.params[i])
				return false;
		return true;
	}
	const XObjCmd8& b;
};

const float	shiny_rat_undef = -2.0;
const float shiny_rat_default = -1.0;

enum {
	cmd_nop=0,					// Used to pad alignment.
	draw_tris=1,				// unsigned short offset, unsigned short count
	attr_lod=2,					// float near, float far, ushort old jump, [then optional int: bytes to skip (used only if short jump is 0)]
	attr_poly_offset=3,			// char offset
	attr_begin=4,
	attr_end=5,
	attr_translate_static=6,	// float x, y, z
	attr_translate=7,			// float x1, y1, z1, x2, y2, z2, v1, v2, dref
	attr_rotate=8,				// float ax, ay, az, r1, r2, v1, v2, dref
	attr_show=9,				// float v1, v2, dref
	attr_hide=10,				// float v1, v2, dref
	attr_light_named=11,		// uchar light idx float x, y, z
	attr_light_bulk=12,			// uchar light idx ushort count, float [xyz] x count
	cmd_stop=13,
	attr_light_level=14,		// float v1, float v2, dref
	attr_cockpit_tex=15,
	attr_no_cockpit_tex=16,
	attr_manip=17,				// uchar type,float vx,vy,vz,v1,v2, dref
								// uchar num_verts float[3] for each vert.
	attr_light_param=18,		// uchar pcount, ushort real count, int idx, float x, y, z [float x pcount]
	attr_loop = 19,				// float wrap value

	attr_manip2 = 20,			// 4-BYTE ALIGNED
								// uchar type,ushort num_verts,			
								// float:vx,vy,vz,v1min,v1max,v2min,v2max,wheel,
								// short dref1, short dref2
								// short tooltip, short 0 pad
								// float[3] for each vert.
								// 11 + 3xverts words

	attr_emitter = 21,			// short name, short dref,
								// float x y z psi the phi v1 v2
	
	attr_metalness_obsolete = 22// No paayload
};

enum {
	manip_noop=0,
	manip_axis=1,
	manip_cmd=2,
	manip_push=3,
	manip_toggle=4,
	manip_radio=5,
	manip_axis_2d=6,
	manip_axis_pix=7,
	manip_cmd_axis=8,
	manip_delta=9,
	manip_wrap=10,
	manip_panel=11,
	manip_cmd_knob=12,
	manip_cmd_switch_ud=13,
	manip_cmd_switch_lr=14,
	manip_axis_knob=15,
	manip_axis_swich_ud=16,
	manip_axis_switch_lr=17
};

struct	embed_props_t {
	volatile int	ref_count;
	int				layer_group;
	int				tex_day;		// string offset becomes obj
	int				tex_lit;		// string offset becoems obj
	float			cull_xyzr[4];
	float			max_lod;
	float			scale_vert;		// scale for XYZ
	unsigned short	hard_verts;		// count of hard verticies
	unsigned short	light_off;		// Offset to light cmds in bytes
	unsigned int	vbo_geo;
	unsigned int	vbo_idx;
	int				pool;			// undef
	int				dref_info;		// always NULL
	int				tex_nrm;
//	void *			light_info;		// always null
	float			scale_tex[2];
	unsigned short	particle_system;
	unsigned short	emitter_count;
	unsigned short	use_metalness;
	unsigned short	unused_padding;
};

struct master_header_t {
	char	magic[4];
	int		prp_off;
	int		prp_len;
	int		geo_off;
	int		geo_len;
	int		idx_off;
	int		idx_len;
	int		str_off;
	int		str_len;
	int		cmd_off;
	int		cmd_len;
};


//••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••
// LIGHT HANDLING
//••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••

/*
struct named_light_info_t {
	const char *	name;
	int				custom;
	const char *	dataref;
};

static named_light_info_t k_light_info[] = {
	"airplane_landing",			1,		"sim/graphics/animation/lights/airplane_landing_light",
	"airplane_nav_left",		1,		"sim/graphics/animation/lights/airplane_nav_light_left",
	"airplane_nav_right",		1,		"sim/graphics/animation/lights/airplane_nav_light_right",
	"airplane_nav_tail",		1,		"sim/graphics/animation/lights/airplane_nav_light_tail",
	"airplane_nav_l",			1,		"sim/graphics/animation/lights/airplane_nav_light_left",		// Old - for compatibility.
	"airplane_nav_r",			1,		"sim/graphics/animation/lights/airplane_nav_light_right",
	"airplane_nav_t",			1,		"sim/graphics/animation/lights/airplane_nav_light_tail",
	"airplane_strobe",			1,		"sim/graphics/animation/lights/airplane_strobe_light",
	"airplane_beacon",			1,		"sim/graphics/animation/lights/airplane_beacon_light",

	"rwy_papi_1",				1,		"sim/graphics/animation/lights/rwy_papi_1",
	"rwy_papi_2",				1,		"sim/graphics/animation/lights/rwy_papi_2",
	"rwy_papi_3",				1,		"sim/graphics/animation/lights/rwy_papi_3",
	"rwy_papi_4",				1,		"sim/graphics/animation/lights/rwy_papi_4",

	"rwy_papi_rev_1",			1,		"sim/graphics/animation/lights/rwy_papi_rev_1",
	"rwy_papi_rev_2",			1,		"sim/graphics/animation/lights/rwy_papi_rev_2",
	"rwy_papi_rev_3",			1,		"sim/graphics/animation/lights/rwy_papi_rev_3",
	"rwy_papi_rev_4",			1,		"sim/graphics/animation/lights/rwy_papi_rev_4",

	"rwy_ww",					0,		"sim/graphics/animation/lights/runway_ww",
	"rwy_wy",					0,		"sim/graphics/animation/lights/runway_wy",
	"rwy_yw",					0,		"sim/graphics/animation/lights/runway_yw",
	"rwy_yy",					0,		"sim/graphics/animation/lights/runway_yy",
	"rwy_gr",					0,		"sim/graphics/animation/lights/runway_gr",
	"rwy_rg",					0,		"sim/graphics/animation/lights/runway_rg",
	"rwy_xw",					0,		"sim/graphics/animation/lights/runway_xw",
	"rwy_xr",					0,		"sim/graphics/animation/lights/runway_xr",
	"rwy_wx",					0,		"sim/graphics/animation/lights/runway_wx",
	"rwy_rx",					0,		"sim/graphics/animation/lights/runway_rx",
	"taxi_b",					0,		"sim/graphics/animation/lights/taxi_b",
	
	"carrier_center_white",		0,		"sim/graphics/animation/carrier_center_white",		
	"carrier_deck_blue_e",		0,		"sim/graphics/animation/carrier_deck_blue_e",		
	"carrier_deck_blue_n",		0,		"sim/graphics/animation/carrier_deck_blue_n",		
	"carrier_deck_blue_s",		0,		"sim/graphics/animation/carrier_deck_blue_s",		
	"carrier_deck_blue_w",		0,		"sim/graphics/animation/carrier_deck_blue_w",		
	"carrier_edge_white",		0,		"sim/graphics/animation/carrier_edge_white",		
	"carrier_foul_line_red",	0,		"sim/graphics/animation/carrier_foul_line_red",	
	"carrier_foul_line_white",	0,		"sim/graphics/animation/carrier_foul_line_white",	
	"carrier_thresh_white",		0,		"sim/graphics/animation/carrier_thresh_white",		
	"ship_nav_left",			0,		"sim/graphics/animation/ship_nav_left",			
	"ship_nav_right",			0,		"sim/graphics/animation/ship_nav_right",			
	"ship_nav_tail",			0,		"sim/graphics/animation/ship_nav_tail",
	"ship_mast_obs"	,			0,		"sim/graphics/animation/ship_mast_obs",
	"ship_mast_powered",		0,		"sim/graphics/animation/ship_mast_powered",
	"carrier_mast_strobe",		0,		"sim/graphics/animation/carrier_mast_strobe",
	"carrier_pitch_lights",		0,		"sim/graphics/animation/carrier_pitch_lights",
	"carrier_datum",			1,		"sim/graphics/animation/carrier_datum",			
	"carrier_meatball1",		1,		"sim/graphics/animation/carrier_meatball1",		
	"carrier_meatball2",		1,		"sim/graphics/animation/carrier_meatball2",		
	"carrier_meatball3",		1,		"sim/graphics/animation/carrier_meatball3",		
	"carrier_meatball4",		1,		"sim/graphics/animation/carrier_meatball4",		
	"carrier_meatball5",		1,		"sim/graphics/animation/carrier_meatball5",		
	"carrier_waveoff",			1,		"sim/graphics/animation/carrier_waveoff",			
	
	0,0,0
};

*/

//••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••
// MEMORY BLOCK UTILITY
//••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••
// A crude memory accumulator - fixed size - we can predict that an OBJe command list won't be that big, and that we have a LOT of memory on the converter machine.

struct	mem_block {

	unsigned char *		begin;
	unsigned char *		end;
	unsigned char *		lim;

 	 mem_block(int len) { begin = (unsigned char *) malloc(len); end = begin; lim = end + len; }
	~mem_block() { free(begin); }

	int len() { return end - begin; }
	void *	accum_mem(void * mem, int len)
	{
		if((lim - end) < len)	Assert(!"Out of mem");
		memcpy(end,mem,len);
		void * p = end;
		end += len;
		return p;
	}

	template <class T>
	T *		accum(T v) { return (T *) accum_mem(&v,sizeof(T)); }
	
	void align(int gran, unsigned char fill)
	{
		while(len() % gran != 0)
			accum(fill);
	}
	
};

int accum_str_hi(vector<string>& strs, const string& ns)
{
	for(int n = 0; n < strs.size(); ++n)
	{
		if(strs[n] == ns) return n;
	}
	strs.push_back(ns);
	return strs.size()-1;
}

int accum_str_lo(vector<string>& strs, const string& ns)
{
	if(ns.empty())
	{
		Assert(!strs.empty());
		Assert(strs[0].empty());
		return 0;
	}
	for(int n = 1; n < 255; ++n)
	{
		if(strs[n] == ns) return n;
	}

	for(int n = 1; n < 255; ++n)
	{
		if(strs[n].empty())
		{
			strs[n] = ns;
			return n;
		}
	}

	Assert(!"Out of low string table space.");
	return 0;
}


//••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••
//••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••
//••••OBJECT COMPILATION••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••
//••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••
//••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••

// The idea here is that our exporters sometimes vomit up fairly substantial amounts of animation, and that's expensive
// on IOS. So this code tells us whether the entire begin/end block is a no-op.  There are two interesting cases:
// 1. If there is an 
bool skip_anim(vector<XObjCmd8>::const_iterator& cur_cmd, vector<XObjCmd8>::const_iterator stop_cmd)
{
	bool hit_state = false;
	int nest = 0;
	for (vector<XObjCmd8>::const_iterator i = cur_cmd; i != stop_cmd; ++i)
	switch(i->cmd) {
	case anim_Begin:
		++nest;
		break;
	case obj8_Tris:
	case obj8_LightNamed:
		return false;
		break;
	case attr_poly_offset:
	case attr_Hard:
	case attr_Hard_Deck:
	case attr_Reset:
	case attr_No_Hard:
	case attr_Tex_Normal:
	case attr_Tex_Cockpit:
	case attr_Shiny_Rat:
	case attr_Light_Level:
	case attr_Light_Level_Reset:
	case attr_Draw_Disable:
	case attr_Draw_Enable:
	case attr_Manip_None:
	case attr_Manip_Drag_2d:
	case attr_Manip_Drag_Axis:
	case attr_Manip_Command:
	case attr_Manip_Command_Axis:
	case attr_Manip_Noop:
	case attr_Manip_Push:
	case attr_Manip_Radio:
	case attr_Manip_Toggle:
	case attr_Manip_Delta:
	case attr_Manip_Wrap:
	case attr_Manip_Drag_Axis_Pix:
	case attr_Manip_Command_Knob:
	case attr_Manip_Command_Switch_Up_Down:
	case attr_Manip_Command_Switch_Left_Right:
	case attr_Manip_Axis_Knob:
	case attr_Manip_Axis_Switch_Up_Down:
	case attr_Manip_Axis_Switch_Left_Right:
	case attr_Manip_Command_Knob2:
	case attr_Manip_Command_Switch_Up_Down2:
	case attr_Manip_Command_Switch_Left_Right2:
		hit_state = true;
		break;
	case anim_End:
		--nest;
		if(nest == 0)
		{
			if(hit_state)
				Assert(!"Object has state inside an animation with no geometry or lights.");
			cur_cmd = i;
			return true;
		}
		break;
	}
	Assert(!"Error: ran off the end of an animation group with no clear decision to skip or do the group.");
	return false;
}

int manip_translate(int cmd)
{
	switch(cmd) {
	case attr_Manip_Drag_2d:					return manip_axis_2d;
	case attr_Manip_Drag_Axis:					return manip_axis;
	case attr_Manip_Command:					return manip_cmd;
	case attr_Manip_Command_Axis:				return manip_cmd_axis;
	case attr_Manip_Noop:						return manip_noop;
	case attr_Manip_Push:						return manip_push;
	case attr_Manip_Radio:						return manip_radio;
	case attr_Manip_Toggle:						return manip_toggle;
	case attr_Manip_Delta:						return manip_delta;
	case attr_Manip_Wrap:						return manip_wrap;
	case attr_Manip_Drag_Axis_Pix:				return manip_axis_pix;
	case attr_Manip_Command_Knob:				return manip_cmd_knob;
	case attr_Manip_Command_Switch_Up_Down:		return manip_cmd_switch_ud;
	case attr_Manip_Command_Switch_Left_Right:	return manip_cmd_switch_lr;
	case attr_Manip_Axis_Knob:					return manip_axis_knob;
	case attr_Manip_Axis_Switch_Up_Down:		return manip_axis_swich_ud; 
	case attr_Manip_Axis_Switch_Left_Right:		return manip_axis_switch_lr;
	case attr_Manip_Command_Knob2:				return manip_cmd_knob;
	case attr_Manip_Command_Switch_Up_Down2:	return manip_cmd_switch_ud;
	case attr_Manip_Command_Switch_Left_Right2:	return manip_cmd_switch_lr;
	}
	
	Assert(!"Internal error -bad manip cmd for translation");
	return manip_noop;
	
}

//int light_from_name(const char * name)
//{
//	int n = 0;
//	while(k_light_info[n].name)
//	{
//		if(strcmp(k_light_info[n].name,name)==0) return n;
//		++n;
//	}
//	printf("ERROR: unknown light %s\n", name);	
//	exit(1);
//}

static void make_res_path(string& path)
{
	path.erase(path.end()-4,path.end());
	path += ".pvr";
//	string::size_type p = path.find_last_of(":/\\");
//	if(p != path.npos) path.erase(0,p+1);
}


bool	XObjWriteEmbedded(const char * inFile, const XObj8& inObj)
{
	// find scale from points
	vector<string>			str;
	str.resize(256,string());

	//••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••
	// SCALING CALCS
	//••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••

	float max_c = 1.0;
	float max_s = 0.1;
	float max_t = 0.1;

	float cur_shiny_rat = shiny_rat_undef;

	for(int n = 0; n < inObj.geo_tri.count(); ++n)
	{
		const float * xyz_st = inObj.geo_tri.get(n);
		max_c = max(max_c,fabsf(xyz_st[0]));
		max_c = max(max_c,fabsf(xyz_st[1]));
		max_c = max(max_c,fabsf(xyz_st[2]));
		max_s = max(max_s,fabsf(xyz_st[6]));
		max_t = max(max_t,fabsf(xyz_st[7]));
	}

	for(int n = 0; n < inObj.geo_lines.count(); ++n)
	{
		const float * xyz_rgb = inObj.geo_lines.get(n);
		max_c = max(max_c,fabsf(xyz_rgb[0]));
		max_c = max(max_c,fabsf(xyz_rgb[1]));
		max_c = max(max_c,fabsf(xyz_rgb[2]));
	}

	for(vector<XObjLOD8>::const_iterator L = inObj.lods.begin(); L != inObj.lods.end(); ++L)
	for(vector<XObjCmd8>::const_iterator C = L->cmds.begin(); C != L->cmds.end(); ++C)
	if(C->cmd == obj8_LightNamed)
	{
		float xyz[3] = { C->params[0],C->params[1],C->params[2] };
		max_c = max(max_c,fabsf(xyz[0]));
		max_c = max(max_c,fabsf(xyz[1]));
		max_c = max(max_c,fabsf(xyz[2]));
	}

	max_s = ceil(max_s);
	max_t = ceil(max_t);
	
	float scale_up_vert = 32766.0 / max_c;	// off by one to make sure round-up doesn't exceed max!
	float scale_up_tex_s = fltmin2(4096.0f, 65534.0 / max_s);
	float scale_up_tex_t = fltmin2(4096.0f, 65534.0 / max_t);
	float scale_up_nrm = 16384.0;

	//••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••
	// MAIN PROPERTIES
	//••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••

	string tex_day(inObj.texture);
	string tex_lit(inObj.texture_lit);
	string tex_nrm(inObj.texture_nrm);
	if(!tex_day.empty()) make_res_path(tex_day);
	if(!tex_lit.empty()) make_res_path(tex_lit);
	if(!tex_nrm.empty()) make_res_path(tex_nrm);

	embed_props_t	embed_props;
	embed_props.layer_group = 1950;
	embed_props.ref_count = 0;
	embed_props.tex_day = accum_str_hi(str,tex_day);
	if(inObj.texture_lit.empty())
		embed_props.tex_lit = 0;
	else
		{embed_props.tex_lit = accum_str_hi(str,tex_lit);}
	if(inObj.texture_nrm.empty())
		embed_props.tex_nrm = 0;
	else
		{embed_props.tex_nrm = accum_str_hi(str,tex_nrm);}

	if(!inObj.particle_system.empty())
		embed_props.particle_system = accum_str_hi(str,inObj.particle_system);
	else
		embed_props.particle_system = 0;
	embed_props.emitter_count = 0;
	embed_props.use_metalness = inObj.use_metalness;

	embed_props.cull_xyzr[0] =
	embed_props.cull_xyzr[1] =
	embed_props.cull_xyzr[2] =
	embed_props.cull_xyzr[3] = 0;

	float min_xyz[3], max_xyz[3];

	if(inObj.geo_tri.count())
	{
		bounding_sphere(inObj.geo_tri.get(0),inObj.geo_tri.count(),8,min_xyz,max_xyz,embed_props.cull_xyzr);
	}
	else
	{
		embed_props.cull_xyzr[0] = embed_props.cull_xyzr[1] = embed_props.cull_xyzr[2] = 0.0f;
		embed_props.cull_xyzr[3] = -1.0f;
	}
	
	if(inObj.geo_lines.count() > 0)
	{
		float minp[3], maxp[3];
		float	rgb_bounds[4];
		bounding_sphere(inObj.geo_lines.get(0),inObj.geo_lines.count(),6,minp,maxp,rgb_bounds);
		for(int n = 0; n < 3; ++n)
		{
			min_xyz[n] = fltmin2(min_xyz[n],minp[n]);
			max_xyz[n] = fltmax2(max_xyz[n],maxp[n]);
		}
		grow_sphere(embed_props.cull_xyzr,rgb_bounds);
	}

	for(vector<XObjLOD8>::const_iterator L = inObj.lods.begin(); L != inObj.lods.end(); ++L)
	for(vector<XObjCmd8>::const_iterator C = L->cmds.begin(); C != L->cmds.end(); ++C)
	if(C->cmd == obj8_LightNamed)
	{
		float xyzr[4] = { C->params[0],C->params[1],C->params[2], 0.0 };
		grow_sphere(embed_props.cull_xyzr,xyzr);
	}

	embed_props.max_lod = inObj.lods.back().lod_far;

	if(embed_props.max_lod <= 0)
	{
		float diff_x=max_xyz[0]-min_xyz[0];
		float diff_y=max_xyz[1]-min_xyz[1];
		float diff_z=max_xyz[2]-min_xyz[2];

		if (diff_x <= 0 && diff_y <= 0 && diff_z <= 0)
		{
			embed_props.max_lod = 0;
		} else {

			// From each side, what's the smallest dimension...this is the one that will disappear and
			// is used to calculate the lesser radius.
			float lesser_front =fltmin2(diff_y, diff_z);
			float lesser_top   =fltmin2(diff_x, diff_z);///3.0;	// Reduce apparent size from top for buildings, etc.
			float lesser_side  =fltmin2(diff_x, diff_y);

			// Take the biggest of the lesser radii, that's the one we need to worry about.
			float radius=0.5*fltmax3(lesser_front,lesser_top,lesser_side);
			float tan_semi_width=tan(45.0*0.5*DEG2RAD);

			// BEN SAYS: we used to have the current FOV put in here but this is WRONG.  Remember that objs with the LOD attribute
			// contain a hard-coded LOD - that LOD dist doesn't change with FOV.  So the renderer has to compensate, and that would
			// happen in 1_terrain.  So we should generate a DEFALT LOD based on a "typical" 45 degree FOV here, not use the ren
			// settings.

			// What's this calc?  Well, at a 90 degree FOV, at 512 meters (half a screen width) from the object,
			// 1 meter is equivalent to 1 pixel. So at "radius" times that distance, the whole object is one pixel.
			// We divide by the tangent of the FOV to do this for any FOV.
			float LOD_dis=480*0.5*radius/tan_semi_width*1.5;	// throw on a 50% fudge-factor there... i can see things popping if i dont.. half a pixels still aliases between pixels
//			if (!new_obj->lites.empty())
//				LOD_dis = fltmax2(LOD_dis, 16000);
				embed_props.max_lod=LOD_dis;
		}
	}
	
//	printf("Naive cull sphere: %f,%f,%f (%f)\n", 
//		embed_props.cull_xyzr[0],
//		embed_props.cull_xyzr[1],
//		embed_props.cull_xyzr[2],
//		embed_props.cull_xyzr[3]);
	embed_props.cull_xyzr[3] = -1.0f;	// Reset cull - we will rebuild it.

	if(scale_up_tex_s < 1024.0f || scale_up_tex_t  < 1024.0f)
		printf("UV scale: %f, %f\n", scale_up_tex_s,scale_up_tex_t);
	embed_props.scale_vert= 1.0 / scale_up_vert;
	embed_props.scale_tex[0] = 1.0 / scale_up_tex_s;
	embed_props.scale_tex[1] = 1.0 / scale_up_tex_t;
	embed_props.hard_verts = 0;
	embed_props.vbo_geo = 0;
	embed_props.vbo_idx = 0;
//	embed_props.light_info = NULL;
	embed_props.dref_info = NULL;

	//••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••
	// BUILD VBOS
	//••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••

	// build index list
	vector<unsigned short>	idx;
	for(vector<int>::const_iterator I = inObj.indices.begin(); I != inObj.indices.end(); ++I)
	{
		Assert(*I <= 65535);
		idx.push_back(*I);
	}

	// build geo list
	vector<short>			geo_short;
	vector<float>			geo_float;

	for(int n = 0; n < inObj.geo_tri.count(); ++n)
	{
		// WTF is this padding? Here's the deal:
		// 1. The PowerVR SGX chipset requires 4-byte alignment for every _type_ of input data.  So the start of the "normal" section of your 
		// VBO must be 4-byte aligned.  If we pack shorts in XYZNNNST format like we would on desktop, the normal is only 2-byte aligned; the
		// GL must unpack and "fix" our VBO - this is about a 3-x hit in perf...not only do we burn CPU time, but the sw unpack doesn't retain
		// indices because it can't handle wide "spans" between indices.  (That is, they peephole unpack.)
		//
		// So....first, we have to pad  to make 4-byte alignment. 
		// Now the PowerVB MBX requires the stupid CPU to spoon-feed it.  So believe it or not, 4-component coords are better than 3!  Since the
		// GPU eats vec4, if we feed it vec3 the spoon-feeder is going to need to pad 1.0 per unit.  If we say vec4 it gets to run in its most
		// efficient mode.  Note that this depends on the developer correctly recognizing the 4-component case as a hot path.
		const float * xyz_st = inObj.geo_tri.get(n);
		geo_short.push_back(xyz_st[0] * scale_up_vert);
		geo_short.push_back(xyz_st[1] * scale_up_vert);
		geo_short.push_back(xyz_st[2] * scale_up_vert);
		geo_short.push_back(1.0);
		geo_short.push_back(xyz_st[3] * scale_up_nrm);
		geo_short.push_back(xyz_st[4] * scale_up_nrm);
		geo_short.push_back(xyz_st[5] * scale_up_nrm);
		geo_short.push_back(0.0);
		geo_short.push_back(xyz_st[6] * scale_up_tex_s);
		geo_short.push_back(xyz_st[7] * scale_up_tex_t);
	}

	//••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••
	// TRANSLATE COMMAND LIST
	//••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••

	unsigned char * last_lod = 0;
	unsigned int * patch_lod = NULL;
	mem_block	cmds(1024*1024*4);	// 4 MB - if we have more than that on iphone, I will stab myself!

	int hard_start = 0;
	int hard_stop = 0;
	int is_hard = false;
	int is_draw_disable = false;	
	int has_poly_os = 0;
	int nest = 0;
	list<XObjCmd8>	detached_lites;
	

	{
		for(vector<XObjLOD8>::const_iterator L = inObj.lods.begin(); L != inObj.lods.end(); ++L)
		{				
			is_hard = false;
			unsigned char * this_lod = cmds.accum<unsigned char>(attr_lod);

			if(patch_lod && last_lod)
				*patch_lod = this_lod - last_lod;
			last_lod = this_lod;

			cmds.accum<float>(L->lod_near);
			if(L->lod_far <= 0)
			{
				Assert(inObj.lods.size() == 1);
				cmds.accum<float>(embed_props.max_lod);
			}
			else
				cmds.accum<float>(L->lod_far);
			cmds.accum<unsigned short>(0);				// We are using next-gen LOD command - 16-bit flag (the old bits, 32-bit jump
			patch_lod = cmds.accum<unsigned int>(0);

			const XObjManip8 * cur_manip = NULL;
			int cur_manip_cmd = 0;

			list<vector<anim_xform> >	anim_stack;
			anim_stack.push_back(vector<anim_xform>());

			for(vector<XObjCmd8>::const_iterator C = L->cmds.begin(); C != L->cmds.end(); ++C)
			switch(C->cmd) {
			case attr_Reset:
				if(has_poly_os)
				{
					cmds.accum<unsigned char>(attr_poly_offset);
					cmds.accum<unsigned char>(0);
					has_poly_os = 0;
				}
				break;
			case attr_Offset:
				if(C->params[0] != has_poly_os)
				{
					cmds.accum<unsigned char>(attr_poly_offset);
					Assert(C->params[0] >= 0);
					cmds.accum<unsigned char>(C->params[0]);
					has_poly_os = C->params[0];
				}
				break;
			case obj8_Tris:
				{
					if(is_draw_disable && cur_manip == NULL)
					{
						AssertPrintf("Only use draw-disable for manipulators");
					}
					if(!is_draw_disable && C->idx_count > 0)	// weird 3DS exporter leaves this in.
					{
						if(cur_shiny_rat == shiny_rat_undef)
							cur_shiny_rat = shiny_rat_default; 
						cmds.accum<unsigned char>(draw_tris);
						cmds.accum<unsigned short>(C->idx_offset);
						cmds.accum<unsigned short>(C->idx_count);
						
						vector<float>	pts;
						for(int i = 0; i < C->idx_count; ++i)
						{
							int idx = inObj.indices[i+C->idx_offset];
							const float * p = inObj.geo_tri.get(idx);
							pts.insert(pts.end(),p,p+3);
						}
						float cs[4], minxyz[3], maxxyz[3];
						bounding_sphere_random(&pts[0], pts.size()/3, 3, minxyz, maxxyz, cs);
						grow_sphere_with_list(embed_props.cull_xyzr, cs, anim_stack.back());						
					}
					Assert(C->idx_offset <= 65535);
					Assert(C->idx_count <= 65535);
					if(is_hard)
					{
						int s = C->idx_offset;
						int e = C->idx_offset + C->idx_count;
						if(s > hard_stop || e < hard_start)
							Assert(!"ERROR: hard start and stop regions are discontiguous!");
						hard_start = min(s,hard_start);
						hard_stop = max(e,hard_stop);
					}
					if(cur_manip)
					{
						cmds.align(4, cmd_nop);
						cmds.accum<unsigned char>(attr_manip2);
						cmds.accum<unsigned char>(cur_manip_cmd);
						if(C->idx_count > 65535)	AssertPrintf("ERROR: manipulator TRI commands can only have 65535 verts at most per manip.");
						cmds.accum<unsigned short>(C->idx_count);	// probably overkill but i don't feel like rewriting binary IO in 2 years.						
						cmds.accum<float>(cur_manip->axis[0]);
						cmds.accum<float>(cur_manip->axis[1]);
						cmds.accum<float>(cur_manip->axis[2]);
						cmds.accum<float>(cur_manip->v1_min);
						cmds.accum<float>(cur_manip->v1_max);
						cmds.accum<float>(cur_manip->v2_min);
						cmds.accum<float>(cur_manip->v2_max);
						cmds.accum<float>(cur_manip->mouse_wheel_delta);
						cmds.accum<unsigned short>(accum_str_hi(str,cur_manip->dataref1));
						Assert(str.size() < 65535);
						cmds.accum<unsigned short>(accum_str_hi(str,cur_manip->dataref2));
						Assert(str.size() < 65535);
						cmds.accum<unsigned short>(accum_str_hi(str,cur_manip->tooltip));
						Assert(str.size() < 65535);
						cmds.accum<unsigned short>(0);		// 4-byte padding
						for(int i = 0; i < C->idx_count; ++i)
						{
							int vidx = C->idx_offset + i;
							const float * my_vert = inObj.geo_tri.get(inObj.indices[vidx]);
							cmds.accum<float>(my_vert[0] * scale_up_vert);
							cmds.accum<float>(my_vert[1] * scale_up_vert);
							cmds.accum<float>(my_vert[2] * scale_up_vert);							
						}
					}
				}
				break;
			case attr_Hard:
				is_hard=true;
				break;
			case attr_Hard_Deck:
				is_hard=true;
				break;
			case attr_No_Hard:
				is_hard=false;
				break;
			case anim_Begin:
				if(!skip_anim(C,L->cmds.end()))
				{
					anim_stack.push_back(anim_stack.back());
					cmds.accum<unsigned char>(attr_begin);
					++nest;
				}
				break;
			case anim_End:
				anim_stack.pop_back();
				cmds.accum<unsigned char>(attr_end);
				--nest;
				break;
			case anim_Rotate:
				if(inObj.animation[C->idx_offset].loop != 0.0f)
				{
					cmds.accum<unsigned char>(attr_loop);
					cmds.accum<float>(inObj.animation[C->idx_offset].loop);
				}			
				anim_stack.back().push_back(anim_xform(inObj.animation[C->idx_offset].axis,inObj.animation[C->idx_offset].keyframes));
				cmds.accum<unsigned char>(attr_rotate);
				cmds.accum<float>(inObj.animation[C->idx_offset].axis[0]);
				cmds.accum<float>(inObj.animation[C->idx_offset].axis[1]);
				cmds.accum<float>(inObj.animation[C->idx_offset].axis[2]);
				cmds.accum<unsigned char>(inObj.animation[C->idx_offset].keyframes.size());
				for(vector<XObjKey>::const_iterator i = inObj.animation[C->idx_offset].keyframes.begin(); i != inObj.animation[C->idx_offset].keyframes.end(); ++i)
				{
					cmds.accum<float>(i->key);
					cmds.accum<float>(i->v[0]);
				}
				cmds.accum<unsigned char>(accum_str_lo(str,inObj.animation[C->idx_offset].dataref));
				break;
			case anim_Translate:
				anim_stack.back().push_back(anim_xform(inObj.animation[C->idx_offset].keyframes));
			
				if (inObj.animation[C->idx_offset].keyframes.size() == 2 &&
					inObj.animation[C->idx_offset].keyframes[0].v[0] == inObj.animation[C->idx_offset].keyframes[1].v[0] &&
					inObj.animation[C->idx_offset].keyframes[0].v[1] == inObj.animation[C->idx_offset].keyframes[1].v[1] &&
					inObj.animation[C->idx_offset].keyframes[0].v[2] == inObj.animation[C->idx_offset].keyframes[1].v[2])
				{
					cmds.accum<unsigned char>(attr_translate_static);
					cmds.accum<float>(inObj.animation[C->idx_offset].keyframes[0].v[0] * scale_up_vert);
					cmds.accum<float>(inObj.animation[C->idx_offset].keyframes[0].v[1] * scale_up_vert);
					cmds.accum<float>(inObj.animation[C->idx_offset].keyframes[0].v[2] * scale_up_vert);
				} else {
					if(inObj.animation[C->idx_offset].loop != 0.0f)
					{
						cmds.accum<unsigned char>(attr_loop);
						cmds.accum<float>(inObj.animation[C->idx_offset].loop);
					}
					cmds.accum<unsigned char>(attr_translate);
					cmds.accum<unsigned char>(inObj.animation[C->idx_offset].keyframes.size());
					for(vector<XObjKey>::const_iterator i = inObj.animation[C->idx_offset].keyframes.begin(); i != inObj.animation[C->idx_offset].keyframes.end(); ++i)
					{
						cmds.accum<float>(i->key);
						cmds.accum<float>(i->v[0] * scale_up_vert);
						cmds.accum<float>(i->v[1] * scale_up_vert);
						cmds.accum<float>(i->v[2] * scale_up_vert);
					}
					cmds.accum<unsigned char>(accum_str_lo(str,inObj.animation[C->idx_offset].dataref));
				}
				break;
			case anim_Hide:
				Assert(inObj.animation[C->idx_offset].keyframes.size() == 2);
				if(inObj.animation[C->idx_offset].loop != 0.0f)
				{
					cmds.accum<unsigned char>(attr_loop);
					cmds.accum<float>(inObj.animation[C->idx_offset].loop);
				}
				cmds.accum<unsigned char>(attr_hide);
				cmds.accum<float>(inObj.animation[C->idx_offset].keyframes[0].key);
				cmds.accum<float>(inObj.animation[C->idx_offset].keyframes[1].key);
				cmds.accum<unsigned char>(accum_str_lo(str,inObj.animation[C->idx_offset].dataref));
				break;
			case anim_Show:
				Assert(inObj.animation[C->idx_offset].keyframes.size() == 2);
				if(inObj.animation[C->idx_offset].loop != 0.0f)
				{
					cmds.accum<unsigned char>(attr_loop);
					cmds.accum<float>(inObj.animation[C->idx_offset].loop);
				}			
				cmds.accum<unsigned char>(attr_show);
				cmds.accum<float>(inObj.animation[C->idx_offset].keyframes[0].key);
				cmds.accum<float>(inObj.animation[C->idx_offset].keyframes[1].key);
				cmds.accum<unsigned char>(accum_str_lo(str,inObj.animation[C->idx_offset].dataref));
				break;
			case obj8_LightNamed:
				if(is_draw_disable)
					AssertPrintf("Do not draw-disable lights!\n");
				if(nest)
				{
					cmds.align(4,cmd_nop);
					cmds.accum<unsigned char>(attr_light_param);
					cmds.accum<unsigned char>(C->idx_count);
					cmds.accum<unsigned short>(0);
					cmds.accum<unsigned int>(accum_str_hi(str,C->name.c_str()));
					cmds.accum<float>(C->params[0] * scale_up_vert);
					cmds.accum<float>(C->params[1] * scale_up_vert);
					cmds.accum<float>(C->params[2] * scale_up_vert);
					for(int i = 0; i < C->idx_count; ++i)
						cmds.accum<float>(C->params[3+i]);
					
					float cs[4] = { C->params[0], C->params[1], C->params[2], 0.5f };
					grow_sphere_with_list(embed_props.cull_xyzr, cs, anim_stack.back());												
				}
				else
				{
					if(find_if(detached_lites.begin(),detached_lites.end(),match_lite(*C)) == detached_lites.end())
						detached_lites.push_back(*C);

					float cs[4] = { C->params[0], C->params[1], C->params[2], 0.5f };
					grow_sphere_with_list(embed_props.cull_xyzr, cs, anim_stack.back());												
				}
				break;
			case attr_Layer_Group:
				if(C->name == "terrain"						 )	embed_props.layer_group = 5 + C->params[0];
				if(C->name == "beaches"						 )	embed_props.layer_group = 25 + C->params[0];
				if(C->name == "shoulders" && C->params[0] < 0)	embed_props.layer_group = 70 + C->params[0];
				if(C->name == "shoulders" && C->params[0] >=0)	embed_props.layer_group = 90 + C->params[0];
				if(C->name == "taxiways" && C->params[0] < 0 )	embed_props.layer_group = 100 + C->params[0];
				if(C->name == "taxiways" && C->params[0] >=0 )	embed_props.layer_group = 1000 + C->params[0];
				if(C->name == "runways" && C->params[0] < 0  )	embed_props.layer_group = 1100 + C->params[0];
				if(C->name == "runways" && C->params[0] >=0	 )	embed_props.layer_group = 1900 + C->params[0];
				if(C->name == "markings"					 )	embed_props.layer_group = 1920 + C->params[0];
				if(C->name == "airports" && C->params[0] < 0 )	embed_props.layer_group = 60 + C->params[0];
				if(C->name == "airports" && C->params[0] >=0 )	embed_props.layer_group = 1930 + C->params[0];
				if(C->name == "roads"						 )	embed_props.layer_group = 1940 + C->params[0];
				if(C->name == "objects"						 )	embed_props.layer_group = 1950 + C->params[0];
				if(C->name == "light_objects"				 )	embed_props.layer_group = 1955 + C->params[0];
				if(C->name == "cars"						 )	embed_props.layer_group = 1960 + C->params[0];
				break;
			case attr_Cull:
			case attr_NoCull:
				Assert(!"No 2-sided geometrey please.");
				break;
			case obj_Smoke_Black:
			case obj_Smoke_White:
				Assert(!"Smoke puffs not supported.\n");
				break;
			case obj8_Lights:
				Assert(!"Old RGB lights are not supported.\n");
				break;
			case attr_Tex_Normal:
				cmds.accum<unsigned char>(attr_no_cockpit_tex);
				cur_manip = NULL;		// turning OFF cockpit texture clears manip too since it MIGHT be panel manip (e.g. for compatibility)
				break;
			case attr_Tex_Cockpit:
				cmds.accum<unsigned char>(attr_cockpit_tex);
				cur_manip = NULL;	/* THIS WOULD BE CLICKABLE PANEL */
				break;
				break;
			case attr_No_Blend:
			case attr_Blend:
				Assert(!"Blend control is not suppported.\n");
				break;
			case obj8_LightCustom:			// all in name??  param is pos?
				Assert(!"No custom lights.\n");
				break;
			case attr_Tex_Cockpit_Subregion:
				if(C->params[0] != 0)
					Assert(!"Cockpit texture regions other than zero are not supported..\n");
				cmds.accum<unsigned char>(attr_cockpit_tex);
				cur_manip = NULL;					/* THIS WOULD BE CLICKABLE PANEL */
				break;
			case attr_Shade_Flat:
			case attr_Shade_Smooth:
				Assert(!"Flat shading is not supported.\n");
				break;
			case attr_Ambient_RGB:
			case attr_Diffuse_RGB:
			case attr_Specular_RGB:
			case attr_Emission_RGB:
				Assert(!"RGB Lighting materials are not supported.\n");
				break;			
			case attr_Shiny_Rat:
				if(C->params[0] < 0.0)
					Assert(!"ATTR_shiny_rat must not have a NEGATIVE value!!!");
				if(cur_shiny_rat == shiny_rat_undef)
				{
					cur_shiny_rat = fltlim(C->params[0],0.0f,1.0f);
					if(cur_shiny_rat != 0.0 && cur_shiny_rat != 1.0)
						Assert(!"Your shiny ratio should either be 0.0 or 1.0.");
				}
				else if(cur_shiny_rat == shiny_rat_default)
				{
					Assert(!"You cannot use ATTR_shiny_rat in the middle of your objects!");
				}
				else if(cur_shiny_rat != C->params[0])
				{
					Assert(!"You cannot change the shiny ratio mid-object.  Pick one shininess for the ENTIRE object.");
				}
				break;
			case attr_No_Depth:
			case attr_Depth:
				Assert(!"Depth-write disable is not supported.\n");
				break;
			case attr_LOD:
				Assert(!"Unexpected LOD command.\n");
				break;
			case attr_Light_Level:
				cmds.accum<unsigned char>(attr_light_level);
				cmds.accum<float>(C->params[0]);
				cmds.accum<float>(C->params[1]);
				cmds.accum<unsigned char>(accum_str_lo(str,C->name.c_str()));
				break;
			case attr_Light_Level_Reset:
				cmds.accum<unsigned char>(attr_light_level);
				cmds.accum<float>(1.0);
				cmds.accum<float>(1.0);
				cmds.accum<unsigned char>(accum_str_lo(str,"NONE"));
				break;
			case attr_Manip_None:
				cur_manip = NULL;
				break;
			case attr_Manip_Drag_Axis:
				cur_manip_cmd = manip_axis;
				cur_manip = &inObj.manips[C->idx_offset];
				if(cur_manip->axis[0] == 0.0f &&
					cur_manip->axis[1] == 0.0f &&
					cur_manip->axis[2] == 0.0f)
				{
					cur_manip_cmd = manip_noop;
				}
				break;
			case attr_Manip_Drag_2d:
			case attr_Manip_Command:
			case attr_Manip_Command_Axis:
			case attr_Manip_Noop:
			case attr_Manip_Push:
			case attr_Manip_Radio:
			case attr_Manip_Toggle:
			case attr_Manip_Delta:
			case attr_Manip_Wrap:
			case attr_Manip_Drag_Axis_Pix:
			case attr_Manip_Command_Knob:
			case attr_Manip_Command_Switch_Up_Down:
			case attr_Manip_Command_Switch_Left_Right:
			case attr_Manip_Axis_Knob:
			case attr_Manip_Axis_Switch_Up_Down:
			case attr_Manip_Axis_Switch_Left_Right:
			case attr_Manip_Command_Knob2:
			case attr_Manip_Command_Switch_Up_Down2:
			case attr_Manip_Command_Switch_Left_Right2:
				cur_manip_cmd = manip_translate(C->cmd);
				cur_manip = &inObj.manips[C->idx_offset];
				break;
			case attr_Draw_Disable				:
				is_draw_disable = 1;
				break;
			case attr_Draw_Enable				:
				is_draw_disable = 0;
				break;
			case attr_Emitter:
				{
					++embed_props.emitter_count;
					const XObjEmitter8 * e = &inObj.emitters[C->idx_offset];
					cmds.accum<unsigned char>(attr_emitter);
					cmds.accum<unsigned short>(accum_str_hi(str,e->name));
					cmds.accum<unsigned short>(accum_str_hi(str,e->dataref));
					cmds.accum<float>(e->x * scale_up_vert);
					cmds.accum<float>(e->y * scale_up_vert);
					cmds.accum<float>(e->z * scale_up_vert);
					cmds.accum<float>(e->psi);
					cmds.accum<float>(e->the);
					cmds.accum<float>(e->phi);
					cmds.accum<float>(e->v_min);
					cmds.accum<float>(e->v_max);
				}
				break;
//			case attr_NormalMetalness:
//				{
//					cmds.accum<unsigned char>(attr_metalness);
//					break;
//				}
			default:
				AssertPrintf("Unsupported command: %s\n", gCmds[FindIndexForCmd(C->cmd)].name);
			}

			if(has_poly_os)
			{
				cmds.accum<unsigned char>(attr_poly_offset);
				cmds.accum<unsigned char>(0);
				has_poly_os = 0;
			}
			Assert(nest == 0);
		}

//	printf("Careful cull sphere: %f,%f,%f (%f)\n", 
//		embed_props.cull_xyzr[0],
//		embed_props.cull_xyzr[1],
//		embed_props.cull_xyzr[2],
//		embed_props.cull_xyzr[3]);

		embed_props.hard_verts = hard_stop;

		// We are going to write a "stop" cmd after the last LOD.  If we are off the end of the LOD,
		// jumping to the stop cmd tells the LOD-finder we're done.  If we are executing the LOD, the
		// stop command is a "break", just like a "next LOD" cmd.

		unsigned char * end_cmd = cmds.accum<unsigned char>(cmd_stop);
		if(patch_lod && last_lod)
			*patch_lod = end_cmd - last_lod;
			
		// Clear out remnants of last LOD so that if we go for the lighting pass, we don't link to the previous cmds.
		patch_lod = NULL;
		last_lod = NULL;
	}
	
	{
		cmds.align(4,0);	// pad the light table up 4 bytes...since the light table is a new command, we use our brain and give it good internal alignment.
		embed_props.light_off = cmds.len();

		for(list<XObjCmd8>::iterator C = detached_lites.begin(); C != detached_lites.end(); ++C)
		{
			cmds.accum<unsigned char>(attr_light_param);
			cmds.accum<unsigned char>(C->idx_count);
			cmds.accum<unsigned short>(0);
			cmds.accum<unsigned int>(accum_str_hi(str,C->name.c_str()));
			cmds.accum<float>(C->params[0] * scale_up_vert);
			cmds.accum<float>(C->params[1] * scale_up_vert);
			cmds.accum<float>(C->params[2] * scale_up_vert);
			for(int i = 0; i < C->idx_count; ++i)
				cmds.accum<float>(C->params[3+i]);
		}
		
		cmds.accum<unsigned char>(cmd_stop);
	}
		
	
	
	//••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••
	// WRITE OUT
	//••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••

	// Master header calc
	master_header_t	mheader;
	mheader.magic[0] = 'O';
	mheader.magic[1] = 'B';
	mheader.magic[2] = 'J';
	mheader.magic[3] = '3';		// Rewrite magic with vers number, e = original, 2 = newer 10-unit stride revision.
								// 3 = extensible headers.

	mheader.prp_off = sizeof(mheader);
	mheader.prp_len = sizeof(embed_props_t);
	mheader.cmd_off = mheader.prp_off + mheader.prp_len;
	mheader.cmd_len = cmds.len();
	mheader.geo_off = mheader.cmd_off + mheader.cmd_len;
	mheader.geo_len = geo_short.size() * sizeof(geo_short[0]);
	mheader.idx_off = mheader.geo_off + mheader.geo_len;
	mheader.idx_len = idx.size() * 2;
	mheader.str_off = mheader.idx_off + mheader.idx_len;
	mheader.str_len = str.size();

	for(vector<string>::iterator s = str.begin(); s != str.end(); ++s)
		mheader.str_len += s->length();


	// Write out file!

	FILE * fi = fopen(inFile, "wb");
	if(fi)
	{
		fwrite(&mheader,1,sizeof(mheader),fi);

		fwrite(&embed_props,1,sizeof(embed_props),fi);

		fwrite(cmds.begin, 1, cmds.len(), fi);

		fwrite(&*geo_short.begin(),sizeof(geo_short[0]),geo_short.size(),fi);

		fwrite(&*idx.begin(),2,idx.size(),fi);

		for(vector<string>::iterator s = str.begin(); s != str.end(); ++s)
			fwrite(s->c_str(),1,s->length()+1,fi);

		fclose(fi);
		return 1;
	}
	return 0;
}


void	XObjFixKeyframes(XObj8& inObj8)
{
	// WTF?  well...
	// First, X-Plane auto-resorts key frames on OBJ load...mobile does not, so we sort here.
	// Second, X-Plane copes with non-unique KF input but we don't trust it, so we 'unique' the
	// whole table.  
	//
	// If this removes items in a 3+ KF table, we flag this as an illegal object - that's usually bad
	// Blender input.  But if we had 2 KFs and now we have 1, that's just a typical 'static' animation,
	// where both the input and output values are completely fixed -- the two KFs are necessary because
	// that is the syntax of a non-KF table.

	for(vector<XObjAnim8>::iterator a = inObj8.animation.begin(); a != inObj8.animation.end(); ++a)
	{
		if(a->cmd == anim_Show || a->cmd == anim_Hide)
			continue;

		// This code rebuilds the KF table exactly the way X-Plane sees it, including how ambiguous crap is handled.
		// The goal is to handle borked 3DS exports the way desktop does.

		vector<XObjKey>	 fixed_keyframes;
		
		for(vector<XObjKey>::iterator k = a->keyframes.begin(); k != a->keyframes.end(); ++k)
		{
			vector<XObjKey>::iterator i = lower_bound(fixed_keyframes.begin(),fixed_keyframes.end(), *k, sort_by_key());
			if(i != fixed_keyframes.end() && i->key == k->key)
				*i = *k;
			else
				fixed_keyframes.insert(i,*k);
		}
		
		// Zero KF...wtf?!
		Assert(!fixed_keyframes.empty());
		
		// ONE special case: mobile's KF reader CANNOT handle single-kf tables.  Desktop has a special caes "if one KF, return its value"
		// line...we simulate this by duping the one KF.
		if(fixed_keyframes.size() == 1)
			fixed_keyframes.push_back(fixed_keyframes.back());
		
//		printf("======\n");
//		dump(a->keyframes);
//		printf(" - - -\n");
//		dump(fixed_keyframes);
		
		swap(a->keyframes,fixed_keyframes);
		


/*		

		sort(a->keyframes.begin(),a->keyframes.end(),sort_by_key());

		// we're going to do two passes of moving the first key frame of a truly dupe pair 'out' a bit...
		// The preferred 'clamp' is (K0-1->V0), (K0->V0), (K1->V1) etc. but a lot of authors just dupe
		// the first Kf - I think the tools do it.  So catch this case.

		float off = -1.0f;
		for(int i = 0; i < 2; ++i)
		{
			if(a->keyframes.size() > 1)
			{
				// IF the two kf keysa re dupes
				if(a->keyframes[0].eq_key(a->keyframes[1]))
				{
					// if the vals are NOT dupes this is just a fubar table - squawk!
					if(!a->keyframes[0].eq_val(a->keyframes[1]))
					{
						if(off > 0.0)
							Assert(!"Animation has key frames of equal key but not equal value at the end of a KF table.");
						else
							Assert(!"Animation has key frames of equal key but not equal value at the beginning of a KF table.");
					}
					
					a->keyframes[0].key += off;
				}
			}
			
			// stupid trick - reverse the table, do the front again, then reverse again to put things back.
			reverse(a->keyframes.begin(), a->keyframes.end());
			
			off = 1.0f;
		}
		int os = a->keyframes.size();

		a->keyframes.erase(unique(a->keyframes.begin(),a->keyframes.end(), cmp_by_all()),a->keyframes.end());

		if(os != a->keyframes.size() && os > 2)
		{	
			printf("WARNING: REMOVED DUPLICATE INTERNAL KEY FRAME!\n");
		}

		os = a->keyframes.size();

		a->keyframes.erase(unique(a->keyframes.begin(),a->keyframes.end(), cmp_by_key()),a->keyframes.end());
		if(os != a->keyframes.size() && os > 2)
			Assert(!"Bad keyframe table - duplicate values!");
		if(a->keyframes.size() == 1)
			a->keyframes.push_back(a->keyframes.front());
*/			
	}
	
}


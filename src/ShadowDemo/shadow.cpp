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
#if APL
#define GL_SGIX_depth_texture 1
#define GL_SGIX_shadow 1
#define GL_EXT_texture_env_combine 1
#endif

#include <glut.h>
#include <glext.h>
#include <stdio.h>
#include <stdlib.h>
#include <gl.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include "HLCamera.h"
#include "MatrixUtils.h"
#include "TexUtils.h"
#include "MacImageToClipboard.h"

using namespace std;
#include <vector>


#include "XObjReadWrite.h"
#include "ObjUtils.h"
#include "ObjUtilsGL.h"



/*

Consider what it's like to be the sun.  (It is quite hot.)
For every building you shine on, for every pixel
of that building, you can say how far that pixel is from youj.  Some are farther than
others.  At high noon, the tops of buildings are closer than the streets.  As the sunsets,
the western buildings are closer than the eastern ones.

Also, consider what the world looks like from the sun's perspective.  Only certain 
buildings will be visible to you.  For each point that you see, you can say how close
the nearest buildings are.  Walls of buildings that you can't see are shadowed.

Call the first distance D(p), the distance of any point to the sun.  Call the second
distance D(c), the distance of the closest building to the sun.

swFor any given pixel of any building, that pixel is shadowed if D(c) < D(p), in other
words, the closest pixel along that ray from the sun is not the one we're drawing.
That's casue that closest pixel is shadowing us!

This algorithm produces projected shadows from the sun by calculating these two
quantities and comparing them to calculate the shadowed pixels.  In particular, it
does it in hardware which makes it quite zippy.

PROJECTIVE TEXTURING

We can use texture coordinate generation to 'project' a texture.  Basically the same
two matrices (projection, then model view) that we apply to all our coordinates, we
can apply to vertices to get texture coordinate S & Ts.

We combine those matrices with a rescaling matrix (our projection and model view 
matrices give us coords from -1 to 1, but we really want 0 to 1 for S & T) with
X, Y and Z mapped to S, T and R.  Thus our S & T projects onto Our scene.  But since
our matrix can have a rotation, we can project a texture anywhere.

We use this technique to project our depth map (darker = closer) onto all our buildings
from the sun's perspective.  Since we use the same matrices to project the texture
and take the original picture, they line up perfectly.

As a minor note: we must use "eye plane" coordinates to do this.  If we use "object
plane" coordinates, the current model-view matrix is applied to the vertices as
they are emitted.  This isn't really what we want.

THOSE NIFTY SGI EXTENSIONS

SGI provides a great set of extension that allow us to do per-pixel shadow mapping
with ONE TEXTURE UNIT!  This is very exciting because we still need the other three
for the primary texture, night lighting and any cloud shadows.  Anyway, here's how it
works:

 - First we use a depth texture instead of a luminance one.  A depth texture is a one
   channel texture, but it is special in that it may be any number of bits we want.
   This is important because the number of bits in the depth texture is basically a 
   distorted version of our depth buffer.  If we only have 8 bits per pixel our shadows
   will Z fight as badly as if x-plane had an 8 bit depth buffer.
 - The shadow mapping extension is a filter.  Before the texture pixels are output, the
   depth texture pixel value (which is how far away the closest object is for the pixel 
   we're drawing) is compared to the R ("Z") coordinate of the texture coordinates, which 
   is how far away the current object is at the pixel we're drawing.  The resulting 
   comparison is a 0 or 1, 1 if the pixel is not shadowed, 0 if it is.
   
   (What really rules about this technique is that the R (Z) coord of the texture
   is pretty damned close to free...we're projecting vertex modelview XYZ coordinates
   into sun-camera-eye coordinates STR, and we need S&T anyway to fetch the texel.  
   Using the hardware to get R is much nicer than having to use another texture unit.)
   
So we basically bind the shadow map texture to one texture unit, set up projective
texturing on all four coordiantes, and turn on this filter and we're good to go.
We can then use the various combiner functions to input this into the lighting equation.

We end up with a boolean value as our texel.  We can then use a combiner operation like
blend to transition from the primary colour (which is the lit value) to a static colour.
We just feed the ambient levels into the texture environment and there we are. 

We can then modulate this texture with a cloud map texture to provide cloud shadows, 
modulate it with a spotlight texture to produce a spotlight, and then modulate it with
the texture color of the fragment and/or any incoming overlays.

A NOTE ON USING TEXTURE COORDINATE GENERATION TO FIND D(p)

This technique is weirder.  We need the distance from the pixel to the sun at every
pixel.  How can we use the hardware to do this for us?

The answer again is projective texturing.  We first set up our texture matrix
again using the sun's view and projection matrices to set it up as if we're looking
from the sun's perspective.  But now think of what we have...the Z axis runs 
straight to and from the sun (that's where the depth buffer comes from, right)?  So
whatever our Z coordinate is, that's the answer.

It's not quite that simple, we need to turn Z coordinates into a texture value.
So we use a simple/stupid texture lookup table, a 1-d texture that has black on
the left and white on the right.  We index this via Z.  The higher Z, the higher
color we get.  Now we have a texture color for our distance from the sun.

Conveniently, projective texturing is hardware accelerated on modern cards. :-)


REQUIREMENTS:

We must be able to draw our model twice with different projection angles.
The first time we draw, we draw from the sun's perspective, and really all
we care about is the depth component.

The second time we draw, we draw from the camera's perspective and we apply
shadows as we go.

We need a few OGL extensions:

The basic algorithm is:

1. Clear the back buffer.
2. Position the camera where the sun is.
3. Draw the scene.
4. Transfer the depth buffer to a texture.  This texture is our set of D(c)s.
5. Clear the back buffer.
6. Position the camera where the sun is.
7. Draw the scene again.
7a. We use projective texturing to project D(c) into texture unit 0.  Thus for
   each pixel we know, the alpha of texture 0 is D(c).
7b. We use projective texturing and a lookup table to calculate D(p) into texture
   unit 1.  Thus for each pixel we know, the alpha of texture unit 1 is D(p).
7c. We use a fragment program to figure out whether we want diffuse lighting based
   on a comparison of the texture 0 and 1 alphas.
   

   

*/


// This code has been hijacked to do a bump mapping demo; set this to 0
// to ignore this.  BUMP MAPPING AND SHADOW MAPPIGN DO NOT WORK TOGETHER
// in this demo because I let the OGL states munge each other.

#define DO_NORMAL_MAP	0
#define	FORCE_NO_COLOR	0


// We have to find the multi-texture func for Windows.
#if defined(__POWERPC__)
//PFNGLACTIVETEXTUREARBPROC 	GL_ActiveTextureARB = glActiveTextureARB;
//PFNGLMULTITEXCOORD2FARBPROC	GL_MultiTexCoord2fARB = glMultiTexCoord2fARB;
#define GL_ActiveTextureARB   glActiveTextureARB
#define GL_MultiTexCoord2fARB glMultiTexCoord2fARB
#else
PFNGLACTIVETEXTUREARBPROC GL_ActiveTextureARB = NULL;
PFNGLMULTITEXCOORD2FARBPROC	GL_MultiTexCoord2fARB = NULL;
#endif

void glut_print_string(int x, int y, char *string)
{
	int len, i;

	glRasterPos2f(x, y);
	len = (int) strlen(string);
	for (i = 0; i < len; i++) 
	{
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, string[i]);
	}
}


void	copy_to_clipboard(void)
{
	int	width = glutGet(GLUT_WINDOW_WIDTH);
	int	height = glutGet(GLUT_WINDOW_HEIGHT);
	
	unsigned char * buf = (unsigned char *) malloc(width * height * 3);
	if (buf)
	{
		glReadPixels(0,0, width, height, GL_RGB, GL_UNSIGNED_BYTE, buf);
#if APL				
		MacImageToClipboard(buf, width, height);
#endif		
		free(buf);
	}
}




// This routine forms a naive bump map from a height map.  The input map
// is a set of heights from 0 to 255, the output is a set of RGB normals,
// where the X vector is in R, Y vector is in G, and Z vector is in B.  The
// height map is taken as if S = X, T = Z, and Y is the height.
void	create_normal_map(
				unsigned char *		inMap,
				unsigned char *		outMap,
				int					width,
				int					height,
				float				inScale)
{
	for (int x = 0; x < width; ++x)
	for (int y = 0; y < height; ++y)
	{
		float	dx = 0, dy = 1.0,  dz = 0;
		if (x > 0)
			dx += 0.5 * inScale * (float) inMap[(x-1)+(y  )*width];
		if (x < (width-1))
			dx -= 0.5 * inScale * (float) inMap[(x+1)+(y  )*width];

		if (y > 0)
			dz += 0.5 * inScale * (float) inMap[(x  )+(y-1)*width];
		if (y < (height-1))
			dz -= 0.5 * inScale * (float) inMap[(x  )+(y+1)*width];

		float	l = sqrt(dx * dx + dy * dy + dz * dz);
		l = 1.0 / l;
		dx *= l;
		dy *= l;
		dz *= l;
		outMap[x * 3 + y * 3 * width + 0] = (dx * 0.5 + 0.5) * 255.0;
		outMap[x * 3 + y * 3 * width + 1] = (dy * 0.5 + 0.5) * 255.0;
		outMap[x * 3 + y * 3 * width + 2] = (dz * 0.5 + 0.5) * 255.0;
	}
}				



#define NORMAL_MAP_WIDTH	32	
#define	NORMAL_MAP_HEIGHT	32
#define	NORMAL_FREQ			1.0
#define	NORMAL_PIXEL(x,y) (x * 3 + y * 3 * NORMAL_MAP_WIDTH)

unsigned char normal_map[NORMAL_MAP_WIDTH * NORMAL_MAP_HEIGHT * 3];
unsigned char height_map[NORMAL_MAP_WIDTH * NORMAL_MAP_HEIGHT];


unsigned char	decalBuf[32 * 32] = {
	0xD0, 0xD0, 0xD0, 0xD0, 0xD0, 0xD0, 0xD0, 0xD0, 
	0xD0, 0xD0, 0xD0, 0xD0, 0xD0, 0xD0, 0xD0, 0xD0, 
	0xD0, 0xD0, 0x30, 0x30, 0x30, 0x30, 0xD0, 0xD0, 
	0xD0, 0xD0, 0x30, 0xD0, 0xD0, 0x30, 0xD0, 0xD0, 
	0xD0, 0xD0, 0x30, 0xD0, 0xD0, 0x30, 0xD0, 0xD0, 
	0xD0, 0xD0, 0x30, 0x30, 0x30, 0x30, 0xD0, 0xD0, 
	0xD0, 0xD0, 0xD0, 0xD0, 0xD0, 0xD0, 0xD0, 0xD0, 
	0xD0, 0xD0, 0xD0, 0xD0, 0xD0, 0xD0, 0xD0, 0xD0
};	

XObj		gObj;
double		gGroundDist = 100.0;
int			gListID;


// Degree-based trig function macros
#define cosd(x) cos((x) * M_PI / 180.0)
#define sind(x) sin((x) * M_PI / 180.0)

// OpenGL features that we care about

bool	gl_Has_Shadow = false;



bool	show_from_light = false;	// Draw the main scene from teh sun's viewpoint.
bool	show_lighting = true;		// Draw the main scene with lighting.
bool	show_depth = false;			// Show the scene showing D(p)
bool	show_depth_map = false;		// Show the secne showing D(c)
bool 	show_depth_tex = false;		// Show the depth map for the user (so they can see it)
bool	do_shadows = false;			// Draw the actual shadows

// It is necessary to use polygon offseting when drawing the depth map.  Why?  Well,
// if D(p) and D(c) are damn close enough, OGL jitters cause objects to randomly 
// self-shadow on every other pixel, which looks most shitty.  So we use polygon
// offset to make the depth texture map D(c) a little bit too far away.  This resolves
// our jitters.
float	poly_slope = 1.1;			// Seems to work good.
int		poly_offset = 2;			// This is good for 8 bit maps.
int		depth_scale = 1;			// This we set up when we learn our real depth buffer size.

// Camera position
double	camera_heading[2] = { 0.0, 0.0 };			// Besides a heading, tilt and dist we keep
double	camera_tilt[2] = { 40.0, 40.0 };
double	camera_dist[2] = { 200.0, 200.0 };
xcam_class	camera_camera[2];
int		cur_cam = 0;

// Sun position
double	light_heading = 180;
double	light_tilt = 45;
double	light_dist = 200;
xcam_class	light_camera;

// This matrix scales our fully projected 3-d coordinates [-1..1] into
// texture S & T friendly coords [0..1].  We use this when calculating
// D(c)

GLdouble	SMatrix[16] = {
	0.5, 0,   0,   0,
	0,   0.5, 0,   0,
	0,   0,   0.5, 0,
	0.5, 0.5, 0.5, 1.0 
};	

// This takes our depth value and puts it into S, and it puts 256 * 
// the depth and puts it in T.  It also leaves Q and zeros out R.
// We use this for now to get our D(p) value.  (Note: if we wanted to use
// 16 bits of depth info, we would need two texture components, like
// luminance and alpha.)

GLdouble	RSMatrix[16] = { 
	0,   0,   0, 0,
	0,   0,   0, 0,
	0.5, 128, 0, 0,
	0.5, 128, 0, 1.0
};	


// Memory storage for our depth buffer...eventually this will be done on card.
GLubyte depthBuf[512 * 512];

// Two textures.  The first is a 1-d texture lookup table.  The second is our
// shadow map.
enum {
	TEX_1D_LOOKUP = 1,
	SHADOW_MAP,
	NORMAL_MAP,
	DECAL	
};	


/*******************************************************************************************
 * TEXTURING UTILITIES
 *******************************************************************************************/
 
/* This routine builds our 1-d lookup texture, used to convert texture-coordinate S
 * into an alpha of the same value.  Really this maps 0-1 to 0-255. */
void	build_original_textures(void)
{
	GLubyte	texmap[256];
	for (int n =0 ;n < 256; ++n)
		texmap[n] = n;
	
	glBindTexture(GL_TEXTURE_1D, TEX_1D_LOOKUP);
	glTexImage1D (GL_TEXTURE_1D, 0, GL_INTENSITY8, 256, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, texmap);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	
	glBindTexture(GL_TEXTURE_2D, SHADOW_MAP);
	if (gl_Has_Shadow)
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32_SGIX, 512, 512, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, depthBuf);
	else
		glTexImage2D(GL_TEXTURE_2D, 0, GL_INTENSITY8, 512, 512, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, depthBuf);
	if (gl_Has_Shadow)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_SGIX, GL_TRUE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_OPERATOR_SGIX, GL_TEXTURE_LEQUAL_R_SGIX);
	}
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	
	glBindTexture(GL_TEXTURE_2D, DECAL);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 8, 8, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, decalBuf);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);	
}

/* This routine transfers the frame buffer's depth component into our shadow map.
 * This can be optimized for machines that have shadow mapping hardware. */
void	framebuffer_to_shadowmap(void)
{
	glBindTexture(GL_TEXTURE_2D, SHADOW_MAP);

	if (gl_Has_Shadow)
	{
		glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32_SGIX, 0, 0, 512, 512, 0);
	} else {	
		glReadPixels(0, 0, 512, 512, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, depthBuf);
		glTexSubImage2D (GL_TEXTURE_2D, 0, 0, 0, 512, 512, GL_LUMINANCE, GL_UNSIGNED_BYTE, depthBuf);
	}
}

void reset_textures(void)
{
	GL_ActiveTextureARB(GL_TEXTURE1_ARB);	
	glDisable(GL_TEXTURE_1D);
	glDisable(GL_TEXTURE_2D);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_Q);
	glDisable(GL_TEXTURE_GEN_T);	
	glDisable(GL_TEXTURE_GEN_R);		
	GL_ActiveTextureARB(GL_TEXTURE0_ARB);	
	glDisable(GL_TEXTURE_1D);
	glDisable(GL_TEXTURE_2D);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_Q);
	glDisable(GL_TEXTURE_GEN_T);	
	glDisable(GL_TEXTURE_GEN_R);		
}

void	setup_textures_for_obj_mapped_dist(void)
{
	GLdouble	m1[16], m2[16];
	
	glBindTexture(GL_TEXTURE_2D, SHADOW_MAP);
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_TEXTURE_1D);

	// Given a vertex in world-space, we apply three transforms on
	// it.  (We multiply in reverse order.)
	// 1. We transform it by the camera's model view to get it into
	//	  "eye" coordinates for the sun camera.
	// 2. We transform it by the perspective matrix for the sun camera.
	// 3. We transform it by a scaling matrix that maps all texture 
	//	  params to be 0..1 instead of -1..1.
	//
	// You might ask yourself: is it important whether we're in eye linear
	// or object-linear coordinate generation mode?  The ansewr is YES!
	// The difference between eye and object tex gen is that the tex gen 
	// transform "remembers" the world modelview coordinate system when it was
	// applied by (1) building it into the texgen 'matrix' and (2) always
	// apolying the current modelview's inverse as we draw.
	//
	// This is crucial if we mess with the modelview matrix during drawing.
	// It is useful to think of there actually being at least THREE coordinate systems:
	//  - Object space is whatever the model view is as we draw this object, possibly
	//    offset in the case of an OBJ7 object.
	//  - World space is the real global cartesian space we're used to.
	//  - Camera space is...just that, camera space, camera's at the origin looking down -Z.
	//
	// So when we use eye linear coordinates, every vertex is effectively given to us in 
	// the modelview space when we set this operation up, no matter what the modelview
	// system is doing as we emit our vertices.  So changes to the modelview coordinate
	// system do NOT change the mapping of the projected shadow texture onto the world.
	
	// (Compare this to object space texgen, which is useful for always mapping the same
	// texture to the same object as we make multiple copies of that object by messing
	// with the model-view matrix.  This is a truly local texgen...unmolested coordinates go
	// to the texgen matrix right off of glVertex.)

	copyMatrix(m1, SMatrix);
	
	multMatrices(m2, m1, light_camera.mPerspective);
	multMatrices(m1, m2, light_camera.mModelView);
	
	float p[4];

	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
	glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
	glTexGeni(GL_Q, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
	
	p[0] = m1[0];
	p[1] = m1[4];
	p[2] = m1[8];
	p[3] = m1[12];		
	glTexGenfv(GL_S, GL_EYE_PLANE, p);
	p[0] = m1[1];
	p[1] = m1[5];
	p[2] = m1[9];
	p[3] = m1[13];		
	glTexGenfv(GL_T, GL_EYE_PLANE, p);
	p[0] = m1[2];
	p[1] = m1[6];
	p[2] = m1[10];
	p[3] = m1[14];	
	glTexGenfv(GL_R, GL_EYE_PLANE, p);
	p[0] = m1[3];
	p[1] = m1[7];
	p[2] = m1[11];
	p[3] = m1[15];	
	glTexGenfv(GL_Q, GL_EYE_PLANE, p);
	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);	
	glEnable(GL_TEXTURE_GEN_R);		
	glEnable(GL_TEXTURE_GEN_Q);
}

void	setup_textures_for_obj_dist(void)
{
	GLdouble	m1[16], m2[16];

	glBindTexture(GL_TEXTURE_1D, TEX_1D_LOOKUP);
	glEnable(GL_TEXTURE_1D);
	glDisable(GL_TEXTURE_2D);
	copyMatrix(m1, RSMatrix);
	
	multMatrices(m2, m1, light_camera.mPerspective);
	multMatrices(m1, m2, light_camera.mModelView);
	
	float p[4];
	
	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
	p[0] = m1[0];
	p[1] = m1[4];
	p[2] = m1[8];
	p[3] = m1[12];	
	glTexGenfv(GL_S, GL_EYE_PLANE, p);
	p[0] = m1[3];
	p[1] = m1[7];
	p[2] = m1[11];
	p[3] = m1[15];	
	glTexGenfv(GL_Q, GL_EYE_PLANE, p);
	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_Q);
	glDisable(GL_TEXTURE_GEN_T);	
	glDisable(GL_TEXTURE_GEN_R);	
}

void	setup_textures_for_shadows(void)
{
	// Tex 0 is the mapped distance, darker means your element is closer.
	// Tex 1 is the real distance, darker means your element is closer.
	
	GL_ActiveTextureARB(GL_TEXTURE0_ARB);
	setup_textures_for_obj_mapped_dist();
	
	if (gl_Has_Shadow)
	{
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT);

		glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_COMBINE_EXT);
		glTexEnvf(GL_TEXTURE_ENV,GL_COMBINE_RGB_EXT,GL_INTERPOLATE_EXT);
		glTexEnvf(GL_TEXTURE_ENV,GL_SOURCE0_RGB_EXT,GL_PRIMARY_COLOR_EXT);
		glTexEnvf(GL_TEXTURE_ENV,GL_OPERAND0_RGB_EXT,GL_SRC_COLOR);
		glTexEnvf(GL_TEXTURE_ENV,GL_SOURCE1_RGB_EXT,GL_CONSTANT_EXT);
		glTexEnvf(GL_TEXTURE_ENV,GL_OPERAND1_RGB_EXT,GL_SRC_COLOR);
		glTexEnvf(GL_TEXTURE_ENV,GL_SOURCE2_RGB_EXT,GL_TEXTURE);
		glTexEnvf(GL_TEXTURE_ENV,GL_OPERAND2_RGB_EXT,GL_SRC_COLOR);

		glTexEnvf(GL_TEXTURE_ENV,GL_COMBINE_ALPHA_EXT,GL_REPLACE);
		glTexEnvf(GL_TEXTURE_ENV,GL_SOURCE0_ALPHA_EXT,GL_TEXTURE);
		glTexEnvf(GL_TEXTURE_ENV,GL_OPERAND0_ALPHA_EXT,GL_SRC_ALPHA);


		
//		GLfloat	col[4] = { 0.0, 1.0, 0.0, 1.0 };
//		glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, col);
		
	
	} else {
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT);
		
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_REPLACE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_EXT, GL_PRIMARY_COLOR_EXT);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_EXT, GL_SRC_COLOR);
		
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA_EXT, GL_REPLACE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_EXT, GL_TEXTURE);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA_EXT, GL_SRC_ALPHA);

		GL_ActiveTextureARB(GL_TEXTURE1_ARB);
		setup_textures_for_obj_dist();	

		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT);

		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_ADD_SIGNED_EXT);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_EXT, GL_TEXTURE);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_EXT, GL_ONE_MINUS_SRC_ALPHA);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_EXT, GL_PREVIOUS_EXT);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB_EXT, GL_SRC_ALPHA);

		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA_EXT, GL_ADD_SIGNED_EXT);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_EXT, GL_TEXTURE);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA_EXT, GL_ONE_MINUS_SRC_ALPHA);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA_EXT, GL_PREVIOUS_EXT);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA_EXT, GL_SRC_ALPHA);
		GL_ActiveTextureARB(GL_TEXTURE0_ARB);
	}
	
	if (gl_Has_Shadow)
	{
		glDisable(GL_ALPHA_TEST);
	} else {
		glAlphaFunc(GL_GEQUAL, 0.5);
		glEnable(GL_ALPHA_TEST);	
	}
}

void	build_camera_matrices(void)
{
	for (int n = 0; n < 2; ++n)
	{
		camera_camera[n].SetupPerspective(60, 1.0, 50.0, 400.0 + 5000.0 * n);
		camera_camera[n].LookAtPtFromDir(0.0, 0.0, 0.0, camera_heading[n], camera_tilt[n], camera_dist[n]);
	}
}

void	build_light_matrices(void)
{
//	light_camera.SetupPerspective(60, 1.0, 50.0, 400.0);
//	light_camera.LookAtPtFromDir(0.0, 0.0, 0.0, light_heading, light_tilt, light_dist);				

	GLdouble	look_dir[3];
	
	look_dir[0] = cos(-light_tilt * DEG2RAD) * cos(light_heading * DEG2RAD);
	look_dir[1] = sin(-light_tilt * DEG2RAD);
	look_dir[2] = cos(-light_tilt * DEG2RAD) * sin(light_heading * DEG2RAD);

	light_camera.LookAtCamera(&camera_camera[0], look_dir, light_dist);
}

void	setup_light_world(void)
{
	light_camera.LoadPerspective();
	light_camera.LoadModelView();
}

void	setup_camera_world(void)
{
	camera_camera[cur_cam].LoadPerspective();
	camera_camera[cur_cam].LoadModelView();
}

void	setup_lighting(bool doLights)
{
	// NOTE: THIS IS WHERE WE WOULD CONTROL THE COLOR BALANCE OF THE SUN VS. AMBIENCE!!
	if (doLights)
	{
   		static GLfloat	ogl_base_amb[4] = { 0.0, 0.0, 0.0, 1.0 };
		static GLfloat	ambient[4] = { 0.2, 0.2, 0.2, 1.0 };
		static GLfloat	diffuse[4] = { 0.8, 0.8, 0.8, 1.0 };
		static GLfloat	position[4] = { 0.0, 0.0, 0.0, 0.0 };
		GLdouble	position_eye[4] = { 0.0, 0.0, -1.0, 1.0 };
		GLdouble	position_mod[4];
		GLdouble	im[16]; 
		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);
		glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
		
		invertMatrix(im, light_camera.mModelView);
		
		multMatrixVec(position_mod, im, position_eye);
		
		position[0] = position_mod[0];
		position[1] = position_mod[1];
		position[2] = position_mod[2];
		
		glLightfv(GL_LIGHT0, GL_POSITION, position);
		glLightModelfv(GL_LIGHT_MODEL_AMBIENT,ogl_base_amb);

		glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, ambient);
		
		static	GLfloat	materialAmb[4] = { 1.0, 1.0, 1.0, 1.0 };
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, materialAmb);

	} else
		glDisable(GL_LIGHTING);
}

void	preview_camera(xcam_class * inCamera, GLfloat color[3])
{
	glColor3fv(color);
	
	glPushMatrix();
	
	GLdouble	lightProjMatrix_i[16],lightModViewMatrix_i[16];
					
	invertMatrix(lightProjMatrix_i, inCamera->mPerspective);
	invertMatrix(lightModViewMatrix_i, inCamera->mModelView);
	
	glMultMatrixd(lightModViewMatrix_i);
	glPushMatrix();
	glMultMatrixd(lightProjMatrix_i);
	
	glutWireCube(2.0);

	glPopMatrix();

	glPointSize(5);
	glBegin(GL_POINTS);
	glVertex3f(0, 0, 0);
	glEnd();
	glPointSize(1);
	
	glPopMatrix();
}

void	draw_world(bool drawLightDiagram)
{
	// First draw
	glEnable(GL_DEPTH_TEST);
	glShadeModel(GL_FLAT);

	setup_lighting(show_lighting);	

#if DO_NORMAL_MAP
	
	for (int x = 0; x < NORMAL_MAP_HEIGHT; ++x)
	for (int y = 0; y < NORMAL_MAP_WIDTH; ++y)
	{
		height_map[x + y * NORMAL_MAP_WIDTH] = 
			(x > 5 && x < 20 && y > 5 && y < 20) ? 3.0 : 1.0;
	}

	create_normal_map(height_map, normal_map, NORMAL_MAP_WIDTH, NORMAL_MAP_HEIGHT, 1.0);
	
	glBindTexture(GL_TEXTURE_2D, NORMAL_MAP);
	glTexImage2D (GL_TEXTURE_2D, 0, GL_RGB, NORMAL_MAP_WIDTH, NORMAL_MAP_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, normal_map);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glEnable(GL_TEXTURE_2D);
	
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT);
	
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, 0x86AE);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_EXT, GL_PRIMARY_COLOR_EXT);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_EXT, GL_SRC_COLOR);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_EXT, GL_TEXTURE);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB_EXT, GL_SRC_COLOR);
	
	GLdouble	look_dir[3];
	
	look_dir[0] = -cos(-light_tilt * DEG2RAD) * cos(light_heading * DEG2RAD);
	look_dir[1] = -sin(-light_tilt * DEG2RAD);
	look_dir[2] = -cos(-light_tilt * DEG2RAD) * sin(light_heading * DEG2RAD);
	glColor3f(
		look_dir[0] * 0.5 + 0.5,
		look_dir[1] * 0.5 + 0.5,
		look_dir[2] * 0.5 + 0.5);
	
	glNormal3f(0.0, 1.0, 0.0);

	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(-100.0, 0.0, -100.0);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(-100.0, 0.0,  100.0);
	glTexCoord2f(1.0, 1.0);
	glVertex3f( 100.0, 0.0,  100.0);
	glTexCoord2f(1.0, 0.0);
	glVertex3f( 100.0, 0.0, -100.0);
	glEnd();
	
	glDisable(GL_TEXTURE_2D);
	
#else	
#if FORCE_NO_COLOR
	glColor3f(1.0, 1.0, 1.0);
#else
	glColor3f(0.6, 0.7, 0.8);
#endif	

	glBegin(GL_QUADS);
	glNormal3f(0.0, 1.0, 0.0);
	glVertex3f(-gGroundDist, 0.0, -gGroundDist);
	glVertex3f(-gGroundDist, 0.0,  gGroundDist);
	glVertex3f( gGroundDist, 0.0,  gGroundDist);
	glVertex3f( gGroundDist, 0.0, -gGroundDist);
	glEnd();
	
#endif	

	GL_ActiveTextureARB(GL_TEXTURE1_ARB);
	glBindTexture(GL_TEXTURE_2D, DECAL);
	glEnable(GL_TEXTURE_2D);
	GL_ActiveTextureARB(GL_TEXTURE0_ARB);

	if (!gObj.cmds.empty())
	{
		glAlphaFunc		(GL_GREATER,0						);	// when ALPHA-TESTING  is enabled, we display the GREATEST alpha
		glBlendFunc		(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);	// when ALPHA-BLENDING is enabled, we blend normally				
	
		glEnable(GL_ALPHA_TEST);
		glEnable(GL_BLEND);
		glPointSize(3);
		glShadeModel(GL_FLAT);
		glFrontFace(GL_CW);
		glCullFace(GL_BACK);
		glEnable(GL_CULL_FACE);
		glColor3f(1.0, 1.0, 1.0);
		glCallList(gListID);
//		ObjectToDL(gListID, gObj, 1, 50, false, GL_MultiTexCoord2fARB);
		glDisable(GL_CULL_FACE);
		glPointSize(1);
	} else {

		for (float x = -80.0; x < 80.0; x += 50.0)
		for (float z = -80.0; z < 80.0; z += 50.0)
		{
			int height = ((int) fabs(x * (z + 20)) % 40) + 10;
			
			bool	visible = camera_camera[0].SphereInView(x, 0.0, z, 10.0);
	#if FORCE_NO_COLOR
		glColor3f(1.0, 1.0, 1.0);
	#else
			if (visible)
				glColor3f(0.6, 0.7, 0.6);
			else
				glColor3f(1.0, 0, 0.0);
	#endif			
			glBegin(GL_QUAD_STRIP);
			glNormal3f(-1.0, 0.0, 0.0);
			GL_MultiTexCoord2fARB(GL_TEXTURE1_ARB, 0.0, 0.0);		glVertex3f(x - 10.0,  0, z - 10.0);
			GL_MultiTexCoord2fARB(GL_TEXTURE1_ARB, 0.0, 1.0);		glVertex3f(x - 10.0, height, z - 10.0);
			GL_MultiTexCoord2fARB(GL_TEXTURE1_ARB, 1.0, 0.0);		glVertex3f(x - 10.0,  0, z + 10.0);
			GL_MultiTexCoord2fARB(GL_TEXTURE1_ARB, 1.0, 1.0);		glVertex3f(x - 10.0, height, z + 10.0);
			glNormal3f(0.0, 0.0, 1.0);
			GL_MultiTexCoord2fARB(GL_TEXTURE1_ARB, 0.0, 0.0);		glVertex3f(x + 10.0,  0, z + 10.0);
			GL_MultiTexCoord2fARB(GL_TEXTURE1_ARB, 0.0, 1.0);		glVertex3f(x + 10.0, height, z + 10.0);
			glNormal3f(1.0, 0.0, 0.0);
			GL_MultiTexCoord2fARB(GL_TEXTURE1_ARB, 1.0, 0.0);		glVertex3f(x + 10.0,  0, z - 10.0);
			GL_MultiTexCoord2fARB(GL_TEXTURE1_ARB, 1.0, 1.0);		glVertex3f(x + 10.0, height, z - 10.0);
			glNormal3f(0.0, 0.0, -1.0);
			GL_MultiTexCoord2fARB(GL_TEXTURE1_ARB, 0.0, 0.0);		glVertex3f(x - 10.0,  0, z - 10.0);
			GL_MultiTexCoord2fARB(GL_TEXTURE1_ARB, 0.0, 1.0);		glVertex3f(x - 10.0, height, z - 10.0);
			glEnd();

			glNormal3f(0.0, 1.0, 0.0);		
	#if FORCE_NO_COLOR
		glColor3f(1.0, 1.0, 1.0);
	#else
			if (visible)
				glColor3f(0.3, 0.7, 0.6);
	#endif			
			glBegin(GL_QUADS);
			glVertex3f(x - 10.0, height, z - 10.0);
			glVertex3f(x - 10.0, height, z + 10.0);
			glVertex3f(x + 10.0, height, z + 10.0);
			glVertex3f(x + 10.0, height, z - 10.0);
			glEnd();
		}	
	}
	
	GL_ActiveTextureARB(GL_TEXTURE1_ARB);
	glDisable(GL_TEXTURE_2D);
	GL_ActiveTextureARB(GL_TEXTURE0_ARB);


	if (drawLightDiagram)
	{		
		setup_lighting(false);
		
		GLfloat	light[3] =  { 1.0, 1.0, 0.0 };
		GLfloat	obj[3] = { 0.5, 0.5, 0.5 };
		
		preview_camera(&light_camera, light);
		if (cur_cam == 1)
			preview_camera(&camera_camera[0], obj);
		
	}
}

void handle_redisplay(void)
{
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	setup_light_world();

	reset_textures();	

	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(poly_slope, poly_offset * depth_scale);
	draw_world(false);
	glDisable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(0.0, 0);

	framebuffer_to_shadowmap();
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if (show_from_light)
		setup_light_world();
	else
		setup_camera_world();

	reset_textures();	
		
	glDisable(GL_TEXTURE_1D);
	glDisable(GL_TEXTURE_2D);
	if (show_depth)
		setup_textures_for_obj_dist();
	if (show_depth_map)
		setup_textures_for_obj_mapped_dist();	
	if (do_shadows)
		setup_textures_for_shadows();
	
	draw_world(true);
	
	if (show_depth_tex)
	{
		reset_textures();
		setup_lighting(false);
		glDisable(GL_DEPTH_TEST);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(-256, 256, -256, 256, -1, 1);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glRasterPos2i(-256,-256);
		glDrawPixels(512, 512, GL_LUMINANCE, GL_UNSIGNED_BYTE, depthBuf);
	}
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-256, 256, -256, 256, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	static	char	buf[1024], camera[256], light[256], oper[256];
	static	clock_t	last_time = clock();
	
	clock_t	now = clock();
	double	dif = now - last_time;
	last_time = now;
	double	fps = (double) CLOCKS_PER_SEC / dif;

	if (show_depth_tex)
		sprintf(buf, "FPS: %1.3f - Showing actual shadow map texture.", fps);
	else {
		sprintf(light, "Lighting is %s.", show_lighting ? "on" : "off");
		sprintf(camera, "Camera at %s.", show_from_light ? "sun's position" : (cur_cam ? "God's-eye-view" : "eye location"));
		sprintf(oper, "Showing %s.", do_shadows ? (gl_Has_Shadow ? "shadows" : "disabled shadows") : 
			(show_depth_map ? "projected depth map texture" :
			(show_depth ? "generated depth of scene" : "raw geometry")));
		sprintf(buf, "FPS: %1.3f %s %s %s", fps, light, camera, oper);		
	}
	if (show_depth_tex)
		glColor3f(1.0, 0.0, 0.0);
	else
		glColor3f(1.0, 1.0, 1.0);
	glut_print_string(-250, -230, buf);
	
	glutSwapBuffers();
}

void handle_key(unsigned char key, int x, int y)
{
	switch(key) {
	case ' ':
		show_from_light = !show_from_light;
		glutPostRedisplay();
		break;
	case 'l':
		show_lighting = !show_lighting;
		glutPostRedisplay();
		break;		
	case 'd':
		show_depth = !show_depth;
		if (show_depth) show_depth_map = false, do_shadows = false;
		glutPostRedisplay();
		break;
	case 't':
		show_depth_tex = !show_depth_tex;
		glutPostRedisplay();
		break;
	case 'p':
		show_depth_map = !show_depth_map;
		if (show_depth_map) show_depth = false, do_shadows = false;
		glutPostRedisplay();
		break;
	case 's':
		do_shadows = !do_shadows;
		if (do_shadows) show_depth = false, show_depth_map = false;
		glutPostRedisplay();
		break;
	case 'q':
		exit(1);
		break;
	case '-':
		camera_dist[cur_cam] += 10;
		build_camera_matrices();
		build_light_matrices();
		glutPostRedisplay();
		break;
	case '=':
		camera_dist[cur_cam] -= 10;
		build_camera_matrices();
		build_light_matrices();
		glutPostRedisplay();
		break;
	case '_':
		camera_dist[0] += 10;
		build_camera_matrices();
		build_light_matrices();
		glutPostRedisplay();
		break;
	case '+':
		camera_dist[0] -= 10;
		build_camera_matrices();
		build_light_matrices();
		glutPostRedisplay();
		break;
	case 'b':
		poly_slope -= 0.1;
		glutPostRedisplay();
		break;
	case 'B':
		poly_slope += 0.1;
		glutPostRedisplay();
		break;
	case 'o':
		poly_offset--;
		glutPostRedisplay();
		break;
	case 'O':
		poly_offset++;
		glutPostRedisplay();
		break;
	case '\t':
		cur_cam = 1 - cur_cam;
		build_camera_matrices();
		build_light_matrices();
		glutPostRedisplay();
		break;
	case 'x':
		copy_to_clipboard();
		break;
	}
}

void handle_key_special(int key, int x, int y)
{
	switch(key) {
	case GLUT_KEY_LEFT:
		if (glutGetModifiers() & GLUT_ACTIVE_SHIFT)
			light_heading -= 10;
		else if (glutGetModifiers() & GLUT_ACTIVE_CTRL)
			camera_heading[0] -= 10;
		else
			camera_heading[cur_cam] -= 10;
		build_camera_matrices();
		build_light_matrices();
		glutPostRedisplay();
		break;
	case GLUT_KEY_RIGHT:
		if (glutGetModifiers() & GLUT_ACTIVE_SHIFT)
			light_heading += 10;
		else if (glutGetModifiers() & GLUT_ACTIVE_CTRL)		
			camera_heading[0] += 10;
		else
			camera_heading[cur_cam] += 10;
		build_camera_matrices();
		build_light_matrices();
		glutPostRedisplay();
		break;
	case GLUT_KEY_DOWN:
		if (glutGetModifiers() & GLUT_ACTIVE_SHIFT)
			light_tilt -= 5;
		else if (glutGetModifiers() & GLUT_ACTIVE_CTRL)
			camera_tilt[0] -= 5;
		else
			camera_tilt[cur_cam] -= 5;
		build_camera_matrices();
		build_light_matrices();
		glutPostRedisplay();
		break;
	case GLUT_KEY_UP:
		if (glutGetModifiers() & GLUT_ACTIVE_SHIFT)
			light_tilt += 5;
		else if (glutGetModifiers() & GLUT_ACTIVE_CTRL)
			camera_tilt[0] += 5;
		else
			camera_tilt[cur_cam] += 5;
		build_camera_matrices();
		build_light_matrices();
		glutPostRedisplay();
		break;
	}
}

void	handle_idle(void)
{
	glutPostRedisplay();
}

void handle_mouse(int button, int state, int x, int y)
{
}

#if !defined(__POWERPC__)
#undef glutInit
extern "C" extern void APIENTRY glutInit(int * argcp, char **argv);
#endif

int main(int argc, char ** argv)
{
	glutInit(&argc, argv);		
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowPosition(10, 40);
	glutInitWindowSize(512, 512);
	
	glutCreateWindow("Shadow Demo");
	
#if !defined(__POWERPC__)
	GL_ActiveTextureARB = (PFNGLACTIVETEXTUREARBPROC) wglGetProcAddress("glActiveTextureARB"   );
	GL_MultiTexCoord2fARB = (PFNGLMULTITEXCOORD2FARBPROC) wglGetProcAddress("glMultiTexCoord2fARB");
#endif	

	// In order to use hardware shadow mapping we need both the GL_SGIX_shadow extension, which
	// gives us the compare on one texture unit op, and the GL_SGIX_depth_texture op, which lets us
	// make depth textures.  But no card will have one and not the other.  Someday we should check
	// for the ARB versions too just in case; my GF-FX-MX has both.
	gl_Has_Shadow = (glutExtensionSupported("GL_SGIX_shadow") || glutExtensionSupported("GL_ARB_shadow")) &&
					(glutExtensionSupported("GL_SGIX_depth_texture") || glutExtensionSupported("GL_ARB_depth_texture"));
	
	glutDisplayFunc(handle_redisplay);
	glutKeyboardFunc(handle_key);
	glutSpecialFunc(handle_key_special);
	glutMouseFunc(handle_mouse);
	glutIdleFunc(handle_idle);

	build_original_textures();
	build_camera_matrices();
	build_light_matrices();
	
	GLint	depthBits;
	glGetIntegerv(GL_DEPTH_BITS, &depthBits);
	if (depthBits < 8)
		depth_scale = 1;
	else
		depth_scale = 1 << (depthBits - 8);
	
	glutPostRedisplay();

   glEnable			(GL_COLOR_MATERIAL					);	// GL_COLOR_MATERIAL lets us use OGL_setcolor to set material PROPERTIES FOR LIGHTING TO BOUNCE OFF OF. needed for specularity.
   glColorMaterial	(GL_FRONT,GL_AMBIENT_AND_DIFFUSE	);	// use color material for ambient and diffuse lighting

	XObjRead("shadow.obj", gObj);
	gListID = glGenLists(1);	
	if (!gObj.cmds.empty())
	{
		ObjectToDL(gListID, gObj, 1, 50, true, (PFNGLMULTITEXCOORD2FARBPROC) GL_MultiTexCoord2fARB);
		LoadTextureFromFile("shadow.png", DECAL, tex_MagentaAlpha + tex_Mipmap + tex_Linear, NULL, NULL, NULL, NULL);
		float	minc[3], maxc[3];
		GetObjDimensions(gObj, minc, maxc);
		gGroundDist = max((maxc[0] - minc[0]), (maxc[2] - minc[2]));
		gGroundDist += maxc[1] - minc[1];
	}
	
	glutMainLoop();
	return 0;
}

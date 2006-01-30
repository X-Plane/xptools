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
 
 #ifndef XOBJBUILDER_H
 #define XOBJBUILDER_H
 
struct	XObj8;
struct	XObjLOD8;
 
 class	XObjBuilder {
 public:
 
 	 XObjBuilder(XObj8 * inObj);
 	~XObjBuilder();
 	
 	void	BeginLOD(float inNear, float inFar);
 	void	EndLOD(void);
 	
 	void	SetAttribute(int inAttribute);
 	void	SetAttribute1(int inAttribute, float v);
 	void	SetAttribute3(int inAttribute, float v[3]);
 	
 	// X Y Z nX ny nZ S T repeated 3 times
 	void	AccumTri(float inTri[24]);
 	
 	// X Y Z R G B repeated 2 times, RGB = 0-1
 	void	AccumLine(float inLine[12]);
 	
 	// X Y Z R G B
 	void	AccumLight(float inPoint[6]);
 
 	void	AccumTranslate(float xyz1[3], float xyz2[3], float v1, float v2, const char * ref);
 	
 	void	AccumRotate(float axis[3], float r1, float r2, float v1, float v2, const char * ref);
 
 private:
 
 	void	AssureLOD(void);
 	void	SetDefaultState(void);
 
 	XObj8 *		obj;
 	XObjLOD8 *	lod;
 	int			hard;
// 	int			no_depth;
 	int			flat;
 	int			two_sided;
 	int			no_blend;
 	int			cockpit;
 	float		offset;

//	float		ambient[3];			// Ambient not used - no ambient control in x-plane
	float		diffuse[3];			// Diffuse STRONGLY not recommended!  Use texture
	float		emission[3];		// Used for self-lit signs
//	float		specular[3];		// Specular not used - set automatically by shiny-rat!
	float		shiny;				// Used for metal

};
 
 #endif
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
#include "OE_TexProjector.h"
#include "XPLMGraphics.h"
#include "MatrixUtils.h"

void	OE_PlanarProjector::DrawProjector(void)
{
	XPLMSetGraphicsState(0,0,0,  0,1, 0,0);
		
	glBegin(GL_LINES);

	glColor4f(0.5, 1.0, 1.0, 1.0);
	glVertex3f(-1.0,  1.0,  1.0);		glVertex3f( 1.0,  1.0,  1.0);
	glVertex3f(-1.0, -1.0,  1.0);		glVertex3f( 1.0, -1.0,  1.0);
	glColor4f(1.0, 1.0, 1.0, 0.2);
	glVertex3f(-1.0,  1.0, -1.0);		glVertex3f( 1.0,  1.0, -1.0);
	glVertex3f(-1.0, -1.0, -1.0);		glVertex3f( 1.0, -1.0, -1.0);

	glColor4f(0.5, 1.0, 1.0, 1.0);
	glVertex3f( 1.0, -1.0,  1.0);		glVertex3f( 1.0,  1.0,  1.0);	
	glVertex3f(-1.0, -1.0,  1.0);		glVertex3f(-1.0,  1.0,  1.0);	
	glColor4f(1.0, 1.0, 1.0, 0.2);
	glVertex3f( 1.0, -1.0, -1.0);		glVertex3f( 1.0,  1.0, -1.0);	
	glVertex3f(-1.0, -1.0, -1.0);		glVertex3f(-1.0,  1.0, -1.0);	
	
	glVertex3f( 1.0,  1.0, -1.0);		glVertex3f( 1.0,  1.0,  1.0);
	glVertex3f(-1.0,  1.0, -1.0);		glVertex3f(-1.0,  1.0,  1.0);
	glVertex3f( 1.0, -1.0, -1.0);		glVertex3f( 1.0, -1.0,  1.0);
	glVertex3f(-1.0, -1.0, -1.0);		glVertex3f(-1.0, -1.0,  1.0);
	
	glEnd();
}

void	OE_PlanarProjector::ProjectVertex(double x, double y, double z, double& s, double& t)
{
	s = (x * 0.5) + 0.5;
	t = (y * 0.5) + 0.5;
}

void	OE_CylinderProjector::DrawProjector(void)
{
	static GLUquadricObj * myQuad = gluNewQuadric();
	gluQuadricDrawStyle(myQuad, GLU_LINE);
	XPLMSetGraphicsState(0,0,0,  0,1, 0,0);
	glColor4f(1.0, 1.0, 1.0, 0.3);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glRotatef(90.0, 1.0, 0.0, 0.0);
	glTranslatef(0.0, 0.0, -1.0);
	gluCylinder(myQuad, 1.0, 1.0, 2.0, 12, 4);
	glPopMatrix();
}

void	OE_CylinderProjector::ProjectVertex(double x, double y, double z, double& s, double& t)
{
	s = (atan2(x, z) / M_PI) * 0.5 + 0.5;
	t = (y * 0.5) + 0.5;
}

void	OE_SphereProjector::DrawProjector(void)
{
	static GLUquadricObj * myQuad = gluNewQuadric();
	gluQuadricDrawStyle(myQuad, GLU_LINE);
	XPLMSetGraphicsState(0,0,0,  0,1, 0,0);
	glColor4f(1.0, 1.0, 1.0, 0.3);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glRotatef(90.0, 1.0, 0.0, 0.0);
	gluSphere(myQuad, 1.0, 12, 12);
	glPopMatrix();
}

void	OE_SphereProjector::ProjectVertex(double x, double y, double z, double& s, double& t)
{
	s = (atan2(x, z) / M_PI) * 0.5 + 0.5;
	double xz = sqrt(x * x + z * z);
	t = (atan2(xz, y) / M_PI) * 0.5 + 0.5;
}

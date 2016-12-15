//
//  GLDebugMacros.cpp
//  SceneryTools_xcode6
//
//  Created by Ben Supnik on 12/1/15.
//
//

#include "GLDebugMacros.h"

void __gl_error_check(const char * file, int line)
{
	int e = glGetError();
	if(e != GL_NO_ERROR)
	{
		printf("OpenGL error: %x in %s:%d\n", e, file, line);
	}
}


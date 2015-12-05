//
//  GLDebugMacros.hpp
//  SceneryTools_xcode6
//
//  Created by Ben Supnik on 12/1/15.
//
//

#ifndef GLDebugMacros_h
#define GLDebugMacros_h

#if APL
	#include <OpenGL/gl.h>
#else
	#include <GL/gl.h>
#endif


void __gl_error_check(const char * file, int line);

#define glDrawArrays(...) do { glDrawArrays(__VA_ARGS__); __gl_error_check(__FILE__,__LINE__); } while(0)
#define glDrawElements(...) do { glDrawElements(__VA_ARGS__); __gl_error_check(__FILE__,__LINE__); } while(0)
#define glBegin(...) do { __gl_error_check(__FILE__,__LINE__); glBegin(__VA_ARGS__); } while(0)
#define glEnd(...) do { glEnd(__VA_ARGS__); __gl_error_check(__FILE__,__LINE__); } while(0)

#define glTexImage1D(...) do { glTexImage1D(__VA_ARGS__); __gl_error_check(__FILE__,__LINE__); } while(0)
#define glTexImage2D(...) do { glTexImage2D(__VA_ARGS__); __gl_error_check(__FILE__,__LINE__); } while(0)
#define glTexImage3D(...) do { glTexImage3D(__VA_ARGS__); __gl_error_check(__FILE__,__LINE__); } while(0)
#define glTexParameterf(...) do { glTexParameterf(__VA_ARGS__); __gl_error_check(__FILE__,__LINE__); } while(0)
#define glTexParameterfv(...) do { glTexParameterfv(__VA_ARGS__); __gl_error_check(__FILE__,__LINE__); } while(0)
#define glTexParameteri(...) do { glTexParameteri(__VA_ARGS__); __gl_error_check(__FILE__,__LINE__); } while(0)
#define glTexSubImage1D(...) do { glTexSubImage1D(__VA_ARGS__); __gl_error_check(__FILE__,__LINE__); } while(0)
#define glTexSubImage2D(...) do { glTexSubImage2D(__VA_ARGS__); __gl_error_check(__FILE__,__LINE__); } while(0)
#define glTexSubImage3D(...) do { glTexSubImage3D(__VA_ARGS__); __gl_error_check(__FILE__,__LINE__); } while(0)

#define glBindTexture(...) do { glBindTexture(__VA_ARGS__); __gl_error_check(__FILE__,__LINE__); } while(0)
#define glEnable(...) do { glEnable(__VA_ARGS__); __gl_error_check(__FILE__,__LINE__); } while(0)
#define glDisable(...) do { glDisable(__VA_ARGS__); __gl_error_check(__FILE__,__LINE__); } while(0)

#define glClear(...) do { glClear(__VA_ARGS__); __gl_error_check(__FILE__,__LINE__); } while(0)
#define glClearColor(...) do { glClearColor(__VA_ARGS__); __gl_error_check(__FILE__,__LINE__); } while(0)

#endif /* GLDebugMacros_h */

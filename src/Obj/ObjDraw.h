#ifndef OBJDRAW_H
#define OBJDRAW_H

// OBJ7:
// These APIs will handle merged tris/quads/lines for an OBJ7, but will
// not attempt to consolidate begin/end statements or other unnecessary state.
// This routine will attempt to change textures only when needed.

// OBJ8: 
// OBJ8 drawing will not eliminate unneeded state changes or consolidate
// draws, but for well-formed OBJ8s, the TRIs command should produce long runs.

struct XObj;
struct XObj8;

struct	ObjDrawFuncs_t {
	// These routines are called to set up OGL for a certain mode.
	void (* SetupPoly_f)(void * ref);
	void (* SetupLine_f)(void * ref);
	void (* SetupLight_f)(void * ref);
	void (* SetupMovie_f)(void * ref);
	void (* SetupPanel_f)(void * ref);
	// This is your multi-tex-coord routine.
	void (* TexCoord_f)(const float * st, void * ref);
	void (* TexCoordPointer_f)(int size, unsigned long type, long stride, const void * pointer, void * ref);
	// Return anim params here.
	float (* GetAnimParam)(const char * string, float v1, float v2, void * ref);
};

void	ObjDraw(const XObj& obj, float dist, ObjDrawFuncs_t * funcs, void * ref);
void	ObjDraw8(const XObj8& obj, float dist, ObjDrawFuncs_t * funcs, void * ref);

#endif

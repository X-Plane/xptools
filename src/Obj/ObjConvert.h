#ifndef OBJCONVERT_H
#define OBJCONVERT_H

struct XObj;
struct XObj8;

// OBJECT CONVERSION
// These convert from one obj to the other.  This can take overloaded
// tris/quads/lines in the OBJ7 but generates separate tris/quads/lines
// on convert-back.
void	Obj7ToObj8(const XObj& obj7, XObj8& obj8);
void	Obj8ToObj7(const XObj8& obj8, XObj& obj7);

// This merges consecutive index commands in an OBJ8 for you.
void	Obj8_ConsolidateIndexCommands(XObj8& obj8);
// This calculates OBJ8 normals frmo tris, editing the point pool.
void	Obj8_CalcNormals(XObj8& obj8);


#endif
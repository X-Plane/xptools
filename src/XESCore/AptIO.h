#ifndef APTIO_H
#define APTIO_H

struct XAtomContainer;

#include "AptDefs.h"
#include <set>

//void	WriteApts(FILE * fi, const AptVector& inApts);
bool	ReadApts(XAtomContainer& container, AptVector& outApts);

bool	ReadAptFile(const char * inFileName, AptVector& outApts);
bool	ReadAptFileMem(const char * inBegin, const char * inEnd, AptVector& outApts);
bool	WriteAptFile(const char * inFileName, const AptVector& outApts);
bool	WriteAptFileOpen(FILE * inFile, const AptVector& outApts);

void	IndexAirports(const AptVector& apts, AptIndex& index);
void	FindAirports(const Bbox2& bounds, const AptIndex& index, set<int>& apts);

void	ConvertForward(AptInfo_t& io_apt);

#endif

#ifndef WED_BUFFER_H
#define WED_BUFFER_H

#include "IODefs.h"

#include <list>
#include <vector>
using std::list;
using std::vector;

class	WED_Buffer : public IOReader, public IOWriter {
public:

					WED_Buffer();
					WED_Buffer(const WED_Buffer& rhs);
					~WED_Buffer();
	WED_Buffer&		operator=(const WED_Buffer& rhs);

			void	Reserve(unsigned long inBytes);
			void	ResetRead(void);

	virtual	void	ReadShort(short&);
	virtual	void	ReadInt(int&);
	virtual	void	ReadFloat(float&);
	virtual	void	ReadDouble(double&);
	virtual	void	ReadBulk(char * inBuf, int inLength, bool inZip);
	
	virtual	void	WriteShort(short);
	virtual	void	WriteInt(int);
	virtual	void	WriteFloat(float);
	virtual	void	WriteDouble(double);
	virtual	void	WriteBulk(const char * inBuf, int inLength, bool inZip);

private:

			void	ReadInternal (char * p, unsigned long l);
			void	WriteInternal(const char * p, unsigned long l);

	typedef vector<char>		Block;
	typedef list<Block>			Storage;
	
	Storage				mStorage;
	
	Storage::iterator	mReadIterator;
	Block::size_type	mReadSubpos;
	
};	


#endif /* WED_BUFFER_H */

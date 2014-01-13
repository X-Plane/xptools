#pragma once

enum ValidatorLevel
{
	VLEV_WHITESPACE,
	VLEV_ISALLASCII,
	VLEV_ELEMENTS,
	VLEV_CURLY,
	VLEV_CURLY_INSIDE
};
	
class ParserMain
{
public:
	ParserMain(void);
	~ParserMain(void);

	int main(int argc, const char* argv[]);
};


//
//  main.cpp
//  SignParser
//
//  Created by Tyler Young on 2/6/17.
//
//

#include <iostream>
#include "SignParser_Canonicalize.h"

using namespace std;

int main(int argc, const char * argv[])
{
#if DEV
	canonicalize_taxi_sign_auto_test();
#endif

	if(argc >= 2)
	{
		string sign_text;
		for(int arg = 1; arg < argc; ++arg)
		{
			sign_text += string(argv[arg]);
		}

		pair<string, string> canonical_form = canonicalize_taxi_sign(sign_text);
		if(canonical_form.first.empty())
		{
			cerr << canonical_form.second << endl;
			return 1;
		}
		else
		{
			cout << canonical_form.first << endl;
		}
	}
	else
	{
		cerr << "Too few arguments." << endl;
		cerr << "Next time, run:" << endl;
		cerr << endl;
		cerr << argv[0] << " [sign text]" << endl;
		cerr << endl;
		cerr << "...where sign text looks like: {@L}A1{@R}31R-13L" << endl;
		return 1;
	}
}





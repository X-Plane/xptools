#ifndef __signparser_canonicalize_h__
#define __signparser_canonicalize_h__

#include <string>
#include <utility> // for std::pair

using namespace std;

pair<string, string> canonicalize_taxi_sign(const string &sign_text);
void canonicalize_taxi_sign_auto_test();

#endif // defined(__signparser_canonicalize_h__)

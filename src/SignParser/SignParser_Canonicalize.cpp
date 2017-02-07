#include "SignParser_Canonicalize.h"
#include "WED_Taxi_Sign.h"
#include "AssertUtils.h"

/**
 * @return Canonical taxi sign (first), or error text (second)
 */
pair<string, string> canonicalize_taxi_sign(const string &sign_text)
{
	sign_data sign;
	const bool succeeded = sign.from_code(sign_text);
	if(succeeded)
	{
		return make_pair(sign.to_code(), "");
	}
	else
	{
		parser_out_info raw;
		ParserTaxiSign(parser_in_info(sign_text), raw);
		stringstream err;
		err << "Skipping sign \"" << sign_text << "\" due to errors:" << endl;
		for(auto e : raw.errors)
		{
			err << "\t" << e.msg << " from pos " << e.position << " to " << e.position + e.length << endl;
		}
		return make_pair("", err.str());
	}
}

void canonicalize_taxi_sign_auto_test()
{
	// Meatball case
	auto results0 = canonicalize_taxi_sign("{@R}04L");
	DebugAssert(results0.first == "{@R}04L");
	DebugAssert(results0.second.empty());
	// Weird syntax variants
	auto results1 = canonicalize_taxi_sign("{@R,0,4,L}");
	DebugAssert(results1 == results0);
	auto results2 = canonicalize_taxi_sign("{@R}0{}4{L}");
	DebugAssert(results2 == results0);
}

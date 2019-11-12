#include "CmdLine.h"
#include "AssertUtils.h"

static CmdLine::storage_type parse_unix_style(int argc, char const * const argv[]);
static CmdLine::storage_type parse_windows_style(const char * args);
static void test();

#if LIN || APL
	CmdLine::CmdLine(int argc, char const * const * argv) : m_options(parse_unix_style(argc, argv))
#else // Windows
	CmdLine::CmdLine(const char * cmdline) : m_options(parse_windows_style(cmdline))
#endif
{
	#if DEV
		test();
	#endif
}

bool CmdLine::has_option(const string & option) const
{
	return m_options.count(option) > 0;
}

string CmdLine::get_value(const string & option) const
{
	const auto found = m_options.find(option);
	return found == m_options.end() ? "" : found->second;
}


static CmdLine::storage_type parse_unix_style(int argc, char const * const * argv)
{
	CmdLine::storage_type out;
	for(int n = 1; n < argc; ++n)
	{
		const char * en = argv[n] + strlen(argv[n]);
		const char * eq = strchr(argv[n], '=');
		if(eq == NULL)
			eq = en;

		const string arg(argv[n], eq);
		if(!arg.empty())
		{
			if(eq && eq != en)
				eq++;
			
			string val(eq, en);
			if(val[0] == '"' && val[val.size() - 1] == '"') // If we got a quoted string from the CLI...
			{
				val = val.substr(1, val.size() - 2); // nuke the first and last chars
			}
			out.insert(make_pair(arg, val));
		}
	}
	return out;
}

static CmdLine::storage_type parse_windows_style(const char * cmdline)
{
	CmdLine::storage_type out;
	const char * p = cmdline;
	while(*p)
	{
		while(*p && isspace(*p))
			++p;
		const char * wb = p;
		const bool whole_arg_is_quoted = (*wb == '"'); // the Python "subprocess" module does this, rather than quoting the stuff after the =
		if(whole_arg_is_quoted)
			wb++; // skip that first quote
		while(*p && !isspace(*p) && *p != '=')
			++p;
		
		const char * we = p;
		const char * vb = p, * ve = p;

		if(*p == '=')
		{
			++p;
			if(*p == '"' || whole_arg_is_quoted) // read until the ending quote
			{
				if(*p == '"')
					++p;
				vb = p;
				while(*p && *p != '"') ++p;
				ve = p;
				++p;
			}
			else
			{
				vb = p;
				while(*p && !isspace(*p)) ++p;
				ve = p;
			}
		}

		const string arg(wb, we);
		if(!arg.empty())
		{
			const string val(vb, ve);
			out.insert(make_pair(arg, val));
		}
	}
	return out;
}

#if DEV
static void dump_options_map(const CmdLine::storage_type &m, const char * label=nullptr)
{
	if(label)
		printf("%s:\n", label);
	for(auto & kv : m)
	{
		if(kv.second.empty())
		{
			printf("  %s\n", kv.first.c_str());
		}
		else
		{
			printf("  %s => %s\n", kv.first.c_str(), kv.second.c_str());
		}
	}
}

struct test_case
{
	string windows_version;
	vector<const char *> unix_version;
	CmdLine::storage_type expected;
	
	bool test_passes() const
	{
		const auto windows_output = parse_windows_style(windows_version.c_str());
		const auto unix_output = parse_unix_style(unix_version.size(), unix_version.data());
		if(windows_output != expected || unix_output != expected)
		{
			printf("Test case failed\n");
			if(windows_output != expected)
			{
				dump_options_map(windows_output, "Windows output");
			}
			if(unix_output != expected)
			{
				dump_options_map(unix_output, "*nix output");
			}
			dump_options_map(expected, "Expected output");
			return false;
		}
		return true;
	}
};

static void test()
{
	const vector<test_case> test_cases = {
		{"", {}, {}}, // empty case
		// Handle both simple flags and options with values
		{"--foo --bar=baz",     {"app.sh", "--foo", "--bar=baz"},     {{"--foo", ""}, {"--bar", "baz"}} },
		// Quotes get stripped
		{"--foo --bar=\"baz\"", {"app.sh", "--foo", "--bar=\"baz\""}, {{"--foo", ""}, {"--bar", "baz"}} },
		// Handle spaces in quoted strings
		{"--foo=\"val with spaces\" --bar=baz --bang", {"app.sh", "--foo=\"val with spaces\"", "--bar=baz", "--bang"}, {{"--foo", "val with spaces"}, {"--bar", "baz"}, {"--bang", ""}} },
		// Preserves case
		{"--aBc=DeF --gHi=\"JkL mNo\" --pQr", {"app.sh", "--aBc=DeF", "--gHi=\"JkL mNo\"", "--pQr"}, {{"--aBc", "DeF"}, {"--gHi", "JkL mNo"}, {"--pQr", ""}} },
		// Short options
		{"-a -b -c=\"has spaces\"", {"app.sh", "-a", "-b", "-c=\"has spaces\""}, {{"-a", ""}, {"-b", ""}, {"-c", "has spaces"}} },
		
		// Absolutely requires an = to associate parameters.
		// This is a crummy limitation, and one we should fix eventually, but I put it here mostly to document it.
		// TODO: Support --key value instead of only --key=value
		{"-a b -c=\"has spaces\"", {"app.sh", "-a", "b", "-c=\"has spaces\""}, {{"-a", ""}, {"b", ""}, {"-c", "has spaces"}} },
	};
	const bool tests_passed = std::all_of(test_cases.begin(), test_cases.end(), [](const test_case & tc) { return tc.test_passes(); });
	DebugAssert(tests_passed);
}
#endif // DEV


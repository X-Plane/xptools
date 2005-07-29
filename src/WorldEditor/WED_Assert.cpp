#include "WED_Assert.h"
#include "AssertUtils.h"
#include "PlatformUtils.h"

static char gAssertBuf[65536];

void WED_AssertHandler_f(const char * condition, const char * file, int line)
{
	FILE * efile = fopen("error.out", "a");
	fprintf(efile ? efile : stderr, "ASSERTION FAILED: %s (%s, %d.)\n", condition, file, line);
	if (efile) fclose(efile);
	
	sprintf(gAssertBuf, "WorldEditor has hit an error due to a bug.  Please report the following to Ben:\n"
						"%s (%s, %d.)\n", condition, file, line);

	DoUserAlert(gAssertBuf);
	
	throw wed_assert_fail_exception(condition, file, line);
}

void	WED_AssertInit(void)
{
	InstallDebugAssertHandler(WED_AssertHandler_f);
	InstallAssertHandler(WED_AssertHandler_f);
}

#ifndef ASSERTUTILS_H
#define ASSERTUTILS_H

/************************************************************
 * ASSERTION MACROS
 ************************************************************/
 
/*
 * These macros trigger a handler when the condition evaluates
 * to false.  WARNING: DebugAssert is COMPILED OUT of the 
 * release build.  Use DebugAssert when you do not want the
 * condition run in final code!  Beware of putting code with
 * side effects in DebugAssert.
 *
 */

#if DEV

	#define DebugAssert(__Condition)		\
		((__Condition) ? ((void) 0) : __DebugAssertHandler(#__Condition, __FILE__, __LINE__))

#else

	#define DebugAssert(__Condition)

#endif

#define Assert(__Condition)				\
	((__Condition) ? ((void) 0) : __AssertHandler(#__Condition, __FILE__, __LINE__))

void	AssertPrintf(const char * fmt, ...);

void __DebugAssertHandler(const char *, const char *, int);
void __AssertHandler(const char *, const char *, int);

/************************************************************
 * ASSERTION HANDLERS
 ************************************************************/
 
/*
 * These functions let you 'hook' the assertion system.
 *
 */
 
typedef void (* AssertHandler_f)(const char * condition, const char * file, int line);

AssertHandler_f		InstallDebugAssertHandler(AssertHandler_f);
AssertHandler_f		InstallAssertHandler(AssertHandler_f);

/************************************************************
 * TESTING
 ************************************************************/

void	TEST_SetInteractive(bool);
bool	TEST_Handler(const char *, const char *, int);
#define TEST_Run(__Condition)	\
	(																\
		(__Condition) ? 											\
		((void) 0) : 												\
		(															\
			(TEST_Handler(#__Condition, __FILE__, __LINE__)) ? 		\
			(void) (__Condition) : 									\
			((void) 0)												\
		)															\
	)

#endif /* ASSERTUTILS_H */

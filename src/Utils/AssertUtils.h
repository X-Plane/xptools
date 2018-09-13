/*
 * Copyright (c) 2007, Laminar Research.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#ifndef ASSERTUTILS_H
#define ASSERTUTILS_H

#if __clang__
	#define ATTR_DISABLE_UB_SAN __attribute__((no_sanitize("undefined")))
#else
	#define ATTR_DISABLE_UB_SAN
#endif

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

	#define DebugAssertWithExplanation(__Condition, explanation_string) \
		((__Condition) ? ((void) 0) : __DebugAssertHandler(explanation_string, __FILE__, __LINE__))

	#define DebugAssertPrintf(...) AssertPrintf(__VA_ARGS__);

#else

	#define DebugAssert(__Condition)
	#define DebugAssertWithExplanation(__Condition, explanation_on_failure)
	#define DebugAssertPrintf(...) printf(__VA_ARGS__)

#endif

#define Assert(__Condition)				\
	((__Condition) ? ((void) 0) : __AssertHandler(#__Condition, __FILE__, __LINE__))

void	AssertPrintf(const char * fmt, ...);
void	AssertPrintfv(const char * fmt, va_list args);

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

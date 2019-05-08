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

#ifndef GISTOOL_UTILS_H
#define GISTOOL_UTILS_H

/*
 * GISTool_Command_f
 *
 * Command format: take a bunch of params (not including self), max count.
 * Return - 0 ok, 1 for err.
 *
 */
typedef int (* GISTool_Command_f)(const vector<const char *>& inParms);

struct	GISTool_RegCmd_t {
	const char *		cmdname;
	int					minp;
	int					maxp;
	GISTool_Command_f	cmd;
	const char *		help_short;
	const char *		help_long;
};

void	GISTool_RegisterCommand(
						const char *		inName,
						int					inMinParams,
						int					inMaxParams,
						GISTool_Command_f	inCommand,
						const char *		inHelpSummary,
						const char *		inHelpLong);

void	GISTool_RegisterCommands(
				const GISTool_RegCmd_t			cmds[]);

bool	GISTool_FindCommand(
						const char *		inName,
						int&				outMinParams,
						int&				outMaxParams,
						GISTool_Command_f&	outCommand);

bool	GISTool_IsCommand(
						const char *		inName);

void	GISTool_PrintHelpSummary(void);

int		GISTool_PrintHelpCommand(const char * inCommand);

int		GISTool_ParseCommands(const vector<const char *>& inArgs);

void	GISTool_SetSkip(int n);

#endif /* GISTOOL_UTILS_H */

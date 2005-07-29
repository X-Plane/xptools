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
				GISTool_RegCmd_t			cmds[]);
												
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

#endif /* GISTOOL_UTILS_H */

#include "GISTool_Utils.h"
#include <map>
#include "PerfUtils.h"
#include "GISTool_Globals.h"

struct	GISTool_CmdInfo_t {
	int						min_params;
	int						max_params;
	GISTool_Command_f		cmd;
	string					help_summary;
	string					help_long;
};

static map<string, GISTool_CmdInfo_t>		sCmds;

void	GISTool_RegisterCommand(
						const char *		inName,
						int					inMinParams,
						int					inMaxParams,
						GISTool_Command_f	inCommand,
						const char *		inHelpSummary,
						const char *		inHelpLong)
{
	if (sCmds.count(inName))
	{
		fprintf(stderr, "Internal error: trying to re-register %s\n", inName);
	} else {
		sCmds[inName].min_params = inMinParams;
		sCmds[inName].max_params = inMaxParams;
		sCmds[inName].cmd = inCommand;
		sCmds[inName].help_summary = inHelpSummary;
		sCmds[inName].help_long = inHelpLong;		
	}
}

void	GISTool_RegisterCommands(
				GISTool_RegCmd_t			cmds[])
{
	int n = 0;
	while (cmds[n].cmdname)
	{
		GISTool_RegisterCommand(
						cmds[n].cmdname,
						cmds[n].minp,
						cmds[n].maxp,
						cmds[n].cmd,
						cmds[n].help_short,
						cmds[n].help_long);
		++n;
	}
}
				

						
bool	GISTool_FindCommand(
						const char *		inName,
						int&				outMinParams,
						int&				outMaxParams,
						GISTool_Command_f&	outCommand)
{
	if (sCmds.count(inName) == 0) return false;	
	outMinParams = sCmds[inName].min_params;
	outMaxParams = sCmds[inName].max_params;
	outCommand = sCmds[inName].cmd;
	return true;
}

bool	GISTool_IsCommand(
						const char *		inName)
{
	return sCmds.count(inName) > 0;
}


void	GISTool_PrintHelpSummary(void)
{
	for (map<string, GISTool_CmdInfo_t>::iterator cmd = sCmds.begin(); cmd != sCmds.end(); ++cmd)
	{
		printf("%s - %s\n", cmd->first.c_str(), cmd->second.help_summary.c_str());
	}
}

int	GISTool_PrintHelpCommand(const char * inCommand)
{
	if (sCmds.count(inCommand) == 0)
	{
		printf("Command %s - unknown command.\n", inCommand);
		return 1;
	} else {
		printf("%s - %s\n%s\n", inCommand, 
			sCmds[inCommand].help_summary.c_str(),
			sCmds[inCommand].help_long.c_str());
	}
	return 0;
}

int	GISTool_ParseCommands(const vector<const char *>& args)
{
	int n = 0;
	GISTool_Command_f	cmd;
	int					minp;
	int					maxp;
	const char *		cname;
	while (n < args.size())
	{
		cname = args[n];	
		if (!GISTool_FindCommand(cname, minp, maxp, cmd))
		{
			fprintf(stderr, "Command %s not known.\n", cname);
			return 1;
		} else {
			printf("Doing: %s, verbose=%d\n",cname,gVerbose);
			++n;
			vector<const char *> cmdargs;
			while (n < args.size() && !GISTool_IsCommand(args[n]) && (maxp == -1 || cmdargs.size() < maxp))
			{
				cmdargs.push_back(args[n]);
				++n;
			}
			
			if (cmdargs.size() < minp || cmdargs.size() > maxp)
			{
				fprintf(stderr, "%s - needs %d-%d args, got %d args.\n",
					cname, minp, maxp, cmdargs.size());
				return 1;
			} else {
				StElapsedTime * timer = (gTiming ? new StElapsedTime(cname) : NULL);
				int result = cmd(cmdargs);
				delete timer;
				if (result != 0) return result;
			}
		}
	}
	return 0;
}

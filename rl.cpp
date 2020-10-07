#include <string>
#include "rl.h"

using namespace std;

void init_readline()
{
	// TODO: Configure...
}

bool rl_getline(string& str, const string& prompt = "")
{
	static char* psz_line;
	psz_line = readline(prompt.c_str());
	if (psz_line && *psz_line)
		add_history(psz_line);
	if (!psz_line)
		return false;
	str = psz_line;
	free(psz_line);
	return true;
}

// vim:ts=4:sw=4:noet:tw=80

#pragma once
#include <string>
#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>

using namespace std;

void init_readline();
bool rl_getline(string&, const string& prompt);


// vim:ts=4:sw=4:noet:tw=80

#include <iostream>
#include <map>
#include "parser.h"

using namespace std;

void run_repl()
{
	string line;
	symtbl syms;

	while (getline(cin, line)) {
		Lexer lxr{line};
		Parser p{lxr, syms};
		auto res = p();
		cout << ">> " << res << "\n% ";
		line.clear();
	}
}

int main()
{
	run_repl();

	return 0;
}

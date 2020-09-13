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
		try {
			auto res = p();
			// FIXME: Better way - overload output operator...
			if (holds_alternative<None>(res))
				cout << ">> (Null)\n% ";
			else {
				cout << ">> " << get<int>(res) << "\n% ";
				//cout << ">> " << get<double>(res) << "\n% ";
			}

		} catch(exception& e) {
			cout << "Error: " << e.what() << endl;
		}
		line.clear();
	}
}

int main()
{
	run_repl();

	return 0;
}

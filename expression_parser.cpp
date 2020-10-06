#include <iostream>
#include <map>
#include "parser.h"

using namespace std;

void run_repl()
{
	string line;
	symtbl syms;
	cout << "Expression Parser REPL\n% ";
	cout.setf(ios::showpoint);
	while (getline(cin, line)) {
		Lexer lxr{line};
		Parser p{lxr, syms};
		try {
			// Note: Handle possibility of multiple return values.
			// TODO: Play with rval refs...
			auto res = p();
			for (auto& v : res)
				cout << ">> " << v << "\n";
			// Note: The CR entered by user obviates need for a newline here.
			// FIXME: Look at output operator to see where extra linebreak is coming from...
			cout << "% ";

		} catch(exception& e) {
			cout << "Error: " << e.what() << "\n% ";
		}
		line.clear();
	}
}

int main()
{
	run_repl();

	return 0;
}

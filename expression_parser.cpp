#include <iostream>
#include <map>
#include "parser.h"

int main()
{
	symtbl syms {{"foo", 42}, {"baz", 100}, {"bammo", 10}};
	string s = "foo+5-10 *  (baz+(foo / bammo + blah))";
	//string s = "7+5-10 *  (3+(6 / 2))";

	Lexer lxr{s};
	Parser p{lxr, syms};
	cout << "Expression: " << s << endl;
	cout << "-- Symbol table --" << endl;
	for (auto el : syms)
		cout << el.first << ": " << el.second << endl;
	cout << "Result=" << p() << endl;

	return 0;
}

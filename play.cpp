#include <iostream>
#include <variant>
#include <unordered_map>
#include <vector>
#include <functional>
#include "parser.h"

int main()
{
	Dispatcher disp;
	Number result;
	vector<Number> d_args {100.0};
	vector<Number> dd_args {100.0, 200.0};
	//vector<long> l_args {1000};
	//vector<long> ll_args {1000, 2000};
	result = disp("foo", d_args);
	cout << "foo(100.0)=" << get<double>(result) << endl;
	result = disp("bar", dd_args);
	cout << "bar(100.0, 200.0)=" << get<double>(result) << endl;
	//disp("baz", l_args, result);
	//cout << "baz(1000, 2000)=" << result << endl;


	return 0;
}

// vim:ts=4:sw=4:noet:tw=80

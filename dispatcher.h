#ifndef _DISPATCHER_H_
#define _DISPATCHER_H_

#include <iostream>
#include <variant>
#include <unordered_map>
#include <vector>
#include <functional>
#include "parser_types.h"

using namespace std;

namespace {
	double foo(double x) { return 42 + x; }
	double bar(double x, double y) { return 43 + x + y; }
	long baz(long x) { return 44 + x; }
	unordered_map<string, Fv> fmap = {
		{ "foo", Fv{Fd_d{foo}} },
		{ "bar", Fv{Fd_dd{bar}} },
		{ "baz", Fv{Fl_l{baz}} },

	};

}

class Dispatcher {
	public:
	bool has(string name) { return !!fmap.count(name); }
	Number operator()(const string& name, const vector<Number>& args) {
		Number ret;
		// Look up function name to get type.
		if (!fmap.count(name))
			return ret;

		auto& fv = fmap[name];

		visit(overloaded {
				[&](Fd_d f) {
					ret = f(get<double>(args[0]));
				},
				[&](Fd_dd f) {
					ret = f(get<double>(args[0]), get<double>(args[1]));
				},
				[&](Fl_l f) {
					ret = f(get<long>(args[0]));
				},
		}, fv);

		return ret;
	}
	

};


#endif // _DISPATCHER_H_
// vim:ts=4:sw=4:noet:tw=80

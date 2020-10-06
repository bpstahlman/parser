#ifndef _DISPATCHER_H_
#define _DISPATCHER_H_

#include <iostream>
#include <variant>
#include <unordered_map>
#include <vector>
#include <unordered_map>
#include <functional>
#include "parser_types.h"
#include "overloaded.h"

using namespace std;

namespace {

}

class Dispatcher {
	public:
	bool has(string name) { return !!fmap.count(name); }
	Vararg operator()(const string& name, const vector<Vararg>& args) {
		Vararg ret;
		// Look up function name to get type.
		if (!fmap.count(name))
			return ret;

		auto& fv = fmap[name];

		visit(overloaded {
				[&](Fd_d f) {
					ret = f(cast_arg<double>(args[0]));
				},
				[&](Fd_dd f) {
					ret = f(cast_arg<double>(args[0]), cast_arg<double>(args[1]));
				},
				[&](Fb_d f) {
					ret = f(cast_arg<double>(args[0]));
				},
				[&](Fl_s f) {
					ret = f(cast_arg<string>(args[0]));
				},
				[&](Fi_ss f) {
					ret = f(cast_arg<string>(args[0]), cast_arg<string>(args[1]));
				},
				[&](Fb_ss f) {
					ret = f(cast_arg<string>(args[0]), cast_arg<string>(args[1]));
				},
				[&](FN_N f) {
					ret = Vararg{f(args[0])};
				},
		}, fv);

		return ret;
	}

	private:
	static unordered_map<string, Fv> fmap;
	

};


#endif // _DISPATCHER_H_
// vim:ts=4:sw=4:noet:tw=80

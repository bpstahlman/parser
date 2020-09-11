#include <iostream>
#include <variant>
#include <unordered_map>
#include <vector>
#include <functional>
#include "parser.h"

using namespace std;

namespace {
	using Fd_d = function<double(double)>;
	using Fd_dd = function<double(double, double)>;
	using Fl_l = function<long(long)>;
	using Fl_ll = function<long(long, long)>;
	using Fv = variant<Fd_d, Fd_dd, Fl_l, Fl_ll>;
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

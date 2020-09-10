#include <iostream>
#include <unordered_map>
#include <vector>
#include <functional>

using namespace std;

namespace {
	using Fd_d = function<double(double)>;
	using Fd_dd = function<double(double, double)>;
	using Fl_l = function<long(long)>;
	using Fl_ll = function<long(long, long)>;
	using Fv = variant<Fd_d, Fd_dd, Fl_l, Fl_ll>;
	double foo(double x) { return 42 + x; }
	double bar(double x) { return 43 + x; }
	double baz(double x) { return 44 + x; }
	enum class FType {
		d,
		d_d,
		d_d_d,
		i,
		i_i,
		i_i_i,
		l,
		l_l,
		l_l_l,
	};
	struct FDesc {
		FType ftyp;
		void (*fptr)();
	};
	unordered_map<string, Fv> fmap = {
		{ "sin", Fv{Fd_d{foo}} },
		{ "cos", Fv{Fd_d{bar}} },
		{ "tan", Fv{Fd_d{baz}} },

	};

}

class Dispatcher {
	public:
	bool operator()(const string& name, const vector<double>& args, double& res) {
		// Look up function name to get type.
		if (!fmap.count(name))
			return false;

		auto& fdesc = fmap[name];

		visit(overloaded {
				[](Fd_d f) {

				},
				[](Fd_dd f) {

				},
		}, fdesc);
#if 0
		switch (fdesc.ftyp) {
			case FType::d:
				res = (function<double(double)>(reinterpret_cast<double(*)(double)>(fdesc.fptr)))(static_cast<double>(args[0]));
				break;
			default:
				throw runtime_error("Internal error!\n");
		}
#endif

		return true;
	}
	

};


int main()
{
	Dispatcher disp;
	double result;
	vector<double> args {100.0};
	disp("sin", args, result);

	cout << "sin(100.0)=" << result << endl;


	return 0;
}

// vim:ts=4:sw=4:noet:tw=80

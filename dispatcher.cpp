#include <utility>
#include <cmath>

#include "dispatcher.h"

using namespace std;

unordered_map<string, Fv> Dispatcher::fmap {
	// Note: Use global namespace qualifier to avoid inline template
	// functions in math.h.
	{ "sin", Fv{Fd_d(::sin)} },
	{ "cos", Fv{Fd_d{::cos}} },
	{ "tan", Fv{Fd_d{::tan}} },
	{ "asin", Fv{Fd_d(::asin)} },
	{ "acos", Fv{Fd_d{::acos}} },
	{ "atan", Fv{Fd_d{::atan}} },
	{ "atan2", Fv{Fd_dd{::atan2}} },
	{ "sinh", Fv{Fd_d(::sinh)} },
	{ "cosh", Fv{Fd_d{::cosh}} },
	{ "tanh", Fv{Fd_d{::tanh}} },
	{ "asinh", Fv{Fd_d(::asinh)} },
	{ "acosh", Fv{Fd_d{::acosh}} },
	{ "atanh", Fv{Fd_d{::atanh}} },
	{ "exp", Fv{Fd_d{::exp}} },
	{ "exp2", Fv{Fd_d{::exp2}} },
	{ "log", Fv{Fd_d{::log}} },
	{ "log2", Fv{Fd_d{::log2}} },
	{ "log10", Fv{Fd_d{::log10}} },
	{ "pow", Fv{Fd_dd{::pow}} },
	{ "sqrt", Fv{Fd_d{::sqrt}} },
	{ "cbrt", Fv{Fd_d{::cbrt}} },
	{ "hypot", Fv{Fd_dd{::hypot}} },
	{ "ceil", Fv{Fd_d{::ceil}} },
	{ "floor", Fv{Fd_d{::floor}} },
	{ "fmod", Fv{Fd_dd{::fmod}} },
	{ "trunc", Fv{Fd_d{::trunc}} },
	{ "round", Fv{Fd_d{::round}} },
	{ "remainder", Fv{Fd_dd{::remainder}} },
	{ "fdim", Fv{Fd_dd{::fdim}} },
	{ "fmax", Fv{Fd_dd{::fmax}} },
	{ "fmin", Fv{Fd_dd{::fmin}} },
	{ "fabs", Fv{Fd_d{::fabs}} },
	// FIXME: Need to convert from double to Vararg with cast_arg<double>()
	{ "isfinite", Fv{Fb_d{[](double x) { return isfinite(x); }}} },
	{ "isinf", Fv{Fb_d{[](double x) { return isinf(x); }}} },
	{ "isnan", Fv{Fb_d{[](double x) { return isnan(x); }}} },
	{ "isnormal", Fv{Fb_d{[](double x) { return isnormal(x); }}} },
	{ "signbit", Fv{Fb_d{[](double x) { return signbit(x); }}} },
	// FIXME: Add cast functions to convert Vararg to a different type of Vararg.
	{ "long", Fv{FN_N{[](Vararg x) { return Vararg{cast_arg<long>(x)}; }}} },
	{ "double", Fv{FN_N{[](Vararg x) { return Vararg{cast_arg<double>(x)}; }}} },
	{ "str", Fv{FN_N{[](Vararg x) { return Vararg{cast_arg<string>(x)}; }}} },
	// String functions
	// FIXME: Other types? E.g., size_t so we don't need to cast to long?
	{ "length", Fv{Fl_s{[](string x) { return static_cast<long>(x.length()); }}} },
	{ "starts_with", Fv{Fb_ss{[](string x, string prefix) { return x.starts_with(prefix); }}} },
	{ "ends_with", Fv{Fb_ss{[](string x, string suffix) { return x.ends_with(suffix); }}} },
	{ "compare", Fv{Fi_ss{[](string x, string y) { return x.compare(y); }}} },
	
	

};

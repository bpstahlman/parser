#pragma once

#include <variant>
#include <functional>
#include <unordered_map>

using namespace std;

// TODO: Decide on home for these types. Perhaps a common header?
using Fd_d = function<double(double)>;
using Fd_dd = function<double(double, double)>;
using Fb_d = function<bool(double)>;

// Generic arg type
using Fv = variant<Fd_d, Fd_dd, Fb_d>;

// FIXME: Decide on home for this...
struct None {};
struct NaN {};
using Number = variant<None, bool, int, long, float, double>;

template<size_t I> using num_type_t = variant_alternative_t<I, Number>;

using symtbl = unordered_map<string, Number>;

// TODO: Define in class?
using parse_result = pair<bool, double>;
using list_result = pair<bool, vector<Number>>;
using numeric_result = pair<bool, Number>;


ostream& operator<<(ostream& os, Number x);
Number operator+(Number x, Number y);
Number operator-(Number x, Number y);
Number operator*(Number x, Number y);
Number operator/(Number x, Number y);

template <typename T>
void promote_and_set(Number& x, Number y, T val)
{
	auto idx = y.index();
	visit([&](auto&& yval) {
		using Ty = decay_t<decltype(yval)>;
		visit([&](auto&& xval) {
			if constexpr (is_same_v<None, Ty>) {
				throw runtime_error("Can't promote to type None");
				// Prevent compiler warning.
				x.emplace<None>(None{});
			} else {
				x.emplace<Ty>(static_cast<Ty>(val));
			}
		}, x);
	}, y);
}

template <typename R>
R cast_arg(Number x)
{
	return visit([&](auto&& val) {
		using T = decay_t<decltype(val)>;
		// TODO: Probably remove type None.
		if constexpr (is_same_v<None, T> || is_same_v<None, R>) {
			throw runtime_error("Can't cast to/from type None");
			// Prevent compiler warning.
			return R{};
		} else {
			return static_cast<R>(val);
		}
	}, x);
}


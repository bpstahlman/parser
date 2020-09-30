#pragma once

#include <variant>
#include <functional>
#include <unordered_map>
#include <charconv>
#include <iostream> // FIXME: Debug only!

using namespace std;

// FIXME: Decide on home for this...
struct None {};
struct NaN {};
// FIXME: Can't call this Number any more; maybe Arg or Var or Scalar?
using Number = variant<None, bool, int, long, float, double, string>;

// TODO: Decide on home for these types. Perhaps a common header?
using Fd_d = function<double(double)>;
using Fd_dd = function<double(double, double)>;
using Fb_d = function<bool(double)>;
using FN_N = function<Number(Number)>;

// Generic arg type
using Fv = variant<Fd_d, Fd_dd, Fb_d, FN_N>;

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
	visit([&](auto&& yval) {
		using Ty = decay_t<decltype(yval)>;
		visit([&](auto&& xval) {
			using Tx = decay_t<decltype(xval)>;
			if constexpr (is_same_v<Tx, Ty>) {
				// Nothing to do
				return;
			} else if constexpr (is_same_v<None, Ty>) {
				throw runtime_error("Can't promote to type None");
				// Prevent compiler warning.
				x.emplace<None>(None{});
			} else if constexpr (is_same_v<string, Tx>) {
				// Convert string to number.
				// FIXME!!!! Meaningful conversion
				x.emplace<Ty>(0);
			} else if constexpr (is_same_v<string, Ty>) {
				// Convert number to string.
				// FIXME!!!! Meaningful conversion
				x.emplace<Ty>("");
			} else {
				x.emplace<Ty>(static_cast<Ty>(val));
			}
		}, x);
	}, y);
}

// FIXME: Probably make this return Number of R, not actual R.
template <typename R>
R cast_arg(Number x)
{
	return visit([&](auto&& val) {
		using T = decay_t<decltype(val)>;
		// TODO: Probably remove type None.
		if constexpr (is_same_v<T, R>) {
			// No conversion necessary; return a copy.
			return get<R>(x);
		} else if constexpr (is_same_v<None, T> || is_same_v<None, R>) {
			throw runtime_error("Can't cast to/from type None");
			// Prevent compiler warning.
			return R{};
		} else if constexpr (is_same_v<T, string>) {
			// String to number
			R res;
			// TODO: Probably put this in a helper function.
			char *p_end;
			auto e_off = val.find_last_not_of(" \t");
			if (e_off == string::npos)
				// FIXME: No non-whitespace in string.
				return R{};
			++e_off; // make offset exclusive
			auto s_off = val.find_first_not_of(" \t");
			errno = 0;
			// Now, we've effectively discarded leading/trailing ws
			if constexpr (is_integral_v<R>)
				res = strtoul(val.c_str() + s_off, &p_end, 0);
			else
				res = strtod(val.c_str() + s_off, &p_end);
			auto m_off = p_end - val.c_str();
			if (m_off == s_off) {
				// FIXME: No conversion performed
				return R{};
			} else if (m_off < e_off) {
				// FIXME: String not fully consumed
			} else if (errno) {
				// FIXME: ERANGE - special/extreme values placed in res
			}
			return res;
		} else if constexpr (is_same_v<R, string>) {
			// Number to string
			// Assumption: Number can always be converted to string.
			return to_string(val);
		} else {
			return static_cast<R>(val);
		}
	}, x);
}

// vim:ts=4:sw=4:noet:tw=80

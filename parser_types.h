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

using symtbl = unordered_map<string, double>;

// FIXME: Decide on home for this...
struct None {};
struct NaN {};
using Number = variant<None, bool, int, long, float, double>;

template<size_t I> using num_type_t = variant_alternative_t<I, Number>;

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
	visit([&x,idx,val](auto&& curval) {
		switch (idx) {
			case 1:
				x.emplace<1>(static_cast<num_type_t<1>>(val));
				break;
			case 2:
				x.emplace<2>(static_cast<num_type_t<2>>(val));
				break;
			case 3:
				x.emplace<3>(static_cast<num_type_t<3>>(val));
				break;
			case 4:
				x.emplace<4>(static_cast<num_type_t<4>>(val));
				break;
			case 5:
				x.emplace<5>(static_cast<num_type_t<5>>(val));
				break;
		}
	}, x);
}



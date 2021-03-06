#include <ostream>
#include <iomanip>
#include "parser_types.h"
#include "overloaded.h"

using namespace std;

namespace {
};

ostream& operator<<(ostream& os, Vararg v)
{
	visit([&](auto&& x) {
		using T = decay_t<decltype(x)>;
		if constexpr (is_same_v<T, None>) {
			os << "<None>";
		} else {
			if constexpr (is_same_v<T, string>) {
				// Display strings properly (e.g., to be sure "42" and 42 are
				// visually distinct).
				os << quoted(x.c_str()); 
			} else {
				os << x;
			}
		}
	}, v);
	return os;
}

#ifdef CFG_NEED_PROMOTE
template<typename R>
R get_variant_val(const Vararg& x)
{
	return visit(overloaded {
			[](None y) {
				throw runtime_error("Can't cast None to number");
				// Prevent warning about unignored void return.
				return static_cast<R>(0);
			},
			[](bool y) {
				return static_cast<R>(y);
			},
			[](int y) {
				return static_cast<R>(y);
			},
			[](long y) {
				return static_cast<R>(y);
			},
			[](float y) {
				return static_cast<R>(y);
			},
			[](double y) {
				return static_cast<R>(y);
			},
	}, x);
}

static void promote(size_t idx, Vararg& v)
{
	visit([&,idx](auto&& x) {
		using T = decay_t<decltype(x)>;
		if constexpr (is_same_v<T, None>) {
			// FIXME!!!: Do we even need None type?
		throw runtime_error("Can't promote type None");
		} else {
			switch (idx) {
				case 1:
					v.emplace<1>(static_cast<num_type_t<1>>(x));
					break;
				case 2:
					v.emplace<2>(static_cast<num_type_t<2>>(x));
					break;
				case 3:
					v.emplace<3>(static_cast<num_type_t<3>>(x));
					break;
				case 4:
					v.emplace<4>(static_cast<num_type_t<4>>(x));
					break;
				case 5:
					v.emplace<5>(static_cast<num_type_t<5>>(x));
					break;
			}
		}
	}, v);
}
static void promote(Vararg& x, Vararg& y)
{
	auto idx = max(x.index(), y.index());
	promote(idx, x);
	promote(idx, y);
}
#endif


template<typename Oper>
static Vararg do_operation(Oper op, Vararg x, Vararg y)
{
	return visit([&](auto&& xval) {
		using Tx = decay_t<decltype(xval)>;
		return visit([&](auto&& yval) {
			using Ty = decay_t<decltype(yval)>;
			if constexpr (is_same_v<Tx, None> || is_same_v<Ty, None>) {
				return Vararg{};
			} else if constexpr (is_same_v<Tx, string> || is_same_v<Ty, string>) {
				// Note: plus<void> is the template specialization that deduces
				// the operand types.
				if constexpr (is_same_v<Oper, plus<>>) {
					// Apply binary arg to operands cast to string.
					return Vararg{op(cast_arg<string>(x), cast_arg<string>(y))};
				} else {
					// FIXME: Need operator - maybe do the type checking in caller?
					throw runtime_error("Invalid binary operator use on string");
					return Vararg{};
				}
			} else {
				// Let promotion be handled naturally by the operator.
				return Vararg{op(xval, yval)};
			}
		}, y);
	}, x);
}

// Unary operators
Vararg operator-(Vararg x)
{
	return visit([](auto&& x) {
		using T = decay_t<decltype(x)>;
		if constexpr (is_same_v<T, None>) {
			// FIXME!!!: Do we even need None type?
			throw runtime_error("Can't negate type None");
			return Vararg{None{}};
		} else if constexpr (is_same_v<T, string>) {
			throw runtime_error("Can't negate type string");
			return Vararg{None{}};
		} else {
			return Vararg{-x};
		}
	}, x);
}

// Binary operators
Vararg operator+(Vararg x, Vararg y)
{
	return do_operation(plus{}, x, y);
}
Vararg operator-(Vararg x, Vararg y)
{
	return do_operation(minus{}, x, y);
}
Vararg operator*(Vararg x, Vararg y)
{
	return do_operation(multiplies{}, x, y);
}
Vararg operator/(Vararg x, Vararg y)
{
	return do_operation(divides{}, x, y);
}


#undef TEST
#ifdef TEST
#include <iostream>
int main()
{
	Vararg x{true}, y{42}, z{100.0};

	Vararg res;
	res = x * y;
	cout << "x * y: " << res.index() << " res=" << res << endl;
	res = y + z;
	cout << "y + z: " << res.index() << " res=" << res << endl;
	res = z - y;
	cout << "z - y: " << res.index() << " res=" << res << endl;
	res = z / y;
	cout << "z / y: " << res.index() << " res=" << res << endl;
	return 0;
}
#endif
// vim:ts=4:sw=4:noet:tw=80

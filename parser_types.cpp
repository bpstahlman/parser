#include <ostream>
#include "parser_types.h"
#include "overloaded.h"

ostream& operator<<(ostream& os, Number v)
{
	visit([&](auto&& x) {
		using T = decay_t<decltype(x)>;
		if constexpr (is_same_v<T, None>) {
			os << "<None>";
		} else {
			os << x;
		}
	}, v);
	return os;
}

template<typename R>
R get_variant_val(const Number& x)
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

static void promote(size_t idx, Number& v)
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
static void promote(Number& x, Number& y)
{
	auto idx = max(x.index(), y.index());
	promote(idx, x);
	promote(idx, y);
}

template<typename Oper>
static Number do_operation(Oper op, Number x, Number y)
{
	// Promote numbers to common type.
	promote(x, y);
	switch (x.index()) {
		case 1:
			return op(get_variant_val<num_type_t<1>>(x), get_variant_val<num_type_t<1>>(y));
		case 2:
			return op(get_variant_val<num_type_t<2>>(x), get_variant_val<num_type_t<2>>(y));
		case 3:
			return op(get_variant_val<num_type_t<3>>(x), get_variant_val<num_type_t<3>>(y));
		case 4:
			return op(get_variant_val<num_type_t<4>>(x), get_variant_val<num_type_t<4>>(y));
		case 5:
			return op(get_variant_val<num_type_t<5>>(x), get_variant_val<num_type_t<5>>(y));
		default:
			// FIXME!
			return Number{};
	}

}

Number operator+(Number x, Number y)
{
	return do_operation(plus{}, x, y);
}
Number operator-(Number x, Number y)
{
	return do_operation(minus{}, x, y);
}
Number operator*(Number x, Number y)
{
	return do_operation(multiplies{}, x, y);
}
Number operator/(Number x, Number y)
{
	return do_operation(divides{}, x, y);
}


#undef TEST
#ifdef TEST
#include <iostream>
int main()
{
	Number x{true}, y{42}, z{100.0};

	Number res;
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
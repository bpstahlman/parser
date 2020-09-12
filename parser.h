#include <iostream>
#include <iterator>
#include <variant>
#include <exception>
#include "lexer.h"
#include "dispatcher.h"

using namespace std;

// -- Expression Grammar --
// exprs          -> assign_or_expr ( ',' assign_or_expr )*
// assign_or_expr -> (symbol =)? expr
// expr           -> term ('+' term)?
// term           -> term ('*' factor)?
// factor         -> '(' expr ')'
//                 | number
//
// Add iterator trait
class Parser {
	public:
	Parser(Lexer &lexer_in, symtbl& symbols_in) :
		lexer{lexer_in}, symbols{symbols_in} {}

	// FIXME: Decide on return value.
	vector<Number> exprs(Lexer::LexIter& it, Lexer::LexIter& ite, bool scalar_ctx = false);
	numeric_result assign_or_expr(Lexer::LexIter& it, Lexer::LexIter& ite);
	numeric_result expr(Lexer::LexIter& it, Lexer::LexIter& ite);
	numeric_result term(Lexer::LexIter& it, Lexer::LexIter& ite);
	numeric_result factor(Lexer::LexIter& it, Lexer::LexIter& ite);
	Number operator()();

	private:
	// FIXME: Decide about constness of reference, and implications for
	// obtaining iterators. E.g., ok to get const iterator from Lexer?
	Lexer& lexer;
	symtbl& symbols;
	Dispatcher dispatcher;

};
Number Parser::operator()() {
	auto it {lexer.begin()}, ite {lexer.end()};
	try {
		// FIXME: Make sure nothing remains after top-level...
		// FIXME: Decide where to check for empty expression.
		auto values = exprs(it, ite, true);
		if (values.empty())
			return Number{};
		// FIXME: This is a temporary kludge... Need to cast other types at least...
		return get<double>(values.back());
	} catch(runtime_error& e) {
		cout << "Error parsing expression: " << e.what() << endl;
		throw;
	}
}

// FIXME: Decide on return value. RVO should help, but do we want
// Fv<vector<LexVariant>>?.
vector<Number> Parser::exprs(Lexer::LexIter& it, Lexer::LexIter& ite, bool scalar)
{
	// FIXME: Decide whether to use the scalar arg and remove if not.
	vector<Number>values;
	// exprs          -> assign_or_expr ( ',' assign_or_expr )*
	if (it == ite)
		// Special case: Call to assign_or_expr() at eof throws by design, so if
		// we know we're already at eof (e.g., in empty input case), skip call
		// to assign_or_expr and let caller decide whether empty list is ok.
		return values;
	while (true) {
		auto [success, value] = assign_or_expr(it, ite);
		if (!success) {
			if (values.empty())
				// Note: Let caller determine whether empty list is exceptional.
				break;
			throw runtime_error("Expected term!");
		}

		// Accumulate
		// FIXME: Eventually, assign_or_expr will *return* Fv in lieu of double.
		values.push_back(value);

		if (it != ite) {
			// Check for `,' signifying presence of another element.
			if (holds_alternative<Comma>(*it)) {
				++it; // Advance past comma operator to subsequent expr
				continue;
			}
		}
		break;
	}

	return move(values);
}

numeric_result Parser::assign_or_expr(Lexer::LexIter& it, Lexer::LexIter& ite)
{
	// assign_or_expr -> (symbol =)? expr
	
	string sym_name;
	if (auto* sym = get_if<Sym>(&*it)) {
		sym_name = (string)*sym;
		++it; // move past symbol
		if (it != ite && holds_alternative<Eq>(*it))
			++it; // move past `='
		else {
			// Prevent assignment.
			sym_name.clear();
			--it; // let symbol be parsed as expr
		}
	}

	auto [success, value] = expr(it, ite);
	if (!success)
		throw runtime_error("Expected expr!");

	// Perform assignment if applicable.
	if (!sym_name.empty())
		// TODO: Probably store Number, not double.
		symbols[sym_name] = get<double>(value);

	return make_pair(true, value);

}

numeric_result Parser::expr(Lexer::LexIter& it, Lexer::LexIter& ite)
{
	// TODO: Consider performing arithmetic with Numbers', with operator
	// overloads handling promotion.
	double ret{};
	auto opc = '+'; // implied addition of first term
	while (true) {
		// expr           -> term ('+' term)?
		auto [success, value] = term(it, ite);
		if (!success)
			throw runtime_error("Expected term!");

		// Accumulate
		if (opc == '+')
			ret += get<double>(value);
		else
			ret -= get<double>(value);

		if (it != ite) {
			// Check for + or - operator
			if (auto* op = get_if<Op>(&*it)) {
				opc = ((string)*op)[0];
				// FIXME: mul/div terms would have been gobbled up by term, so
				// this test is probably unnecessary.
				if (opc == '+' || opc == '-') {
					++it; // Advance past operator to subsequent expr
					continue;
				}
			}
		}
		break;
	}

	return make_pair(true, ret);

}

numeric_result Parser::term(Lexer::LexIter& it, Lexer::LexIter& ite)
{
	double ret{1};
	auto opc = '*'; // implied addition of first term
	// term           -> term ('*' factor)?
	while (true) {
		auto [success, value] = factor(it, ite);
		if (!success)
			throw runtime_error("Expected factor!");

		// Accumulate
		if (opc == '*')
			ret *= get<double>(value);
		else
			ret /= get<double>(value);

		if (it != ite) {
			// Check for * or / operator.
			if (auto* op = get_if<Op>(&*it)) {
				opc = ((string)*op)[0];
				// Note: Just fall through on add/sub.
				// TODO: Make add/sub mul/div different types to simplify logic.
				if (opc == '*' || opc == '/') {
					++it; // Advance past operator to subsequent term
					continue;
				}
			}
		}
		break;
	}

	// FIXME: Are the pairs even necessary, given that first value will always
	// be true in non-exceptional case?
	return make_pair(true, ret);

}

numeric_result Parser::factor(Lexer::LexIter& it, Lexer::LexIter& ite)
{
	double ret;
	// factor         -> '(' expr ')'
	//                 | number
	//                 | sym
	//                 | sym '(' exprs ')'

	if (holds_alternative<Lp>(*it)) {
		// '(' expr ')'
		++it;
		auto values = exprs(it, ite, true);
		if (values.empty())
			throw runtime_error("Expected expr!");
		// FIXME: Need to extract the final element as long as exprs scalar arg is unimplemented.
		// FIXME: Don't hardcode double?
		ret = get<double>(values.back()); // FIXME
		// Require closing paren.
		if (!holds_alternative<Rp>(*it))
			throw runtime_error("Missing ')'!");
	} else if (int* i = get_if<int>(&*it)) {
		ret = (double)*i;
	} else if (double* d = get_if<double>(&*it)) {
		ret = *d;
	} else if (Sym* sym = get_if<Sym>(&*it)) {
		string sym_name = (string)*sym;
		// Important Note: Vars and Functions exist in distinct namespaces, with
		// the presence of postcircumfix parens used to differentiate.
		if (it + 1 != ite && holds_alternative<Lp>(*(it + 1))) {
			// Looks like function call.
			it += 2; // move past symbol and `('
			if (dispatcher.has(sym_name)) {
				// Get the args.
				auto values = exprs(it, ite);
				if (!holds_alternative<Rp>(*it))
					throw runtime_error("Missing ')'!");
				// Call the function with args.
				auto v = dispatcher(sym_name, values);
				// FIXME: Revisit return mechanism...
				ret = get<double>(v);
			} else
				throw runtime_error("Unknown function `"s + sym_name);
		} else if (auto p = symbols.find(sym_name); p != symbols.end()) {
			// Variable expansion
			ret = p->second;
		} else {
			// Neither var nor function.
			throw runtime_error("Undefined symbol `"s + sym_name + "'");
		}
	}
	++it;

	return make_pair(true, ret);
}

// vim:ts=4:sw=4:noet:tw=80

#include <iostream>
#include <iterator>
#include <variant>
#include <exception>
#include "lexer.h"

using namespace std;

using symtbl = map<string, double>;

// Anon namespace
namespace {

	// TODO: Define in class?
	using parse_result = pair<bool, double>;
}

// -- Expression Grammar --
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

	parse_result assign_or_expr(Lexer::LexIter& it, Lexer::LexIter& ite);
	parse_result expr(Lexer::LexIter& it, Lexer::LexIter& ite);
	parse_result term(Lexer::LexIter& it, Lexer::LexIter& ite);
	parse_result factor(Lexer::LexIter& it, Lexer::LexIter& ite);
	double operator()();

	private:
	// FIXME: Decide about constness of reference, and implications for
	// obtaining iterators. E.g., ok to get const iterator from Lexer?
	Lexer& lexer;
	symtbl& symbols;

};
double Parser::operator()() {
	auto it {lexer.begin()}, ite {lexer.end()};
	try {
		// FIXME: Make sure nothing remains after top-level...
		auto [success, value] = assign_or_expr(it, ite);
		return value;
	} catch(runtime_error& e) {
		cout << "Error parsing expression: " << e.what() << endl;
		throw;
	}
}

parse_result Parser::assign_or_expr(Lexer::LexIter& it, Lexer::LexIter& ite)
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
		symbols[sym_name] = value;

	return make_pair(true, value);

}

parse_result Parser::expr(Lexer::LexIter& it, Lexer::LexIter& ite)
{
	double ret{};
	auto opc = '+'; // implied addition of first term
	while (true) {
		// expr           -> term ('+' term)?
		auto [success, value] = term(it, ite);
		if (!success)
			throw runtime_error("Expected term!");

		// Accumulate
		if (opc == '+')
			ret += value;
		else
			ret -= value;

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

parse_result Parser::term(Lexer::LexIter& it, Lexer::LexIter& ite)
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
			ret *= value;
		else
			ret /= value;

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

parse_result Parser::factor(Lexer::LexIter& it, Lexer::LexIter& ite)
{
	double ret;
	// factor         -> '(' expr ')'
	//                 | number

	if (auto* lp = get_if<Lp>(&*it)) {
		// '(' expr ')'
		++it;
		auto [success, value] = assign_or_expr(it, ite);
		if (!success)
			throw runtime_error("Expected expr!");
		if (!get_if<Rp>(&*it))
			throw runtime_error("Missing ')'!");
		ret = value;
	} else if (int* i = get_if<int>(&*it)) {
		ret = (double)*i;
	} else if (double* d = get_if<double>(&*it)) {
		ret = *d;
	} else if (Sym* sym = get_if<Sym>(&*it)) {
		string sym_name = (string)*sym;
		// TODO: Look up in sym table.
		auto p = symbols.find(sym_name);
		if (p == symbols.end())
			throw runtime_error("Undefined symbol `"s + sym_name + "'");
		ret = p->second;
	}
	++it;

	return make_pair(true, ret);
}

// vim:ts=4:sw=4:noet:tw=80

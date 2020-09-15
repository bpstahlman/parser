#include <iostream>
#include <iterator>
#include <variant>
#include <exception>
#include "lexer.h"
#include "dispatcher.h"
#include "parser_types.h"

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
	vector<Number> exprs(Lexer::iterator& it, Lexer::iterator& ite, bool scalar_ctx = false);
	numeric_result assign_or_expr(Lexer::iterator& it, Lexer::iterator& ite);
	numeric_result expr(Lexer::iterator& it, Lexer::iterator& ite);
	numeric_result term(Lexer::iterator& it, Lexer::iterator& ite);
	numeric_result factor(Lexer::iterator& it, Lexer::iterator& ite);
	vector<Number> operator()();

	private:
	// FIXME: Decide about constness of reference, and implications for
	// obtaining iterators. E.g., ok to get const iterator from Lexer?
	Lexer& lexer;
	symtbl& symbols;
	Dispatcher dispatcher;

};
// vim:ts=4:sw=4:noet:tw=80

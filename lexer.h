#pragma once

#include <iostream>
#include <iterator>
#include <regex>
#include <variant>
#include <exception>
#include <string>
#include "overloaded.h"

using namespace std;
using namespace regex_constants;

enum class TokType : unsigned long {
	NONE, OP, SYM, INT, FLT, STR, CHR, LP, RP, EQ, COMMA
};

struct Op {
	Op(string op_in) : op{op_in} {}
	operator string() const { return op; }
	string op;
};

struct Sym {
	Sym(string n) : name{n} {}
	operator string() const { return name; }
	string name;
};

struct Chr { char chr; };
struct Lp {};
struct Rp {};
struct Eq {};
struct Comma {};
using LexVariant = variant<Op, Sym, int, double, string, /*Chr,*/ Lp, Rp, Eq, Comma>;

ostream& operator<<(ostream& os, TokType tt);
ostream& operator<<(ostream& os, const LexVariant& lv);

// forward decl needed for reference in LexIter
class Lexer;

// TODO: Consider breaking this into its own files...
struct LexIter {
	// TODO: Should we make end() return a nonnegative sentinel?
	// Note: Lexer& complicates creating default iterator; consider making
	// lexer a shared pointer. This would have other advantages...
	//LexIter()
	LexIter(Lexer& lexer_in, int idx = -1) : lexer{lexer_in}, i{idx} {}
	LexIter operator+(int idx) { return LexIter {this->lexer, i + idx}; }
	LexIter operator-(int idx) { return LexIter {this->lexer, i - idx}; }
	LexIter& operator+=(int idx) { i += idx; return *this; }
	LexIter& operator-=(int idx) { i -= idx; return *this; }
	LexIter operator++(int) {
		// Defer offset validation.
		// Note: Auto post-increment changes this's index but returns
		// iterator with the old index.
		return LexIter {lexer, i++};
	}
	LexIter& operator++();
	LexIter& operator--();
	LexVariant& operator*();
	bool operator!=(const LexIter& rhs);
	bool operator==(const LexIter& rhs);

	Lexer& lexer;
	int i;
};

// Add iterator trait
class Lexer {
	public:
	friend class LexIter;
	using iterator = LexIter;

	Lexer() = delete;
	Lexer(const string& expr) :
		beg_it{expr.begin()}, cur_it{expr.begin()}, end_it{expr.end()} {}

	LexIter begin() { return LexIter{*this, 0}; }
	LexIter end() { return LexIter{*this}; }

	// Ensure the string is lexed through the idx'th token, returning true
	// unless there are fewer than idx+1 tokens.
	bool materialize_to(int idx);
	LexVariant& operator[](size_t i);
	
	// Convert regex submatch index to the correct variant and append to toks
	// collection.
	void add_var(TokType idx, string tok);

	// Update string iterator to skip past any whitespace at current location.
	string::const_iterator skip_ws();

	// Lex another token and append it to the toks collection, returning false
	// if there are no more tokens.
	bool next();

	private:
	vector<LexVariant> toks {};
	string::const_iterator beg_it, end_it, cur_it, cur_next;
	bool eof {false};

};

// vim:ts=4:sw=4:noet:tw=80

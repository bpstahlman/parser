#include <iostream>
#include <iterator>
#include <regex>
#include <variant>
#include <exception>

using namespace std;
using namespace regex_constants;

enum class TokType : unsigned long {
	NONE, OP, SYM, INT, FLT, LP, RP, EQ
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

struct Lp {};
struct Rp {};
struct Eq {};
struct None {};
using LexVariant = variant<None, Op, Sym, int, double, Lp, Rp, Eq>;

// Anon namespace
namespace {

	// TODO: Consider making this class static; to do so, would need to make
	// class methods non-inline.
	struct {
		TokType type;
		regex re;
	} re_toks[] = {
		{TokType::OP,  regex{R"([-+/*])"}},
		{TokType::SYM, regex{R"(\b([[:alpha:]]+))"}},
		{TokType::INT, regex{R"(\b(0|[1-9][0-9]*))"}},
		// FIXME: Make this true float format...
		{TokType::FLT, regex{R"(\b(0|[1-9][0-9]*))"}},
		{TokType::LP,  regex{R"(\()"}},
		{TokType::RP,  regex{R"(\))"}},
		{TokType::EQ,  regex{R"(=)"}}
	};
}

ostream& operator<<(ostream& os, TokType tt)
{
    os << (tt == TokType::OP
			? "operator"
			: tt == TokType::SYM
			? "symbol"
			: tt == TokType::INT
			? "integer"
			: tt == TokType::FLT
			? "float"
			: tt == TokType::LP
			? "left paren"
			: tt == TokType::RP
			? "right paren"
			: tt == TokType::EQ
			? "equals"
			: "unknown");
    return os;
}

ostream& operator<<(ostream& os, const LexVariant& lv)
{
	switch (static_cast<TokType>(lv.index())) {
		case TokType::OP:
			//cout << "Yep" << endl;
			cout << "Operator: "s << static_cast<string>(get<Op>(lv));
			break;
		case TokType::SYM:
			cout << "Symbol: "s + static_cast<string>(get<Sym>(lv));
			break;
		case TokType::INT:
			cout << "Integer: "s << get<int>(lv);
			break;
		case TokType::FLT:
			cout << "Float: "s << get<double>(lv);
			break;
		case TokType::LP:
			cout << "'('";
			break;
		case TokType::RP:
			cout << "')'";
			break;
		case TokType::EQ:
			cout << "'='";
			break;
		default:
			cout << "Unknown Lex variant!\n";
	}
	return os;
}

// Add iterator trait
class Lexer {
	public:

	Lexer() = delete;
	Lexer(const string& expr) :
		beg_it{expr.begin()}, cur_it{expr.begin()}, end_it{expr.end()} {}

	struct LexIter {
		// TODO: Should we make end() return a nonnegative sentinel?
		// Note: Lexer& complicates creating default iterator; consider making
		// lexer a shared pointer. This would have other advantages...
		//LexIter()
		LexIter(Lexer& lexer_in, int idx = -1) : lexer{lexer_in}, i{idx} {}
		LexIter operator+(int idx) {
			return LexIter {this->lexer, i + idx};
		}
		LexIter operator-(int idx) {
			return LexIter {this->lexer, i - idx};
		}
		LexIter& operator+=(int idx) { i += idx; return *this; }
		LexIter& operator-=(int idx) { i -= idx; return *this; }
		LexIter operator++(int) {
			// Defer offset validation.
			// Note: Auto post-increment changes this's index but returns
			// iterator with the old index.
			return LexIter {lexer, i++};
		}
		LexIter& operator++() {
			// Defer offset validation.
			++i;
			return *this;
		}
		LexIter& operator--() {
			// Defer offset validation.
			--i;
			return *this;
		}
		LexVariant& operator*() {
			if (i < 0)
				throw runtime_error("Invalid access to end!"s); // FIXME!!!
			// Note: Lexer's operator[] may throw.
			return lexer[i];
		}
		bool operator!=(const LexIter& rhs) {
			return !(*this == rhs);
		}
		bool operator==(const LexIter& rhs) {
			if (rhs.i >= 0) {
				// Intentionally not validating either index.
				return rhs.i == i;
			} else {
				// rhs points past end, so see whether lhs does as well.
				return !lexer.materialize_to(i);
			}
		}
		Lexer& lexer;
		int i;
	};

	LexIter begin() {
		return LexIter{*this, 0};
	}
	LexIter end() {
		return LexIter{*this};
	}

	// Ensure the string is lexed through the idx'th token, returning true
	// unless there are fewer than idx+1 tokens.
	bool materialize_to(int idx) {
		size_t len = toks.size();
		while (!eof && len++ <= idx) {
			// This may throw.
			if (!next())
				return false;
		}
		return len > idx;
	}

	LexVariant& operator[](size_t i) {
		// Ensure we've parsed through element i.
		if (!materialize_to(i))
			throw runtime_error("Invalid access!"s); // FIXME!!!
		return toks[i];
	}
	
	// Convert regex submatch index to the correct variant and append to toks
	// collection.
	void add_var(TokType idx, string tok)
	{
		switch (idx) {
			case TokType::OP:
				toks.emplace_back(static_cast<Op>(tok));
				break;
			case TokType::SYM:
				toks.emplace_back(static_cast<Sym>(tok));
				break;
			case TokType::INT:
				toks.emplace_back(stoi(tok));
				break;
			case TokType::FLT:
				toks.emplace_back(stod(tok));
				break;
			case TokType::LP:
				toks.emplace_back(Lp{});
				break;
			case TokType::RP:
				toks.emplace_back(Rp{});
				break;
			case TokType::EQ:
				toks.emplace_back(Eq{});
				break;
			default:
				throw runtime_error("Invalid token type!"s); // FIXME!!!
		}
	}

	// Update string iterator to skip past any whitespace at current location.
	string::const_iterator skip_ws() {
		static regex re_ws{R"(\s+)"};
		smatch m;

		return regex_search(cur_it, end_it, m, re_ws, match_continuous)
			? m[0].second
			: cur_it;

	}

	// Lex another token and append it to the toks collection, returning false
	// if there are no more tokens.
	bool next() {
		smatch m;
		// match_prev_avail flag allows consideration of leading context if not
		// at beginning, and match_continuous requires match to begin at cur_it.
		match_flag_type mflags = match_continuous |
			(cur_it != beg_it ? match_prev_avail : match_default);

		// Is the input exhausted?
		if (cur_it == end_it) {
			// TODO: Consider use of variant exceptional value.
			eof = true;
			return false;
		}

		// Try each token type looking for match.
		for (const auto& td : re_toks) {
			// Attempt match.
			if (regex_search(cur_it, end_it, m, td.re, mflags)) {
				// Found match!
				add_var(td.type, m[0].str());
				break;
			}
		}

		// Normal EOF should have been caught above. Only invalid token can get
		// us here.
		if (m.empty())
			throw runtime_error("Parse error: "s + string(cur_it, end_it));
		
		// Found a matching token.
		cur_it = m[0].second;
		// Skip trailing whitespace.
		cur_it = skip_ws();

		return true;
	}

	private:
	vector<LexVariant> toks {};
	string::const_iterator beg_it, end_it, cur_it, cur_next;
	bool eof {false};

};

template<typename... Ts> struct overloaded : Ts... {
	using Ts::operator()...;
};
template<typename... Ts> overloaded(Ts...) -> overloaded<Ts...>;

static void parse_with_overloaded(string expr)
{
	// TODO: Convert from stream to beg/end iterators.
	cout << "Expression string: " << expr << endl;
	Lexer lxr{expr};
	for (auto li {lxr.begin()}, end {lxr.end()}; li != end; ++li) {
		visit(overloaded {
				[](Op op) {
					cout << "Op: " << static_cast<string>(op) << endl;
				},
				[](Sym sym) {
					cout << "Sym: " << static_cast<string>(sym) << endl;
				},
				[](int i) {
					cout << "Int: " << i << endl;
				},
				[](double d) {
					cout << "Double: " << d << endl;
				},
				[](Lp lp) {
					cout << "Left paren" << endl;
				},
				[](Rp rp) {
					cout << "Right paren" << endl;
				},
				[](Eq eq) {
					cout << "Equals" << endl;
				},
				[](None none) {
					cout << "None!\n";
				}
			}, *li);

	}
}

static void parse(string expr)
{
	cout << expr << endl;
	Lexer lxr{expr};
	for (auto li {lxr.begin()}, end{lxr.end()}; li != end; ++li) {
		cout << *li << endl;
	}

	auto li{lxr.begin()};
	cout << "*(li+3)=" << *(li+3) << endl;
	li += 5;
	cout << "*(li-1)=" << *(li-1) << endl;

}

// vim:ts=4:sw=4:noet:tw=80

#include <iostream>
#include <iterator>
#include <queue>
#include <regex>
#include <variant>
#include <exception>

using namespace std;
using namespace regex_constants;

enum class TokType : unsigned long {
	NONE, OP, SYM, INT, LP, RP
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

struct LP {};
struct RP {};
struct None {};
using LexVariant = variant<None, Op, Sym, int, LP, RP>;

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
		{TokType::LP,  regex{R"(\()"}},
		{TokType::RP,  regex{R"(\))"}}
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
			: tt == TokType::LP
			? "left paren"
			: tt == TokType::RP
			? "right paren"
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
		case TokType::LP:
			cout << "'('";
			break;
		case TokType::RP:
			cout << "')'";
			break;
		default:
			cout << "Unknown Lex variant!\n";
	}
	return os;
}

// Add iterator trait
class LexIter {
	public:

	LexIter(const LexIter& rhs) = default;
	LexIter() = default;

	bool operator!=(const LexIter& rhs) {
		return !(*this == rhs);
	}
	bool operator==(const LexIter& rhs) {
		// Make sure we're not already at EOF.
		set_cur_match();

		// Defer to the string iterators
		// Caveat: Can't compare current iterator with string::const_iterator{}
		// because str.end() returns an actual pointer to one past end.
		return cur == rhs.cur || cur == end && rhs.cur == decltype(rhs.cur){};
	}
	
	// Convert regex submatch index to variant of 
	// TODO: Should tok be const& or rval???
	LexVariant create_var(TokType idx, string tok)
	{
		switch (idx) {
			case TokType::OP:
				return LexVariant(static_cast<Op>(tok));
			case TokType::SYM:
				return LexVariant(static_cast<Sym>(tok));
			case TokType::INT:
				return LexVariant(stoi(tok));
			case TokType::LP:
				return LexVariant{LP{}};
			case TokType::RP:
				return LexVariant{RP{}};
			default:
				return LexVariant{};
		}
	}

	string::const_iterator skip_ws() {
		static regex re_ws{R"(\s+)"};
		smatch m;

		return regex_search(cur, end, m, re_ws) ? m[0].second : cur;

	}

	// Find match at current position and set cur_var accordingly.
	void set_cur_match() {
		smatch m;
		// match_prev_avail flag allows consideration of leading context if not
		// at beginning, and match_continuous requires match to begin at cur.
		match_flag_type mflags =
			cur != beg ? match_prev_avail | match_continuous : match_default;

		// Is the input exhausted?
		if (cur == end) {
			// TODO: Consider use of variant exceptional value.
			cur_var = None{};
			return;
		}

		// Try each token type looking for match.
		for (auto& td : re_toks) {
			// Attempt match.
			if (regex_search(cur, end, m, td.re, mflags)) {
				// Found match!
				cur_var = create_var(td.type, m[0].str());
				break;
			}
		}

		if (m.empty())
			throw runtime_error("Parse error: "s + string(cur, end));
		
		// Found a matching token.
		cur = m[0].second;
		// Skip trailing whitespace.
		cur = skip_ws();

		// FIXME!!!: Do we need to test for eof here or will it be caught
		// naturally by auto-increment???
	}

	bool is_eos() {
		return cur == string::const_iterator{};
	}

	bool has_cur_match() {
		return !holds_alternative<None>(cur_var);
	}

	LexVariant operator*() {
		// Use cached variant if it exists.
		if (!has_cur_match())
			set_cur_match();

		return cur_var;
	}

	LexIter& operator++() {
		if (!has_cur_match())
			set_cur_match();

		// Invalidate existing match.
		cur_var = None{};
		//cout << "ptr_diff=" << (end - cur) << endl;
		return *this;
	}

	// TODO: Add post-increment ++: No! Implementation would be complicated for
	// current approach...

	LexIter(string::const_iterator s, string::const_iterator e) :
		beg{s}, cur{s}, end{e}
	{
	}


	private:
	static regex re_tok;
	//static tok_def re_toks[];
	vector<string::const_iterator> hist;
	string::const_iterator beg, end, cur, cur_next;
	LexVariant cur_var;

};
regex LexIter::re_tok {
	R"(\s*(?:)"
	R"(([-+/*]))"
	R"(|\b([[:alpha:]]+))"
	R"(|\b(0|[1-9][0-9]*))"
	R"(|(\())"
	R"(|(\)))"
	R"())"
};

template<typename... Ts> struct overloaded : Ts... {
	using Ts::operator()...;
};
template<typename... Ts> overloaded(Ts...) -> overloaded<Ts...>;

static void parse_with_overloaded(string expr)
{
	// TODO: Convert from stream to beg/end iterators.
	cout << "Expression string: " << expr << endl;
	for (LexIter li {expr.begin(), expr.end()}, end{}; li != end; ++li) {
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
				[](LP lp) {
					cout << "Left paren" << endl;
				},
				[](RP rp) {
					cout << "Right paren" << endl;
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
	for (LexIter li {expr.begin(), expr.end()}, end{}; li != end; ++li) {
		cout << *li << endl;
	}
}

#define TEST_MAIN
#ifdef TEST_MAIN
int main()
{
	string s = "foo+5-10 *  (baz+(foo / bammo))";
	cout << "First parse...\n";
	parse_with_overloaded(s);
	cout << "Second parse...\n";
	parse(s);

	return 0;
}
#endif

// vim:ts=4:sw=4:noet:tw=80

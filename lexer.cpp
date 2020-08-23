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
			cout << "Operator: "s << static_cast<string>(get<Op>(lv)) << endl;
			break;
		case TokType::SYM:
			cout << "Symbol: "s + static_cast<string>(get<Sym>(lv)) << endl;
			break;
		case TokType::INT:
			cout << "Integer: "s << get<int>(lv) << endl;
			break;
		case TokType::LP:
			cout << "'('" << endl;
			break;
		case TokType::RP:
			cout << "')'" << endl;
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
		// Defer to the string iterators
		// Caveat: Can't compare current iterator with string::const_iterator{}
		// because str.end() returns an actual pointer to one past end.
		return cur == rhs.cur || cur == end && rhs.cur == decltype(rhs.cur){};
	}
	
	// Convert regex submatch index to variant of 
	// TODO: Should tok be const& or rval???
	LexVariant create_var(int idx, string tok)
	{
		switch (static_cast<TokType>(idx)) {
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

	// Find match at current position and set cur_var accordingly.
	void set_cur_match() {
		smatch m;
		// match_prev_avail flag allows consideration of leading context if not
		// at beginning, and match_continuous requires match to begin at cur.
		bool b = regex_search(cur, end, m, re_tok,
				cur != beg ? match_prev_avail | match_continuous : match_default);

		if (!b)
			throw runtime_error("Parse error: "s + string(cur, end));

		// Figure out which type of match we have and create the corresponding
		// LexVariant.
		// TODO: Is it safe to add 1 apart from pre-increment, given that this
		// is a string iterator?
		auto&& p{m.cbegin() + 1}, pe{m.cend()};
		for (auto i = 1; p != pe; ++p, ++i) {
			if (p->matched) {
				cur_var = create_var(i, p->str());
				break;
			}
		}

		// Cache the next start location.
		cur_next = m[0].second;
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

		// Advance to next match.
		cur = cur_next;

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

#include <iostream>
#include <iterator>
#include <queue>
#include <regex>
#include <variant>
#include <exception>

using namespace std;

enum class TokType {
	OP, SYM, INT
};

ostream& operator<<(ostream& os, TokType tt)
{
    os << (tt == TokType::OP
			? "operator"
			: tt == TokType::SYM
			? "symbol"
			: tt == TokType::INT
			? "integer"
			: "unknown");
    return os;
}

using Op = string; // TODO: Consider changing to enum
//using Sym = string;
struct Sym {
	Sym(string n) : name{n} {}
	operator string() {
		return name;
	}
	string name;
};
using LexVariant = variant<Op, Sym, int>;

// Add iterator trait
class LexIter {
	public:

	LexIter(const LexIter& rhs) = default;
	LexIter() = default;

	bool operator!=(const LexIter& rhs) {
		return !(*this == rhs);
	}
	bool operator==(const LexIter& rhs) {
		// Defer to the sregex_token_iterators.
		return tok_it == rhs.tok_it;
	}
	
	LexVariant operator*() {
		// TODO: Check for end...
		//
		// First check queue
		if (!cache.empty()) {
			return cache.front();
		}

		// Use sregex_iterator
		if (tok_it == sregex_token_iterator{})
			throw runtime_error("Bad iterator");

		auto tok_midx = tok_idx % 3;
		//cout << "*tok_it=" << *tok_it << " tok_idx=" << tok_idx << " tok_idx%3=" << (tok_idx % 3) << endl;
		return static_cast<TokType>(tok_midx) == TokType::OP
				? LexVariant(static_cast<Op>(*tok_it))
				: static_cast<TokType>(tok_midx) == TokType::SYM
					? LexVariant(static_cast<Sym>(*tok_it))
					: /*static_cast<TokType>(tok_midx) == TokType::INT
						?*/ LexVariant(stoi(*tok_it));
	}

	LexIter& operator++() {
		// Advance to next match.
		do {
			++tok_it;
			++tok_idx;
		} while (tok_it != sregex_token_iterator{} && !tok_it->matched);
		//cout << "Advanced: " << *tok_it << " tok_idx=" << tok_idx << endl;
		/*
		if (tok_it == sregex_token_iterator{})
			return LexIter{};
			*/
		return *this;
	}
	// TODO: Add post-increment ++: No! Implementation would be complicated for
	// current approach...

	void put_back(LexVariant lv) {
		// TODO: Push onto front of queue.
	}

	LexIter(string::const_iterator s, string::const_iterator e) :
		tok_idx{0}, tok_it {s, e, re_tok, {1, 2, 3}}
	{
		// Advance to first match.
		for (; tok_it != sregex_token_iterator{} && !tok_it->matched; ++tok_it, ++tok_idx) {}

	}

	private:
	static regex re_tok;
	queue<LexVariant> cache;
	sregex_token_iterator tok_it;
	int tok_idx;


};
regex LexIter::re_tok {R"(([-+/*])|([[:alpha:]]+)|(0|[1-9][0-9]*))"};

template<typename... Ts> struct overloaded : Ts... {
	using Ts::operator()...;
};
template<typename... Ts> overloaded(Ts...) -> overloaded<Ts...>;

#define TEST_MAIN
#ifdef TEST_MAIN
int main()
{
	
	string s = "foo+5-10 *  % (baz+(foo / bammo))";
	
	// TODO: Convert from stream to beg/end iterators.
	cout << s << endl;
	for (LexIter li {s.begin(), s.end()}, lie{}; li != lie; ++li) {
			visit(overloaded {
					[](Op op) {
						cout << "Op: " << op << endl;
					},
					[](Sym sym) {
						cout << "Sym: " << static_cast<string>(sym) << endl;
					},
					[](int i) {
						cout << "Int: " << i << endl;
					}}, *li);

	}

	return 0;
}
#endif

// vim:ts=4:sw=4:noet:tw=80

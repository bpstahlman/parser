#include "lexer.h"

using namespace std;
using namespace regex_constants;

static const string sign {R"([-+]?)"};
static const string dig_seq {R"((?:[0-9]+))"};
static const string exp {R"((?:[eE][-+]?)"s + dig_seq + ")"s};
static const string hexp {R"((?:[pP][-+]?)"s + dig_seq + ")"s};
static const string hex_dig_seq {R"((?:[0-9a-fA-F]+))"};
// TODO: Consider making this class static; to do so, would need to make
// class methods non-inline.
static const struct {
	TokType type;
	regex re;
} re_toks[] = {
	// Allow all valid float formats (including hex with mandatory
	// exponent).
	{TokType::FLT, regex{
			sign + "(?:"s
			+ dig_seq + exp
			+ "|"s + dig_seq + "?\\."s + dig_seq + exp + "?"s
			+ "|"s + dig_seq + "\\."s + exp + "?"s
			+ "|0[xX](?:"s + hex_dig_seq + hexp
					  + "|"s + hex_dig_seq + "?\\."s + hex_dig_seq + hexp
				  + "|"s + hex_dig_seq + "\\."s + hexp
			+ ")"s
			+ "|(?:nan|inf(?:inite)?)"s
			+ ")"s,
			regex::icase
	}},
	// Allow decimal, octal and hex integers.
	// TODO: Weed out bad octal, etc...
	{TokType::INT, regex{
			sign + "(?:"s
			+        "0[xX][0-9a-fA-F]+"s // hex
			+ "|"s + "0[0-7]*"s           // oct
			+ "|"s + "[1-9][0-9]*"s       // dec
			+ ")"s,
			regex::icase
	}},
	{TokType::SYM, regex{R"(\b([[:alpha:]]+))"}},
	{TokType::OP,  regex{R"([-+/*])"}},
	{TokType::LP,  regex{R"(\()"}},
	{TokType::RP,  regex{R"(\))"}},
	{TokType::EQ,  regex{"="}},
	{TokType::COMMA,  regex{","}},
};

ostream& operator<<(ostream& os, TokType tt)
{
    os << (tt == TokType::OP
			? "operator"
			: tt == TokType::SYM
			? "symbol"
			: tt == TokType::FLT
			? "float"
			: tt == TokType::INT
			? "integer"
			: tt == TokType::LP
			? "left paren"
			: tt == TokType::RP
			? "right paren"
			: tt == TokType::EQ
			? "equals"
			: tt == TokType::COMMA
			? "comma"
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
		case TokType::FLT:
			break;
		case TokType::INT:
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
		case TokType::COMMA:
			cout << "','";
			break;
		default:
			cout << "Unknown Lex variant!\n";
	}
	return os;
}

LexIter& LexIter::operator++() {
	// Defer offset validation.
	++i;
	return *this;
}
LexIter& LexIter::operator--() {
	// Defer offset validation.
	--i;
	return *this;
}
LexVariant& LexIter::operator*() {
	if (i < 0)
		throw runtime_error("Invalid access to end!"s); // FIXME!!!
	// Note: Lexer's operator[] may throw.
	return lexer[i];
}
bool LexIter::operator!=(const LexIter& rhs) {
	return !(*this == rhs);
}
bool LexIter::operator==(const LexIter& rhs) {
	if (rhs.i >= 0) {
		// Intentionally not validating either index.
		return rhs.i == i;
	} else {
		// rhs points past end, so see whether lhs does as well.
		return !lexer.materialize_to(i);
	}
}

// Ensure the string is lexed through the idx'th token, returning true
// unless there are fewer than idx+1 tokens.
bool Lexer::materialize_to(int idx) {
	size_t len = toks.size();
	while (!eof && len++ <= idx) {
		// This may throw.
		if (!next())
			return false;
	}
	return len > idx;
}

LexVariant& Lexer::operator[](size_t i) {
	// Ensure we've parsed through element i.
	if (!materialize_to(i))
		throw runtime_error("Invalid access!"s); // FIXME!!!
	return toks[i];
}

// Convert regex submatch index to the correct variant and append to toks
// collection.
void Lexer::add_var(TokType idx, string tok)
{
	switch (idx) {
		case TokType::OP:
			toks.emplace_back(static_cast<Op>(tok));
			break;
		case TokType::SYM:
			toks.emplace_back(static_cast<Sym>(tok));
			break;
		case TokType::INT:
			toks.emplace_back(std::stoi(tok, 0, 0));
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
		case TokType::COMMA:
			toks.emplace_back(Comma{});
			break;
		default:
			throw runtime_error("Invalid token type!"s); // FIXME!!!
	}
}

// Update string iterator to skip past any whitespace at current location.
string::const_iterator Lexer::skip_ws() {
	static regex re_ws{R"(\s+)"};
	smatch m;

	return regex_search(cur_it, end_it, m, re_ws, match_continuous)
		? m[0].second
		: cur_it;

}

// Lex another token and append it to the toks collection, returning false
// if there are no more tokens.
bool Lexer::next() {
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
	int i = 0;
	for (const auto& td : re_toks) {
		// Attempt match.
		if (regex_search(cur_it, end_it, m, td.re, mflags)) {
			// Found match!
			add_var(td.type, m[0].str());
			break;
		}
		i++;
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

#undef TEST
#ifdef TEST
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
				[](Comma comma) {
					cout << "Comma" << endl;
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

#endif // TEST

#pragma once

// TODO: Shouldn't need this once LexVariant is modified/moved here.
#include "lexer.h"

// TODO: Decide on home for these types. Perhaps a common header?
using Fd_d = function<double(double)>;
using Fd_dd = function<double(double, double)>;
using Fl_l = function<long(long)>;
using Fl_ll = function<long(long, long)>;

// Generic arg type
using Fv = variant<Fd_d, Fd_dd, Fl_l, Fl_ll>;

using symtbl = map<string, double>;

// FIXME: Decide on home for this...
struct None {};
struct NaN {};
using Number = variant<None, NaN, int, long, float, double>;

// TODO: Define in class?
using parse_result = pair<bool, double>;
using list_result = pair<bool, vector<Number>>;
using numeric_result = pair<bool, Number>;



#pragma once

#include <variant>
#include <functional>
#include <unordered_map>

using namespace std;

// TODO: Decide on home for these types. Perhaps a common header?
using Fd_d = function<double(double)>;
using Fd_dd = function<double(double, double)>;
using Fb_d = function<bool(double)>;

// Generic arg type
using Fv = variant<Fd_d, Fd_dd, Fb_d>;

using symtbl = unordered_map<string, double>;

// FIXME: Decide on home for this...
struct None {};
struct NaN {};
using Number = variant<None, bool, int, long, float, double>;

// TODO: Define in class?
using parse_result = pair<bool, double>;
using list_result = pair<bool, vector<Number>>;
using numeric_result = pair<bool, Number>;


Number operator+(Number x, Number y);
Number operator-(Number x, Number y);
Number operator*(Number x, Number y);
Number operator/(Number x, Number y);


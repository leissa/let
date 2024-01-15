#include <fe/assert.h>

#include "let/ast.h"

namespace let {

using Tag = Tok::Tag;

/*
 * Expr
 */

// clang-format off
uint64_t ErrExpr::eval(Env&) const { return 0; }
uint64_t LitExpr::eval(Env&) const { return u64(); }
uint64_t SymExpr::eval(Env& env) const { return env.emplace(sym(), 0).first->second;}
// clang-format on

uint64_t BinExpr::eval(Env& env) const {
    auto l = lhs()->eval(env);
    auto r = rhs()->eval(env);
    switch (tag()) {
        case Tag::O_add: return l + r;
        case Tag::O_sub: return l - r;
        case Tag::O_mul: return l * r;
        case Tag::O_div: return r ? l / r : 0; // div by zero = 0
        default: fe::unreachable();
    }
}

uint64_t UnaryExpr::eval(Env& env) const {
    auto r = rhs()->eval(env);
    switch (tag()) {
        case Tag::O_add: return r;
        case Tag::O_sub: return -r;
        default: fe::unreachable();
    }
}

/*
 * Stmt
 */

void LetStmt::eval(Env& env) const {
    auto i     = init()->eval(env);
    env[sym()] = i;
}

void PrintStmt::eval(Env& env) const { std::cout << expr()->eval(env) << std::endl; }

void Prog::eval() const {
    Env env;
    for (auto&& stmt : stmts()) stmt->eval(env);
}

} // namespace let

#include <iostream>

#include "let/ast.h"

namespace let {

// stream

void Node::dump() const { stream(std::cout); }

/*
 * Expr
 */

// clang-format off
void ErrExpr::stream(std::ostream& os) const { os << "<error expression>"; }
void LitExpr::stream(std::ostream& os) const { os << u64(); }
void SymExpr::stream(std::ostream& os) const { os << sym(); }
// clang-format on

void UnaryExpr::stream(std::ostream& os) const { std::print(os, "({}{})", tag(), *rhs()); }
void BinExpr::stream(std::ostream& os) const { std::print(os, "({} {} {})", *lhs(), tag(), *rhs()); }

/*
 * Stmt
 */

void LetStmt::stream(std::ostream& os) const { std::println(os, "let {} = {};", sym(), *init()); }
void PrintStmt::stream(std::ostream& os) const { std::println(os, "print {};", *expr()); }

/*
 * Prog
 */

void Prog::stream(std::ostream& o) const {
    for (auto&& stmt : stmts())
        stmt->stream(o);
}

} // namespace let

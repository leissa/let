#include <iostream>

#include "let/ast.h"

namespace let {

// stream

void Node::dump() const { stream(std::cout); }

/*
 * Expr
 */

// clang-format off
void ErrExpr::stream(std::ostream& o) const { o << "<error expression>"; }
void LitExpr::stream(std::ostream& o) const { o << u64(); }
void SymExpr::stream(std::ostream& o) const { o << sym(); }
// clang-format on

void UnaryExpr::stream(std::ostream& o) const {
    o << '(' << tag();
    rhs()->stream(o);
    o << ')';
}

void BinExpr::stream(std::ostream& o) const {
    o << '(';
    lhs()->stream(o);
    o << ' ' << tag() << ' ';
    rhs()->stream(o);
    o << ')';
}

/*
 * Stmt
 */

void LetStmt::stream(std::ostream& o) const {
    o << "let " << sym() << " = ";
    init()->stream(o);
    o << ';' << std::endl;
}

void PrintStmt::stream(std::ostream& o) const {
    o << "print ";
    expr()->stream(o);
    o << ';' << std::endl;
}

/*
 * Prog
 */

void Prog::stream(std::ostream& o) const {
    for (auto&& stmt : stmts())
        stmt->stream(o);
}

} // namespace let

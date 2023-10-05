#include <iostream>

#include "let/ast.h"

namespace let {

// stream

void Node::dump() const { stream(std::cout); }

/*
 * Expr
 */

// clang-format off
std::ostream& ErrExpr::stream(std::ostream& o) const { return o << "<error expression>"; }
std::ostream& LitExpr::stream(std::ostream& o) const { return o << u64(); }
std::ostream& SymExpr::stream(std::ostream& o) const { return o << sym(); }
// clang-format on

std::ostream& UnaryExpr::stream(std::ostream& o) const {
    o << '(' << tag();
    rhs()->stream(o);
    return o << ')';
}

std::ostream& BinExpr::stream(std::ostream& o) const {
    o << '(';
    lhs()->stream(o);
    o << ' ' << tag() << ' ';
    rhs()->stream(o);
    return o << ')';
}

/*
 * Stmt
 */

std::ostream& LetStmt::stream(std::ostream& o) const {
    o << "let " << sym() << " = ";
    return init()->stream(o) << ';' << std::endl;
}

std::ostream& PrintStmt::stream(std::ostream& o) const {
    o << "print ";
    return expr()->stream(o) << ';' << std::endl;
}

/*
 * Prog
 */

std::ostream& Prog::stream(std::ostream& o) const {
    for (auto&& stmt : stmts()) stmt->stream(o);
    return o;
}

} // namespace let

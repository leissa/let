#include <iostream>

#include "let/ast.h"

namespace let {

// stream

void Node::dump() const { stream(std::cout) << std::endl; }

/*
 * Type
 */

std::ostream& SimpleType::stream(std::ostream& o) const {
    o << tag();
    if (not_null()) o << " NOT NULL";
    return o;
}

/*
 * Expr
 */

// clang-format off
std::ostream& ErrExpr  ::stream(std::ostream& o) const { return o << "<error expression>"; }
std::ostream& SimpleVal::stream(std::ostream& o) const { return o << tag(); }
std::ostream& IntVal   ::stream(std::ostream& o) const { return o << u64(); }
// clang-format on

std::ostream& Id::stream(std::ostream& o) const {
    for (auto sep = ""; auto&& sym : syms()) {
        o << sep << sym;
        sep = ".";
    }

    if (asterisk()) o << ".*";
    return o;
}

std::ostream& UnExpr::stream(std::ostream& o) const {
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

std::ostream& Join::stream(std::ostream& o) const {
    o << '(';
    lhs()->stream(o);

    if (tag() & Cross) {
        o << " CROSS";
    } else {
        if (tag() & Natural) o << " NATURAL";
        // clang-format off
        else if (tag() & Full)  o << " FULL";
        else if (tag() & Left)  o << " LEFT";
        else if (tag() & Right) o << " RIGHT";
        else                    o << " INNER";
        // clang-format on
    }
    o << " JOIN ";
    rhs()->stream(o);

    if (auto on = std::get_if<On>(&spec())) {
        (*on)->stream(o << " ON ");
    } else if (auto syms = std::get_if<Using>(&spec())) {
        o << " USING (";
        for (auto sep = ""; auto sym : *syms) {
            o << sep << sym;
            sep = ", ";
        }
        o << ")";
    }
    return o << ')';
}

std::ostream& Create::stream(std::ostream& o) const {
    o << "CREATE TABLE " << sym() << " (";
    for (auto sep = ""; auto&& elem : elems()) {
        elem->stream(o << sep);
        sep = ", ";
    }
    return o << ")";
}

std::ostream& Create::Elem::stream(std::ostream& o) const {
    o << sym() << " ";
    return type()->stream(o);
}

std::ostream& Select::stream(std::ostream& o) const {
    o << "SELECT ";
    if (distinct()) o << "DISTINCT ";

    if (elems().empty()) {
        o << "*";
    } else {
        for (auto sep = ""; auto&& elem : elems()) {
            o << sep;
            elem->stream(o);
            sep = ", ";
        }
    }

    o << " FROM ";
    for (auto sep = ""; auto&& from : froms()) {
        from->stream(o << sep);
        sep = ", ";
    }

    if (where()) where()->stream(o << " WHERE ");
    if (group()) group()->stream(o << " GROUP BY ");
    if (having()) having()->stream(o << " HAVING ");

    return o;
}

std::ostream& Select::Elem::stream(std::ostream& o) const {
    expr()->stream(o);
    switch (syms().size()) {
        case 0: break;
        case 1: o << " AS " << syms().front(); break;
        default:
            o << " AS (";
            for (auto sep = ""; auto&& sym : syms()) {
                o << sep << sym;
                sep = ", ";
            }
            o << ")";
    }
    return o;
}

/*
 * Misc
 */

std::ostream& Prog::stream(std::ostream& o) const {
    for (auto sep = ""; auto&& expr : exprs()) {
        expr->stream(o << sep) << ';';
        sep = "\n";
    }
    return o;
}

} // namespace let

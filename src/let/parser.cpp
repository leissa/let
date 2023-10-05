#include "let/parser.h"

#include <fstream>
#include <iostream>

using namespace std::literals;

namespace let {

using Tag = Tok::Tag;

Parser::Parser(fe::Driver& driver, std::istream& istream, const std::filesystem::path* path)
    : lexer_(driver, istream, path)
    , error_(driver.sym("<error>"s)) {
    init(path);
}

void Parser::err(const std::string& what, const Tok& tok, std::string_view ctxt) {
    driver().err(tok.loc(), "expected {}, got '{}' while parsing {}", what, tok, ctxt);
}

Sym Parser::parse_sym(std::string_view ctxt) {
    if (ahead().isa(Tag::V_sym)) return lex().sym();
    err("identifier", ctxt);
    return error_;
}

/*
 * stmt
 */

Ptr<Stmt> Parser::parse_stmt() {
    switch (ahead().tag()) {
        case Tag::K_let: return parse_let_stmt();
        default: return {};
    }
}

Ptr<Stmt> Parser::parse_let_stmt() {
    auto track = tracker();
    eat(Tag::K_let);
    auto sym = parse_sym("name of a let-statement");
    expect(Tag::T_ass, "let-statement");
    auto init = parse_expr("initialization expression of a let-statement");
    return mk<LetStmt>(track, sym, std::move(init));
}

/*
 * expr
 */

Ptr<Expr> Parser::parse_expr(std::string_view ctxt, Tok::Prec cur_prec) {
    auto track = tracker();
    auto lhs   = parse_primary_or_unary_expr(ctxt);

    while (true) {
        if (auto prec = Tok::bin_prec(ahead().tag())) {
            if (*prec < cur_prec) break;

            auto op  = lex().tag();
            auto rhs = parse_expr("right-hand side of binary expression", *prec);
            lhs      = mk<BinExpr>(track, std::move(lhs), op, std::move(rhs));
        } else {
            break;
        }
    }

    return lhs;
}

Ptr<Expr> Parser::parse_primary_or_unary_expr(std::string_view ctxt) {
    switch (ahead().tag()) {
        case Tag::V_sym: return mk<SymExpr>(lex());
        case Tag::V_int: return mk<LitExpr>(lex());
        default: break;
    }

    auto track = tracker();
    if (auto prec = Tok::un_prec(ahead().tag())) {
        auto op = lex().tag();
        return mk<UnExpr>(track, op, parse_expr("operand of unary expression", *prec));
    }

    if (accept(Tag::D_paren_l)) {
        auto expr = parse_expr("parenthesized expression");
        expect(Tag::D_paren_r, "parenthesized expression");
        return expr;
    }

    if (!ctxt.empty()) {
        err("primary or unary expression", ctxt);
        return mk<ErrExpr>(prev_);
    }
    fe::unreachable();
}

} // namespace let

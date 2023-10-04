#include "let/parser.h"

#include <fstream>
#include <iostream>

using namespace std::literals;

namespace let {

Parser::Parser(fe::Driver& driver, std::istream& istream, const std::filesystem::path* path)
    : lexer_(driver, istream, path)
    , error_(driver.sym("<error>"s)) {
    init(path);
}

void Parser::err(const std::string& what, const Tok& tok, std::string_view ctxt) {
    driver().err(tok.loc(), "expected {}, got '{}' while parsing {}", what, tok, ctxt);
}

/*
 * misc
 */

Ptr<Prog> Parser::parse_prog() {
    auto track = tracker();
    Ptrs<Expr> exprs;

    while (!ahead().isa(Tok::Tag::EoF)) {
        auto expr = parse_expr("program");
        if (expr->isa<ErrExpr>()) lex(); // consume one token to prevent endless loop
        exprs.emplace_back(std::move(expr));
        expect(Tok::Tag::T_semicolon, "expression list");
    }

    eat(Tok::Tag::EoF);
    return mk<Prog>(track, std::move(exprs));
}

Sym Parser::parse_sym(std::string_view ctxt) {
    if (ahead().isa(Tok::Tag::V_id)) return lex().sym();
    err("identifier", ctxt);
    return error_;
}

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
        case Tok::Tag::V_id: return parse_id();
        case Tok::Tag::V_int: {
            auto tok = lex();
            return mk<IntVal>(tok.loc(), tok.u64());
        }
        default: break;
    }

    auto track = tracker();
    if (auto prec = Tok::un_prec(ahead().tag())) {
        auto op = lex().tag();
        return mk<UnExpr>(track, op, parse_expr("operand of unary expression", *prec));
    }

    if (accept(Tok::Tag::D_paren_l)) {
        auto expr = parse_expr("parenthesized expression");
        expect(Tok::Tag::D_paren_r, "parenthesized expression");
        return expr;
    }

    if (!ctxt.empty()) {
        err("primary or unary expression", ctxt);
        return mk<ErrExpr>(prev_);
    }
    fe::unreachable();
}

Ptr<Expr> Parser::parse_id() {
    auto track = tracker();
    assert(ahead().isa(Tok::Tag::V_id));

    bool asterisk = false;
    Syms syms;
    syms.emplace_back(lex().sym());

    while (accept(Tok::Tag::T_dot)) {
        if (accept(Tok::Tag::O_mul)) {
            asterisk = true;
            break;
        }
        syms.emplace_back(parse_sym("identifer chain"));
    }

    return mk<Id>(track, std::move(syms), asterisk);
}

} // namespace let

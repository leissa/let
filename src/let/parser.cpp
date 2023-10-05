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

void Parser::syntax_err(Tag tag, std::string_view ctxt) {
    std::string msg("'");
    msg.append(Tok::str(tag)).append("'");
    err(msg, ctxt);
}

Sym Parser::parse_sym(std::string_view ctxt) {
    if (ahead().isa(Tag::V_sym)) return lex().sym();
    err("identifier", ctxt);
    return error_;
}

/*
 * Expr
 */

Ptr<Expr> Parser::parse_expr(std::string_view ctxt, Tok::Prec curr_prec) {
    auto track = tracker();
    auto lhs   = parse_primary_or_unary_expr(ctxt);

    while (true) {
        auto prec = Tok::bin_prec(ahead().tag());
        if (prec < curr_prec) break;
        auto op  = lex().tag();
        auto rhs = parse_expr("right-hand side of binary expression", prec);
        lhs      = mk<BinExpr>(track, std::move(lhs), op, std::move(rhs));
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
    if (auto prec = Tok::un_prec(ahead().tag()); prec != Tok::Prec::Error) {
        auto op = lex().tag();
        return mk<UnaryExpr>(track, op, parse_expr("operand of unary expression", prec));
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

/*
 * Stmt
 */

Ptr<Stmt> Parser::parse_let_stmt() {
    auto track = tracker();
    eat(Tag::K_let);
    auto sym = parse_sym("name of a let-statement");
    expect(Tag::T_ass, "let-statement");
    auto init = parse_expr("initialization expression of a let-statement");
    expect(Tag::T_semicolon, "return-statement");
    return mk<LetStmt>(track, sym, std::move(init));
}

Ptr<Stmt> Parser::parse_print_stmt() {
    auto track = tracker();
    eat(Tag::K_print);
    auto expr = parse_expr("print-statement");
    expect(Tag::T_semicolon, "print-statement");
    return mk<PrintStmt>(track, std::move(expr));
}

/*
 * Prog
 */

Ptr<Prog> Parser::parse_prog() {
    auto track = tracker();
    Ptrs<Stmt> stmts;
    while (true) {
        // clang-format off
        switch (ahead().tag()) {
            case Tag::K_let:   stmts.emplace_back(parse_let_stmt());   break;
            case Tag::K_print: stmts.emplace_back(parse_print_stmt()); break;
            case Tag::EoF:     return mk<Prog>(track, std::move(stmts));
            default:
                auto tok = lex();
                driver().err(tok.loc(), "expected statement, got '{}' while parsing program", tok);
        }
        // clang-format on
    }
}

} // namespace let

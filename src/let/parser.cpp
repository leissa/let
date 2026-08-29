#include "let/parser.h"

#include <iostream>

using namespace std::literals;

namespace let {

using Tag = Tok::Tag;

Parser::Parser(Driver& driver, fe::Error& err, const fe::Src& src)
    : err_(err)
    , lexer_(driver, err, src)
    , error_(driver.sym("<error>"s)) {
    init();
}

void Parser::expected_err(std::string_view what, const Tok& tok, std::string_view ctxt) {
    err_.error(tok.loc(), "expected {}, got `{}` while parsing {}", what, tok, ctxt);
}

void Parser::unanchored_err(const Tok& tok, std::string_view ctxt) {
    err_.error(tok.loc(), "ignoring unmatched `{}` while parsing {}", tok, ctxt);
}

void Parser::syntax_err(Tag tag, std::string_view ctxt) {
    expected_err(std::format("`{}`", Tok::str(tag)), ctxt);
    // The note drops itself again if paren_l_ is already covered by the error's own snippet.
    if (tag == Tag::D_paren_r && paren_l_) err_.note(paren_l_, "unmatched `{}` opened here", Tok::str(Tag::D_paren_l));
}

Dbg Parser::parse_sym(std::string_view ctxt) {
    if (ahead().isa(Tag::V_sym)) return lex().dbg();
    expected_err("identifier", ctxt);
    return {ahead().loc(), error_};
}

/*
 * Expr
 */

AST<Expr> Parser::parse_expr(std::string_view ctxt, Tok::Prec curr_prec) {
    recover(Tag::D_paren_r, ctxt);
    auto track = tracker();
    auto lhs   = parse_primary_or_unary_expr(ctxt);

    while (true) {
        recover(Tag::D_paren_r, ctxt);
        auto prec = Tok::bin_prec(ahead().tag());
        if (prec <= curr_prec) break;
        auto op  = lex().tag();
        auto rhs = parse_expr("right-hand side of binary expression", prec);
        lhs      = ast<BinExpr>(track, std::move(lhs), op, std::move(rhs));
    }

    return lhs;
}

AST<Expr> Parser::parse_primary_or_unary_expr(std::string_view ctxt) {
    switch (ahead().tag()) {
        case Tag::V_sym: return ast<SymExpr>(lex().dbg());
        case Tag::V_int: return ast<LitExpr>(lex());
        default: break;
    }

    auto track = tracker();
    if (auto prec = Tok::un_prec(ahead().tag()); prec != Tok::Prec::Error) {
        auto op = lex().tag();
        return ast<UnaryExpr>(track, op, parse_expr("operand of unary expression", prec));
    }

    if (auto paren_l = accept(Tag::D_paren_l)) {
        auto restore = fe::Restore(paren_l_, paren_l.loc());
        auto _       = this->anchor(Tag::D_paren_r);
        auto expr    = parse_expr("parenthesized expression");
        expect(Tag::D_paren_r, "parenthesized expression");
        return expr;
    }

    if (!ctxt.empty()) {
        expected_err("primary or unary expression", ctxt);
        return ast<ErrExpr>(curr_);
    }

    fe::unreachable();
}

/*
 * Stmt
 */

AST<Stmt> Parser::parse_let_stmt() {
    auto track = tracker();
    eat(Tag::K_let);
    auto dbg  = parse_sym("name of a let-statement");
    auto ctxt = dbg.sym() == error_ ? "let-statement"s : std::format("let-statement `{}`", dbg);
    expect(Tag::T_ass, ctxt);
    auto init = parse_expr("initialization expression of a let-statement");
    expect(Tag::T_semicolon, ctxt);
    return ast<LetStmt>(track, dbg, std::move(init));
}

AST<Stmt> Parser::parse_print_stmt() {
    auto track = tracker();
    eat(Tag::K_print);
    auto expr = parse_expr("print-statement");
    expect(Tag::T_semicolon, "print-statement");
    return ast<PrintStmt>(track, std::move(expr));
}

/*
 * Prog
 */

AST<Prog> Parser::parse_prog() {
    auto track = tracker();
    ASTs<Stmt> stmts;
    while (true) {
        // clang-format off
        switch (ahead().tag()) {
            case Tag::T_semicolon: lex(); break; // empty statement
            case Tag::K_let:       stmts.emplace_back(parse_let_stmt());   break;
            case Tag::K_print:     stmts.emplace_back(parse_print_stmt()); break;
            case Tag::EoF:         return ast<Prog>(track, std::move(stmts));
            default:
                auto tok = lex();
                err_.error(tok.loc(), "expected statement, got `{}` while parsing program", tok);
        }
        // clang-format on
    }
}

} // namespace let

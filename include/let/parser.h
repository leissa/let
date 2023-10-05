#pragma once

#include <fe/parser.h>

#include "let/ast.h"
#include "let/lexer.h"

namespace let {

class Parser : public fe::Parser<Tok, Tok::Tag, 1, Parser> {
public:
    Parser(fe::Driver&, std::istream&, const std::filesystem::path* = nullptr);

    fe::Driver& driver() { return lexer_.driver(); }
    Lexer& lexer() { return lexer_; }

    Ptr<Prog> parse_prog();

private:
    Sym parse_sym(std::string_view ctxt = {});

    Ptr<Expr> parse_expr(std::string_view ctxt, Tok::Prec = Tok::Prec::Bottom);
    Ptr<Expr> parse_primary_or_unary_expr(std::string_view ctxt);

    Ptr<Stmt> parse_let_stmt();
    Ptr<Stmt> parse_print_stmt();

    /// Issue an error message of the form:
    /// `expected <what>, got '<tok>' while parsing <ctxt>`
    void err(const std::string& what, const Tok& tok, std::string_view ctxt);

    /// Same above but uses Parser::ahead() as Tok%en.
    void err(const std::string& what, std::string_view ctxt) { err(what, ahead(), ctxt); }

    void syntax_err(Tok::Tag tag, std::string_view ctxt);

    Lexer lexer_;
    Sym error_;

    friend class fe::Parser<Tok, Tok::Tag, 1, Parser>;
};

} // namespace let

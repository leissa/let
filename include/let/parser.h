#pragma once

#include <fe/parser.h>

#include "let/ast.h"
#include "let/driver.h"
#include "let/lexer.h"

namespace let {

class Parser : public fe::Parser<Tok, Tok::Tag, 1, Parser> {
public:
    Parser(Driver&, std::istream&, const std::filesystem::path* = nullptr);

    Driver& driver() { return lexer_.driver(); }
    Lexer& lexer() { return lexer_; }

    AST<Prog> parse_prog();

private:
    template<class T, class... Args>
    auto ast(Args&&... args) {
        return driver().ast<T>(std::forward<Args&&>(args)...);
    }

    Sym parse_sym(std::string_view ctxt = {});

    AST<Expr> parse_expr(std::string_view ctxt, Tok::Prec = Tok::Prec::Bottom);
    AST<Expr> parse_primary_or_unary_expr(std::string_view ctxt);

    AST<Stmt> parse_let_stmt();
    AST<Stmt> parse_print_stmt();

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

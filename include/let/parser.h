#pragma once

#include <fe/parser.h>

#include "let/ast.h"
#include "let/driver.h"
#include "let/lexer.h"

namespace let {

class Parser : public fe::Parser<Tok, Tok::Tag, 1, Parser> {
public:
    Parser(Driver&, const fe::Src&);

    Driver& driver() { return lexer_.driver(); } ///< fe::Parser's default diagnostics go to its Driver::error.
    Lexer& lexer() { return lexer_; }

    AST<Prog> parse_prog();

private:
    template<class T>
    auto ast(auto&&... args) {
        return driver().ast<T>(std::forward<decltype(args)>(args)...);
    }

    Dbg parse_sym(fe::Cite);

    AST<Expr> parse_expr(fe::Cite, Tok::Prec = Tok::Prec::Bottom);
    AST<Expr> parse_primary_or_unary_expr(fe::Cite);

    AST<Stmt> parse_let_stmt();
    AST<Stmt> parse_print_stmt();

    Lexer lexer_;
    Sym error_;
};

} // namespace let

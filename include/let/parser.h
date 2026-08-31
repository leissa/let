#pragma once

#include <fe/parser.h>
#include <fe/restore.h>

#include "let/ast.h"
#include "let/driver.h"
#include "let/lexer.h"

namespace let {

class Parser : public fe::Parser<Tok, Tok::Tag, 1, Parser> {
    using Super = fe::Parser<Tok, Tok::Tag, 1, Parser>;

public:
    Parser(Driver&, const fe::Src&);

    Driver& driver() { return lexer_.driver(); } ///< fe::Parser's default diagnostics go to its Driver::error.
    Lexer& lexer() { return lexer_; }

    AST<Prog> parse_prog();

private:
    template<class T, class... Args>
    auto ast(Args&&... args) {
        return driver().ast<T>(std::forward<Args&&>(args)...);
    }

    Dbg parse_sym(std::string_view ctxt = {});

    AST<Expr> parse_expr(std::string_view ctxt, Tok::Prec = Tok::Prec::Bottom);
    AST<Expr> parse_primary_or_unary_expr(std::string_view ctxt);

    AST<Stmt> parse_let_stmt();
    AST<Stmt> parse_print_stmt();

    using Super::syntax_err;

    /// As fe::Parser::syntax_err but a missing `)` also gets a note pointing back at its `(`.
    void syntax_err(Tok::Tag tag, std::string_view ctxt);

    Lexer lexer_;
    Sym error_;
    Loc paren_l_; ///< The `(` currently being parenthesized; a missing `)` gets a note pointing back at it.

    friend class fe::Parser<Tok, Tok::Tag, 1, Parser>;
};

} // namespace let

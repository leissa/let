#pragma once

#include <fe/error.h>
#include <fe/parser.h>
#include <fe/restore.h>

#include "let/ast.h"
#include "let/driver.h"
#include "let/lexer.h"

namespace let {

class Parser : public fe::Parser<Tok, Tok::Tag, 1, Parser> {
public:
    Parser(Driver&, fe::Error&, const fe::Src&);

    Driver& driver() { return lexer_.driver(); }
    fe::Error& err() { return err_; }
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

    /// Issue an error message of the form:
    /// ``expected <what>, got `<tok>` while parsing <ctxt>``
    void expected_err(std::string_view what, const Tok& tok, std::string_view ctxt);

    /// Same above but uses Parser::ahead() as Tok%en.
    void expected_err(std::string_view what, std::string_view ctxt) { expected_err(what, ahead(), ctxt); }

    void syntax_err(Tok::Tag tag, std::string_view ctxt);

    /// Issue an error message of the form:
    /// ``ignoring unmatched `<tok>` while parsing <ctxt>``
    void unanchored_err(const Tok& tok, std::string_view ctxt);

    fe::Error& err_;
    Lexer lexer_;
    Sym error_;
    Loc paren_l_; ///< The `(` currently being parenthesized; a missing `)` gets a note pointing back at it.

    friend class fe::Parser<Tok, Tok::Tag, 1, Parser>;
};

} // namespace let

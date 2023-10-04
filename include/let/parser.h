#pragma once

#include <fe/parser.h>

#include "let/ast.h"
#include "let/lexer.h"

namespace let {

class Parser : public fe::Parser<Tok, Tok::Tag, 1, Parser> {
public:
    Parser(fe::Driver&, std::istream&, const std::filesystem::path* = nullptr);

    fe::Driver& driver() { return lexer_.driver(); }
    Ptr<Prog> parse_prog();
    Lexer& lexer() { return lexer_; }

private:
    Sym parse_sym(std::string_view ctxt);

    Ptr<Expr> parse_expr(std::string_view ctxt, Tok::Prec = Tok::Prec::Bot);
    Ptr<Expr> parse_primary_or_unary_expr(std::string_view ctxt);
    Ptr<Expr> parse_id();

    template<class F>
    void parse_list(F f, Tok::Tag delim, Tok::Tag sep = Tok::Tag::T_comma) {
        if (!ahead().isa(delim)) {
            do { f(); } while (accept(sep) && !ahead().isa(delim));
        }
    }

    template<class F>
    void parse_list(std::string ctxt, F f, Tok::Tag delim_l = Tok::Tag::D_paren_l, Tok::Tag sep = Tok::Tag::T_comma) {
        expect(delim_l, ctxt);
        auto delim_r = (Tok::Tag)((int)delim_l + 1);
        parse_list(f, delim_r, sep);
        expect(delim_r, std::string("closing delimiter of a ") + ctxt);
    }

    /// Issue an error message of the form:
    /// `expected <what>, got '<tok>' while parsing <ctxt>`
    void err(const std::string& what, const Tok& tok, std::string_view ctxt);

    /// Same above but uses Parser::ahead() as Tok%en.
    void err(const std::string& what, std::string_view ctxt) { err(what, ahead(), ctxt); }

    void syntax_err(Tok::Tag tag, std::string_view ctxt) {
        std::string msg("'");
        msg.append(Tok::str(tag)).append("'");
        err(msg, ctxt);
    }

    Lexer lexer_;
    Sym error_;

    friend class fe::Parser<Tok, Tok::Tag, 1, Parser>;
};

} // namespace let

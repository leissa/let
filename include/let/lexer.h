#pragma once

#include <cassert>

#include <fe/error.h>
#include <fe/lexer.h>

#include "let/driver.h"
#include "let/tok.h"

namespace let {

class Lexer : public fe::Lexer<1, Lexer> {
public:
    Lexer(Driver&, fe::Error&, const fe::Src&);

    Tok lex(); ///< Get next Tok in stream.
    Driver& driver() { return driver_; }
    fe::Error& err() { return err_; }

private:
    void eat_comments();

    Driver& driver_;
    fe::Error& err_;
    fe::SymMap<Tok::Tag> keywords_;
};

} // namespace let

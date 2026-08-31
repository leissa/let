#pragma once

#include <cassert>

#include <fe/lexer.h>

#include "let/driver.h"
#include "let/tok.h"

namespace let {

class Lexer : public fe::Lexer<1, Lexer> {
public:
    Lexer(Driver&, const fe::Src&);

    Tok lex();                           ///< Get next Tok in stream.
    Driver& driver() { return driver_; } ///< fe::Lexer's default diagnostics go to its Driver::error.

private:
    void eat_comments();

    Driver& driver_;
    fe::SymMap<Tok::Tag> keywords_;
};

} // namespace let

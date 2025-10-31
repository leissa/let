#pragma once

#include <cassert>

#include <istream>

#include <fe/lexer.h>

#include "let/driver.h"
#include "let/tok.h"

namespace let {

class Lexer : public fe::Lexer<1, Lexer> {
public:
    Lexer(Driver&, std::istream&, const std::filesystem::path*);

    Tok lex(); ///< Get next Tok in stream.
    Driver& driver() { return driver_; }

private:
    void eat_comments();

    Driver& driver_;
    fe::SymMap<Tok::Tag> keywords_;
};

} // namespace let

#pragma once

#include <cassert>

#include <istream>
#include <unordered_map>

#include <fe/driver.h>
#include <fe/lexer.h>

#include "let/tok.h"

namespace let {

class Lexer : public fe::Lexer<1, Lexer> {
public:
    Lexer(fe::Driver&, std::istream&, const std::filesystem::path*);

    Tok lex(); ///< Get next Tok in stream.
    fe::Driver& driver() { return driver_; }

private:
    void eat_comments();

    fe::Driver& driver_;
    fe::SymMap<Tok::Tag> keywords_;
};

} // namespace let

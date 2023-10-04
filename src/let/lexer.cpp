#include "let/lexer.h"

#include <ranges>

#include <fe/loc.cpp.h>

using namespace std::literals;

namespace let {

static std::string to_lower(std::string_view sv) {
    std::string res;
    for (auto c : sv) res += tolower(c);
    return res;
}

Lexer::Lexer(fe::Driver& driver, std::istream& istream, const std::filesystem::path* path)
    : fe::Lexer<1, Lexer>(istream, path)
    , driver_(driver) {
    if (!istream_) throw std::runtime_error("stream is bad");
#define CODE(t, str) keywords_[driver_.sym(to_lower(str##s))] = Tok::Tag::t;
    LET_KEY(CODE)
#undef CODE
}

Tok Lexer::lex() {
    while (true) {
        begin();

        if (accept(fe::utf8::EoF)) return {loc_, Tok::Tag::EoF};
        if (accept_if(isspace)) continue;
        if (accept('(')) return {loc_, Tok::Tag::D_paren_l};
        if (accept(')')) return {loc_, Tok::Tag::D_paren_r};
        if (accept('<')) {
            if (accept('>')) return {loc_, Tok::Tag::T_ne};
            if (accept('=')) return {loc_, Tok::Tag::T_le};
            return {loc_, Tok::Tag::T_l};
        }
        if (accept('>')) {
            if (accept('=')) return {loc_, Tok::Tag::T_ge};
            return {loc_, Tok::Tag::T_g};
        }
        if (accept('=')) return {loc_, Tok::Tag::T_eq};
        if (accept(',')) return {loc_, Tok::Tag::T_comma};
        if (accept('.')) return {loc_, Tok::Tag::T_dot};
        if (accept(';')) return {loc_, Tok::Tag::T_semicolon};
        if (accept('+')) return {loc_, Tok::Tag::O_add};
        if (accept('-')) return {loc_, Tok::Tag::O_sub};
        if (accept('*')) return {loc_, Tok::Tag::O_mul};
        if (accept('/')) {
            if (accept('*')) {
                eat_comments();
                continue;
            }
            if (accept('/')) {
                while (ahead() != fe::utf8::EoF && ahead() != '\n') next();
                continue;
            }

            return {loc_, Tok::Tag::O_div};
        }

        // integer value
        if (accept_if(isdigit)) {
            while (accept_if(isdigit)) {}
            return {loc_, std::strtoull(str_.c_str(), nullptr, 10)};
        }

        // lex identifier or keyword
        if (accept_if<Append::Lower>([](int i) { return i == '_' || isalpha(i); })) {
            while (accept_if<Append::Lower>([](int i) { return i == '_' || isalpha(i) || isdigit(i); })) {}
            auto sym = driver_.sym(str_);
            if (auto i = keywords_.find(sym); i != keywords_.end()) return {loc_, i->second}; // keyword
            return {loc_, sym};                                                               // identifier
        }

        driver_.err({loc_.path, peek_}, "invalid input char: '{}'", (char)ahead());
        next();
    }
}

void Lexer::eat_comments() {
    while (true) {
        while (ahead() != fe::utf8::EoF && ahead() != '*') next();
        if (ahead() == fe::utf8::EoF) {
            driver_.err(loc_, "non-terminated multiline comment");
            return;
        }
        next();
        if (accept('/')) break;
    }
}

} // namespace let

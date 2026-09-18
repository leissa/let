#include "let/lexer.h"

#include <charconv>

#include <algorithm>

using namespace std::literals;

namespace let {

namespace utf8 = fe::utf8;

namespace {
/// std::from_chars leaves @p res alone on overflow, so saturate to the widest literal instead.
uint64_t to_u64(std::string_view sv, int base) {
    uint64_t res = 0;
    auto ec      = std::from_chars(sv.data(), sv.data() + sv.size(), res, base).ec;
    return ec == std::errc::result_out_of_range ? std::numeric_limits<uint64_t>::max() : res;
}

bool needs_fold(std::string_view sv) {
    return std::ranges::any_of(sv, [](char c) { return utf8::isupper(c); });
}
} // namespace

Lexer::Lexer(Driver& driver, const fe::Src& src)
    : fe::Lexer<1, Lexer>(src)
    , driver_(driver)
    , keys_(driver.keys()) {}

Tok Lexer::lex() {
    while (true) {
        start();

        if (accept(utf8::EoF)) return {loc_, Tok::Tag::EoF};
        if (accept(utf8::isspace)) continue;
        if (recover_utf8()) continue;
        if (accept('(')) return {loc_, Tok::Tag::D_paren_l};
        if (accept(')')) return {loc_, Tok::Tag::D_paren_r};
        if (accept('=')) return {loc_, Tok::Tag::T_ass};
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
                while (ahead() != utf8::EoF && ahead() != '\n')
                    next();
                continue;
            }

            return {loc_, Tok::Tag::O_div};
        }

        // integer value
        if (accept(utf8::isdigit)) {
            while (accept(utf8::isdigit)) {}
            return {loc_, to_u64(view(), 10)};
        }

        // lex identifier or keyword
        if (accept([](char32_t c) { return c == '_' || utf8::isalpha(c); })) {
            accept_while([](char32_t c) { return c == '_' || utf8::isalpha(c) || utf8::isdigit(c); });
            auto sym = needs_fold(view()) ? driver_.sym(lower()) : driver_.sym(view());
            if (auto tag = keys_.find(sym)) return {loc_, *tag}; // keyword
            return {loc_, sym};                                  // identifier
        }

        recover_char();
    }
}

void Lexer::eat_comments() {
    accept_until("*/");
    if (!accept('*')) {
        error().e(loc_, "non-terminated multiline comment");
        return;
    }
    accept('/');
}

} // namespace let

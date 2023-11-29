#pragma once

#include <cassert>

#include <fe/format.h>
#include <fe/loc.h>
#include <fe/sym.h>

namespace let {

using fe::Loc;
using fe::Pos;
using fe::Sym;

// clang-format off
#define LET_KEY(m)      \
    m(K_let,   "let")   \
    m(K_print, "print") \

#define LET_VAL(m)                        \
    m(V_int,        "<interger literal>") \
    m(V_sym,        "<identifier>")       \

#define LET_TOK(m)                   \
    m(EoF,          "<end of file>") \
    /* delimiter */                  \
    m(D_paren_l,    "(")             \
    m(D_paren_r,    ")")             \
    /* further tokens */             \
    m(T_ass,        "=")             \
    m(T_semicolon,  ";")             \

#define CODE(t, str) + 1
constexpr auto Num_Keys = 0 LET_KEY(CODE);
#undef CODE

#define LET_OP(m)      \
    m(O_add, "+", Add) \
    m(O_sub, "-", Add) \
    m(O_mul, "*", Mul) \
    m(O_div, "/", Mul) \

class Tok {
public:
    // clang-format off
    enum class Tag {
        Nil,
#define CODE(t, _) t,
        LET_KEY(CODE)
        LET_VAL(CODE)
        LET_TOK(CODE)
#undef CODE
#define CODE(t, str, prec) t,
        LET_OP (CODE)
#undef CODE
    };
    // clang-format on

    enum class Prec {
        Error,
        Bottom,
        Add,
        Mul,
        Unary,
    };

    Tok() {}
    Tok(Loc loc, Tag tag)
        : loc_(loc)
        , tag_(tag) {}
    Tok(Loc loc, Sym sym)
        : loc_(loc)
        , tag_(Tag::V_sym)
        , sym_(sym) {}
    Tok(Loc loc, uint64_t u64)
        : loc_(loc)
        , tag_(Tag::V_int)
        , u64_(u64) {}

    Loc loc() const { return loc_; }
    Tag tag() const { return tag_; }
    bool isa(Tag tag) const { return tag == tag_; }
    bool isa_key() const { return (int)tag() < Num_Keys; }
    explicit operator bool() const { return tag_ != Tag::Nil; }

    Sym sym() const {
        assert(isa(Tag::V_sym));
        return sym_;
    }
    uint64_t u64() const { return u64_; }

    static std::string_view str(Tok::Tag);
    static Prec un_prec(Tok::Tag);
    static Prec bin_prec(Tok::Tag);

    friend std::ostream& operator<<(std::ostream&, Tag);
    friend std::ostream& operator<<(std::ostream&, Tok);

private:
    Loc loc_;
    Tag tag_ = Tag::Nil;
    union {
        Sym sym_;
        uint64_t u64_;
    };
};

} // namespace let

template<>
struct std::formatter<let::Tok> : fe::ostream_formatter {};

#include "let/tok.h"

#include <fe/assert.h>

using namespace std::literals;

namespace let {

std::string_view Tok::str(Tok::Tag tag) {
    switch (tag) {
#define CODE(t, str) \
    case Tok::Tag::t: return str##sv;
        LET_KEY(CODE)
        LET_VAL(CODE)
        LET_TOK(CODE)
#undef CODE
#define CODE(t, str, prec) \
    case Tok::Tag::t: return str##sv;
        LET_OP(CODE)
#undef CODE
        default: fe::unreachable();
    }
}

std::ostream& operator<<(std::ostream& o, Tok::Tag tag) { return o << Tok::str(tag); }

std::ostream& operator<<(std::ostream& o, Tok tok) {
    if (tok.isa(Tok::Tag::V_sym)) return o << *tok.sym();
    if (tok.isa(Tok::Tag::V_int)) return o << tok.u64();
    return o << Tok::str(tok.tag());
}

Tok::Prec Tok::un_prec(Tag tag) { return (tag == Tag::O_add || tag == Tag::O_sub) ? Prec::Unary : Prec::Error; }

Tok::Prec Tok::bin_prec(Tok::Tag tag) {
    switch (tag) {
#define CODE(t, str, prec) \
    case Tok::Tag::t: return Prec::prec;
        LET_OP(CODE)
#undef CODE
        default: return Prec::Error;
    }
}

} // namespace let

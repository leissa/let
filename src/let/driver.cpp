#include "let/driver.h"

using namespace std::literals;

namespace let {

Driver::Driver() {
#define CODE(t, str) keys_.emplace(sym(str##sv), Tok::Tag::t);
    LET_KEY(CODE)
#undef CODE
}

} // namespace let

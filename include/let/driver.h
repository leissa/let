#pragma once

#include <fe/arena.h>
#include <fe/driver.h>

#include "let/tok.h"

namespace let {

/// The keywords the Lexer looks up, keyed by the Sym it has just interned.
using Keys = fe::SymTab<Tok::Tag, Num_Keys>;

class Driver : public fe::Driver {
public:
    Driver();

    template<class T, class... Args>
    auto ast(Args&&... args) {
        return arena_.ref<const T>(std::forward<Args&&>(args)...);
    }

    /// The keywords, interned once here and borrowed by every Lexer this Driver serves.
    const Keys& keys() const { return keys_; }

private:
    fe::Arena arena_;
    Keys keys_;
};

} // namespace let

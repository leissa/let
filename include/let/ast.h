#pragma once

#include <memory>
#include <ostream>
#include <unordered_set>
#include <variant>

#include <fe/cast.h>

#include "let/tok.h"

namespace let {

template<class T>
using Ptr = std::unique_ptr<const T>;
template<class T>
using Ptrs = std::deque<Ptr<T>>;
using Syms = std::deque<Sym>;

template<class T, class... Args>
Ptr<T> mk(Args&&... args) {
    return std::make_unique<T>(std::forward<Args>(args)...);
}

/// Base class for all @p Expr%essions.
class Node : public fe::RuntimeCast<Node> {
public:
    Node(Loc loc)
        : loc_(loc) {}
    virtual ~Node() {}

    Loc loc() const { return loc_; }
    void dump() const;

    /// Stream to @p o.
    virtual std::ostream& stream(std::ostream& o) const = 0;

private:
    Loc loc_;
};

/*
 * Expr
 */

/// Base class for all @p Expr%essions.
class Expr : public Node {
public:
    Expr(Loc loc)
        : Node(loc) {}
};

class LitExpr : public Expr {
public:
    LitExpr(Tok tok)
        : Expr(tok.loc())
        , u64_(tok.u64()) {}

    uint64_t u64() const { return u64_; }
    virtual std::ostream& stream(std::ostream&) const;

private:
    uint64_t u64_;
};

class SymExpr : public Expr {
public:
    SymExpr(Tok tok)
        : Expr(tok.loc())
        , sym_(tok.sym()) {}

    Sym sym() const { return sym_; }

    std::ostream& stream(std::ostream&) const override;

private:
    Sym sym_;
};

class UnExpr : public Expr {
public:
    UnExpr(Loc loc, Tok::Tag tag, Ptr<Expr>&& rhs)
        : Expr(loc)
        , tag_(tag)
        , rhs_(std::move(rhs)) {}

    Tok::Tag tag() const { return tag_; }
    const Expr* rhs() const { return rhs_.get(); }
    std::ostream& stream(std::ostream&) const override;

private:
    Tok::Tag tag_;
    Ptr<Expr> rhs_;
};

class BinExpr : public Expr {
public:
    BinExpr(Loc loc, Ptr<Expr>&& lhs, Tok::Tag tag, Ptr<Expr>&& rhs)
        : Expr(loc)
        , lhs_(std::move(lhs))
        , tag_(tag)
        , rhs_(std::move(rhs)) {}

    const Expr* lhs() const { return lhs_.get(); }
    Tok::Tag tag() const { return tag_; }
    const Expr* rhs() const { return rhs_.get(); }
    std::ostream& stream(std::ostream&) const override;

private:
    Ptr<Expr> lhs_;
    Tok::Tag tag_;
    Ptr<Expr> rhs_;
};

/// Just a dummy that does nothing and will only be constructed during parse errors.
class ErrExpr : public Expr {
public:
    ErrExpr(Loc loc)
        : Expr(loc) {}

    std::ostream& stream(std::ostream&) const override;
};

/*
 * Stmt
 */

/// Base class for all @p Stmt%ements.
class Stmt : public Node {
public:
    Stmt(Loc loc)
        : Node(loc) {}
};

class LetStmt : public Stmt {
public:
    LetStmt(Loc loc, Sym sym, Ptr<Expr>&& init)
        : Stmt(loc)
        , sym_(sym)
        , init_(std::move(init)) {}

    Sym sym() const { return sym_; }
    const Expr* init() const { return init_.get(); }
    std::ostream& stream(std::ostream&) const override;

private:
    Sym sym_;
    Ptr<Expr> init_;
};

} // namespace let

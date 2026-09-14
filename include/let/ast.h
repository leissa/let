#pragma once

#include <ostream>
#include <tuple>

#include <fe/arena.h>
#include <fe/cast.h>
#include <fe/span.h>
#include <fe/vector.h>
#include <fe/vla.h>

#include "let/tok.h"

namespace let {

// clang-format off
/// Nodes live in the Driver's Arena and are never destroyed, so this merely points at one.
template<class T> using AST  = fe::Arena::Ref<const T>;
template<class T> using ASTs = fe::Vector<AST<T>>; ///< Scratch buffer the Parser fills before it creates a node.
template<class T> using View = fe::View<AST<T>>;   ///< Non-owning view of a node's own list - see fe::VLA.
using Env                    = fe::SymMap<uint64_t>;
// clang-format on

/// Base class for all @p Expr%essions.
/// @note No destructor, virtual or otherwise: the Arena reclaims every node at once.
class Node : public fe::RuntimeCast<Node> {
public:
    Node(Loc loc)
        : loc_(loc) {}

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
    virtual uint64_t eval(Env&) const = 0;
};

class LitExpr : public Expr {
public:
    LitExpr(Tok tok)
        : Expr(tok.loc())
        , u64_(tok.u64()) {}

    uint64_t u64() const { return u64_; }

    std::ostream& stream(std::ostream&) const override;
    uint64_t eval(Env&) const override;

private:
    uint64_t u64_;
};

class SymExpr : public Expr {
public:
    SymExpr(Dbg dbg)
        : Expr(dbg.loc())
        , sym_(dbg.sym()) {}

    Sym sym() const { return sym_; }
    Dbg dbg() const { return {loc(), sym()}; }

    std::ostream& stream(std::ostream&) const override;
    uint64_t eval(Env&) const override;

private:
    Sym sym_;
};

class UnaryExpr : public Expr {
public:
    UnaryExpr(Loc loc, Tok::Tag tag, AST<Expr> rhs)
        : Expr(loc)
        , tag_(tag)
        , rhs_(rhs) {}

    Tok::Tag tag() const { return tag_; }
    const Expr* rhs() const { return rhs_.get(); }

    std::ostream& stream(std::ostream&) const override;
    uint64_t eval(Env&) const override;

private:
    Tok::Tag tag_;
    AST<Expr> rhs_;
};

class BinExpr : public Expr {
public:
    BinExpr(Loc loc, AST<Expr> lhs, Tok::Tag tag, AST<Expr> rhs)
        : Expr(loc)
        , lhs_(lhs)
        , tag_(tag)
        , rhs_(rhs) {}

    const Expr* lhs() const { return lhs_.get(); }
    Tok::Tag tag() const { return tag_; }
    const Expr* rhs() const { return rhs_.get(); }

    std::ostream& stream(std::ostream&) const override;
    uint64_t eval(Env&) const override;

private:
    AST<Expr> lhs_;
    Tok::Tag tag_;
    AST<Expr> rhs_;
};

/// Just a dummy that does nothing and will only be constructed during parse errors.
class ErrExpr : public Expr {
public:
    ErrExpr(Loc loc)
        : Expr(loc) {}

    std::ostream& stream(std::ostream&) const override;
    uint64_t eval(Env&) const override;
};

/*
 * Stmt
 */

/// Base class for all @p Stmt%ements.
class Stmt : public Node {
public:
    Stmt(Loc loc)
        : Node(loc) {}

    virtual void eval(Env&) const = 0;
};

class LetStmt : public Stmt {
public:
    LetStmt(Loc loc, Dbg dbg, AST<Expr> init)
        : Stmt(loc)
        , dbg_(dbg)
        , init_(init) {}

    Dbg dbg() const { return dbg_; } ///< @note Dbg::loc is the bound name - not the whole statement.
    Sym sym() const { return dbg_.sym(); }
    const Expr* init() const { return init_.get(); }

    std::ostream& stream(std::ostream&) const override;
    void eval(Env&) const override;

private:
    Dbg dbg_;
    AST<Expr> init_;
};

class PrintStmt : public Stmt {
public:
    PrintStmt(Loc loc, AST<Expr> expr)
        : Stmt(loc)
        , expr_(expr) {}

    const Expr* expr() const { return expr_.get(); }

    std::ostream& stream(std::ostream&) const override;
    void eval(Env&) const override;

private:
    AST<Expr> expr_;
};

/*
 * Prog
 */

class Prog : public Node, public fe::VLA<Prog> {
public:
    using VLA_Types = std::tuple<AST<Stmt>>;

    Prog(Loc loc)
        : Node(loc) {}

    auto stmts() const { return vla<0>(); }

    std::ostream& stream(std::ostream&) const override;
    void eval() const;
};

} // namespace let

#pragma once

#include <deque>
#include <ostream>

#include <fe/cast.h>

#include "let/tok.h"

namespace let {

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

template<class T> using AST  = fe::Arena::Ptr<const T>;
template<class T> using ASTs = std::deque<AST<T>>;
using Env                    = fe::SymMap<uint64_t>;

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
    SymExpr(Tok tok)
        : Expr(tok.loc())
        , sym_(tok.sym()) {}

    Sym sym() const { return sym_; }

    std::ostream& stream(std::ostream&) const override;
    uint64_t eval(Env&) const override;

private:
    Sym sym_;
};

class UnaryExpr : public Expr {
public:
    UnaryExpr(Loc loc, Tok::Tag tag, AST<Expr>&& rhs)
        : Expr(loc)
        , tag_(tag)
        , rhs_(std::move(rhs)) {}

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
    BinExpr(Loc loc, AST<Expr>&& lhs, Tok::Tag tag, AST<Expr>&& rhs)
        : Expr(loc)
        , lhs_(std::move(lhs))
        , tag_(tag)
        , rhs_(std::move(rhs)) {}

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
    LetStmt(Loc loc, Sym sym, AST<Expr>&& init)
        : Stmt(loc)
        , sym_(sym)
        , init_(std::move(init)) {}

    Sym sym() const { return sym_; }
    const Expr* init() const { return init_.get(); }

    std::ostream& stream(std::ostream&) const override;
    void eval(Env&) const override;

private:
    Sym sym_;
    AST<Expr> init_;
};

class PrintStmt : public Stmt {
public:
    PrintStmt(Loc loc, AST<Expr>&& expr)
        : Stmt(loc)
        , expr_(std::move(expr)) {}

    Sym sym() const { return sym_; }
    const Expr* expr() const { return expr_.get(); }

    std::ostream& stream(std::ostream&) const override;
    void eval(Env&) const override;

private:
    Sym sym_;
    AST<Expr> expr_;
};

/*
 * Prog
 */

class Prog : public Node {
public:
    Prog(Loc loc, ASTs<Stmt>&& stmts)
        : Node(loc)
        , stmts_(std::move(stmts)) {}

    const ASTs<Stmt>& stmts() const { return stmts_; }

    std::ostream& stream(std::ostream&) const override;
    void eval() const;

private:
    Sym sym_;
    ASTs<Stmt> stmts_;
};

} // namespace let

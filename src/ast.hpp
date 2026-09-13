#pragma once

#include <memory>
#include <string>
#include <variant>

namespace hopgood {
enum class BinaryOp { Add, Multiply };

struct IdentifierExpr {
    std::string name;
};

struct BinaryExpr;

using Expr = std::variant<IdentifierExpr, std::unique_ptr<BinaryExpr>>;

struct BinaryExpr {
    BinaryOp op;
    Expr lhs;
    Expr rhs;
};

std::string format_expr(const Expr& expr);
} // namespace hopgood

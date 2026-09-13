#include "ast.hpp"

#include <memory>
#include <stdexcept>
#include <string>
#include <variant>

using hopgood::BinaryExpr;
using hopgood::BinaryOp;
using hopgood::Expr;
using hopgood::IdentifierExpr;

namespace {
int precedence(BinaryOp op) {
    switch (op) {
    case BinaryOp::Add:
        return 1;
    case BinaryOp::Multiply:
        return 2;
    }

    throw std::logic_error("unknown binary operator");
}

const char* operator_text(BinaryOp op) {
    switch (op) {
    case BinaryOp::Add:
        return "+";
    case BinaryOp::Multiply:
        return "*";
    }

    throw std::logic_error("unknown binary operator");
}

std::string format_expr_with_parent_precedence(const Expr& expr, int parent_precedence) {
    if (const auto* identifier = std::get_if<IdentifierExpr>(&expr)) {
        return identifier->name;
    }

    const auto& binary = std::get<std::unique_ptr<BinaryExpr>>(expr);
    if (!binary) {
        throw std::logic_error("BinaryExpr pointer must not be null");
    }

    const int current_precedence = precedence(binary->op);
    std::string text = format_expr_with_parent_precedence(binary->lhs, current_precedence) + " " +
                       operator_text(binary->op) + " " +
                       format_expr_with_parent_precedence(binary->rhs, current_precedence);

    if (current_precedence < parent_precedence) {
        return "(" + text + ")";
    }

    return text;
}
} // namespace

std::string hopgood::format_expr(const Expr& expr) {
    return format_expr_with_parent_precedence(expr, 0);
}

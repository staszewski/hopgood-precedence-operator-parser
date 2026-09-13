#include "precedence.hpp"
#include "token.hpp"
#include <array>

using hopgood::PrecedenceRelation;
using hopgood::TokenKind;

namespace {
struct PrecedenceRule {
    TokenKind stack_kind;
    TokenKind input_kind;
    PrecedenceRelation relation;
};
} // namespace

constexpr std::array<PrecedenceRule, 21> rules{
    // Start token
    PrecedenceRule{TokenKind::Start, TokenKind::Plus, PrecedenceRelation::Yields},
    PrecedenceRule{TokenKind::Start, TokenKind::Multiply, PrecedenceRelation::Yields},
    PrecedenceRule{TokenKind::Start, TokenKind::LeftParen, PrecedenceRelation::Yields},
    // Plus token
    PrecedenceRule{TokenKind::Plus, TokenKind::Plus, PrecedenceRelation::Takes},
    PrecedenceRule{TokenKind::Plus, TokenKind::Multiply, PrecedenceRelation::Yields},
    PrecedenceRule{TokenKind::Plus, TokenKind::LeftParen, PrecedenceRelation::Yields},
    PrecedenceRule{TokenKind::Plus, TokenKind::RightParen, PrecedenceRelation::Takes},
    PrecedenceRule{TokenKind::Plus, TokenKind::End, PrecedenceRelation::Takes},
    // Multiply Token
    PrecedenceRule{TokenKind::Multiply, TokenKind::Plus, PrecedenceRelation::Takes},
    PrecedenceRule{TokenKind::Multiply, TokenKind::Multiply, PrecedenceRelation::Takes},
    PrecedenceRule{TokenKind::Multiply, TokenKind::LeftParen, PrecedenceRelation::Yields},
    PrecedenceRule{TokenKind::Multiply, TokenKind::RightParen, PrecedenceRelation::Takes},
    PrecedenceRule{TokenKind::Multiply, TokenKind::End, PrecedenceRelation::Takes},
    // LeftParen Token
    PrecedenceRule{TokenKind::LeftParen, TokenKind::Plus, PrecedenceRelation::Yields},
    PrecedenceRule{TokenKind::LeftParen, TokenKind::Multiply, PrecedenceRelation::Yields},
    PrecedenceRule{TokenKind::LeftParen, TokenKind::LeftParen, PrecedenceRelation::Yields},
    PrecedenceRule{TokenKind::LeftParen, TokenKind::RightParen, PrecedenceRelation::Equal},
    // RightParen Token
    PrecedenceRule{TokenKind::RightParen, TokenKind::Plus, PrecedenceRelation::Takes},
    PrecedenceRule{TokenKind::RightParen, TokenKind::Multiply, PrecedenceRelation::Takes},
    PrecedenceRule{TokenKind::RightParen, TokenKind::RightParen, PrecedenceRelation::Takes},
    PrecedenceRule{TokenKind::RightParen, TokenKind::End, PrecedenceRelation::Takes},
};

PrecedenceRelation hopgood::precedence_relation(TokenKind stack_kind, TokenKind input_kind) {
    for (const auto& rule : rules) {
        if (rule.stack_kind == stack_kind && rule.input_kind == input_kind) {
            return rule.relation;
        }
    }
    return PrecedenceRelation::None;
}

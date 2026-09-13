#pragma once

#include <string>
#include <string_view>

namespace hopgood {
enum class TokenKind { Identifier, Plus, Multiply, LeftParen, RightParen, End, Start };

struct Token {
    TokenKind kind;
    std::string lexeme;

    bool operator==(const Token&) const = default;
};

std::string_view token_kind_name(TokenKind kind);
} // namespace hopgood

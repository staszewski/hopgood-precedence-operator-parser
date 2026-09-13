#include "token.hpp"
#include <string_view>

using hopgood::TokenKind;

std::string_view hopgood::token_kind_name(TokenKind kind) {
    switch (kind) {
    case TokenKind::Identifier:
        return "Identifier";
    case TokenKind::Plus:
        return "Plus";
    case TokenKind::Multiply:
        return "Multiply";
    case TokenKind::LeftParen:
        return "LeftParen";
    case TokenKind::RightParen:
        return "RightParen";
    case TokenKind::End:
        return "End";
    case TokenKind::Start:
        return "Start";
    }
}

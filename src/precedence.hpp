#pragma once

#include "token.hpp"

namespace hopgood {
enum class PrecedenceRelation { Yields, Equal, Takes, None };

PrecedenceRelation precedence_relation(TokenKind stack_kind, TokenKind input_kind);
} // namespace hopgood

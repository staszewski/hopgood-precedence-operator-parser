#include "precedence.hpp"
#include "token.hpp"
#include <catch2/catch_test_macros.hpp>

using hopgood::precedence_relation;
using hopgood::PrecedenceRelation;
using hopgood::TokenKind;

TEST_CASE("yields precedence") {
    const TokenKind stack_kind = TokenKind::Plus;
    const TokenKind input_kind = TokenKind::Multiply;
    const auto result = precedence_relation(stack_kind, input_kind);

    REQUIRE(result == PrecedenceRelation::Yields);
}

TEST_CASE("takes precedence") {
    const TokenKind stack_kind = TokenKind::Multiply;
    const TokenKind input_kind = TokenKind::Plus;
    const auto result = precedence_relation(stack_kind, input_kind);

    REQUIRE(result == PrecedenceRelation::Takes);
}

TEST_CASE("equal precedence") {
    const TokenKind stack_kind = TokenKind::LeftParen;
    const TokenKind input_kind = TokenKind::RightParen;
    const auto result = precedence_relation(stack_kind, input_kind);

    REQUIRE(result == PrecedenceRelation::Equal);
}

TEST_CASE("none precedence") {
    const TokenKind stack_kind = TokenKind::RightParen;
    const TokenKind input_kind = TokenKind::LeftParen;
    const auto result = precedence_relation(stack_kind, input_kind);

    REQUIRE(result == PrecedenceRelation::None);
}

TEST_CASE("initial boundary precedence") {
    const TokenKind stack_kind = TokenKind::Start;
    const TokenKind input_kind = TokenKind::Plus;
    const auto result = precedence_relation(stack_kind, input_kind);

    REQUIRE(result == PrecedenceRelation::Yields);
}

TEST_CASE("final boundary precedence") {
    const TokenKind stack_kind = TokenKind::Plus;
    const TokenKind input_kind = TokenKind::End;
    const auto result = precedence_relation(stack_kind, input_kind);

    REQUIRE(result == PrecedenceRelation::Takes);
}

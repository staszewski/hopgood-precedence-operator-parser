#include "token.hpp"
#include <catch2/catch_test_macros.hpp>
#include <string_view>

using hopgood::Token;
using hopgood::token_kind_name;
using hopgood::TokenKind;

TEST_CASE("tokens with the same kind and lexeme are equal") {
    Token t1{TokenKind::Identifier, "a"}, t2{TokenKind::Identifier, "a"};

    REQUIRE(t1 == t2);
}

TEST_CASE("tokens with the same kind but different lexeme are NOT equal") {
    Token t1{TokenKind::Identifier, "a"}, t2{TokenKind::Identifier, "b"};

    REQUIRE(t1 != t2);
}

TEST_CASE("tokens with the different kinds but same lexeme are NOT equal") {
    Token t1{TokenKind::End, "a"}, t2{TokenKind::Identifier, "a"};

    REQUIRE(t1 != t2);
}

TEST_CASE("readable name of TokenKind::Identifier is Identifier") {
    const auto result = token_kind_name(TokenKind::Identifier);

    REQUIRE(result == "Identifier");
}

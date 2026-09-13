#include "ast.hpp"
#include "parser.hpp"
#include "precedence.hpp"
#include "token.hpp"
#include <catch2/catch_test_macros.hpp>
#include <memory>
#include <variant>
#include <vector>

using hopgood::BinaryExpr;
using hopgood::BinaryOp;
using hopgood::Expr;
using hopgood::IdentifierExpr;
using hopgood::parse;
using hopgood::ParseActionRecord;
using hopgood::ParseErrorKind;
using hopgood::ParserAction;
using hopgood::PrecedenceRelation;
using hopgood::Token;
using hopgood::TokenKind;
using hopgood::TraceStep;

namespace {
const IdentifierExpr& require_identifier(const Expr& expr) {
    REQUIRE(std::holds_alternative<IdentifierExpr>(expr));
    return std::get<IdentifierExpr>(expr);
}

const BinaryExpr& require_binary(const Expr& expr) {
    REQUIRE(std::holds_alternative<std::unique_ptr<BinaryExpr>>(expr));
    const auto& binary = std::get<std::unique_ptr<BinaryExpr>>(expr);
    REQUIRE(binary != nullptr);
    return *binary;
}
} // namespace

TEST_CASE("a single identifier followed by End is accepted") {
    std::vector<Token> input_tokens{Token{TokenKind::Identifier, "a"}, Token{TokenKind::End, ""}};

    const auto result = parse(input_tokens);

    REQUIRE(result.outcome.has_value());
    REQUIRE(std::holds_alternative<IdentifierExpr>(result.outcome.value()));

    const IdentifierExpr& identifier = std::get<IdentifierExpr>(result.outcome.value());
    REQUIRE(identifier.name == "a");
}

TEST_CASE("reduction works for a + b") {
    std::vector<Token> input_tokens{Token{TokenKind::Identifier, "a"}, Token{TokenKind::Plus, "+"},
                                    Token(TokenKind::Identifier, "b"), Token{TokenKind::End, ""}};

    const std::vector<TraceStep> expected_trace_steps{
        TraceStep{
            std::vector<Token>{
                Token{TokenKind::Plus, "+"},
                Token{TokenKind::Identifier, "b"},
                Token{TokenKind::End, ""},
            },
            std::vector<Token>{
                Token{TokenKind::Start, ""},
            },
            std::vector<std::string>{"a"},
            PrecedenceRelation::Yields,
            ParseActionRecord{ParserAction::Shift, "+"},
        },
        TraceStep{
            std::vector<Token>{
                Token{TokenKind::End, ""},
            },
            std::vector<Token>{
                Token{TokenKind::Start, ""},
                Token{TokenKind::Plus, "+"},
            },
            std::vector<std::string>{"a", "b"},
            PrecedenceRelation::Takes,
            ParseActionRecord{ParserAction::Reduce, "a + b"},
        },
        TraceStep{
            std::vector<Token>{
                Token{TokenKind::End, ""},
            },
            std::vector<Token>{
                Token{TokenKind::Start, ""},
            },
            std::vector<std::string>{"a + b"},
            PrecedenceRelation::None,
            ParseActionRecord{ParserAction::Accept, ""},
        },
    };

    const auto result = parse(input_tokens);

    REQUIRE(result.outcome.has_value());
    REQUIRE(result.trace_steps == expected_trace_steps);
    REQUIRE(result.actions.size() == 3);

    REQUIRE(result.actions[0].action == ParserAction::Shift);
    REQUIRE(result.actions[0].text == "+");

    REQUIRE(result.actions[1].action == ParserAction::Reduce);
    REQUIRE(result.actions[1].text == "a + b");

    REQUIRE(result.actions.back().action == ParserAction::Accept);

    REQUIRE(std::holds_alternative<std::unique_ptr<BinaryExpr>>(result.outcome.value()));

    const auto& binary_ptr = std::get<std::unique_ptr<BinaryExpr>>(result.outcome.value());
    REQUIRE(binary_ptr != nullptr);

    const BinaryExpr& binary_expr = *binary_ptr;
    REQUIRE(binary_expr.op == BinaryOp::Add);

    REQUIRE(std::holds_alternative<IdentifierExpr>(binary_expr.lhs));
    REQUIRE(std::holds_alternative<IdentifierExpr>(binary_expr.rhs));

    const IdentifierExpr& lhs = std::get<IdentifierExpr>(binary_expr.lhs);
    const IdentifierExpr& rhs = std::get<IdentifierExpr>(binary_expr.rhs);

    REQUIRE(lhs.name == "a");
    REQUIRE(rhs.name == "b");
}

TEST_CASE("reduction works for a * b + c") {
    std::vector<Token> input_tokens{
        Token{TokenKind::Identifier, "a"}, Token{TokenKind::Multiply, "*"},
        Token{TokenKind::Identifier, "b"}, Token{TokenKind::Plus, "+"},
        Token{TokenKind::Identifier, "c"}, Token{TokenKind::End, ""}};

    const auto result = parse(input_tokens);

    REQUIRE(result.outcome.has_value());
    REQUIRE(result.actions.size() == 5);

    REQUIRE(result.actions[0].action == ParserAction::Shift);
    REQUIRE(result.actions[0].text == "*");

    REQUIRE(result.actions[1].action == ParserAction::Reduce);
    REQUIRE(result.actions[1].text == "a * b");

    REQUIRE(result.actions[2].action == ParserAction::Shift);
    REQUIRE(result.actions[2].text == "+");

    REQUIRE(result.actions[3].action == ParserAction::Reduce);
    REQUIRE(result.actions[3].text == "a * b + c");

    REQUIRE(result.actions.back().action == ParserAction::Accept);
}

TEST_CASE("reduction works for a + b * c") {
    std::vector<Token> input_tokens{
        Token{TokenKind::Identifier, "a"}, Token{TokenKind::Plus, "+"},
        Token{TokenKind::Identifier, "b"}, Token{TokenKind::Multiply, "*"},
        Token{TokenKind::Identifier, "c"}, Token{TokenKind::End, ""}};

    const auto result = parse(input_tokens);

    REQUIRE(result.outcome.has_value());
    REQUIRE(result.actions.size() == 5);

    REQUIRE(result.actions[0].action == ParserAction::Shift);
    REQUIRE(result.actions[0].text == "+");

    REQUIRE(result.actions[1].action == ParserAction::Shift);
    REQUIRE(result.actions[1].text == "*");

    REQUIRE(result.actions[2].action == ParserAction::Reduce);
    REQUIRE(result.actions[2].text == "b * c");

    REQUIRE(result.actions[3].action == ParserAction::Reduce);
    REQUIRE(result.actions[3].text == "a + b * c");

    REQUIRE(result.actions.back().action == ParserAction::Accept);

    const BinaryExpr& root = require_binary(result.outcome.value());
    REQUIRE(root.op == BinaryOp::Add);
    REQUIRE(require_identifier(root.lhs).name == "a");

    const BinaryExpr& product = require_binary(root.rhs);
    REQUIRE(product.op == BinaryOp::Multiply);
    REQUIRE(require_identifier(product.lhs).name == "b");
    REQUIRE(require_identifier(product.rhs).name == "c");
}

TEST_CASE("reduction works for (a + b) * c") {
    std::vector<Token> input_tokens{
        Token{TokenKind::LeftParen, "("},  Token{TokenKind::Identifier, "a"},
        Token{TokenKind::Plus, "+"},       Token{TokenKind::Identifier, "b"},
        Token{TokenKind::RightParen, ")"}, Token{TokenKind::Multiply, "*"},
        Token{TokenKind::Identifier, "c"}, Token{TokenKind::End, ""}};

    const auto result = parse(input_tokens);

    REQUIRE(result.outcome.has_value());
    REQUIRE(result.trace_steps.size() == result.actions.size());
    REQUIRE(result.actions.size() == 8);

    REQUIRE(result.actions[0].action == ParserAction::Shift);
    REQUIRE(result.actions[0].text == "(");

    REQUIRE(result.actions[1].action == ParserAction::Shift);
    REQUIRE(result.actions[1].text == "+");

    REQUIRE(result.actions[2].action == ParserAction::Reduce);
    REQUIRE(result.actions[2].text == "a + b");

    REQUIRE(result.actions[3].action == ParserAction::Shift);
    REQUIRE(result.actions[3].text == ")");

    REQUIRE(result.actions[4].action == ParserAction::Reduce);
    REQUIRE(result.actions[4].text == "(a + b)");

    REQUIRE(result.actions[5].action == ParserAction::Shift);
    REQUIRE(result.actions[5].text == "*");

    REQUIRE(result.actions[6].action == ParserAction::Reduce);
    REQUIRE(result.actions[6].text == "(a + b) * c");

    REQUIRE(result.actions.back().action == ParserAction::Accept);

    const BinaryExpr& root = require_binary(result.outcome.value());
    REQUIRE(root.op == BinaryOp::Multiply);
    REQUIRE(require_identifier(root.rhs).name == "c");

    const BinaryExpr& sum = require_binary(root.lhs);
    REQUIRE(sum.op == BinaryOp::Add);
    REQUIRE(require_identifier(sum.lhs).name == "a");
    REQUIRE(require_identifier(sum.rhs).name == "b");
}

TEST_CASE("a + end is wrong; expression missing right operand is rejected") {
    std::vector<Token> input_tokens{Token{TokenKind::Identifier, "a"}, Token{TokenKind::Plus, "+"},
                                    Token{TokenKind::End, ""}};

    const auto result = parse(input_tokens);

    REQUIRE_FALSE(result.outcome.has_value());
    REQUIRE(result.outcome.error().kind == ParseErrorKind::MissingOperand);
    REQUIRE(result.outcome.error().position == 2);
    REQUIRE(result.trace_steps.size() == result.actions.size());
    REQUIRE(result.trace_steps.back().action_record.action == ParserAction::Error);
    REQUIRE(result.actions.size() == 2);

    REQUIRE(result.actions[0].action == ParserAction::Shift);
    REQUIRE(result.actions[0].text == "+");

    REQUIRE(result.actions.back().action == ParserAction::Error);
}

TEST_CASE("expression missing closing parenthesis is rejected") {
    std::vector<Token> input_tokens{Token{TokenKind::LeftParen, "("},
                                    Token{TokenKind::Identifier, "a"}, Token{TokenKind::End, ""}};

    const auto result = parse(input_tokens);

    REQUIRE_FALSE(result.outcome.has_value());
    REQUIRE(result.outcome.error().kind == ParseErrorKind::UnmatchedParenthesis);
    REQUIRE(result.outcome.error().position == 2);
    REQUIRE(result.actions.size() == 2);

    REQUIRE(result.actions[0].action == ParserAction::Shift);
    REQUIRE(result.actions[0].text == "(");

    REQUIRE(result.actions.back().action == ParserAction::Error);
}

TEST_CASE("expression without end token is rejected") {
    std::vector<Token> input_tokens{Token{TokenKind::Identifier, "a"}};

    const auto result = parse(input_tokens);

    REQUIRE_FALSE(result.outcome.has_value());
    REQUIRE(result.outcome.error().kind == ParseErrorKind::UnexpectedToken);
    REQUIRE(result.outcome.error().position == 1);
    REQUIRE(result.actions.size() == 1);

    REQUIRE(result.actions[0].action == ParserAction::Error);
}

TEST_CASE("start is automatically added so first token of start has to be rejected") {
    std::vector<Token> input_tokens{Token{TokenKind::Start, ""}, Token{TokenKind::Identifier, "a"},
                                    Token{TokenKind::End, ""}};

    const auto result = parse(input_tokens);

    REQUIRE_FALSE(result.outcome.has_value());
    REQUIRE(result.outcome.error().kind == ParseErrorKind::UnexpectedToken);
    REQUIRE(result.outcome.error().position == 0);
    REQUIRE(result.actions.size() == 1);

    REQUIRE(result.actions[0].action == ParserAction::Error);
}

TEST_CASE("no left parenthesis leads to rejection") {
    std::vector<Token> input_tokens{Token{TokenKind::Identifier, "a"},
                                    Token{TokenKind::RightParen, ")"}, Token{TokenKind::End, ""}};

    const auto result = parse(input_tokens);

    REQUIRE_FALSE(result.outcome.has_value());
    REQUIRE(result.outcome.error().kind == ParseErrorKind::UnmatchedParenthesis);
    REQUIRE(result.outcome.error().position == 1);
    REQUIRE(result.actions.size() == 1);

    REQUIRE(result.actions[0].action == ParserAction::Error);
}

TEST_CASE("two adjacent identifiers are rejected as a missing operator") {
    std::vector<Token> input_tokens{Token{TokenKind::Identifier, "a"},
                                    Token{TokenKind::Identifier, "b"},
                                    Token{TokenKind::End, ""}};

    const auto result = parse(input_tokens);

    REQUIRE_FALSE(result.outcome.has_value());
    REQUIRE(result.outcome.error().kind == ParseErrorKind::MissingOperator);
    REQUIRE(result.outcome.error().position == 1);
    REQUIRE_FALSE(result.actions.empty());
    REQUIRE(result.actions.back().action == ParserAction::Error);
}

TEST_CASE("an operator where an operand is required is rejected") {
    std::vector<Token> input_tokens{
        Token{TokenKind::Identifier, "a"}, Token{TokenKind::Plus, "+"},
        Token{TokenKind::Multiply, "*"}, Token{TokenKind::Identifier, "b"},
        Token{TokenKind::End, ""}};

    const auto result = parse(input_tokens);

    REQUIRE_FALSE(result.outcome.has_value());
    REQUIRE(result.outcome.error().kind == ParseErrorKind::MissingOperand);
    REQUIRE(result.outcome.error().position == 2);
    REQUIRE_FALSE(result.actions.empty());
    REQUIRE(result.actions.back().action == ParserAction::Error);
}

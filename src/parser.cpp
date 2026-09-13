#include "ast.hpp"
#include "parser.hpp"
#include "precedence.hpp"
#include "token.hpp"
#include <cstddef>
#include <expected>
#include <memory>
#include <string>
#include <utility>
#include <vector>

using hopgood::BinaryExpr;
using hopgood::BinaryOp;
using hopgood::Expr;
using hopgood::format_expr;
using hopgood::IdentifierExpr;
using hopgood::ParseActionRecord;
using hopgood::ParserAction;
using hopgood::ParseResult;
using hopgood::PrecedenceRelation;
using hopgood::Token;
using hopgood::TokenKind;
using hopgood::TraceStep;

namespace {
std::vector<std::string> expression_stack_snapshot(const std::vector<Expr>& expression_stack) {
    std::vector<std::string> snapshot;
    snapshot.reserve(expression_stack.size());

    for (const Expr& expression : expression_stack) {
        snapshot.push_back(format_expr(expression));
    }

    return snapshot;
}

TraceStep create_trace_step(std::span<const Token> tokens, std::size_t current_pos,
                            const std::vector<Token>& operator_stack,
                            const std::vector<Expr>& expression_stack, PrecedenceRelation relation,
                            const ParseActionRecord& action_record) {
    std::span<const Token> remaining_tokens = tokens.subspan(current_pos);
    std::vector<Token> remaining_input(remaining_tokens.begin(), remaining_tokens.end());

    return TraceStep{remaining_input, operator_stack, expression_stack_snapshot(expression_stack),
                     relation, action_record};
};

void record_error(std::vector<ParseActionRecord>& actions, std::vector<TraceStep>& trace_steps,
                  std::span<const Token> tokens, std::size_t current_pos,
                  const std::vector<Token>& operator_stack,
                  const std::vector<Expr>& expression_stack, PrecedenceRelation relation) {
    const ParseActionRecord action_record{ParserAction::Error, ""};
    trace_steps.push_back(create_trace_step(tokens, current_pos, operator_stack, expression_stack,
                                            relation, action_record));
    actions.push_back(action_record);
}

bool try_reduce_binary(std::vector<Token>& operator_stack, std::vector<Expr>& expression_stack,
                       std::vector<ParseActionRecord>& actions, std::vector<TraceStep>& trace_steps,
                       std::span<const Token> tokens, std::size_t current_pos,
                       PrecedenceRelation relation) {

    if (operator_stack.size() > 1 && expression_stack.size() >= 2) {
        const Token operator_token = operator_stack.back();
        if (operator_token.kind == TokenKind::Plus || operator_token.kind == TokenKind::Multiply) {
            const BinaryOp binary_op =
                operator_token.kind == TokenKind::Plus ? BinaryOp::Add : BinaryOp::Multiply;
            ParseActionRecord action_record{ParserAction::Reduce, ""};
            TraceStep trace_step = create_trace_step(tokens, current_pos, operator_stack,
                                                     expression_stack, relation, action_record);

            Expr right = std::move(expression_stack.back());
            expression_stack.pop_back();
            Expr left = std::move(expression_stack.back());
            expression_stack.pop_back();

            operator_stack.pop_back();

            expression_stack.emplace_back(std::make_unique<BinaryExpr>(
                BinaryExpr{binary_op, std::move(left), std::move(right)}));

            const std::string reduced = format_expr(expression_stack.back());
            action_record.text = reduced;
            trace_step.action_record = action_record;

            trace_steps.push_back(std::move(trace_step));
            actions.push_back(action_record);
            return true;
        }
    };
    return false;
};

bool try_reduce_parenthesized(std::vector<Token>& operator_stack,
                              std::vector<Expr>& expression_stack,
                              std::vector<ParseActionRecord>& actions,
                              std::vector<TraceStep>& trace_steps, std::span<const Token> tokens,
                              std::size_t current_pos, PrecedenceRelation relation) {
    if (operator_stack.size() >= 3 && !expression_stack.empty()) {
        const Token top_token = operator_stack.back();
        const Token token_below_top = operator_stack[operator_stack.size() - 2];
        const bool is_right_paren = top_token.kind == TokenKind::RightParen;
        const bool is_left_paren = token_below_top.kind == TokenKind::LeftParen;

        if (is_right_paren && is_left_paren) {
            const std::string reduced =
                token_below_top.lexeme + format_expr(expression_stack.back()) + top_token.lexeme;
            const ParseActionRecord action_record =
                ParseActionRecord{ParserAction::Reduce, reduced};
            trace_steps.push_back(create_trace_step(tokens, current_pos, operator_stack,
                                                    expression_stack, relation, action_record));
            operator_stack.pop_back(); // pop RightParen
            operator_stack.pop_back(); // pop LeftParen
            actions.push_back(action_record);
            return true;
        }
    }
    return false;
}

}; // namespace

ParseResult hopgood::parse(std::span<const Token> tokens) {
    std::size_t index = 0;
    std::vector<Token> operator_stack = {Token{TokenKind::Start, ""}};
    std::vector<Expr> expression_stack = {};
    std::vector<ParseActionRecord> actions = {};
    std::vector<TraceStep> trace_steps = {};
    bool expecting_operand = true;
    while (index < tokens.size()) {
        Token token = tokens[index];
        switch (token.kind) {
        case TokenKind::Start:
            record_error(actions, trace_steps, tokens, index, operator_stack, expression_stack,
                         PrecedenceRelation::None);
            return ParseResult{std::move(actions), std::move(trace_steps),
                               std::unexpected(ParseError{ParseErrorKind::UnexpectedToken, index})};
        case TokenKind::Identifier:
            if (!expecting_operand) {
                record_error(actions, trace_steps, tokens, index, operator_stack, expression_stack,
                             PrecedenceRelation::None);
                return ParseResult{
                    std::move(actions), std::move(trace_steps),
                    std::unexpected(ParseError{ParseErrorKind::MissingOperator, index})};
            }
            expecting_operand = false;
            expression_stack.push_back(IdentifierExpr{token.lexeme});
            index++;
            break;
        case TokenKind::Plus:
        case TokenKind::Multiply:
        case TokenKind::LeftParen:
        case TokenKind::RightParen: {
            if (expecting_operand &&
                (token.kind == TokenKind::Plus || token.kind == TokenKind::Multiply)) {
                record_error(actions, trace_steps, tokens, index, operator_stack, expression_stack,
                             PrecedenceRelation::None);
                return ParseResult{
                    std::move(actions), std::move(trace_steps),
                    std::unexpected(ParseError{ParseErrorKind::MissingOperand, index})};
            }
            const PrecedenceRelation precedence =
                precedence_relation(operator_stack.back().kind, token.kind);
            switch (precedence) {
            case PrecedenceRelation::None: {
                record_error(actions, trace_steps, tokens, index, operator_stack, expression_stack,
                             precedence);

                ParseErrorKind error_kind = ParseErrorKind::UnexpectedToken;

                if (token.kind == TokenKind::RightParen) {
                    error_kind = ParseErrorKind::UnmatchedParenthesis;
                }
                return ParseResult{std::move(actions), std::move(trace_steps),
                                   std::unexpected(ParseError{error_kind, index})};
            }
            case PrecedenceRelation::Yields:
            case PrecedenceRelation::Equal: {
                const ParseActionRecord action_record =
                    ParseActionRecord{ParserAction::Shift, token.lexeme};
                trace_steps.push_back(create_trace_step(
                    tokens, index, operator_stack, expression_stack, precedence, action_record));
                operator_stack.push_back(token);
                actions.push_back(action_record);
                if (token.kind == TokenKind::Plus || token.kind == TokenKind::Multiply ||
                    token.kind == TokenKind::LeftParen) {
                    expecting_operand = true;
                }

                if (token.kind == TokenKind::RightParen) {
                    expecting_operand = false;
                }

                index++;
                break;
            }
            case PrecedenceRelation::Takes:
                const bool binary_reduction_result =
                    try_reduce_binary(operator_stack, expression_stack, actions, trace_steps,
                                      tokens, index, precedence);
                if (!binary_reduction_result) {
                    const bool parenthesized_reduction_result =
                        try_reduce_parenthesized(operator_stack, expression_stack, actions,
                                                 trace_steps, tokens, index, precedence);

                    if (!parenthesized_reduction_result) {
                        // wrong state
                        record_error(actions, trace_steps, tokens, index, operator_stack,
                                     expression_stack, precedence);
                        return ParseResult{
                            std::move(actions), std::move(trace_steps),
                            std::unexpected(ParseError{ParseErrorKind::UnexpectedToken, index})};
                    }
                }
                break;
            }
            break;
        }
        case TokenKind::End:
            const PrecedenceRelation precedence =
                precedence_relation(operator_stack.back().kind, token.kind);
            // acceptation
            const bool is_last_token = index + 1 == tokens.size();
            const bool last_operator_is_start_token =
                operator_stack.size() == 1 && operator_stack.back().kind == TokenKind::Start;
            const bool is_last_operand = expression_stack.size() == 1;

            if (is_last_token && last_operator_is_start_token && is_last_operand) {
                const ParseActionRecord action_record =
                    ParseActionRecord{ParserAction::Accept, token.lexeme};
                trace_steps.push_back(create_trace_step(
                    tokens, index, operator_stack, expression_stack, precedence, action_record));
                actions.push_back(action_record);
                return ParseResult{std::move(actions), std::move(trace_steps),
                                   std::move(expression_stack.back())};
            }

            // reduction
            switch (precedence) {
            case PrecedenceRelation::Equal:
            case PrecedenceRelation::Yields: {
                record_error(actions, trace_steps, tokens, index, operator_stack, expression_stack,
                             precedence);
                return ParseResult{
                    std::move(actions), std::move(trace_steps),
                    std::unexpected(ParseError{ParseErrorKind::UnexpectedToken, index})};
            }
            case PrecedenceRelation::None: {
                record_error(actions, trace_steps, tokens, index, operator_stack, expression_stack,
                             precedence);
                ParseErrorKind error_kind = ParseErrorKind::UnexpectedToken;

                if (!operator_stack.empty() && operator_stack.back().kind == TokenKind::LeftParen) {
                    error_kind = ParseErrorKind::UnmatchedParenthesis;
                }
                return ParseResult{std::move(actions), std::move(trace_steps),
                                   std::unexpected(ParseError{error_kind, index})};
            }
            case PrecedenceRelation::Takes: {
                const bool binary_reduction_result =
                    try_reduce_binary(operator_stack, expression_stack, actions, trace_steps,
                                      tokens, index, precedence);
                if (!binary_reduction_result) {
                    const bool parenthesized_reduction_result =
                        try_reduce_parenthesized(operator_stack, expression_stack, actions,
                                                 trace_steps, tokens, index, precedence);

                    if (!parenthesized_reduction_result) {
                        // wrong state
                        record_error(actions, trace_steps, tokens, index, operator_stack,
                                     expression_stack, precedence);
                        return ParseResult{
                            std::move(actions), std::move(trace_steps),
                            std::unexpected(ParseError{ParseErrorKind::MissingOperand, index})};
                    }
                    break;
                }
            } break;
            }
        };
    };

    record_error(actions, trace_steps, tokens, index, operator_stack, expression_stack,
                 PrecedenceRelation::None);
    return ParseResult{std::move(actions), std::move(trace_steps),
                       std::unexpected(ParseError{ParseErrorKind::UnexpectedToken, index})};
};

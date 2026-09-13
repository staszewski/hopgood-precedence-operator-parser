#pragma once

#include "ast.hpp"
#include "precedence.hpp"
#include "token.hpp"
#include <expected>
#include <span>
#include <string>
#include <vector>

namespace hopgood {

enum class ParseErrorKind {
    UnexpectedToken,
    MissingOperand,
    MissingOperator,
    UnmatchedParenthesis,
    UndefinedPrecedenceRelation
};

struct ParseError {
    ParseErrorKind kind;
    std::size_t position;
};

enum class ParserAction { Shift, Reduce, Accept, Error };

struct ParseActionRecord {
    ParserAction action;
    std::string text;

    bool operator==(const ParseActionRecord&) const = default;
};

struct TraceStep {
    std::vector<Token> remaining_input;
    std::vector<Token> operator_stack;
    std::vector<std::string> operand_stack;
    PrecedenceRelation relation;
    ParseActionRecord action_record;

    bool operator==(const TraceStep&) const = default;
};

struct ParseResult {
    std::vector<ParseActionRecord> actions;
    std::vector<TraceStep> trace_steps;
    std::expected<Expr, ParseError> outcome;
};

ParseResult parse(std::span<const Token> tokens);
} // namespace hopgood

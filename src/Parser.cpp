#include "Parser.h"

Parser::Parser(std::vector<Token> tokens)
{
    this->tokens = tokens;
    position = 0;
}

Token Parser::peek()
{
    return tokens[position];
}

Token Parser::advance()
{
    return tokens[position++];
}

std::vector<std::unique_ptr<ASTNode>> Parser::parse()
{
    std::vector<std::unique_ptr<ASTNode>> nodes;

    while (peek().type != TokenType::END)
    {
        Token token = peek();

        if (token.type == TokenType::PRINT)
        {
            nodes.push_back(
                parsePrint());
        }

        else if (token.type == TokenType::LET)
        {
            nodes.push_back(
                parseVariableDeclaration());
        }

        else if (token.type == TokenType::IDENTIFIER)
        {

            if (tokens[position + 1].type == TokenType::EQUAL)
            {
                nodes.push_back(
                    parseAssignment());
            }

            else
            {
                nodes.push_back(
                    parseExpression());
            }
        }

        else
        {
            advance();
        }
    }

    return nodes;
}

std::unique_ptr<ASTNode> Parser::parseVariableDeclaration()
{

    // let
    advance();

    Token name =
        advance();

    // =
    advance();

    auto value =
        parseExpression();

    return std::make_unique<VariableDeclarationNode>(
        name.value,
        std::move(value));
}

std::unique_ptr<ASTNode> Parser::parseAssignment()
{

    Token name =
        advance();

    // =
    advance();

    auto value =
        parseExpression();

    return std::make_unique<AssignmentNode>(
        name.value,
        std::move(value));
}

std::unique_ptr<ASTNode> Parser::parsePrint()
{

    // print
    advance();

    auto expression =
        parseExpression();

    return std::make_unique<PrintNode>(
        std::move(expression));
}

std::unique_ptr<ASTNode> Parser::parseExpression()
{
    return parseComparison();
}

std::unique_ptr<ASTNode> Parser::parseComparison()
{

    auto left =
        parseTerm();

    while (
        peek().type == TokenType::EQUAL_EQUAL ||
        peek().type == TokenType::NOT_EQUAL ||
        peek().type == TokenType::BIGGER ||
        peek().type == TokenType::SMALLER ||
        peek().type == TokenType::BIGGER_EQUAL ||
        peek().type == TokenType::SMALLER_EQUAL ||
        peek().type == TokenType::THREE_WAY_COMPARE ||
        peek().type == TokenType::RANGE
    )
    {

        Token op =
            advance();

        auto right =
            parseTerm();

        left =
            std::make_unique<BinaryExpressionNode>(
                op.value,
                std::move(left),
                std::move(right));
    }

    return left;
}

std::unique_ptr<ASTNode> Parser::parseTerm()
{

    auto left =
        parseFactor();

    while (
        peek().type == TokenType::MULTIPLY ||
        peek().type == TokenType::DIVIDE ||
        peek().type == TokenType::MODULO)
    {

        Token op =
            advance();

        auto right =
            parseFactor();

        left =
            std::make_unique<BinaryExpressionNode>(
                op.value,
                std::move(left),
                std::move(right));
    }

    return left;
}

std::unique_ptr<ASTNode> Parser::parseFactor()
{

    Token token =
        advance();

    if (token.type == TokenType::NUMBER)
    {
        return std::make_unique<NumberNode>(
            std::stoi(token.value));
    }

    else if (token.type == TokenType::STRING)
    {
        return std::make_unique<StringNode>(
            token.value);
    }

    else if (token.type == TokenType::BOOLEAN)
    {
        return std::make_unique<BooleanNode>(
            token.value == "true");
    }

    else if (token.type == TokenType::IDENTIFIER)
    {
        return std::make_unique<VariableAccessNode>(
            token.value);
    }

    return nullptr;
}
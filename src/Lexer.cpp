#include "Lexer.h"
#include <cctype>

char Lexer::advance()
{
    char current = source[position++];

    if (current == '\n')
    {
        line++;
        column = 1;
    }
    else
    {
        column++;
    }

    return current;
}

void Lexer::addToken(
    std::vector<Token> &tokens,
    TokenType type,
    const std::string &value)
{
    tokens.push_back(
        {type,
         value,
         line,
         column});
}

Lexer::Lexer(std::string source)
{
    this->source = source;

    position = 0;

    line = 1;
    column = 1;
}

std::vector<Token> Lexer::tokenize()
{
    std::vector<Token> tokens;

    auto startsWith = [&](const std::string &prefix)
    {
        return position + prefix.size() <= source.size() &&
               source.compare(position, prefix.size(), prefix) == 0;
    };

    while (position < source.size())
    {
        char current = source[position];

        if (std::isspace(static_cast<unsigned char>(current)))
        {
            advance();
            continue;
        }

        if (current == '"')
        {
            advance();
            std::string text;

            while (position < source.size() && source[position] != '"')
            {
                text += advance();
            }

            if (position < source.size() && source[position] == '"')
            {
                advance();
            }

            addToken(tokens, TokenType::STRING, text);
            continue;
        }

        if (std::isdigit(static_cast<unsigned char>(current)))
        {
            std::string number;

            while (
                position < source.size() &&
                std::isdigit(static_cast<unsigned char>(source[position])))
            {
                number += advance();
            }

            addToken(tokens, TokenType::NUMBER, number);
            continue;
        }
        if (std::isalpha(static_cast<unsigned char>(current)) || current == '_')
        {
            std::string word;

            while (
                position < source.size() &&
                (std::isalnum(static_cast<unsigned char>(source[position])) ||
                 source[position] == '_'))
            {
                word += advance();
            }

            if (word == "print")
            {
                addToken(tokens, TokenType::PRINT, word);
            }
            else if (word == "let")
            {
                addToken(tokens, TokenType::LET, word);
            }
            else if (word == "true" || word == "false")
            {
                addToken(tokens, TokenType::BOOLEAN, word);
            }
            else
            {
                addToken(tokens, TokenType::IDENTIFIER, word);
            }

            continue;
        }

            if (startsWith("<=>"))
            {
                addToken(tokens, TokenType::THREE_WAY_COMPARE, "<=>");
                advance();
                advance();
                advance();
                continue;
            }

            if (startsWith(".."))
            {
                addToken(tokens, TokenType::RANGE, "..");
                advance();
                advance();
                continue;
            }

            if (startsWith(">="))
            {
                addToken(tokens, TokenType::BIGGER_EQUAL, ">=");
                advance();
                advance();
                continue;
            }

            if (startsWith("<="))
            {
                addToken(tokens, TokenType::SMALLER_EQUAL, "<=");
                advance();
                advance();
                continue;
            }

            if (startsWith("=="))
            {
                addToken(tokens, TokenType::EQUAL_EQUAL, "==");
                advance();
                advance();
                continue;
            }

            if (startsWith("!="))
            {
                addToken(tokens, TokenType::NOT_EQUAL, "!=");
                advance();
                advance();
                continue;
            }

            if (current == '=')
            {
                addToken(tokens, TokenType::EQUAL, "=");
                advance();
                continue;
            }

            if (current == '>')
            {
                addToken(tokens, TokenType::BIGGER, ">");
                advance();
                continue;
            }

            if (current == '<')
            {
                addToken(tokens, TokenType::SMALLER, "<");
                advance();
                continue;
            }

            if (current == '+')
            {
                addToken(tokens, TokenType::PLUS, "+");
                advance();
                continue;
            }

            if (current == '-')
            {
                addToken(tokens, TokenType::MINUS, "-");
                advance();
                continue;
            }

            if (current == '*')
            {
                addToken(tokens, TokenType::MULTIPLY, "*");
                advance();
                continue;
            }

            if (current == '/')
            {
                addToken(tokens, TokenType::DIVIDE, "/");
                advance();
                continue;
            }

            if (current == '%')
            {
                addToken(tokens, TokenType::MODULO, "%");
                advance();
                continue;
            }

            if (current == '(')
            {
                addToken(tokens, TokenType::LPAREN, "(");
                advance();
                continue;
            }

            if (current == ')')
            {
                addToken(tokens, TokenType::RPAREN, ")");
                advance();
                continue;
            }

            advance();
        }

        tokens.push_back({TokenType::END, ""});
        return tokens;
    }
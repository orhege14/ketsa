#include "Lexer.h"
#include <cctype>

Lexer::Lexer(std::string source)
{
    this->source = source;
    position = 0;
}

std::vector<Token> Lexer::tokenize()
{
    std::vector<Token> tokens;

    while (position < source.length())
    {

        if (std::isspace(source[position]))
        {
            position++;
            continue;
        }

        // print
        if (source.substr(position, 5) == "print")
        {
            tokens.push_back(
                {TokenType::PRINT, "print"});

            position += 5;
            continue;
        }

        // let
        if (source.substr(position, 3) == "let")
        {
            tokens.push_back(
                {TokenType::LET, "let"});

            position += 3;
            continue;
        }

        // string
        if (source[position] == '"')
        {
            position++;

            std::string text;

            while (position < source.length() && source[position] != '"')
            {
                text += source[position];
                position++;
            }

            position++;

            tokens.push_back(
                {TokenType::STRING, text});

            continue;
        }
        // number
        else if (std::isdigit(source[position]))
        {
            std::string number;

            while (position < source.length() && std::isdigit(source[position]))
            {
                number += source[position];
                position++;
            }

            tokens.push_back(
                {TokenType::NUMBER, number});

            continue;
        }
        // bool
        else if (source.substr(position, 4) == "true" || source.substr(position, 5) == "false")
        {
            std::string boolean;
            if (source.substr(position, 4) == "true")
            {
                boolean = "true";
                position += 4;
            }
            else
            {
                boolean = "false";
                position += 5;
            }

            tokens.push_back(
                {TokenType::BOOLEAN, boolean});

            continue;
        }
        // Operators

        if (source.substr(position, 4) == "?//=")
        {
            tokens.push_back(
                {TokenType::NULL_INTEGER_DIVIDE_EQUAL, "?//="});

            position += 4;
            continue;
        }
        else if (source.substr(position, 4) == "?**=")
        {
            tokens.push_back(
                {TokenType::NULL_POWER_ASSIGN, "?**="});

            position += 4;
            continue;
        }
        else if (source.substr(position, 3) == "//=")
        {
            tokens.push_back(
                {TokenType::INTEGER_DIVIDE_EQUAL, "//="});

            position += 3;
            continue;
        }
        else if (source.substr(position, 3) == "**=")
        {
            tokens.push_back(
                {TokenType::POWER_EQUAL, "**="});

            position += 3;
            continue;
        }
        else if (source.substr(position, 3) == "?/=")
        {
            tokens.push_back(
                {TokenType::NULL_DIVIDE_EQUAL, "?/="});

            position += 3;
            continue;
        }
        else if (source.substr(position, 3) == "?*=")
        {
            tokens.push_back(
                {TokenType::NULL_MULTIPLY_EQUAL, "?*="});

            position += 3;
            continue;
        }
        else if (source.substr(position, 3) == "?-=")
        {
            tokens.push_back(
                {TokenType::NULL_MINUS_EQUAL, "?-="});

            position += 3;
            continue;
        }
        else if (source.substr(position, 3) == "?+=")
        {
            tokens.push_back(
                {TokenType::NULL_PLUS_EQUAL, "?+="});

            position += 3;
            continue;
        }
        else if (source.substr(position, 3) == "<=>")
        {
            tokens.push_back(
                {TokenType::THREE_WAY_COMPARE, "<=>"});

            position += 3;
            continue;
        }
        else if (source.substr(position, 2) == ">=")
        {
            tokens.push_back(
                {TokenType::BIGGER_EQUAL, ">="});

            position += 2;
            continue;
        }
        else if (source.substr(position, 2) == "<=")
        {
            tokens.push_back(
                {TokenType::SMALLER_EQUAL, "<="});

            position += 2;
            continue;
        }
        else if (source.substr(position, 2) == "==")
        {
            tokens.push_back(
                {TokenType::EQUAL_EQUAL, "=="});

            position += 2;
            continue;
        }
        else if (source.substr(position, 2) == "!=")
        {
            tokens.push_back(
                {TokenType::NOT_EQUAL, "!="});

            position += 2;
            continue;
        }
        else if (source.substr(position, 2) == "/=")
        {
            tokens.push_back(
                {TokenType::DIVIDE_EQUAL, "/="});

            position += 2;
            continue;
        }

        else if (source.substr(position, 2) == "*=")
        {
            tokens.push_back(
                {TokenType::MULTIPLY_EQUAL, "*="});

            position += 2;
            continue;
        }
        else if (source.substr(position, 2) == "?=")
        {
            tokens.push_back(
                {TokenType::NULL_EQUAL, "?="});

            position += 2;
            continue;
        }
        else if (source.substr(position, 2) == "-=")
        {
            tokens.push_back(
                {TokenType::MINUS_EQUAL, "-="});

            position += 2;
            continue;
        }
        else if (source.substr(position, 2) == "+=")
        {
            tokens.push_back(
                {TokenType::PLUS_EQUAL, "+="});

            position += 2;
            continue;
        }
        else if (source.substr(position, 2) == "|>")
        {
            tokens.push_back(
                {TokenType::PIPE, "|>"});

            position += 2;
            continue;
        }

        else if (source.substr(position, 2) == "..")
        {
            tokens.push_back(
                {TokenType::RANGE, ".."});

            position += 2;
            continue;
        }
        else if (source[position] == '=')
        {
            tokens.push_back(
                {TokenType::EQUAL, "="});

            position++;
            continue;
        }
        else if (source[position] == '+')
        {
            tokens.push_back(
                {TokenType::PLUS, "+"});

            position++;
            continue;
        }
        else if (source[position] == '-')
        {
            tokens.push_back(
                {TokenType::MINUS, "-"});

            position++;
            continue;
        }
        else if (source[position] == '*')
        {
            tokens.push_back(
                {TokenType::MULTIPLY, "*"});

            position++;
            continue;
        }
        else if (source[position] == '/')
        {
            tokens.push_back(
                {TokenType::DIVIDE, "/"});

            position++;
            continue;
        }
        else if (source[position] == '>')
        {
            tokens.push_back(
                {TokenType::BIGGER, ">"});

            position++;
            continue;
        }
        else if (source[position] == '<')
        {
            tokens.push_back(
                {TokenType::SMALLER, "<"});

            position++;
            continue;
        }
        // değişken adı
        if (std::isalpha(source[position]))
        {
            std::string word;

            while (position < source.length() && std::isalnum(source[position]))
            {
                word += source[position];
                position++;
            }

            tokens.push_back(
                {TokenType::IDENTIFIER, word});

            continue;
        }
        else if (source[position] == '%')
        {
            tokens.push_back(
                {TokenType::MODULO, "%"});

            position++;
            continue;
        }
        position++;
    }

    tokens.push_back(
        {TokenType::END, ""});

    return tokens;
}
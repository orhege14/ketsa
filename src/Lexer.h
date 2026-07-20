#pragma once

#include <string>
#include <vector>

enum class TokenType
{
    END,
    PRINT,
    LET,
    IDENTIFIER,
    NUMBER,
    STRING,
    BOOLEAN,
    EQUAL,
    EQUAL_EQUAL,
    NOT_EQUAL,
    BIGGER,
    SMALLER,
    BIGGER_EQUAL,
    SMALLER_EQUAL,
    THREE_WAY_COMPARE,
    RANGE,
    PLUS,
    MINUS,
    MULTIPLY,
    DIVIDE,
    MODULO,
    AND_AND,
    OR_OR,
    XOR_XOR,
    LPAREN,
    RPAREN
};

struct Token
{
    TokenType type;
    std::string value;

    int line;
    int column;

    Token(
        TokenType type,
        const std::string &value,
        int line = 0,
        int column = 0)
        : type(type),
          value(value),
          line(line),
          column(column)
    {
    }
};

class Lexer
{
private:
    size_t position;
    std::string source;
    int line;
    int column;

    void addToken(
        std::vector<Token> &tokens,
        TokenType type,
        const std::string &value);
        
    char advance();

public:
    Lexer(std::string source);

    std::vector<Token> tokenize();
};
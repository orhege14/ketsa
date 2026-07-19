#pragma once

#include <string>

#include <vector>

enum class TokenType
{
    // Keywords
    PRINT,          // Print command
    LET,            // Variable declaration
    IF,             // Conditional statement
    ELSE,           // Alternative condition

    // Access Modifiers
    PUBLIC,         // Public access modifier
    PRIVATE,        // Private access modifier

    // Values
    IDENTIFIER,     // Variable or name
    STRING,         // Text value
    NUMBER,         // Numeric value
    BOOLEAN,        // True or false value
    NULL_VALUE,     // Empty value

    // Assignment
    EQUAL,          // Assignment operator (=)

    // Arithmetic Operators
    PLUS,           // Addition (+)
    MINUS,          // Subtraction (-)
    MULTIPLY,       // Multiplication (*)
    DIVIDE,         // Division (/)
    MODULO,         // Remainder (%)
    INTEGER_DIVIDE,//Integer Division(//)
    POWER,  // Power (**)

    // Comparison Operators
    EQUAL_EQUAL,    // Checks equality (==)
    NOT_EQUAL,      // Checks inequality (!=)
    BIGGER,         // Greater than (>)
    SMALLER,        // Less than (<)
    BIGGER_EQUAL,   // Greater than or equal (>=)
    SMALLER_EQUAL,  // Less than or equal (<=)
    THREE_WAY_COMPARE, // Three-way comparison (<=>)

    // Compound Assignment Operators
    PLUS_EQUAL,     // Add and assign (+=)
    MINUS_EQUAL,    // Subtract and assign (-=)
    MULTIPLY_EQUAL, // Multiply and assign (*=)
    DIVIDE_EQUAL,   // Divide and assign (/=)
    MODULO_EQUAL,   // Modulo and assign (%=)
    INTEGER_DIVIDE_EQUAL,//Integer Division assign(=//)
    POWER_EQUAL,  // Power assign(=**)


    // Logic Operators
    AND,            // Logical AND (&&)
    OR,             // Logical OR (||)
    XOR,            // Logical XOR (^^)

    // Type Operators
    TYPE_EQUAL,     // Checks if types are equal
    IS_TYPE,        // Checks value type

    // Null / Safe Operators
    NULL_COALESCE,  // Uses fallback value if null (??)
    SAFE_ACCESS,    // Safe access without null error (?.)
    NULL_EQUAL,     // Custom null comparison (?=)
    NULL_PLUS_EQUAL, // Null plus assign (?+=)
    NULL_MINUS_EQUAL, // Null minus assign (?-=)
    NULL_DIVIDE_EQUAL, // Null divide assign (?/=)
    NULL_MULTIPLY_EQUAL,// Null check multiply and assign (?*=)
    NULL_POWER_ASSIGN, // Null power assign (?**=)
    NULL_INTEGER_DIVIDE_EQUAL, // Null integer divide assign(?//=)

    // Range / Pipeline
    RANGE,          // Creates a range (..)
    PIPE,           // Passes value to next operation (|>)

    // Block Symbols
    LEFT_PAREN,     // Opening parenthesis (
    RIGHT_PAREN,    // Closing parenthesis )

    LEFT_BRACE,     // Opening code block {
    RIGHT_BRACE,    // Closing code block }

    LEFT_BRACKET,   // Opening array bracket [
    RIGHT_BRACKET,  // Closing array bracket ]

    COMMA,          // Separates values (,)
    DOT,            // Access member (.)

    // End
    END             // End of source code
};
struct Token
{
    TokenType type;
    std::string value;
};


class Lexer
{
private:
    std::string source;
    size_t position;

public:
    Lexer(std::string source);

    std::vector<Token> tokenize();
};
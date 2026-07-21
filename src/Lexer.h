#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include "errors/Error.h"

enum class TokenType
{
    // Special
    UNKNOWN,

    // Keywords
    PRINT,
    LET,
    CONST,

    IF,
    ELSE,
    ELSE_IF,

    WHILE,
    FOR,
    IN,

    FUNC,
    RETURN,

    BREAK,
    CONTINUE,

    CLASS,
    STRUCT,

    IMPORT,
    FROM,
    AS,

    TRUE,
    FALSE,
    NULL_VALUE,

    TRY,
    CATCH,
    THROW,

    SWITCH,
    CASE,
    DEFAULT,

    MATCH,

    // Types
    INT_TYPE,
    FLOAT_TYPE,
    STRING_TYPE,
    BOOL_TYPE,
    VOID_TYPE,
    ANY_TYPE,

    // Identifiers / Literals
    IDENTIFIER,

    NUMBER,
    FLOAT,
    STRING,
    BOOLEAN,

    CHARACTER,

    // Assignment Operators
    EQUAL,

    PLUS_EQUAL,
    MINUS_EQUAL,
    MULTIPLY_EQUAL,
    DIVIDE_EQUAL,
    MODULO_EQUAL,

    POWER_EQUAL,

    AND_EQUAL,
    OR_EQUAL,
    XOR_EQUAL,


    // Comparison Operators
    EQUAL_EQUAL,
    NOT_EQUAL,

    BIGGER,
    SMALLER,

    BIGGER_EQUAL,
    SMALLER_EQUAL,

    THREE_WAY_COMPARE,

    // Arithmetic Operators
    PLUS,
    MINUS,
    MULTIPLY,
    DIVIDE,
    MODULO,

    POWER,

    INCREMENT,
    DECREMENT,


    // Logical Operators
    AND_AND,
    OR_OR,
    XOR_XOR,

    NOT,

    // Bitwise Operators
    BIT_AND,
    BIT_OR,
    BIT_XOR,

    SHIFT_LEFT,
    SHIFT_RIGHT,


    // Range
    RANGE,
    RANGE_INCLUSIVE,


    // Brackets
    LPAREN,
    RPAREN,

    LBRACE,
    RBRACE,

    LBRACKET,
    RBRACKET,


    // Separators
    COMMA,
    DOT,

    COLON,
    SEMICOLON,


    // Function / Lambda
    ARROW,

    FAT_ARROW,


    // Object access
    OPTIONAL_DOT,


    // Special syntax
    QUESTION,

    AT,

    HASH,


    // Comments
    COMMENT,

    MULTI_COMMENT,


    // String features
    STRING_INTERPOLATION_START,
    STRING_INTERPOLATION_END,


    // Memory / Reference (ileride)
    ADDRESS,

    DEREFERENCE,


    // Async hazırlığı
    ASYNC,
    AWAIT,


    // Error handling
    PIPE,


    // Custom
    PIPELINE,

    SPREAD,


    // End
    END,
     // Advanced Keywords
    USING,
    NAMESPACE,

    PUBLIC,
    PRIVATE,
    PROTECTED,

    STATIC,
    FINAL,
    ABSTRACT,

    NEW,
    DELETE,

    THIS,
    BASE,

    GET,
    SET,

    OPERATOR,

    ENUM,

    INTERFACE,

    IMPLEMENTS,

    EXTENDS,


    // Module System
    EXPORT,
    INCLUDE,

    PACKAGE,

    REQUIRE,


    // Access Modifiers
    INTERNAL,

    READONLY,

    MUTABLE,


    // Loop Control
    LOOP,

    UNTIL,

    DO,

    EACH,


    // Async / Concurrency
    THREAD,

    TASK,

    SPAWN,

    LOCK,

    SYNCHRONIZE,

    CHANNEL,


    // Memory Management
    HEAP,

    STACK,

    REF,

    PTR,

    WEAK,

    OWN,


    // Error / Debug
    ASSERT,

    DEBUG,

    WARNING,

    ERROR_KEYWORD,

    PANIC,


    // Testing
    TEST,

    EXPECT,

    MOCK,


    // Data Structures
    MAP,

    DICTIONARY,

    SET_TYPE,

    TUPLE,

    LIST,

    QUEUE,

    STACK_MEMORY,


    // Collection Operations
    PUSH,

    POP,

    REMOVE,

    INSERT,

    FIND,

    CONTAINS,

    LENGTH,


    // Functional Programming
    LAMBDA,

    YIELD,

    GENERATOR,

    FILTER,

    REDUCE,

    MAP_FUNCTION,


    // Pattern Matching
    WHEN,

    GUARD,

    OTHERWISE,


    // Serialization
    JSON,

    XML,

    SERIALIZE,

    DESERIALIZE,


    // File System
    FILE,

    OPEN,

    CLOSE,

    READ,

    WRITE,


    // Network
    HTTP,

    REQUEST,

    RESPONSE,

    SOCKET,


    // Database Future
    DATABASE,

    QUERY,

    TABLE,


    // Math / Special
    INF,

    NAN_VALUE,

    PI,


    // Compiler Directives
    DEFINE,

    UNDEF,

    IFDEF,

    IFNDEF,


    // Attributes
    ATTRIBUTE,

    ANNOTATION,


    // Reflection
    TYPEOF,

    INSTANCEOF,

    SIZEOF,


    // Casting
    CAST,

    AS_TYPE,


    // Null handling
    OPTIONAL,

    NON_NULL,


    // Versioning
    VERSION,

    DEPRECATED,


    // Meta Programming
    MACRO,

    TEMPLATE,

    GENERIC,


    // Documentation
    DOC,

    COMMENT_BLOCK,


    // Special Symbols
    BACKTICK,

    DOLLAR,

    TILDE,


    // Advanced Operators
    NULL_COALESCING,

    SAFE_ASSIGN,

    ELVIS_OPERATOR,

    MATCH_ARROW,


    // Date / Time
    DATE,

    TIME,

    DATETIME,


    // Random
    RANDOM,


    // Environment
    ENV,

    CONFIG
};

struct Token
{
    TokenType type;
    std::string value;
    int line;
    int column;

    Token(TokenType type, const std::string& value, int line = 0, int column = 0)
        : type(type), value(value), line(line), column(column) {}
};

class Lexer
{
private:
    std::string source;
    size_t position;
    int line;
    int column;
    ErrorHandler* errorHandler;

    static const std::unordered_map<std::string, TokenType> keywords;

    char advance();
    char peek() const;
    char peekNext() const;
    bool isAtEnd() const;
    void skipWhitespaceAndComments();
    bool match(char expected);
    void addToken(std::vector<Token>& tokens, TokenType type, const std::string& value);

public:
    explicit Lexer(std::string source, ErrorHandler* errorHandler = nullptr);

    std::vector<Token> tokenize();
};

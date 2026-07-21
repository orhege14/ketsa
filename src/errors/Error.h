#pragma once

#include <string>
#include <vector>
#include <memory>
#include <cstdint>

enum class ErrorCode : uint16_t
{
    // Lexer Errors (1xxx)
    UNEXPECTED_CHARACTER = 1001,
    UNTERMINATED_STRING = 1002,
    UNTERMINATED_COMMENT = 1003,
    INVALID_NUMBER_FORMAT = 1004,
    INVALID_CHAR_LITERAL = 1005,

    // Parser Errors (2xxx)
    UNEXPECTED_TOKEN = 2001,
    MISSING_SEMICOLON = 2002,
    MISSING_PAREN = 2003,
    MISSING_BRACE = 2004,
    MISSING_BRACKET = 2005,
    MISSING_IDENTIFIER = 2006,
    MISSING_EQUAL = 2007,
    MISSING_COLON = 2008,
    MISSING_COMMA = 2009,
    MISSING_EXPRESSION = 2010,
    INVALID_ASSIGNMENT_TARGET = 2011,
    INVALID_DECLARATION = 2012,
    DUPLICATE_PARAMETER = 2013,
    MISSING_RETURN_TYPE = 2014,
    MISSING_CONDITION = 2015,

    // Type Errors (3xxx)
    TYPE_MISMATCH = 3001,
    UNDEFINED_VARIABLE = 3002,
    REDECLARED_VARIABLE = 3003,
    ASSIGN_TO_CONST = 3004,
    INVALID_UNARY_OPERATION = 3005,
    INVALID_BINARY_OPERATION = 3006,
    CANNOT_CONVERT_TYPE = 3007,
    INVALID_CALL_TARGET = 3008,
    WRONG_ARGUMENT_COUNT = 3009,
    WRONG_ARGUMENT_TYPE = 3010,
    INVALID_INDEX_TYPE = 3011,
    INVALID_MEMBER_ACCESS = 3012,
    RETURN_OUTSIDE_FUNCTION = 3013,
    BREAK_OUTSIDE_LOOP = 3014,
    CONTINUE_OUTSIDE_LOOP = 3015,
    NOT_CALLABLE = 3016,
    UNDEFINED_FUNCTION = 3017,

    // Runtime Errors (4xxx)
    DIVISION_BY_ZERO = 4001,
    NULL_REFERENCE = 4002,
    INDEX_OUT_OF_BOUNDS = 4003,
    STACK_OVERFLOW = 4004,
    OUT_OF_MEMORY = 4005,
    RUNTIME_ERROR = 4006,

    // Module Errors (5xxx)
    MODULE_NOT_FOUND = 5001,
    CIRCULAR_IMPORT = 5002,
    MODULE_LOAD_ERROR = 5003,
    FILE_NOT_FOUND = 5004,

    // Internal Errors (9xxx)
    INTERNAL_ERROR = 9001,
    NOT_IMPLEMENTED = 9002,

    // Warnings (0xxx)
    UNUSED_VARIABLE = 101,
    UNREACHABLE_CODE = 102,
    IMPLICIT_CONVERSION = 103,
    DEPRECATED_SYNTAX = 104
};

enum class ErrorLevel
{
    WARNING,
    ERROR,
    FATAL
};

struct SourceLocation
{
    std::string file;
    int line;
    int column;
    int length;

    SourceLocation()
        : line(0), column(0), length(1) {}

    SourceLocation(std::string file, int line, int column, int length = 1)
        : file(std::move(file)), line(line), column(column), length(length) {}
};

class CompilerError
{
private:
    ErrorCode code;
    ErrorLevel level;
    std::string message;
    SourceLocation location;
    std::string sourceLine;
    std::string hint;

public:
    CompilerError(
        ErrorCode code,
        const std::string& message,
        const SourceLocation& location,
        ErrorLevel level = ErrorLevel::ERROR,
        const std::string& hint = "",
        const std::string& sourceLine = "");

    ErrorCode getCode() const;
    ErrorLevel getLevel() const;
    const std::string& getMessage() const;
    const SourceLocation& getLocation() const;
    const std::string& getHint() const;

    bool isWarning() const;
    bool isError() const;
    bool isFatal() const;

    std::string toString() const;
    std::string toShortString() const;
    std::string getErrorCodeString() const;

    static std::string formatSourcePreview(const std::string& source, int line, int column, int length);
};

class ErrorHandler
{
private:
    std::vector<CompilerError> errors;
    std::vector<CompilerError> warnings;
    std::string source;
    std::string filename;
    int errorCount;
    int warningCount;

public:
    ErrorHandler();
    explicit ErrorHandler(std::string source);

    void setSource(const std::string& source);
    void setFilename(const std::string& filename);

    void report(const CompilerError& error);
    void report(ErrorCode code, const std::string& message, const SourceLocation& location,
                ErrorLevel level = ErrorLevel::ERROR, const std::string& hint = "");
    void warning(ErrorCode code, const std::string& message, const SourceLocation& location,
                 const std::string& hint = "");
    void error(ErrorCode code, const std::string& message, const SourceLocation& location,
               const std::string& hint = "");

    bool hasErrors() const;
    bool hasWarnings() const;
    int getErrorCount() const;
    int getWarningCount() const;

    const std::vector<CompilerError>& getErrors() const;
    const std::vector<CompilerError>& getWarnings() const;

    void printAll() const;
    void printSummary() const;
    void clear();

    std::string getSourceLine(int line) const;
};

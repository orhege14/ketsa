#include "Lexer.h"
#include <cctype>
#include <stdexcept>

const std::unordered_map<std::string, TokenType> Lexer::keywords = {
    {"print", TokenType::PRINT},
    {"let", TokenType::LET},
    {"const", TokenType::CONST},
    {"if", TokenType::IF},
    {"else", TokenType::ELSE},
    {"elif", TokenType::ELSE_IF},
    {"while", TokenType::WHILE},
    {"for", TokenType::FOR},
    {"in", TokenType::IN},
    {"func", TokenType::FUNC},
    {"return", TokenType::RETURN},
    {"break", TokenType::BREAK},
    {"continue", TokenType::CONTINUE},
    {"class", TokenType::CLASS},
    {"struct", TokenType::STRUCT},
    {"import", TokenType::IMPORT},
    {"from", TokenType::FROM},
    {"as", TokenType::AS},
    {"true", TokenType::TRUE},
    {"false", TokenType::FALSE},
    {"null", TokenType::NULL_VALUE},
    {"try", TokenType::TRY},
    {"catch", TokenType::CATCH},
    {"throw", TokenType::THROW},
    {"switch", TokenType::SWITCH},
    {"case", TokenType::CASE},
    {"default", TokenType::DEFAULT},
    {"match", TokenType::MATCH},
    {"int", TokenType::INT_TYPE},
    {"float", TokenType::FLOAT_TYPE},
    {"string", TokenType::STRING_TYPE},
    {"bool", TokenType::BOOL_TYPE},
    {"void", TokenType::VOID_TYPE},
    {"any", TokenType::ANY_TYPE},
    {"using", TokenType::USING},
    {"namespace", TokenType::NAMESPACE},
    {"public", TokenType::PUBLIC},
    {"private", TokenType::PRIVATE},
    {"protected", TokenType::PROTECTED},
    {"static", TokenType::STATIC},
    {"final", TokenType::FINAL},
    {"abstract", TokenType::ABSTRACT},
    {"new", TokenType::NEW},
    {"delete", TokenType::DELETE},
    {"this", TokenType::THIS},
    {"base", TokenType::BASE},
    {"get", TokenType::GET},
    {"set", TokenType::SET},
    {"operator", TokenType::OPERATOR},
    {"enum", TokenType::ENUM},
    {"interface", TokenType::INTERFACE},
    {"implements", TokenType::IMPLEMENTS},
    {"extends", TokenType::EXTENDS},
    {"export", TokenType::EXPORT},
    {"include", TokenType::INCLUDE},
    {"package", TokenType::PACKAGE},
    {"require", TokenType::REQUIRE},
    {"internal", TokenType::INTERNAL},
    {"readonly", TokenType::READONLY},
    {"mutable", TokenType::MUTABLE},
    {"loop", TokenType::LOOP},
    {"until", TokenType::UNTIL},
    {"do", TokenType::DO},
    {"each", TokenType::EACH},
    {"async", TokenType::ASYNC},
    {"await", TokenType::AWAIT},
    {"thread", TokenType::THREAD},
    {"task", TokenType::TASK},
    {"spawn", TokenType::SPAWN},
    {"lock", TokenType::LOCK},
    {"synchronize", TokenType::SYNCHRONIZE},
    {"channel", TokenType::CHANNEL},
    {"heap", TokenType::HEAP},
    {"stack", TokenType::STACK},
    {"ref", TokenType::REF},
    {"ptr", TokenType::PTR},
    {"weak", TokenType::WEAK},
    {"own", TokenType::OWN},
    {"assert", TokenType::ASSERT},
    {"debug", TokenType::DEBUG},
    {"warning", TokenType::WARNING},
    {"error", TokenType::ERROR_KEYWORD},
    {"panic", TokenType::PANIC},
    {"test", TokenType::TEST},
    {"expect", TokenType::EXPECT},
    {"mock", TokenType::MOCK},
    {"map", TokenType::MAP},
    {"dictionary", TokenType::DICTIONARY},
    {"set", TokenType::SET},
    {"tuple", TokenType::TUPLE},
    {"list", TokenType::LIST},
    {"queue", TokenType::QUEUE},
    {"push", TokenType::PUSH},
    {"pop", TokenType::POP},
    {"remove", TokenType::REMOVE},
    {"insert", TokenType::INSERT},
    {"find", TokenType::FIND},
    {"contains", TokenType::CONTAINS},
    {"length", TokenType::LENGTH},
    {"lambda", TokenType::LAMBDA},
    {"yield", TokenType::YIELD},
    {"generator", TokenType::GENERATOR},
    {"filter", TokenType::FILTER},
    {"reduce", TokenType::REDUCE},
    {"when", TokenType::WHEN},
    {"guard", TokenType::GUARD},
    {"otherwise", TokenType::OTHERWISE},
    {"json", TokenType::JSON},
    {"xml", TokenType::XML},
    {"serialize", TokenType::SERIALIZE},
    {"deserialize", TokenType::DESERIALIZE},
    {"file", TokenType::FILE},
    {"open", TokenType::OPEN},
    {"close", TokenType::CLOSE},
    {"read", TokenType::READ},
    {"write", TokenType::WRITE},
    {"http", TokenType::HTTP},
    {"request", TokenType::REQUEST},
    {"response", TokenType::RESPONSE},
    {"socket", TokenType::SOCKET},
    {"database", TokenType::DATABASE},
    {"query", TokenType::QUERY},
    {"table", TokenType::TABLE},
    {"inf", TokenType::INF},
    {"nan", TokenType::NAN_VALUE},
    {"pi", TokenType::PI},
    {"define", TokenType::DEFINE},
    {"undef", TokenType::UNDEF},
    {"ifdef", TokenType::IFDEF},
    {"ifndef", TokenType::IFNDEF},
    {"typeof", TokenType::TYPEOF},
    {"instanceof", TokenType::INSTANCEOF},
    {"sizeof", TokenType::SIZEOF},
    {"cast", TokenType::CAST},
    {"optional", TokenType::OPTIONAL},
    // version is commonly used as a variable name
    // if needed as keyword, will use contextual keyword approach
    {"deprecated", TokenType::DEPRECATED},
    {"macro", TokenType::MACRO},
    {"template", TokenType::TEMPLATE},
    {"generic", TokenType::GENERIC},
    {"doc", TokenType::DOC},
    {"random", TokenType::RANDOM},
    {"env", TokenType::ENV},
    {"config", TokenType::CONFIG},
    {"attribute", TokenType::ATTRIBUTE},
    {"annotation", TokenType::ANNOTATION},
    {"date", TokenType::DATE},
    {"time", TokenType::TIME},
    {"datetime", TokenType::DATETIME}
};

Lexer::Lexer(std::string source, ErrorHandler* errorHandler)
    : source(std::move(source))
    , position(0)
    , line(1)
    , column(1)
    , errorHandler(errorHandler)
{
}

char Lexer::advance()
{
    char c = source[position++];
    if (c == '\n')
    {
        line++;
        column = 1;
    }
    else
    {
        column++;
    }
    return c;
}

char Lexer::peek() const
{
    if (isAtEnd()) return '\0';
    return source[position];
}

char Lexer::peekNext() const
{
    if (position + 1 >= source.size()) return '\0';
    return source[position + 1];
}

bool Lexer::isAtEnd() const
{
    return position >= source.size();
}

bool Lexer::match(char expected)
{
    if (isAtEnd()) return false;
    if (source[position] != expected) return false;
    advance();
    return true;
}

void Lexer::skipWhitespaceAndComments()
{
    while (!isAtEnd())
    {
        char c = peek();
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n')
        {
            advance();
            continue;
        }

        // Single-line comment: //
        if (c == '/' && peekNext() == '/')
        {
            while (!isAtEnd() && peek() != '\n')
                advance();
            continue;
        }

        // Multi-line comment: /* ... */
        if (c == '/' && peekNext() == '*')
        {
            advance();
            advance();
            while (!isAtEnd())
            {
                if (peek() == '*' && peekNext() == '/')
                {
                    advance();
                    advance();
                    break;
                }
                advance();
            }
            continue;
        }

        break;
    }
}

void Lexer::addToken(std::vector<Token>& tokens, TokenType type, const std::string& value)
{
    tokens.emplace_back(type, value, line, column);
}

std::vector<Token> Lexer::tokenize()
{
    std::vector<Token> tokens;

    while (!isAtEnd())
    {
        skipWhitespaceAndComments();
        if (isAtEnd()) break;

        char c = peek();
        int startCol = column;

        // String literals
        if (c == '"')
        {
            advance();
            std::string text;
            while (!isAtEnd() && peek() != '"')
            {
                if (peek() == '\\')
                {
                    advance();
                    switch (peek())
                    {
                        case 'n': text += '\n'; break;
                        case 't': text += '\t'; break;
                        case 'r': text += '\r'; break;
                        case '0': text += '\0'; break;
                        case '\\': text += '\\'; break;
                        case '"': text += '"'; break;
                        default: text += peek(); break;
                    }
                    advance();
                }
                else
                {
                    text += advance();
                }
            }
            if (isAtEnd())
            {
                if (errorHandler)
                    errorHandler->error(ErrorCode::UNTERMINATED_STRING,
                        "Unterminated string literal",
                        SourceLocation("", line, startCol));
                break;
            }
            advance();
            addToken(tokens, TokenType::STRING, text);
            continue;
        }

        // Char literals
        if (c == '\'')
        {
            advance();
            if (isAtEnd())
            {
                if (errorHandler)
                    errorHandler->error(ErrorCode::INVALID_CHAR_LITERAL,
                        "Unterminated char literal",
                        SourceLocation("", line, startCol));
                continue;
            }
            char ch = advance();
            if (ch == '\\')
            {
                switch (peek())
                {
                    case 'n': ch = '\n'; break;
                    case 't': ch = '\t'; break;
                    case '0': ch = '\0'; break;
                    case '\\': ch = '\\'; break;
                    case '\'': ch = '\''; break;
                    default: ch = advance(); break;
                }
                advance();
            }
            if (isAtEnd() || peek() != '\'')
            {
                if (errorHandler)
                    errorHandler->error(ErrorCode::INVALID_CHAR_LITERAL,
                        "Unterminated char literal",
                        SourceLocation("", line, startCol));
                continue;
            }
            advance();
            addToken(tokens, TokenType::CHARACTER, std::string(1, ch));
            continue;
        }

        // Numbers
        if (std::isdigit(static_cast<unsigned char>(c)))
        {
            std::string number;
            bool isFloat = false;
            while (!isAtEnd() && std::isdigit(static_cast<unsigned char>(peek())))
                number += advance();
            if (!isAtEnd() && peek() == '.' && peekNext() != '.' && !std::isalpha(static_cast<unsigned char>(peekNext())))
            {
                isFloat = true;
                number += advance();
                while (!isAtEnd() && std::isdigit(static_cast<unsigned char>(peek())))
                    number += advance();
            }
            if (isFloat)
                addToken(tokens, TokenType::FLOAT, number);
            else
                addToken(tokens, TokenType::NUMBER, number);
            continue;
        }

        // Identifiers and keywords
        if (std::isalpha(static_cast<unsigned char>(c)) || c == '_')
        {
            std::string word;
            while (!isAtEnd() && (std::isalnum(static_cast<unsigned char>(peek())) || peek() == '_'))
                word += advance();
            auto it = keywords.find(word);
            if (it != keywords.end())
                addToken(tokens, it->second, word);
            else
                addToken(tokens, TokenType::IDENTIFIER, word);
            continue;
        }

        // Multi-character operators
        if (c == ':' && peekNext() == ':') { addToken(tokens, TokenType::PIPELINE, "::"); advance(); advance(); continue; }
        if (c == ':' && peekNext() == '=') { addToken(tokens, TokenType::SAFE_ASSIGN, ":="); advance(); advance(); continue; }
        if (c == '?' && peekNext() == ':') { addToken(tokens, TokenType::ELVIS_OPERATOR, "?:"); advance(); advance(); continue; }
        if (c == '?' && peekNext() == '.') { addToken(tokens, TokenType::OPTIONAL_DOT, "?."); advance(); advance(); continue; }
        if (c == '?' && peekNext() == '=') { advance(); advance(); continue; } // ?= skip for custom operators
        if (c == '?' && peekNext() == '+') { advance(); advance(); continue; }
        if (c == '?' && peekNext() == '-') { advance(); advance(); continue; }
        if (c == '?' && peekNext() == '*') { advance(); advance(); continue; }

        if (c == '<' && peekNext() == '=' && peekNext() != '>') { addToken(tokens, TokenType::SMALLER_EQUAL, "<="); advance(); advance(); continue; }
        if (c == '<' && peekNext() == '<') { addToken(tokens, TokenType::SHIFT_LEFT, "<<"); advance(); advance(); continue; }
        if (c == '<' && peekNext() == '=' && source[position + 2] == '>') { addToken(tokens, TokenType::THREE_WAY_COMPARE, "<=>"); advance(); advance(); advance(); continue; }
        if (c == '>' && peekNext() == '=') { addToken(tokens, TokenType::BIGGER_EQUAL, ">="); advance(); advance(); continue; }
        if (c == '>' && peekNext() == '>') { addToken(tokens, TokenType::SHIFT_RIGHT, ">>"); advance(); advance(); continue; }

        if (c == '=' && peekNext() == '=') { addToken(tokens, TokenType::EQUAL_EQUAL, "=="); advance(); advance(); continue; }
        if (c == '!' && peekNext() == '=') { addToken(tokens, TokenType::NOT_EQUAL, "!="); advance(); advance(); continue; }

        if (c == '+' && peekNext() == '=') { addToken(tokens, TokenType::PLUS_EQUAL, "+="); advance(); advance(); continue; }
        if (c == '+' && peekNext() == '+') { addToken(tokens, TokenType::INCREMENT, "++"); advance(); advance(); continue; }
        if (c == '-' && peekNext() == '=') { addToken(tokens, TokenType::MINUS_EQUAL, "-="); advance(); advance(); continue; }
        if (c == '-' && peekNext() == '-') { addToken(tokens, TokenType::DECREMENT, "--"); advance(); advance(); continue; }
        if (c == '-' && peekNext() == '>') { addToken(tokens, TokenType::ARROW, "->"); advance(); advance(); continue; }
        if (c == '*' && peekNext() == '=') { addToken(tokens, TokenType::MULTIPLY_EQUAL, "*="); advance(); advance(); continue; }
        if (c == '*' && peekNext() == '*') { addToken(tokens, TokenType::POWER, "**"); advance(); advance(); continue; }
        if (c == '/' && peekNext() == '=') { addToken(tokens, TokenType::DIVIDE_EQUAL, "/="); advance(); advance(); continue; }
        if (c == '%' && peekNext() == '=') { addToken(tokens, TokenType::MODULO_EQUAL, "%="); advance(); advance(); continue; }

        if (c == '&' && peekNext() == '&') { addToken(tokens, TokenType::AND_AND, "&&"); advance(); advance(); continue; }
        if (c == '&' && peekNext() == '=') { addToken(tokens, TokenType::AND_EQUAL, "&="); advance(); advance(); continue; }
        if (c == '|' && peekNext() == '|') { addToken(tokens, TokenType::OR_OR, "||"); advance(); advance(); continue; }
        if (c == '|' && peekNext() == '=') { addToken(tokens, TokenType::OR_EQUAL, "|="); advance(); advance(); continue; }
        if (c == '^' && peekNext() == '^') { addToken(tokens, TokenType::XOR_XOR, "^^"); advance(); advance(); continue; }
        if (c == '^' && peekNext() == '=') { addToken(tokens, TokenType::XOR_EQUAL, "^="); advance(); advance(); continue; }

        if (c == '.' && peekNext() == '.') { advance(); advance(); continue; } // skip .. for now
        if (c == '.' && peekNext() == '.' && source[position + 2] == '.') { addToken(tokens, TokenType::SPREAD, "..."); advance(); advance(); advance(); continue; }

        if (c == '=' && peekNext() == '>') { addToken(tokens, TokenType::FAT_ARROW, "=>"); advance(); advance(); continue; }
        if (c == '=' && peekNext() == '=' && source[position + 2] == '=') { advance(); advance(); advance(); continue; } // === skip

        // Single-character tokens
        switch (c)
        {
            case '(': addToken(tokens, TokenType::LPAREN, "("); break;
            case ')': addToken(tokens, TokenType::RPAREN, ")"); break;
            case '{': addToken(tokens, TokenType::LBRACE, "{"); break;
            case '}': addToken(tokens, TokenType::RBRACE, "}"); break;
            case '[': addToken(tokens, TokenType::LBRACKET, "["); break;
            case ']': addToken(tokens, TokenType::RBRACKET, "]"); break;
            case ',': addToken(tokens, TokenType::COMMA, ","); break;
            case '.': addToken(tokens, TokenType::DOT, "."); break;
            case ';': addToken(tokens, TokenType::SEMICOLON, ";"); break;
            case ':': addToken(tokens, TokenType::COLON, ":"); break;
            case '=': addToken(tokens, TokenType::EQUAL, "="); break;
            case '+': addToken(tokens, TokenType::PLUS, "+"); break;
            case '-': addToken(tokens, TokenType::MINUS, "-"); break;
            case '*': addToken(tokens, TokenType::MULTIPLY, "*"); break;
            case '/': addToken(tokens, TokenType::DIVIDE, "/"); break;
            case '%': addToken(tokens, TokenType::MODULO, "%"); break;
            case '!': addToken(tokens, TokenType::NOT, "!"); break;
            case '>': addToken(tokens, TokenType::BIGGER, ">"); break;
            case '<': addToken(tokens, TokenType::SMALLER, "<"); break;
            case '&': addToken(tokens, TokenType::BIT_AND, "&"); break;
            case '|': addToken(tokens, TokenType::BIT_OR, "|"); break;
            case '^': addToken(tokens, TokenType::BIT_XOR, "^"); break;
            case '~': addToken(tokens, TokenType::TILDE, "~"); break;
            case '?': addToken(tokens, TokenType::QUESTION, "?"); break;
            case '@': addToken(tokens, TokenType::AT, "@"); break;
            case '#': addToken(tokens, TokenType::HASH, "#"); break;
            case '$': addToken(tokens, TokenType::DOLLAR, "$"); break;
            case '`': addToken(tokens, TokenType::BACKTICK, "`"); break;
            default:
            {
                if (errorHandler)
                    errorHandler->error(ErrorCode::UNEXPECTED_CHARACTER,
                        std::string("Unexpected character: '") + c + "'",
                        SourceLocation("", line, column));
                advance();
                continue;
            }
        }
        advance();
    }

    tokens.emplace_back(TokenType::END, "", line, column);
    return tokens;
}

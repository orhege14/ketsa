#pragma once

#include "Lexer.h"
#include "AST.h"
#include "errors/Error.h"

#include <vector>
#include <memory>
#include <optional>

class Parser
{
private:
    std::vector<Token> tokens;
    size_t position;
    ErrorHandler* errorHandler;

    // Token helpers
    Token peek() const;
    Token previous() const;
    Token advance();
    bool check(TokenType type) const;
    bool match(TokenType type);
    bool matchAny(std::initializer_list<TokenType> types);
    Token consume(TokenType type, const std::string& message);
    Token consumeAssignmentOp();
    [[nodiscard]] bool isAtEnd() const;
    void synchronize();

    // Type parsing
    TypeInfo parseTypeAnnotation();
    std::optional<TypeInfo> tryParseTypeAnnotation();
    TypeInfo parseType();
    std::vector<TypeInfo> parseTypeList();
    std::vector<std::string> parseGenericParams();

    // Statement parsing
    std::unique_ptr<ASTNode> parseStatement();
    std::unique_ptr<ASTNode> parseDeclaration();
    std::unique_ptr<ASTNode> parseVariableDeclaration(bool isConst = false, bool isMutable = false);
    std::unique_ptr<ASTNode> parseTypeAlias();
    std::unique_ptr<ASTNode> parseBlock();
    std::unique_ptr<ASTNode> parseIf();
    std::unique_ptr<ASTNode> parseWhile();
    std::unique_ptr<ASTNode> parseFor();
    std::unique_ptr<ASTNode> parseSwitch();
    std::unique_ptr<ASTNode> parseReturn();
    std::unique_ptr<ASTNode> parseBreak();
    std::unique_ptr<ASTNode> parseContinue();
    std::unique_ptr<ASTNode> parsePrint();
    std::unique_ptr<ASTNode> parseExpressionStatement();

    // Function parsing
    std::unique_ptr<ASTNode> parseFunctionDeclaration();
    std::vector<std::unique_ptr<ASTNode>> parseFunctionBody();
    std::vector<ParameterNode> parseParameters();
    std::unique_ptr<ASTNode> parseLambda();

    // Class parsing
    std::unique_ptr<ASTNode> parseClassDeclaration();

    // Match parsing
    std::unique_ptr<ASTNode> parseMatch();
    MatchPattern parsePattern();

    // Error handling
    std::unique_ptr<ASTNode> parseTry();
    std::unique_ptr<ASTNode> parseThrow();

    // Import parsing
    std::unique_ptr<ASTNode> parseImport();
    std::unique_ptr<ASTNode> parseFromImport();

    // Expression parsing (precedence climbing)
    std::unique_ptr<ASTNode> parseExpression();
    std::unique_ptr<ASTNode> parseAssignment();
    std::unique_ptr<ASTNode> parseLogicalOr();
    std::unique_ptr<ASTNode> parseLogicalAnd();
    std::unique_ptr<ASTNode> parseEquality();
    std::unique_ptr<ASTNode> parseComparison();
    std::unique_ptr<ASTNode> parseTerm();
    std::unique_ptr<ASTNode> parseFactor();
    std::unique_ptr<ASTNode> parseUnary();
    std::unique_ptr<ASTNode> parseCall();
    std::unique_ptr<ASTNode> parsePrimary();
    std::unique_ptr<ASTNode> parseArrayLiteral();
    std::unique_ptr<ASTNode> parseObjectLiteral();

    std::unique_ptr<ASTNode> finishCall(std::unique_ptr<ASTNode> callee);
    std::unique_ptr<ASTNode> finishMemberAccess(std::unique_ptr<ASTNode> object);
    std::unique_ptr<ASTNode> finishIndexAccess(std::unique_ptr<ASTNode> array);

public:
    explicit Parser(std::vector<Token> tokens, ErrorHandler* errorHandler = nullptr);

    std::vector<std::unique_ptr<ASTNode>> parse();
};

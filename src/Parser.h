#pragma once

#include "Lexer.h"
#include "AST.h"

#include <vector>
#include <memory>


class Parser
{

private:

    std::vector<Token> tokens;

    size_t position;


    // Token helpers
    Token peek();

    Token advance();



    // Expressions
    std::unique_ptr<ASTNode> parseExpression();

    std::unique_ptr<ASTNode> parseComparison();

    std::unique_ptr<ASTNode> parseTerm();

    std::unique_ptr<ASTNode> parseFactor();



    // Variables
    std::unique_ptr<ASTNode> parseVariableDeclaration();

    std::unique_ptr<ASTNode> parseAssignment();



    // Statements
    std::unique_ptr<ASTNode> parsePrint();

    std::unique_ptr<ASTNode> parseExpressionStatement();



    // Conditions
    std::unique_ptr<ASTNode> parseIf();

    std::unique_ptr<ASTNode> parseElse();



    // Loops
    std::unique_ptr<ASTNode> parseWhile();

    std::unique_ptr<ASTNode> parseFor();



    // Functions
    std::unique_ptr<ASTNode> parseFunctionDeclaration();

    std::unique_ptr<ASTNode> parseFunctionCall();

    std::unique_ptr<ASTNode> parseReturn();



    // Collections
    std::unique_ptr<ASTNode> parseArray();

    std::unique_ptr<ASTNode> parseIndexAccess();



public:

    Parser(std::vector<Token> tokens);


    std::vector<std::unique_ptr<ASTNode>> parse();

};
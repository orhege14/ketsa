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

    Token peek();

    Token advance();

    std::unique_ptr<ASTNode> parseExpression();
    
    std::unique_ptr<ASTNode> parseVariableDeclaration();

    std::unique_ptr<ASTNode> parsePrint();

public:
    Parser(std::vector<Token> tokens);

    std::vector<std::unique_ptr<ASTNode>> parse();
};
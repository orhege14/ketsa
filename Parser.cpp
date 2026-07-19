#include "Parser.h"


Parser::Parser(std::vector<Token> tokens)
{
    this->tokens = tokens;
    position = 0;
}


Token Parser::peek()
{
    return tokens[position];
}


Token Parser::advance()
{
    return tokens[position++];
}



std::vector<std::unique_ptr<ASTNode>> Parser::parse()
{
    std::vector<std::unique_ptr<ASTNode>> nodes;


    while(peek().type != TokenType::END)
    {

        Token token = peek();


        // print
        if(token.type == TokenType::PRINT)
        {
            nodes.push_back(parsePrint());
        }


        // let
        else if(token.type == TokenType::LET)
        {
            nodes.push_back(parseVariableDeclaration());
        }


        // age ?**= 10
        else if(token.type == TokenType::IDENTIFIER)
        {
            nodes.push_back(parseExpression());
        }


        else
        {
            advance();
        }

    }


    return nodes;
}



std::unique_ptr<ASTNode> Parser::parseVariableDeclaration()
{
    // let
    advance();


    // variable name
    Token name = advance();


    // =
    advance();


    // value
    Token value = advance();


    std::unique_ptr<ASTNode> valueNode;


    if(value.type == TokenType::NUMBER)
    {
        valueNode =
            std::make_unique<NumberNode>(
                std::stoi(value.value)
            );
    }


    else if(value.type == TokenType::STRING)
    {
        valueNode =
            std::make_unique<StringNode>(
                value.value
            );
    }


    else if(value.type == TokenType::BOOLEAN)
    {
        valueNode =
            std::make_unique<BooleanNode>(
                value.value == "true"
            );
    }


    return std::make_unique<VariableDeclarationNode>(
        name.value,
        std::move(valueNode)
    );
}




std::unique_ptr<ASTNode> Parser::parsePrint()
{
    // print
    advance();


    Token value = advance();


    std::unique_ptr<ASTNode> expression;


    if(value.type == TokenType::NUMBER)
    {
        expression =
            std::make_unique<NumberNode>(
                std::stoi(value.value)
            );
    }


    else if(value.type == TokenType::STRING)
    {
        expression =
            std::make_unique<StringNode>(
                value.value
            );
    }


    else if(value.type == TokenType::IDENTIFIER)
    {
        expression =
            std::make_unique<VariableAccessNode>(
                value.value
            );
    }


    return std::make_unique<PrintNode>(
        std::move(expression)
    );
}




// age ?**= 10
std::unique_ptr<ASTNode> Parser::parseExpression()
{

    // age
    Token left = advance();


    auto leftNode =
        std::make_unique<VariableAccessNode>(
            left.value
        );


    // ?**=
    Token op = advance();



    // 10
    Token right = advance();



    std::unique_ptr<ASTNode> rightNode;



    if(right.type == TokenType::NUMBER)
    {
        rightNode =
            std::make_unique<NumberNode>(
                std::stoi(right.value)
            );
    }



    return std::make_unique<BinaryExpressionNode>(
        op.value,
        std::move(leftNode),
        std::move(rightNode)
    );
}
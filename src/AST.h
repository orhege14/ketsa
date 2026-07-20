#pragma once

#include <string>
#include <vector>
#include <memory>

enum class NodeType
{
    // Values
    NUMBER,
    STRING,
    BOOLEAN,
    NULL_VALUE,
    ARRAY,

    // Variables
    VARIABLE_DECLARATION,
    VARIABLE_ACCESS,
    ASSIGNMENT,

    // Operators
    BINARY_EXPRESSION,
    UNARY_EXPRESSION,

    // Statements
    PRINT,
    EXPRESSION_STATEMENT,

    // Control Flow
    IF,
    ELSE,
    WHILE,
    FOR,
    BREAK,
    CONTINUE,

    // Functions
    FUNCTION_DECLARATION,
    FUNCTION_CALL,
    RETURN,

    // Classes / Objects
    CLASS_DECLARATION,
    OBJECT_CREATION,
    MEMBER_ACCESS,

    // Collections
    ARRAY_ACCESS,
    INDEX_ASSIGNMENT,

    // Program
    BLOCK,
    PROGRAM
};

struct ASTNode
{
    NodeType type;

    ASTNode(NodeType type)
        : type(type)
    {
    }

    virtual ~ASTNode() = default;
};

//
// 123
//
struct NumberNode : ASTNode
{
    int value;

    NumberNode(int value)
        : ASTNode(NodeType::NUMBER),
          value(value)
    {
    }
};

//
// "Hello"
//
struct StringNode : ASTNode
{
    std::string value;

    StringNode(std::string value)
        : ASTNode(NodeType::STRING),
          value(value)
    {
    }
};

//
// true / false
//
struct BooleanNode : ASTNode
{
    bool value;

    BooleanNode(bool value)
        : ASTNode(NodeType::BOOLEAN),
          value(value)
    {
    }
};

//
// let age = 123
//
struct VariableDeclarationNode : ASTNode
{
    std::string name;

    std::unique_ptr<ASTNode> value;

    VariableDeclarationNode(
        std::string name,
        std::unique_ptr<ASTNode> value)
        : ASTNode(NodeType::VARIABLE_DECLARATION),
          name(name),
          value(std::move(value))
    {
    }
};

//
// age
//
struct VariableAccessNode : ASTNode
{
    std::string name;

    VariableAccessNode(std::string name)
        : ASTNode(NodeType::VARIABLE_ACCESS),
          name(name)
    {
    }
};

//
// print "Hello"
//
struct PrintNode : ASTNode
{
    std::unique_ptr<ASTNode> expression;

    PrintNode(std::unique_ptr<ASTNode> expression)
        : ASTNode(NodeType::PRINT),
          expression(std::move(expression))
    {
    }
};

//
// age + 5
// age == 10
//
struct BinaryExpressionNode : ASTNode
{
    std::string op;

    std::unique_ptr<ASTNode> left;
    std::unique_ptr<ASTNode> right;

    BinaryExpressionNode(
        std::string op,
        std::unique_ptr<ASTNode> left,
        std::unique_ptr<ASTNode> right)
        : ASTNode(NodeType::BINARY_EXPRESSION),
          op(op),
          left(std::move(left)),
          right(std::move(right))
    {
    }
};

//
// if age > 10
//
struct IfNode : ASTNode
{
    std::unique_ptr<ASTNode> condition;

    std::vector<std::unique_ptr<ASTNode>> body;

    IfNode(
        std::unique_ptr<ASTNode> condition)
        : ASTNode(NodeType::IF),
          condition(std::move(condition))
    {
    }
};

struct AssignmentNode : ASTNode
{
    std::string name;
    std::unique_ptr<ASTNode> value;

    AssignmentNode(
        std::string name,
        std::unique_ptr<ASTNode> value)
        : ASTNode(NodeType::ASSIGNMENT),
          name(name),
          value(std::move(value))
    {
    }
};
//
// Tüm program
//
struct ProgramNode : ASTNode
{
    std::vector<std::unique_ptr<ASTNode>> statements;

    ProgramNode()
        : ASTNode(NodeType::PROGRAM)
    {
    }
};
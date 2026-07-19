#include "Interpreter.h"
#include <iostream>
#include <cmath>


Interpreter::Interpreter()
{
}


void Interpreter::execute(ASTNode *node)
{

    if (node->type == NodeType::VARIABLE_DECLARATION)
    {
        auto variable =
            static_cast<VariableDeclarationNode *>(node);


        Value value =
            evaluate(variable->value.get());


        variables[variable->name] = value;
    }


    else if (node->type == NodeType::PRINT)
    {
        auto print =
            static_cast<PrintNode *>(node);


        Value value =
            evaluate(print->expression.get());


        if (value.type == ValueType::NUMBER)
        {
            std::cout << value.number;
        }

        else if (value.type == ValueType::STRING)
        {
            std::cout << value.string;
        }

        else if (value.type == ValueType::BOOLEAN)
        {
            std::cout << (value.boolean ? "true" : "false");
        }


        std::cout << std::endl;
    }


    else if (node->type == NodeType::BINARY_EXPRESSION)
    {
        auto binary =
            static_cast<BinaryExpressionNode *>(node);


        auto variable =
            static_cast<VariableAccessNode *>(binary->left.get());


        Value& left =
            variables[variable->name];


        Value right =
            evaluate(binary->right.get());


        if (binary->op == "?**=")
        {
            left.number =
                pow(left.number, right.number);
        }else if (binary->op == "*="){
            left.number = left.number * right.number;
        }
    }
}



Value Interpreter::evaluate(ASTNode *node)
{

    if (node->type == NodeType::NUMBER)
    {
        auto number =
            static_cast<NumberNode *>(node);


        return Value::Number(number->value);
    }


    else if (node->type == NodeType::STRING)
    {
        auto string =
            static_cast<StringNode *>(node);


        return Value::String(string->value);
    }


    else if (node->type == NodeType::BOOLEAN)
    {
        auto boolean =
            static_cast<BooleanNode *>(node);


        return Value::Boolean(boolean->value);
    }


    else if (node->type == NodeType::VARIABLE_ACCESS)
    {
        auto variable =
            static_cast<VariableAccessNode *>(node);


        return variables[variable->name];
    }


    return Value();
}
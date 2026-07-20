#include "Interpreter.h"

#include "runtime/values/NumberValue.h"
#include "runtime/values/StringValue.h"
#include "runtime/values/BooleanValue.h"
#include "runtime/values/NullValue.h"
#include "runtime/values/ArrayValue.h"
#include <iostream>
#include <vector>
Interpreter::Interpreter()
{
}

void Interpreter::execute(ASTNode *node)
{

    if (node->type == NodeType::VARIABLE_DECLARATION)
    {

        auto variable =
            static_cast<VariableDeclarationNode *>(node);

        auto value =
            evaluate(variable->value.get());

        environment.define(
            variable->name,
            std::move(value));
    }
    else if (node->type == NodeType::ASSIGNMENT)
    {
        auto assignment =
            static_cast<AssignmentNode *>(node);

        auto value =
            evaluate(assignment->value.get());

        environment.set(
            assignment->name,
            std::move(value));
    }
    else if (node->type == NodeType::PRINT)
    {

        auto print =
            static_cast<PrintNode *>(node);

        auto value =
            evaluate(print->expression.get());

        if (value)
        {
            std::cout
                << value->toString();
        }

        std::cout << std::endl;
    }
}
std::unique_ptr<Value> Interpreter::evaluate(ASTNode *node)
{

    if (node->type == NodeType::NUMBER)
    {

        auto number =
            static_cast<NumberNode *>(node);

        return std::make_unique<NumberValue>(
            number->value);
    }

    else if (node->type == NodeType::STRING)
    {

        auto string =
            static_cast<StringNode *>(node);

        return std::make_unique<StringValue>(
            string->value);
    }

    else if (node->type == NodeType::BOOLEAN)
    {

        auto boolean =
            static_cast<BooleanNode *>(node);

        return std::make_unique<BooleanValue>(
            boolean->value);
    }
    else if (node->type == NodeType::BINARY_EXPRESSION)
    {
        auto binary =
            static_cast<BinaryExpressionNode *>(node);

        auto left =
            evaluate(binary->left.get());

        auto right =
            evaluate(binary->right.get());

        // NUMBER OPERATIONS
        if (
            left->getType() == ValueType::NUMBER &&
            right->getType() == ValueType::NUMBER)
        {

            auto leftNumber =
                static_cast<NumberValue *>(left.get());

            auto rightNumber =
                static_cast<NumberValue *>(right.get());

            if (binary->op == "+")
            {
                return std::make_unique<NumberValue>(
                    leftNumber->getValue() +
                    rightNumber->getValue());
            }

            else if (binary->op == "-")
            {
                return std::make_unique<NumberValue>(
                    leftNumber->getValue() -
                    rightNumber->getValue());
            }

            else if (binary->op == "*")
            {
                return std::make_unique<NumberValue>(
                    leftNumber->getValue() *
                    rightNumber->getValue());
            }

            else if (binary->op == "/")
            {
                return std::make_unique<NumberValue>(
                    leftNumber->getValue() /
                    rightNumber->getValue());
            }

            else if (binary->op == "%")
            {
                return std::make_unique<NumberValue>(
                    leftNumber->getValue() %
                    rightNumber->getValue());
            }

            else if (binary->op == "==")
            {
                return std::make_unique<BooleanValue>(
                    leftNumber->getValue() ==
                    rightNumber->getValue());
            }

            else if (binary->op == "!=")
            {
                return std::make_unique<BooleanValue>(
                    leftNumber->getValue() !=
                    rightNumber->getValue());
            }

            else if (binary->op == ">")
            {
                return std::make_unique<BooleanValue>(
                    leftNumber->getValue() >
                    rightNumber->getValue());
            }

            else if (binary->op == "<")
            {
                return std::make_unique<BooleanValue>(
                    leftNumber->getValue() <
                    rightNumber->getValue());
            }

            else if (binary->op == ">=")
            {
                return std::make_unique<BooleanValue>(
                    leftNumber->getValue() >=
                    rightNumber->getValue());
            }

            else if (binary->op == "<=")
            {
                return std::make_unique<BooleanValue>(
                    leftNumber->getValue() <=
                    rightNumber->getValue());
            }

            else if (binary->op == "<=>")
            {

                int result = 0;

                if (leftNumber->getValue() < rightNumber->getValue())
                {
                    result = -1;
                }

                else if (leftNumber->getValue() > rightNumber->getValue())
                {
                    result = 1;
                }

                return std::make_unique<NumberValue>(result);
            }

            else if (binary->op == "..")
            {

                std::vector<std::unique_ptr<Value>> values;

                int start =
                    leftNumber->getValue();

                int end =
                    rightNumber->getValue();

                if (start <= end)
                {
                    for (
                        int i = start;
                        i <= end;
                        i++)
                    {
                        values.push_back(
                            std::make_unique<NumberValue>(i));
                    }
                }

                else
                {
                    for (
                        int i = start;
                        i >= end;
                        i--)
                    {
                        values.push_back(
                            std::make_unique<NumberValue>(i));
                    }
                }

                return std::make_unique<ArrayValue>(
                    std::move(values));
            }
        }

        // BOOLEAN OPERATIONS
        if (
            left->getType() == ValueType::BOOLEAN &&
            right->getType() == ValueType::BOOLEAN)
        {

            auto leftBool =
                static_cast<BooleanValue *>(left.get());

            auto rightBool =
                static_cast<BooleanValue *>(right.get());

            if (binary->op == "&&")
            {
                return std::make_unique<BooleanValue>(
                    leftBool->getValue() &&
                    rightBool->getValue());
            }

            else if (binary->op == "||")
            {
                return std::make_unique<BooleanValue>(
                    leftBool->getValue() ||
                    rightBool->getValue());
            }

            else if (binary->op == "^^")
            {
                return std::make_unique<BooleanValue>(
                    leftBool->getValue() ^
                    rightBool->getValue());
            }
        }
        return std::make_unique<NullValue>();
    }

    else if (node->type == NodeType::VARIABLE_ACCESS)
    {

        auto variable =
            static_cast<VariableAccessNode *>(node);

        Value *value =
            environment.get(
                variable->name);

        if (value)
        {
            return value->clone();
        }
    }
    return std::make_unique<NullValue>();
}
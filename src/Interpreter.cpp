#include "Interpreter.h"
#include "Parser.h"

#include "runtime/values/NumberValue.h"
#include "runtime/values/FloatValue.h"
#include "runtime/values/CharValue.h"
#include "runtime/values/StringValue.h"
#include "runtime/values/BooleanValue.h"
#include "runtime/values/NullValue.h"
#include "runtime/values/ArrayValue.h"
#include "runtime/values/ObjectValue.h"
#include "runtime/values/FunctionValue.h"
#include "runtime/values/BuiltinFunctionValue.h"
#include "runtime/values/ClassValue.h"
#include "runtime/values/ModuleValue.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <random>
#include <chrono>

// ============================================================
// CONSTRUCTOR / INIT
// ============================================================

Interpreter::Interpreter(ErrorHandler* errorHandler)
    : errorHandler(errorHandler)
{
    globals = std::make_shared<Environment>(nullptr, errorHandler);
    environment = globals;
    registerBuiltins();
}

void Interpreter::registerBuiltins()
{
    globals->define("print", std::make_unique<BuiltinFunctionValue>("print", builtinPrint));
    globals->define("println", std::make_unique<BuiltinFunctionValue>("println", builtinPrintln));
    globals->define("input", std::make_unique<BuiltinFunctionValue>("input", builtinInput));
    globals->define("type", std::make_unique<BuiltinFunctionValue>("type", builtinType));
    globals->define("len", std::make_unique<BuiltinFunctionValue>("len", builtinLen));
    globals->define("toInt", std::make_unique<BuiltinFunctionValue>("toInt", builtinToInt));
    globals->define("toFloat", std::make_unique<BuiltinFunctionValue>("toFloat", builtinToFloat));
    globals->define("toString", std::make_unique<BuiltinFunctionValue>("toString", builtinToString));
    globals->define("range", std::make_unique<BuiltinFunctionValue>("range", builtinRange));
    globals->define("map", std::make_unique<BuiltinFunctionValue>("map", builtinMap));
    globals->define("filter", std::make_unique<BuiltinFunctionValue>("filter", builtinFilter));
    globals->define("reduce", std::make_unique<BuiltinFunctionValue>("reduce", builtinReduce));
    globals->define("foreach", std::make_unique<BuiltinFunctionValue>("foreach", builtinForEach));
}

// ============================================================
// MAIN EXECUTION
// ============================================================

void Interpreter::execute(ASTNode* node)
{
    executeStatement(node);
}

void Interpreter::executeProgram(const std::vector<std::unique_ptr<ASTNode>>& nodes)
{
    for (const auto& node : nodes)
    {
        if (node)
            executeStatement(node.get());
    }
}

// ============================================================
// STATEMENT EXECUTION
// ============================================================

void Interpreter::executeStatement(ASTNode* node)
{
    if (!node) return;

    switch (node->type)
    {
        case NodeType::VARIABLE_DECLARATION:
        {
            auto decl = static_cast<VariableDeclarationNode*>(node);
            std::unique_ptr<Value> value;
            if (decl->initializer)
                value = evaluate(decl->initializer.get());
            else
                value = std::make_unique<NullValue>();

            TypeInfo typeInfo = decl->declaredType.value_or(
                value ? value->getTypeInfo() : TypeInfo(TypeKind::ANY));

            // let = immutable by default, const = immutable, var = mutable
            bool effectivelyConst = decl->isConst || !decl->isMutable;
            environment->define(decl->name, std::move(value), effectivelyConst, typeInfo);
            break;
        }

        case NodeType::ASSIGNMENT:
        {
            auto assign = static_cast<AssignmentNode*>(node);
            auto value = evaluate(assign->value.get());

            if (assign->op != "=")
            {
                Value* currentVal = environment->get(assign->name);
                if (!currentVal)
                {
                    if (errorHandler)
                        errorHandler->error(ErrorCode::UNDEFINED_VARIABLE,
                            "Undefined variable '" + assign->name + "'",
                            SourceLocation("", node->line, node->column));
                    break;
                }

                std::string op = assign->op.substr(0, assign->op.size() - 1);
                auto rightVal = evaluate(assign->value.get());

                // Direct computation without creating AST nodes
                auto getNum = [](Value* v) -> double {
                    if (v->getType() == ValueType::INT)
                        return static_cast<double>(static_cast<NumberValue*>(v)->getValue());
                    if (v->getType() == ValueType::FLOAT)
                        return static_cast<FloatValue*>(v)->getValue();
                    return 0.0;
                };
                auto getInt = [](Value* v) -> int64_t {
                    if (v->getType() == ValueType::INT)
                        return static_cast<NumberValue*>(v)->getValue();
                    if (v->getType() == ValueType::FLOAT)
                        return static_cast<int64_t>(static_cast<FloatValue*>(v)->getValue());
                    return 0;
                };

                bool bothInt = (currentVal->getType() == ValueType::INT && rightVal->getType() == ValueType::INT);
                double l = getNum(currentVal), r = getNum(rightVal.get());
                int64_t li = getInt(currentVal), ri = getInt(rightVal.get());

                if (op == "+") {
                    if (bothInt) value = std::make_unique<NumberValue>(li + ri);
                    else value = std::make_unique<FloatValue>(l + r);
                }
                else if (op == "-") {
                    if (bothInt) value = std::make_unique<NumberValue>(li - ri);
                    else value = std::make_unique<FloatValue>(l - r);
                }
                else if (op == "*") {
                    if (bothInt) value = std::make_unique<NumberValue>(li * ri);
                    else value = std::make_unique<FloatValue>(l * r);
                }
                else if (op == "/") {
                    if (bothInt) value = std::make_unique<NumberValue>(li / ri);
                    else value = std::make_unique<FloatValue>(l / r);
                }
                else if (op == "%")
                    value = std::make_unique<NumberValue>(li % ri);
            }

            environment->set(assign->name, std::move(value));
            break;
        }

        case NodeType::PRINT:
        {
            auto printNode = static_cast<PrintNode*>(node);
            auto value = evaluate(printNode->expression.get());
            if (value)
                std::cout << value->toString();
            std::cout << std::endl;
            break;
        }

        case NodeType::EXPRESSION_STATEMENT:
        {
            auto exprStmt = static_cast<ExpressionStatementNode*>(node);
            if (exprStmt->expression)
            {
                auto innerType = exprStmt->expression->type;
                if (innerType == NodeType::ASSIGNMENT ||
                    innerType == NodeType::FUNCTION_CALL ||
                    innerType == NodeType::MEMBER_ASSIGNMENT ||
                    innerType == NodeType::INDEX_ASSIGNMENT)
                {
                    executeStatement(exprStmt->expression.get());
                }
                else
                {
                    evaluate(exprStmt->expression.get());
                }
            }
            break;
        }

        case NodeType::BLOCK:
        {
            auto block = static_cast<BlockNode*>(node);
            auto blockEnv = environment->createChild();
            executeBlock(block->statements, blockEnv);
            break;
        }

        case NodeType::IF:
        {
            auto ifNode = static_cast<IfNode*>(node);
            auto condition = evaluate(ifNode->condition.get());
            bool truthy = condition && condition->isTruthy();

            if (truthy)
            {
                auto thenEnv = environment->createChild();
                executeBlock(ifNode->thenBlock, thenEnv);
            }
            else if (!ifNode->elseBlock.empty())
            {
                auto elseEnv = environment->createChild();
                executeBlock(ifNode->elseBlock, elseEnv);
            }
            break;
        }

        case NodeType::WHILE:
        {
            auto whileNode = static_cast<WhileNode*>(node);
            loopStack.push(LoopControl::NONE);

            while (true)
            {
                if (!loopStack.empty() && loopStack.top() == LoopControl::BREAK) break;
                if (!loopStack.empty() && loopStack.top() == LoopControl::CONTINUE)
                    loopStack.top() = LoopControl::NONE;

                auto condition = evaluate(whileNode->condition.get());
                if (!condition || !condition->isTruthy()) break;

                auto bodyEnv = environment->createChild();
                executeBlock(whileNode->body, bodyEnv);

                if (!loopStack.empty() && loopStack.top() == LoopControl::BREAK) break;
                if (!loopStack.empty() && loopStack.top() == LoopControl::CONTINUE)
                    loopStack.top() = LoopControl::NONE;
            }

            if (!loopStack.empty()) loopStack.pop();
            break;
        }

        case NodeType::FOR:
        {
            auto forNode = static_cast<ForNode*>(node);
            auto iterable = evaluate(forNode->iterable.get());

            loopStack.push(LoopControl::NONE);

            if (iterable && iterable->getType() == ValueType::ARRAY)
            {
                auto arr = static_cast<ArrayValue*>(iterable.get());
                for (size_t i = 0; i < arr->size(); i++)
                {
                    if (!loopStack.empty() && loopStack.top() == LoopControl::BREAK) break;
                    if (!loopStack.empty() && loopStack.top() == LoopControl::CONTINUE)
                        loopStack.top() = LoopControl::NONE;

                    auto bodyEnv = environment->createChild();
                    bodyEnv->define(forNode->variable, arr->get(i)->clone());
                    executeBlock(forNode->body, bodyEnv);
                }
            }

            if (!loopStack.empty()) loopStack.pop();
            break;
        }

        case NodeType::BREAK:
        {
            if (!loopStack.empty())
                loopStack.top() = LoopControl::BREAK;
            break;
        }

        case NodeType::CONTINUE:
        {
            if (!loopStack.empty())
                loopStack.top() = LoopControl::CONTINUE;
            break;
        }

        case NodeType::SWITCH:
        {
            auto switchNode = static_cast<SwitchNode*>(node);
            auto expr = evaluate(switchNode->expression.get());
            bool matched = false;

            for (auto& case_ : switchNode->cases)
            {
                auto caseVal = evaluate(case_.value.get());
                if (expr && caseVal && expr->toString() == caseVal->toString())
                {
                    matched = true;
                    auto caseEnv = environment->createChild();
                    executeBlock(case_.body, caseEnv);
                    break;
                }
            }

            if (!matched && !switchNode->defaultCase.empty())
            {
                auto defaultEnv = environment->createChild();
                executeBlock(switchNode->defaultCase, defaultEnv);
            }
            break;
        }

        case NodeType::FUNCTION_DECLARATION:
        {
            auto funcDecl = static_cast<FunctionDeclarationNode*>(node);
            std::vector<std::string> params;
            for (const auto& p : funcDecl->parameters)
                params.push_back(p.name);

            auto funcValue = std::make_unique<FunctionValue>(
                funcDecl->name, params,
                std::move(funcDecl->body),
                environment,
                funcDecl->name.empty());

            // Extract default parameter values
            std::vector<std::unique_ptr<DefaultParamInfo>> defaults;
            for (auto& p : funcDecl->parameters)
            {
                auto di = std::make_unique<DefaultParamInfo>();
                di->defaultValue = std::move(p.defaultValue);
                defaults.push_back(std::move(di));
            }
            funcValue->setDefaultParams(std::move(defaults));

            if (!funcDecl->name.empty())
                environment->define(funcDecl->name, std::move(funcValue));
            break;
        }

        case NodeType::RETURN:
        {
            auto retNode = static_cast<ReturnNode*>(node);
            if (retNode->value)
            {
                auto val = evaluate(retNode->value.get());
                throw ReturnSignal{std::move(val), true};
            }
            throw ReturnSignal{nullptr, false};
        }

        case NodeType::CLASS_DECLARATION:
        {
            auto classDecl = static_cast<ClassDeclarationNode*>(node);
            auto classEnv = std::make_shared<Environment>(globals, errorHandler);

            // Store class in environment
            auto classVal = std::make_unique<ClassValue>(
                classDecl->name, classDecl->fields, std::move(classDecl->methods), std::move(classEnv));
            environment->define(classDecl->name, std::move(classVal));
            break;
        }

        case NodeType::IMPORT_STATEMENT:
        {
            auto importNode = static_cast<ImportNode*>(node);
            loadModule(importNode->moduleName);
            break;
        }

        case NodeType::FROM_IMPORT:
        {
            auto fromImport = static_cast<FromImportNode*>(node);
            auto cacheIt = moduleCache.find(fromImport->moduleName);
            if (cacheIt == moduleCache.end())
            {
                loadModule(fromImport->moduleName);
                cacheIt = moduleCache.find(fromImport->moduleName);
            }
            if (cacheIt != moduleCache.end())
            {
                for (const auto& name : fromImport->names)
                {
                    Value* val = cacheIt->second->get(name);
                    if (val)
                        environment->define(name, val->clone());
                    else if (errorHandler)
                        errorHandler->error(ErrorCode::UNDEFINED_VARIABLE,
                            "Name '" + name + "' not found in module '" + fromImport->moduleName + "'",
                            SourceLocation("", node->line, node->column));
                }
            }
            break;
        }

        case NodeType::TRY:
        {
            auto tryNode = static_cast<TryNode*>(node);
            auto tryEnv = environment->createChild();
            try {
                executeBlock(tryNode->tryBody, tryEnv);
            }
            catch (ThrowSignal& ts)
            {
                for (auto& clause : tryNode->catchClauses)
                {
                    auto catchEnv = environment->createChild();
                    if (!clause.variableName.empty())
                        catchEnv->define(clause.variableName, ts.value ? ts.value->clone() : std::make_unique<NullValue>());
                    executeBlock(clause.body, catchEnv);
                    break;
                }
            }
            break;
        }

        case NodeType::THROW:
        {
            auto throwNode = static_cast<ThrowNode*>(node);
            auto val = evaluate(throwNode->expression.get());
            throw ThrowSignal{std::move(val)};
        }

        case NodeType::MATCH:
        {
            auto matchNode = static_cast<MatchNode*>(node);
            auto matchValue = evaluate(matchNode->expression.get());

            bool matched = false;
            for (auto& caseNode : matchNode->cases)
            {
                auto caseEnv = environment->createChild();
                if (matchPattern(caseNode.pattern, matchValue.get(), caseEnv))
                {
                    // Check guard
                    if (caseNode.pattern.guard)
                    {
                        auto guardResult = evaluate(caseNode.pattern.guard.get());
                        if (!guardResult || !guardResult->isTruthy())
                            continue;
                    }

                    matched = true;
                    executeBlock(caseNode.body, caseEnv);
                    break;
                }
            }

            if (!matched && !matchNode->elseBody.empty())
            {
                auto elseEnv = environment->createChild();
                executeBlock(matchNode->elseBody, elseEnv);
            }
            break;
        }

        case NodeType::MEMBER_ASSIGNMENT:
        {
            auto memberAssign = static_cast<MemberAssignmentNode*>(node);
            auto obj = evaluate(memberAssign->object.get());
            auto val = evaluate(memberAssign->value.get());

            if (obj && obj->getType() == ValueType::OBJECT)
            {
                auto objVal = static_cast<ObjectValue*>(obj.get());
                objVal->set(memberAssign->member, std::move(val));
            }
            break;
        }

        case NodeType::INDEX_ASSIGNMENT:
        {
            auto indexAssign = static_cast<IndexAssignmentNode*>(node);
            auto arr = evaluate(indexAssign->array.get());
            auto idx = evaluate(indexAssign->index.get());
            auto val = evaluate(indexAssign->value.get());

            if (arr && arr->getType() == ValueType::ARRAY && idx && idx->getType() == ValueType::INT)
            {
                auto arrVal = static_cast<ArrayValue*>(arr.get());
                arrVal->set(static_cast<size_t>(static_cast<NumberValue*>(idx.get())->getValue()), std::move(val));
            }
            break;
        }

        default:
            evaluate(node);
            break;
    }
}

void Interpreter::executeBlock(const std::vector<std::unique_ptr<ASTNode>>& statements,
                               std::shared_ptr<Environment> blockEnv)
{
    auto prevEnv = environment;
    environment = std::move(blockEnv);

    for (const auto& stmt : statements)
    {
        if (!loopStack.empty() && (loopStack.top() == LoopControl::BREAK ||
                                   loopStack.top() == LoopControl::CONTINUE))
            break;

        try
        {
            executeStatement(stmt.get());
        }
        catch (ReturnSignal& /*ret*/)
        {
            environment = prevEnv;
            throw;
        }
    }

    environment = prevEnv;
}

// ============================================================
// EXPRESSION EVALUATION
// ============================================================

std::unique_ptr<Value> Interpreter::evaluate(ASTNode* node)
{
    if (!node)
        return std::make_unique<NullValue>();

    switch (node->type)
    {
        case NodeType::NUMBER_LITERAL:
        {
            auto num = static_cast<NumberNode*>(node);
            return std::make_unique<NumberValue>(num->value);
        }

        case NodeType::FLOAT_LITERAL:
        {
            auto flt = static_cast<FloatNode*>(node);
            return std::make_unique<FloatValue>(flt->value);
        }

        case NodeType::STRING_LITERAL:
        {
            auto str = static_cast<StringNode*>(node);
            return std::make_unique<StringValue>(str->value);
        }

        case NodeType::BOOLEAN_LITERAL:
        {
            auto bol = static_cast<BooleanNode*>(node);
            return std::make_unique<BooleanValue>(bol->value);
        }

        case NodeType::CHAR_LITERAL:
        {
            auto ch = static_cast<CharNode*>(node);
            return std::make_unique<CharValue>(ch->value);
        }

        case NodeType::NULL_LITERAL:
            return std::make_unique<NullValue>();

        case NodeType::VARIABLE_ACCESS:
        {
            auto var = static_cast<VariableAccessNode*>(node);
            if (var->name == "this")
            {
                auto it = environment->get("this");
                if (it) return it->clone();
                return std::make_unique<NullValue>();
            }
            Value* val = environment->get(var->name);
            if (val)
                return val->clone();
            if (errorHandler)
                errorHandler->error(ErrorCode::UNDEFINED_VARIABLE,
                    "Undefined variable '" + var->name + "'",
                    SourceLocation("", node->line, node->column));
            return std::make_unique<NullValue>();
        }

        case NodeType::BINARY_EXPRESSION:
            return evaluateBinary(static_cast<BinaryExpressionNode*>(node));

        case NodeType::UNARY_EXPRESSION:
            return evaluateUnary(static_cast<UnaryExpressionNode*>(node));

        case NodeType::FUNCTION_CALL:
            return evaluateFunctionCall(static_cast<FunctionCallNode*>(node));

        case NodeType::ARRAY_LITERAL:
        {
            auto arrLit = static_cast<ArrayLiteralNode*>(node);
            std::vector<std::unique_ptr<Value>> elements;
            for (auto& elem : arrLit->elements)
                elements.push_back(evaluate(elem.get()));
            return std::make_unique<ArrayValue>(std::move(elements));
        }

        case NodeType::OBJECT_LITERAL:
        {
            auto objLit = static_cast<ObjectLiteralNode*>(node);
            auto obj = std::make_unique<ObjectValue>();
            for (auto& [key, valNode] : objLit->properties)
                obj->set(key, evaluate(valNode.get()));
            return obj;
        }

        case NodeType::MEMBER_ACCESS:
        {
            auto memberAccess = static_cast<MemberAccessNode*>(node);
            auto obj = evaluate(memberAccess->object.get());
            if (obj && obj->getType() == ValueType::OBJECT)
            {
                auto objVal = static_cast<ObjectValue*>(obj.get());
                Value* member = objVal->get(memberAccess->member);
                if (member) return member->clone();
                if (errorHandler)
                    errorHandler->error(ErrorCode::INVALID_MEMBER_ACCESS,
                        "Member '" + memberAccess->member + "' not found",
                        SourceLocation("", node->line, node->column));
            }
            return std::make_unique<NullValue>();
        }

        case NodeType::ARRAY_ACCESS:
        {
            auto arrAccess = static_cast<ArrayAccessNode*>(node);
            auto arr = evaluate(arrAccess->array.get());
            auto idx = evaluate(arrAccess->index.get());

            if (arr && arr->getType() == ValueType::ARRAY && idx)
            {
                auto arrVal = static_cast<ArrayValue*>(arr.get());
                size_t index = 0;

                if (idx->getType() == ValueType::INT)
                    index = static_cast<size_t>(static_cast<NumberValue*>(idx.get())->getValue());
                else if (idx->getType() == ValueType::FLOAT)
                    index = static_cast<size_t>(static_cast<FloatValue*>(idx.get())->getValue());

                if (index < arrVal->size())
                {
                    Value* elem = arrVal->get(index);
                    if (elem) return elem->clone();
                }

                if (errorHandler)
                    errorHandler->error(ErrorCode::INDEX_OUT_OF_BOUNDS,
                        "Index " + std::to_string(index) + " out of bounds for array of size " +
                        std::to_string(arrVal->size()),
                        SourceLocation("", node->line, node->column));
            }
            return std::make_unique<NullValue>();
        }

        case NodeType::NEW_EXPRESSION:
        {
            auto newExpr = static_cast<NewExpressionNode*>(node);
            Value* classVal = environment->get(newExpr->className);
            if (!classVal || classVal->getType() != ValueType::CLASS)
            {
                if (errorHandler)
                    errorHandler->error(ErrorCode::UNDEFINED_VARIABLE,
                        "Class '" + newExpr->className + "' not defined",
                        SourceLocation("", node->line, node->column));
                return std::make_unique<NullValue>();
            }

            auto classObj = static_cast<ClassValue*>(classVal);
            auto obj = std::make_unique<ObjectValue>(classObj->getName());

            // Create instance environment
            auto instanceEnv = std::make_shared<Environment>(classObj->getClassEnv(), errorHandler);
            instanceEnv->define("this", obj->clone());

            // Initialize fields
            for (const auto& field : classObj->getFields())
                instanceEnv->define(field, std::make_unique<NullValue>());

            auto cloneBody = [](const std::vector<std::unique_ptr<ASTNode>>& src) -> std::vector<std::unique_ptr<ASTNode>> {
                std::vector<std::unique_ptr<ASTNode>> dst;
                for (const auto& node : src) {
                    if (node) dst.push_back(node->clone());
                }
                return dst;
            };

            // Find and execute constructor
            auto& methods = classObj->getMethods();
            for (auto& method : methods)
            {
                if (method.function.name == newExpr->className ||
                    method.function.name == "constructor" ||
                    method.function.name == "init")
                {
                    std::vector<std::string> params;
                    for (const auto& p : method.function.parameters)
                        params.push_back(p.name);

                    auto ctorFunc = std::make_unique<FunctionValue>(
                        method.function.name, params,
                        cloneBody(method.function.body),
                        instanceEnv);

                    // Bind arguments
                    auto callEnv = instanceEnv->createChild();
                    for (size_t i = 0; i < params.size() && i < newExpr->arguments.size(); i++)
                    {
                        auto argVal = evaluate(newExpr->arguments[i].get());
                        callEnv->define(params[i], std::move(argVal));
                    }

                    auto prevEnv = environment;
                    environment = callEnv;
                    try
                    {
                        for (auto& stmt : ctorFunc->getBody())
                            executeStatement(stmt.get());
                    }
                    catch (ReturnSignal&)
                    {
                        // Constructor return is ignored
                    }
                    environment = prevEnv;
                    break;
                }
            }

            // Bind methods
            for (auto& method : classObj->getMethods())
            {
                if (method.function.name != newExpr->className &&
                    method.function.name != "constructor" &&
                    method.function.name != "init")
                {
                    std::vector<std::string> params;
                    for (const auto& p : method.function.parameters)
                        params.push_back(p.name);

                    auto methodFunc = std::make_unique<FunctionValue>(
                        method.function.name, params,
                        cloneBody(method.function.body),
                        instanceEnv);

                    obj->set(method.function.name, std::move(methodFunc));
                }
            }

            return obj;
        }

        default:
            break;
    }

    return std::make_unique<NullValue>();
}

// ============================================================
// BINARY EXPRESSIONS
// ============================================================

std::unique_ptr<Value> Interpreter::evaluateBinary(BinaryExpressionNode* node)
{
    auto left = evaluate(node->left.get());
    auto right = evaluate(node->right.get());

    if (!left || !right)
        return std::make_unique<NullValue>();

    const std::string& op = node->op;

    // String concatenation
    if (left->getType() == ValueType::STRING || right->getType() == ValueType::STRING)
    {
        if (op == "+")
            return std::make_unique<StringValue>(left->toString() + right->toString());
    }

    // Numeric operations
    auto getNum = [](Value* v) -> double {
        if (v->getType() == ValueType::INT)
            return static_cast<double>(static_cast<NumberValue*>(v)->getValue());
        if (v->getType() == ValueType::FLOAT)
            return static_cast<FloatValue*>(v)->getValue();
        return 0.0;
    };

    auto getInt = [](Value* v) -> int64_t {
        if (v->getType() == ValueType::INT)
            return static_cast<NumberValue*>(v)->getValue();
        if (v->getType() == ValueType::FLOAT)
            return static_cast<int64_t>(static_cast<FloatValue*>(v)->getValue());
        return 0;
    };

    bool leftNum = (left->getType() == ValueType::INT || left->getType() == ValueType::FLOAT);
    bool rightNum = (right->getType() == ValueType::INT || right->getType() == ValueType::FLOAT);
    bool bothInt = (left->getType() == ValueType::INT && right->getType() == ValueType::INT);

    if (leftNum && rightNum)
    {
        double l = getNum(left.get());
        double r = getNum(right.get());
        int64_t li = getInt(left.get());
        int64_t ri = getInt(right.get());

        if (op == "+") {
            if (bothInt) return std::make_unique<NumberValue>(li + ri);
            return std::make_unique<FloatValue>(l + r);
        }
        if (op == "-") {
            if (bothInt) return std::make_unique<NumberValue>(li - ri);
            return std::make_unique<FloatValue>(l - r);
        }
        if (op == "*") {
            if (bothInt) return std::make_unique<NumberValue>(li * ri);
            return std::make_unique<FloatValue>(l * r);
        }
        if (op == "/") {
            if (ri == 0 || r == 0.0) {
                if (errorHandler)
                    errorHandler->error(ErrorCode::DIVISION_BY_ZERO,
                        "Division by zero", SourceLocation("", node->line, node->column));
                return std::make_unique<NullValue>();
            }
            if (bothInt) return std::make_unique<NumberValue>(li / ri);
            return std::make_unique<FloatValue>(l / r);
        }
        if (op == "%") {
            if (ri == 0) {
                if (errorHandler)
                    errorHandler->error(ErrorCode::DIVISION_BY_ZERO,
                        "Modulo by zero", SourceLocation("", node->line, node->column));
                return std::make_unique<NullValue>();
            }
            return std::make_unique<NumberValue>(li % ri);
        }
        if (op == "==") return std::make_unique<BooleanValue>(l == r);
        if (op == "!=") return std::make_unique<BooleanValue>(l != r);
        if (op == ">")  return std::make_unique<BooleanValue>(l > r);
        if (op == "<")  return std::make_unique<BooleanValue>(l < r);
        if (op == ">=") return std::make_unique<BooleanValue>(l >= r);
        if (op == "<=") return std::make_unique<BooleanValue>(l <= r);
        if (op == "<=>") {
            int cmp = (l < r) ? -1 : (l > r) ? 1 : 0;
            return std::make_unique<NumberValue>(cmp);
        }
        if (op == "**") {
            double result = std::pow(l, r);
            if (bothInt) return std::make_unique<NumberValue>(static_cast<int64_t>(result));
            return std::make_unique<FloatValue>(result);
        }

        // Range operator
        if (op == ".." || op == "->")
        {
            std::vector<std::unique_ptr<Value>> values;
            int start = static_cast<int>(li);
            int end = static_cast<int>(ri);
            if (start <= end)
            {
                for (int i = start; i <= end; i++)
                    values.push_back(std::make_unique<NumberValue>(i));
            }
            else
            {
                for (int i = start; i >= end; i--)
                    values.push_back(std::make_unique<NumberValue>(i));
            }
            return std::make_unique<ArrayValue>(std::move(values));
        }
    }

    // Boolean operations
    if (left->getType() == ValueType::BOOLEAN && right->getType() == ValueType::BOOLEAN)
    {
        bool lb = static_cast<BooleanValue*>(left.get())->getValue();
        bool rb = static_cast<BooleanValue*>(right.get())->getValue();

        if (op == "&&") return std::make_unique<BooleanValue>(lb && rb);
        if (op == "||") return std::make_unique<BooleanValue>(lb || rb);
        if (op == "^^") return std::make_unique<BooleanValue>(lb ^ rb);
        if (op == "==") return std::make_unique<BooleanValue>(lb == rb);
        if (op == "!=") return std::make_unique<BooleanValue>(lb != rb);
    }

    // String comparison
    if (left->getType() == ValueType::STRING && right->getType() == ValueType::STRING)
    {
        const std::string& ls = static_cast<StringValue*>(left.get())->getValue();
        const std::string& rs = static_cast<StringValue*>(right.get())->getValue();

        if (op == "==") return std::make_unique<BooleanValue>(ls == rs);
        if (op == "!=") return std::make_unique<BooleanValue>(ls != rs);
        if (op == "+")  return std::make_unique<StringValue>(ls + rs);
    }

    // Fallback comparison by string
    if (op == "==") return std::make_unique<BooleanValue>(left->toString() == right->toString());
    if (op == "!=") return std::make_unique<BooleanValue>(left->toString() != right->toString());

    return std::make_unique<NullValue>();
}

// ============================================================
// UNARY EXPRESSIONS
// ============================================================

std::unique_ptr<Value> Interpreter::evaluateUnary(UnaryExpressionNode* node)
{
    auto operand = evaluate(node->operand.get());
    if (!operand) return std::make_unique<NullValue>();

    const std::string& op = node->op;

    if (op == "-")
    {
        if (operand->getType() == ValueType::INT)
            return std::make_unique<NumberValue>(-static_cast<NumberValue*>(operand.get())->getValue());
        if (operand->getType() == ValueType::FLOAT)
            return std::make_unique<FloatValue>(-static_cast<FloatValue*>(operand.get())->getValue());
    }

    if (op == "!")
        return std::make_unique<BooleanValue>(!operand->isTruthy());

    if (op == "+")
        return operand->clone();

    return operand;
}

// ============================================================
// FUNCTION CALLS
// ============================================================

std::unique_ptr<Value> Interpreter::evaluateFunctionCall(FunctionCallNode* node)
{
    // Evaluate positional arguments
    std::vector<std::unique_ptr<Value>> args;
    for (auto& arg : node->arguments)
        args.push_back(evaluate(arg.get()));

    // Evaluate named arguments
    std::unordered_map<std::string, std::unique_ptr<Value>> namedArgs;
    for (auto& namedArg : node->namedArguments)
        namedArgs[namedArg.name] = evaluate(namedArg.value.get());

    Value* callee = environment->get(node->functionName);
    if (!callee)
    {
        if (errorHandler)
            errorHandler->error(ErrorCode::UNDEFINED_FUNCTION,
                "Undefined function '" + node->functionName + "'",
                SourceLocation("", node->line, node->column));
        return std::make_unique<NullValue>();
    }

    if (callee->getType() == ValueType::BUILTIN_FUNCTION)
    {
        auto builtin = static_cast<BuiltinFunctionValue*>(callee);
        return builtin->getFunction()(args, environment.get());
    }

    if (callee->getType() == ValueType::FUNCTION)
    {
        auto func = static_cast<FunctionValue*>(callee);
        auto closure = func->getClosure();
        auto callEnv = std::make_shared<Environment>(closure, errorHandler);

        const auto& params = func->getParameters();
        const auto& defaults = func->getDefaultParams();

        for (size_t i = 0; i < params.size(); i++)
        {
            // Check named argument first
            auto namedIt = namedArgs.find(params[i]);
            if (namedIt != namedArgs.end())
            {
                callEnv->define(params[i], std::move(namedIt->second));
            }
            else if (i < args.size())
            {
                callEnv->define(params[i], std::move(args[i]));
            }
            else if (i < defaults.size() && defaults[i] && defaults[i]->defaultValue)
            {
                auto defaultValue = evaluate(defaults[i]->defaultValue.get());
                callEnv->define(params[i], std::move(defaultValue));
            }
            else
            {
                callEnv->define(params[i], std::make_unique<NullValue>());
            }
        }

        auto prevEnv = environment;
        environment = callEnv;

        std::unique_ptr<Value> result = std::make_unique<NullValue>();
        try
        {
            for (auto& stmt : func->getBody())
                executeStatement(stmt.get());
        }
        catch (ReturnSignal& ret)
        {
            result = ret.hasValue ? std::move(ret.value) : std::make_unique<NullValue>();
        }

        environment = prevEnv;
        return result;
    }

    if (errorHandler)
        errorHandler->error(ErrorCode::NOT_CALLABLE,
            "'" + node->functionName + "' is not callable",
            SourceLocation("", node->line, node->column));
    return std::make_unique<NullValue>();
}

// ============================================================
// BUILT-IN FUNCTIONS
// ============================================================

std::unique_ptr<Value> Interpreter::builtinPrint(const std::vector<std::unique_ptr<Value>>& args, Environment* env)
{
    (void)env;
    for (size_t i = 0; i < args.size(); i++)
    {
        if (i > 0) std::cout << " ";
        if (args[i]) std::cout << args[i]->toString();
    }
    return std::make_unique<NullValue>();
}

std::unique_ptr<Value> Interpreter::builtinPrintln(const std::vector<std::unique_ptr<Value>>& args, Environment* env)
{
    builtinPrint(args, env);
    std::cout << std::endl;
    return std::make_unique<NullValue>();
}

std::unique_ptr<Value> Interpreter::builtinInput(const std::vector<std::unique_ptr<Value>>& args, Environment* env)
{
    (void)env;
    if (!args.empty() && args[0])
        std::cout << args[0]->toString();
    std::string line;
    std::getline(std::cin, line);
    return std::make_unique<StringValue>(line);
}

std::unique_ptr<Value> Interpreter::builtinType(const std::vector<std::unique_ptr<Value>>& args, Environment* env)
{
    (void)env;
    if (args.empty() || !args[0]) return std::make_unique<StringValue>("null");
    return std::make_unique<StringValue>(args[0]->getTypeInfo().toString());
}

std::unique_ptr<Value> Interpreter::builtinLen(const std::vector<std::unique_ptr<Value>>& args, Environment* env)
{
    (void)env;
    if (args.empty() || !args[0]) return std::make_unique<NumberValue>(0);

    if (args[0]->getType() == ValueType::ARRAY)
        return std::make_unique<NumberValue>(static_cast<int64_t>(static_cast<ArrayValue*>(args[0].get())->size()));
    if (args[0]->getType() == ValueType::STRING)
        return std::make_unique<NumberValue>(static_cast<int64_t>(static_cast<StringValue*>(args[0].get())->getValue().size()));

    return std::make_unique<NumberValue>(0);
}

std::unique_ptr<Value> Interpreter::builtinToInt(const std::vector<std::unique_ptr<Value>>& args, Environment* env)
{
    (void)env;
    if (args.empty() || !args[0]) return std::make_unique<NumberValue>(0);

    try {
        if (args[0]->getType() == ValueType::INT)
            return std::make_unique<NumberValue>(static_cast<NumberValue*>(args[0].get())->getValue());
        if (args[0]->getType() == ValueType::FLOAT)
            return std::make_unique<NumberValue>(static_cast<int64_t>(static_cast<FloatValue*>(args[0].get())->getValue()));
        if (args[0]->getType() == ValueType::STRING)
            return std::make_unique<NumberValue>(std::stoll(static_cast<StringValue*>(args[0].get())->getValue()));
    } catch (...) {}

    return std::make_unique<NumberValue>(0);
}

std::unique_ptr<Value> Interpreter::builtinToFloat(const std::vector<std::unique_ptr<Value>>& args, Environment* env)
{
    (void)env;
    if (args.empty() || !args[0]) return std::make_unique<FloatValue>(0.0);

    try {
        if (args[0]->getType() == ValueType::FLOAT)
            return std::make_unique<FloatValue>(static_cast<FloatValue*>(args[0].get())->getValue());
        if (args[0]->getType() == ValueType::INT)
            return std::make_unique<FloatValue>(static_cast<double>(static_cast<NumberValue*>(args[0].get())->getValue()));
        if (args[0]->getType() == ValueType::STRING)
            return std::make_unique<FloatValue>(std::stod(static_cast<StringValue*>(args[0].get())->getValue()));
    } catch (...) {}

    return std::make_unique<FloatValue>(0.0);
}

std::unique_ptr<Value> Interpreter::builtinToString(const std::vector<std::unique_ptr<Value>>& args, Environment* env)
{
    (void)env;
    if (args.empty() || !args[0]) return std::make_unique<StringValue>("null");
    return std::make_unique<StringValue>(args[0]->toString());
}

std::unique_ptr<Value> Interpreter::builtinRange(const std::vector<std::unique_ptr<Value>>& args, Environment* env)
{
    (void)env;
    int start = 0, end = 0, step = 1;

    if (args.size() == 1 && args[0])
    {
        end = static_cast<int>(static_cast<NumberValue*>(args[0].get())->getValue());
    }
    else if (args.size() >= 2 && args[0] && args[1])
    {
        start = static_cast<int>(static_cast<NumberValue*>(args[0].get())->getValue());
        end = static_cast<int>(static_cast<NumberValue*>(args[1].get())->getValue());
    }
    if (args.size() >= 3 && args[2])
        step = static_cast<int>(static_cast<NumberValue*>(args[2].get())->getValue());

    std::vector<std::unique_ptr<Value>> values;
    if (step > 0)
    {
        for (int i = start; i < end; i += step)
            values.push_back(std::make_unique<NumberValue>(i));
    }
    else
    {
        for (int i = start; i > end; i += step)
            values.push_back(std::make_unique<NumberValue>(i));
    }

    return std::make_unique<ArrayValue>(std::move(values));
}

// ============================================================
// FUNCTIONAL BUILTINS (map, filter, reduce, foreach)
// ============================================================

std::unique_ptr<Value> Interpreter::callFunctionWithArgs(FunctionValue* func,
    std::vector<std::unique_ptr<Value>> callArgs, ErrorHandler* errorHandler)
{
    auto closure = func->getClosure();
    auto callEnv = std::make_shared<Environment>(closure, errorHandler);
    const auto& params = func->getParameters();
    for (size_t i = 0; i < params.size(); i++)
    {
        if (i < callArgs.size())
            callEnv->define(params[i], std::move(callArgs[i]));
        else
            callEnv->define(params[i], std::make_unique<NullValue>());
    }

    Interpreter temp(errorHandler);
    temp.setGlobals(callEnv);
    temp.setEnvironment(callEnv);
    temp.executeProgram(func->getBody());
    return std::make_unique<NullValue>();
}

std::unique_ptr<Value> Interpreter::builtinMap(const std::vector<std::unique_ptr<Value>>& args, Environment* env)
{
    (void)env;
    if (args.size() < 2 || !args[0] || !args[1]) return std::make_unique<NullValue>();
    if (args[0]->getType() != ValueType::ARRAY || args[1]->getType() != ValueType::FUNCTION)
        return std::make_unique<NullValue>();
    auto arr = static_cast<ArrayValue*>(args[0].get());
    auto func = static_cast<FunctionValue*>(args[1].get());

    std::vector<std::unique_ptr<Value>> result;
    for (size_t i = 0; i < arr->size(); i++)
    {
        std::vector<std::unique_ptr<Value>> callArgs;
        callArgs.push_back(arr->get(i)->clone());
        auto val = callFunctionWithArgs(func, std::move(callArgs), nullptr);
        result.push_back(std::move(val));
    }
    return std::make_unique<ArrayValue>(std::move(result));
}

std::unique_ptr<Value> Interpreter::builtinFilter(const std::vector<std::unique_ptr<Value>>& args, Environment* env)
{
    (void)env;
    if (args.size() < 2 || !args[0] || !args[1]) return std::make_unique<NullValue>();
    if (args[0]->getType() != ValueType::ARRAY || args[1]->getType() != ValueType::FUNCTION)
        return std::make_unique<NullValue>();
    auto arr = static_cast<ArrayValue*>(args[0].get());
    auto func = static_cast<FunctionValue*>(args[1].get());

    std::vector<std::unique_ptr<Value>> result;
    for (size_t i = 0; i < arr->size(); i++)
    {
        std::vector<std::unique_ptr<Value>> callArgs;
        callArgs.push_back(arr->get(i)->clone());
        auto pred = callFunctionWithArgs(func, std::move(callArgs), nullptr);
        if (pred && pred->isTruthy())
            result.push_back(arr->get(i)->clone());
    }
    return std::make_unique<ArrayValue>(std::move(result));
}

std::unique_ptr<Value> Interpreter::builtinReduce(const std::vector<std::unique_ptr<Value>>& args, Environment* env)
{
    (void)env;
    if (args.size() < 2 || !args[0] || !args[1]) return std::make_unique<NullValue>();
    if (args[0]->getType() != ValueType::ARRAY || args[1]->getType() != ValueType::FUNCTION)
        return std::make_unique<NullValue>();
    auto arr = static_cast<ArrayValue*>(args[0].get());
    auto func = static_cast<FunctionValue*>(args[1].get());

    std::unique_ptr<Value> acc;
    if (args.size() >= 3 && args[2])
        acc = args[2]->clone();
    else if (arr->size() > 0)
        acc = arr->get(0)->clone();

    size_t startIdx = (args.size() >= 3 || !acc) ? 0 : 1;
    for (size_t i = startIdx; i < arr->size(); i++)
    {
        std::vector<std::unique_ptr<Value>> callArgs;
        callArgs.push_back(acc ? acc->clone() : std::make_unique<NullValue>());
        callArgs.push_back(arr->get(i)->clone());
        acc = callFunctionWithArgs(func, std::move(callArgs), nullptr);
    }
    return acc ? std::move(acc) : std::make_unique<NullValue>();
}

std::unique_ptr<Value> Interpreter::builtinForEach(const std::vector<std::unique_ptr<Value>>& args, Environment* env)
{
    (void)env;
    if (args.size() < 2 || !args[0] || !args[1]) return std::make_unique<NullValue>();
    if (args[0]->getType() != ValueType::ARRAY || args[1]->getType() != ValueType::FUNCTION)
        return std::make_unique<NullValue>();
    auto arr = static_cast<ArrayValue*>(args[0].get());
    auto func = static_cast<FunctionValue*>(args[1].get());

    for (size_t i = 0; i < arr->size(); i++)
    {
        std::vector<std::unique_ptr<Value>> callArgs;
        callArgs.push_back(arr->get(i)->clone());
        callFunctionWithArgs(func, std::move(callArgs), nullptr);
    }
    return std::make_unique<NullValue>();
}

// ============================================================
// MODULE SYSTEM
// ============================================================

void Interpreter::loadModule(const std::string& moduleName)
{
    if (moduleCache.find(moduleName) != moduleCache.end())
        return;

    std::vector<std::string> searchPaths = {
        moduleName + ".ketsa",
        "std/" + moduleName + ".ketsa",
        "lib/" + moduleName + ".ketsa",
        moduleName + "/mod.ketsa"
    };

    std::string foundPath;
    for (const auto& path : searchPaths)
    {
        std::ifstream f(path);
        if (f.good())
        {
            foundPath = path;
            break;
        }
    }

    if (foundPath.empty())
    {
        if (errorHandler)
            errorHandler->error(ErrorCode::MODULE_NOT_FOUND,
                "Module '" + moduleName + "' not found",
                SourceLocation("", 0, 0));
        return;
    }

    auto moduleEnv = std::make_shared<Environment>(globals, errorHandler);

    // Read and execute the module file
    std::ifstream file(foundPath);
    if (!file)
    {
        if (errorHandler)
            errorHandler->error(ErrorCode::FILE_NOT_FOUND,
                "Cannot open file: " + foundPath,
                SourceLocation("", 0, 0));
        return;
    }

    std::string source((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());

    ErrorHandler localHandler;
    localHandler.setSource(source);
    localHandler.setFilename(foundPath);

    Lexer lexer(source, &localHandler);
    auto tokens = lexer.tokenize();

    if (localHandler.hasErrors())
    {
        if (errorHandler)
            errorHandler->error(ErrorCode::MODULE_LOAD_ERROR,
                "Error loading module '" + moduleName + "'",
                SourceLocation("", 0, 0));
        return;
    }

    Parser parser(tokens, &localHandler);
    auto ast = parser.parse();

    if (localHandler.hasErrors())
    {
        if (errorHandler)
            errorHandler->error(ErrorCode::MODULE_LOAD_ERROR,
                "Error parsing module '" + moduleName + "'",
                SourceLocation("", 0, 0));
        return;
    }

    // Execute in module environment
    auto prevEnv = environment;
    environment = moduleEnv;

    for (auto& stmt : ast)
    {
        if (stmt) executeStatement(stmt.get());
    }

    environment = prevEnv;

    moduleCache[moduleName] = moduleEnv;
}

// ============================================================
// PATTERN MATCHING
// ============================================================

bool Interpreter::matchLiteral(const MatchPattern& pattern, Value* value)
{
    if (!pattern.literal || !value) return false;

    auto litVal = evaluate(pattern.literal.get());
    if (!litVal) return false;

    return value->toString() == litVal->toString();
}

bool Interpreter::matchPattern(const MatchPattern& pattern, Value* value,
                               std::shared_ptr<Environment> env)
{
    if (!value) return false;

    switch (pattern.kind)
    {
        case PatternKind::WILDCARD:
            return true;

        case PatternKind::LITERAL:
            return matchLiteral(pattern, value);

        case PatternKind::VARIABLE:
            env->define(pattern.variableName, value->clone());
            return true;

        case PatternKind::DESTRUCTURE_ARRAY:
        {
            if (value->getType() != ValueType::ARRAY) return false;
            auto arrVal = static_cast<ArrayValue*>(value);

            size_t minLen = pattern.subPatterns.size();
            if (arrVal->size() < minLen) return false;

            for (size_t i = 0; i < minLen; i++)
            {
                Value* elem = arrVal->get(i);
                if (!elem || !matchPattern(pattern.subPatterns[i], elem, env))
                    return false;
            }
            return true;
        }

        case PatternKind::DESTRUCTURE_OBJECT:
        {
            if (value->getType() != ValueType::OBJECT) return false;
            auto objVal = static_cast<ObjectValue*>(value);

            for (auto& [fieldName, subPat] : pattern.objectPatterns)
            {
                Value* fieldVal = objVal->get(fieldName);
                if (!fieldVal || !matchPattern(subPat, fieldVal, env))
                    return false;
            }
            return true;
        }

        case PatternKind::GUARD:
            return true;
    }
    return false;
}

// ============================================================
// HELPERS
// ============================================================

Value* Interpreter::getValue(const std::string& name)
{
    return environment->get(name);
}

bool Interpreter::checkNumeric(Value* val)
{
    return val && (val->getType() == ValueType::INT || val->getType() == ValueType::FLOAT);
}

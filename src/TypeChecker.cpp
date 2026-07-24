#include "TypeChecker.h"

static bool isNumeric(const TypeInfo& t)
{
    return t.kind == TypeKind::INT || t.kind == TypeKind::FLOAT || t.kind == TypeKind::DOUBLE;
}

TypeChecker::TypeChecker(ErrorHandler* handler)
    : errorHandler(handler)
    , currentScope(nullptr)
    , hasReturnPath(false)
{
    pushScope();

    // Built-in functions
    declareFunction("print", {"print", {TypeInfo(TypeKind::ANY)}, TypeInfo(TypeKind::VOID), {"value"}, false, {}});
    declareFunction("println", {"println", {TypeInfo(TypeKind::ANY)}, TypeInfo(TypeKind::VOID), {"value"}, false, {}});
    declareFunction("input", {"input", {TypeInfo(TypeKind::STRING)}, TypeInfo(TypeKind::STRING), {"prompt"}, false, {}});
    declareFunction("type", {"type", {TypeInfo(TypeKind::ANY)}, TypeInfo(TypeKind::STRING), {"value"}, false, {}});
    declareFunction("len", {"len", {TypeInfo(TypeKind::ANY)}, TypeInfo(TypeKind::INT), {"value"}, false, {}});
    declareFunction("toInt", {"toInt", {TypeInfo(TypeKind::ANY)}, TypeInfo(TypeKind::INT), {"value"}, false, {}});
    declareFunction("toFloat", {"toFloat", {TypeInfo(TypeKind::ANY)}, TypeInfo(TypeKind::FLOAT), {"value"}, false, {}});
    declareFunction("toString", {"toString", {TypeInfo(TypeKind::ANY)}, TypeInfo(TypeKind::STRING), {"value"}, false, {}});
    declareFunction("range", {"range", {TypeInfo(TypeKind::INT), TypeInfo(TypeKind::INT)}, TypeInfo::createArray(std::make_unique<TypeInfo>(TypeKind::INT)), {"start", "end"}, false, {}});
}

TypeChecker::Scope* TypeChecker::pushScope()
{
    auto scope = std::make_unique<Scope>(currentScope);
    Scope* result = scope.get();
    scopeStorage.push_back(std::move(scope));
    currentScope = result;
    return result;
}

void TypeChecker::popScope()
{
    if (currentScope)
        currentScope = currentScope->parent;
}

SymbolInfo* TypeChecker::lookupSymbol(const std::string& name)
{
    for (Scope* s = currentScope; s; s = s->parent)
    {
        auto it = s->symbols.find(name);
        if (it != s->symbols.end())
            return &it->second;
    }
    return nullptr;
}

FunctionSignature* TypeChecker::lookupFunction(const std::string& name)
{
    for (Scope* s = currentScope; s; s = s->parent)
    {
        auto it = s->functions.find(name);
        if (it != s->functions.end())
            return &it->second;
    }
    return nullptr;
}

ClassInfo* TypeChecker::lookupClass(const std::string& name)
{
    auto it = globalClasses.find(name);
    if (it != globalClasses.end())
        return &it->second;
    return nullptr;
}

InterfaceInfo* TypeChecker::lookupInterface(const std::string& name)
{
    auto it = globalInterfaces.find(name);
    if (it != globalInterfaces.end())
        return &it->second;
    return nullptr;
}

void TypeChecker::declareSymbol(const std::string& name, const SymbolInfo& info)
{
    if (currentScope->symbols.find(name) != currentScope->symbols.end())
    {
        if (errorHandler)
            errorHandler->error(ErrorCode::REDECLARED_VARIABLE,
                "Variable '" + name + "' is already declared in this scope",
                SourceLocation("", 0, 0));
        return;
    }
    currentScope->symbols[name] = info;
}

void TypeChecker::declareFunction(const std::string& name, const FunctionSignature& sig)
{
    currentScope->functions[name] = sig;
}

void TypeChecker::declareClass(const std::string& name, const ClassInfo& info)
{
    globalClasses[name] = info;
    currentScope->classes[name] = info;
}

TypeInfo TypeChecker::resolveTypeAnnotation(const std::optional<TypeInfo>& declared, ASTNode* initExpr)
{
    if (declared.has_value())
    {
        TypeInfo t = declared.value();
        if (t.kind == TypeKind::ANY && initExpr)
        {
            TypeInfo inferred = inferType(initExpr);
            if (inferred.kind != TypeKind::ANY && inferred.kind != TypeKind::NULL_TYPE)
                return inferred;
        }
        return t;
    }
    if (initExpr)
        return inferType(initExpr);
    return TypeInfo(TypeKind::ANY);
}

TypeInfo TypeChecker::inferType(ASTNode* node)
{
    if (!node) return TypeInfo(TypeKind::ANY);

    switch (node->type)
    {
        case NodeType::NUMBER_LITERAL:
            return TypeInfo(TypeKind::INT);

        case NodeType::FLOAT_LITERAL:
            return TypeInfo(TypeKind::FLOAT);

        case NodeType::STRING_LITERAL:
            return TypeInfo(TypeKind::STRING);

        case NodeType::BOOLEAN_LITERAL:
            return TypeInfo(TypeKind::BOOLEAN);

        case NodeType::CHAR_LITERAL:
            return TypeInfo(TypeKind::CHAR);

        case NodeType::NULL_LITERAL:
            return TypeInfo(TypeKind::NULL_TYPE);

        case NodeType::VARIABLE_ACCESS:
        {
            auto var = static_cast<VariableAccessNode*>(node);
            SymbolInfo* sym = lookupSymbol(var->name);
            if (sym)
                return sym->type;
            return TypeInfo(TypeKind::ANY);
        }

        case NodeType::BINARY_EXPRESSION:
        {
            auto bin = static_cast<BinaryExpressionNode*>(node);
            TypeInfo leftType = inferType(bin->left.get());
            TypeInfo rightType = inferType(bin->right.get());

            const std::string& op = bin->op;

            if (op == "+" && (leftType.kind == TypeKind::STRING || rightType.kind == TypeKind::STRING))
                return TypeInfo(TypeKind::STRING);

            if (isNumeric(leftType) && isNumeric(rightType))
            {
                if (op == "==" || op == "!=" || op == ">" || op == "<" || op == ">=" || op == "<=")
                    return TypeInfo(TypeKind::BOOLEAN);
                if (op == "..")
                    return TypeInfo::createArray(std::make_unique<TypeInfo>(TypeKind::INT));
                if (leftType.kind == TypeKind::FLOAT || rightType.kind == TypeKind::FLOAT)
                    return TypeInfo(TypeKind::FLOAT);
                return TypeInfo(TypeKind::INT);
            }

            if (leftType.kind == TypeKind::BOOLEAN && rightType.kind == TypeKind::BOOLEAN)
            {
                if (op == "&&" || op == "||" || op == "^^")
                    return TypeInfo(TypeKind::BOOLEAN);
                if (op == "==" || op == "!=")
                    return TypeInfo(TypeKind::BOOLEAN);
            }

            return TypeInfo(TypeKind::ANY);
        }

        case NodeType::UNARY_EXPRESSION:
        {
            auto una = static_cast<UnaryExpressionNode*>(node);
            TypeInfo operandType = inferType(una->operand.get());

            if (una->op == "!")
                return TypeInfo(TypeKind::BOOLEAN);
            if (una->op == "-" && isNumeric(operandType))
                return operandType;

            return operandType;
        }

        case NodeType::FUNCTION_CALL:
        {
            auto call = static_cast<FunctionCallNode*>(node);
            FunctionSignature* sig = lookupFunction(call->functionName);
            if (sig)
                return sig->returnType;
            return TypeInfo(TypeKind::ANY);
        }

        case NodeType::ARRAY_LITERAL:
        {
            auto arrLit = static_cast<ArrayLiteralNode*>(node);
            if (arrLit->elements.empty())
                return TypeInfo::createArray(std::make_unique<TypeInfo>(TypeKind::ANY));
            TypeInfo elemType = inferType(arrLit->elements[0].get());
            return TypeInfo::createArray(std::make_unique<TypeInfo>(elemType));
        }

        case NodeType::OBJECT_LITERAL:
        {
            TypeInfo t(TypeKind::OBJECT);
            t.name = "object";
            return t;
        }

        case NodeType::MEMBER_ACCESS:
        {
            auto member = static_cast<MemberAccessNode*>(node);
            TypeInfo objType = inferType(member->object.get());
            return TypeInfo(TypeKind::ANY);
        }

        case NodeType::ARRAY_ACCESS:
        {
            auto arrAccess = static_cast<ArrayAccessNode*>(node);
            TypeInfo arrType = inferType(arrAccess->array.get());
            if (arrType.kind == TypeKind::ARRAY && arrType.elementType)
                return *arrType.elementType;
            return TypeInfo(TypeKind::ANY);
        }

        case NodeType::NEW_EXPRESSION:
        {
            auto newExpr = static_cast<NewExpressionNode*>(node);
            TypeInfo t(TypeKind::OBJECT);
            t.name = newExpr->className;
            return t;
        }

        case NodeType::LAMBDA:
        {
            auto lambda = static_cast<LambdaNode*>(node);
            std::vector<TypeInfo> paramTypes;
            for (const auto& p : lambda->parameters)
                paramTypes.push_back(p.type);
            TypeInfo retType = lambda->returnType;
            if (retType.kind == TypeKind::ANY)
                retType = TypeInfo(TypeKind::ANY);
            return TypeInfo::createFunction(paramTypes, std::make_unique<TypeInfo>(retType));
        }

        default:
            return TypeInfo(TypeKind::ANY);
    }
}

TypeInfo TypeChecker::resolveAlias(const TypeInfo& type)
{
    if (type.kind == TypeKind::OBJECT && type.name.has_value())
    {
        for (Scope* s = currentScope; s; s = s->parent)
        {
            auto it = s->typeAliases.find(type.name.value());
            if (it != s->typeAliases.end())
                return it->second;
        }
    }
    return type;
}

bool TypeChecker::isAssignable(TypeInfo target, TypeInfo source)
{
    target = resolveAlias(target);
    source = resolveAlias(source);

    if (target.kind == TypeKind::ANY || source.kind == TypeKind::ANY)
        return true;

    if (target.kind == TypeKind::TYPE_PARAM)
        return true;

    if (target.kind == TypeKind::OPTIONAL)
    {
        if (source.kind == TypeKind::NULL_TYPE)
            return true;
        if (target.optionalType)
            return isAssignable(*target.optionalType, source);
        return true;
    }

    if (source.kind == TypeKind::NULL_TYPE)
        return target.kind == TypeKind::NULL_TYPE || target.kind == TypeKind::OBJECT ||
               target.kind == TypeKind::ANY || target.kind == TypeKind::OPTIONAL;

    if (target.kind == TypeKind::UNION)
    {
        for (const auto& ut : target.unionTypes)
        {
            if (isAssignable(ut, source))
                return true;
        }
        return false;
    }

    if (target.kind == source.kind)
    {
        if (target.kind == TypeKind::ARRAY)
        {
            if (!target.elementType || !source.elementType)
                return true;
            return isAssignable(*target.elementType, *source.elementType);
        }
        if (target.kind == TypeKind::OBJECT)
            return target.name == source.name || !target.name.has_value() || !source.name.has_value();
        return true;
    }

    if (target.kind == TypeKind::FLOAT && source.kind == TypeKind::INT)
        return true;

    if (target.kind == TypeKind::INT && source.kind == TypeKind::FLOAT)
    {
        if (errorHandler)
            errorHandler->warning(ErrorCode::IMPLICIT_CONVERSION,
                "Implicit conversion from float to int may lose precision",
                SourceLocation("", 0, 0));
        return true;
    }

    return false;
}

TypeInfo TypeChecker::resolveTypeParam(const TypeInfo& type)
{
    if (type.kind == TypeKind::TYPE_PARAM && type.name.has_value())
    {
        auto it = currentTypeBindings.find(type.name.value());
        if (it != currentTypeBindings.end())
            return it->second;
    }
    if (type.kind == TypeKind::ARRAY && type.elementType)
    {
        TypeInfo resolved = resolveTypeParam(*type.elementType);
        return TypeInfo::createArray(std::make_unique<TypeInfo>(resolved));
    }
    if (type.kind == TypeKind::OPTIONAL && type.optionalType)
    {
        TypeInfo resolved = resolveTypeParam(*type.optionalType);
        return TypeInfo::createOptional(std::make_unique<TypeInfo>(resolved));
    }
    if (type.kind == TypeKind::FUNCTION)
    {
        std::vector<TypeInfo> resolvedParams;
        for (const auto& p : type.paramTypes)
            resolvedParams.push_back(resolveTypeParam(p));
        TypeInfo resolvedRet = type.returnType ?
            resolveTypeParam(*type.returnType) : TypeInfo(TypeKind::VOID);
        return TypeInfo::createFunction(resolvedParams, std::make_unique<TypeInfo>(resolvedRet));
    }
    return type;
}

TypeInfo TypeChecker::checkExpression(ASTNode* node)
{
    if (!node) return TypeInfo(TypeKind::VOID);

    TypeInfo resultType = inferType(node);

    switch (node->type)
    {
        case NodeType::VARIABLE_ACCESS:
        {
            auto var = static_cast<VariableAccessNode*>(node);
            SymbolInfo* sym = lookupSymbol(var->name);
            if (!sym)
            {
                if (errorHandler)
                    errorHandler->error(ErrorCode::UNDEFINED_VARIABLE,
                        "Undefined variable '" + var->name + "'",
                        SourceLocation("", node->line, node->column));
                return TypeInfo(TypeKind::ANY);
            }
            return sym->type;
        }

        case NodeType::ASSIGNMENT:
        {
            auto assign = static_cast<AssignmentNode*>(node);
            SymbolInfo* sym = lookupSymbol(assign->name);
            if (!sym)
            {
                if (errorHandler)
                    errorHandler->error(ErrorCode::UNDEFINED_VARIABLE,
                        "Undefined variable '" + assign->name + "'",
                        SourceLocation("", node->line, node->column));
                return TypeInfo(TypeKind::ANY);
            }
            if (sym->isConst)
            {
                if (errorHandler)
                    errorHandler->error(ErrorCode::ASSIGN_TO_CONST,
                        "Cannot assign to constant variable '" + assign->name + "'",
                        SourceLocation("", node->line, node->column));
            }
            TypeInfo valueType = checkExpression(assign->value.get());
            if (!isAssignable(sym->type, valueType))
            {
                if (errorHandler)
                    errorHandler->error(ErrorCode::TYPE_MISMATCH,
                        "Cannot assign type '" + valueType.toString() +
                        "' to variable '" + assign->name + "' of type '" +
                        sym->type.toString() + "'",
                        SourceLocation("", node->line, node->column));
            }
            return sym->type;
        }

        case NodeType::BINARY_EXPRESSION:
        {
            auto bin = static_cast<BinaryExpressionNode*>(node);
            TypeInfo leftType = checkExpression(bin->left.get());
            TypeInfo rightType = checkExpression(bin->right.get());
            const std::string& op = bin->op;

            if ((op == "+" || op == "-" || op == "*" || op == "/" || op == "%" || op == "**") &&
                !isNumeric(leftType) && leftType.kind != TypeKind::STRING)
            {
                if (errorHandler)
                    errorHandler->error(ErrorCode::INVALID_BINARY_OPERATION,
                        "Operator '" + op + "' not supported for type '" +
                        leftType.toString() + "'",
                        SourceLocation("", node->line, node->column));
            }
            if ((op == "+" || op == "-" || op == "*" || op == "/" || op == "%" || op == "**") &&
                !isNumeric(rightType) && op != "+")
            {
                if (errorHandler)
                    errorHandler->error(ErrorCode::INVALID_BINARY_OPERATION,
                        "Operator '" + op + "' not supported for type '" +
                        rightType.toString() + "'",
                        SourceLocation("", node->line, node->column));
            }
            return resultType;
        }

        case NodeType::UNARY_EXPRESSION:
        {
            auto una = static_cast<UnaryExpressionNode*>(node);
            TypeInfo operandType = checkExpression(una->operand.get());

            if (una->op == "-" && !isNumeric(operandType))
            {
                if (errorHandler)
                    errorHandler->error(ErrorCode::INVALID_UNARY_OPERATION,
                        "Unary '-' not supported for type '" +
                        operandType.toString() + "'",
                        SourceLocation("", node->line, node->column));
            }
            if (una->op == "!" && operandType.kind != TypeKind::BOOLEAN)
            {
                if (errorHandler)
                    errorHandler->error(ErrorCode::INVALID_UNARY_OPERATION,
                        "Unary '!' not supported for type '" +
                        operandType.toString() + "'",
                        SourceLocation("", node->line, node->column));
            }
            return resultType;
        }

        case NodeType::FUNCTION_CALL:
        {
            auto call = static_cast<FunctionCallNode*>(node);
            FunctionSignature* sig = lookupFunction(call->functionName);

            if (!sig)
            {
                SymbolInfo* sym = lookupSymbol(call->functionName);
                if (!sym)
                {
                    if (errorHandler)
                        errorHandler->error(ErrorCode::UNDEFINED_FUNCTION,
                            "Undefined function '" + call->functionName + "'",
                            SourceLocation("", node->line, node->column));
                    return TypeInfo(TypeKind::ANY);
                }
                if (sym->kind != SymbolInfo::Kind::FUNCTION)
                {
                    if (errorHandler)
                        errorHandler->error(ErrorCode::NOT_CALLABLE,
                            "'" + call->functionName + "' is not callable",
                            SourceLocation("", node->line, node->column));
                    return TypeInfo(TypeKind::ANY);
                }
                return sym->type.returnType ? *sym->type.returnType : TypeInfo(TypeKind::ANY);
            }

            // Handle named arguments
            std::vector<TypeInfo> resolvedParamTypes = sig->paramTypes;
            if (!call->namedArguments.empty())
            {
                for (const auto& namedArg : call->namedArguments)
                {
                    for (size_t i = 0; i < sig->paramNames.size(); i++)
                    {
                        if (sig->paramNames[i] == namedArg.name)
                        {
                            resolvedParamTypes[i] = checkExpression(namedArg.value.get());
                            break;
                        }
                    }
                }
            }

            size_t totalArgs = call->arguments.size() + call->namedArguments.size();
            if (totalArgs != sig->paramTypes.size() && !sig->isGeneric)
            {
                if (errorHandler)
                    errorHandler->error(ErrorCode::WRONG_ARGUMENT_COUNT,
                        "Function '" + call->functionName + "' expects " +
                        std::to_string(sig->paramTypes.size()) + " arguments but " +
                        std::to_string(totalArgs) + " provided",
                        SourceLocation("", node->line, node->column));
                return sig->returnType;
            }

            // Check argument count
            if (sig->paramTypes.size() > 0 && totalArgs > sig->paramTypes.size())
            {
                if (errorHandler)
                    errorHandler->error(ErrorCode::WRONG_ARGUMENT_COUNT,
                        "Too many arguments for function '" + call->functionName + "'",
                        SourceLocation("", node->line, node->column));
                return sig->returnType;
            }

            // Type check arguments with generic binding
            for (size_t i = 0; i < call->arguments.size() && i < sig->paramTypes.size(); i++)
            {
                TypeInfo argType = checkExpression(call->arguments[i].get());
                TypeInfo paramType = sig->paramTypes[i];

                if (sig->isGeneric)
                {
                    if (paramType.kind == TypeKind::TYPE_PARAM || paramType.kind == TypeKind::ANY)
                    {
                        for (const auto& tp : sig->typeParams)
                        {
                            currentTypeBindings[tp] = argType;
                        }
                        paramType = argType;
                    }
                }

                if (paramType.kind != TypeKind::ANY && !isAssignable(paramType, argType))
                {
                    if (errorHandler)
                        errorHandler->error(ErrorCode::WRONG_ARGUMENT_TYPE,
                            "Argument " + std::to_string(i + 1) + " to function '" +
                            call->functionName + "' expects type '" +
                            paramType.toString() + "' but got '" +
                            argType.toString() + "'",
                            SourceLocation("", node->line, node->column));
                }
            }

            TypeInfo callResultType = sig->returnType;
            if (sig->isGeneric)
                callResultType = resolveTypeParam(callResultType);
            return callResultType;
        }

        case NodeType::MEMBER_ACCESS:
        {
            auto member = static_cast<MemberAccessNode*>(node);
            TypeInfo objType = checkExpression(member->object.get());
            return TypeInfo(TypeKind::ANY);
        }

        case NodeType::ARRAY_ACCESS:
        {
            auto arrAccess = static_cast<ArrayAccessNode*>(node);
            TypeInfo arrType = checkExpression(arrAccess->array.get());
            TypeInfo idxType = checkExpression(arrAccess->index.get());

            if (arrType.kind != TypeKind::ARRAY)
            {
                if (errorHandler)
                    errorHandler->error(ErrorCode::INVALID_INDEX_TYPE,
                        "Cannot index non-array type '" +
                        arrType.toString() + "'",
                        SourceLocation("", node->line, node->column));
                return TypeInfo(TypeKind::ANY);
            }

            if (idxType.kind != TypeKind::INT)
            {
                if (errorHandler)
                    errorHandler->error(ErrorCode::INVALID_INDEX_TYPE,
                        "Array index must be integer, got '" +
                        idxType.toString() + "'",
                        SourceLocation("", node->line, node->column));
            }
            return arrType.elementType ? *arrType.elementType : TypeInfo(TypeKind::ANY);
        }

        case NodeType::NEW_EXPRESSION:
        {
            auto newExpr = static_cast<NewExpressionNode*>(node);
            ClassInfo* classInfo = lookupClass(newExpr->className);
            if (!classInfo)
            {
                if (errorHandler)
                    errorHandler->error(ErrorCode::UNDEFINED_VARIABLE,
                        "Class '" + newExpr->className + "' not defined",
                        SourceLocation("", node->line, node->column));
            }
            return resultType;
        }

        default:
            return resultType;
    }
}

void TypeChecker::checkStatement(ASTNode* node)
{
    if (!node) return;

    switch (node->type)
    {
        case NodeType::VARIABLE_DECLARATION:
        {
            auto decl = static_cast<VariableDeclarationNode*>(node);
            TypeInfo declType = resolveTypeAnnotation(decl->declaredType, decl->initializer.get());

            if (decl->initializer)
            {
                TypeInfo initType = checkExpression(decl->initializer.get());
                if (decl->declaredType.has_value() && !isAssignable(declType, initType))
                {
                    if (errorHandler)
                        errorHandler->error(ErrorCode::TYPE_MISMATCH,
                            "Cannot initialize variable '" + decl->name +
                            "' of type '" + declType.toString() +
                            "' with value of type '" + initType.toString() + "'",
                            SourceLocation("", node->line, node->column));
                }
            }

            declareSymbol(decl->name, SymbolInfo(SymbolInfo::Kind::VARIABLE, declType, decl->isConst || !decl->isMutable));
            break;
        }

        case NodeType::ASSIGNMENT:
        {
            checkExpression(node);
            break;
        }

        case NodeType::PRINT:
        {
            auto printNode = static_cast<PrintNode*>(node);
            checkExpression(printNode->expression.get());
            break;
        }

        case NodeType::EXPRESSION_STATEMENT:
        {
            auto exprStmt = static_cast<ExpressionStatementNode*>(node);
            checkExpression(exprStmt->expression.get());
            break;
        }

        case NodeType::BLOCK:
        {
            auto block = static_cast<BlockNode*>(node);
            pushScope();
            checkBlock(block->statements);
            popScope();
            break;
        }

        case NodeType::IF:
        {
            auto ifNode = static_cast<IfNode*>(node);
            TypeInfo condType = checkExpression(ifNode->condition.get());
            if (condType.kind != TypeKind::BOOLEAN && condType.kind != TypeKind::ANY)
            {
                if (errorHandler)
                    errorHandler->warning(ErrorCode::TYPE_MISMATCH,
                        "If condition should be boolean, got '" +
                        condType.toString() + "'",
                        SourceLocation("", node->line, node->column));
            }

            pushScope();
            checkBlock(ifNode->thenBlock);
            popScope();

            if (!ifNode->elseBlock.empty())
            {
                pushScope();
                checkBlock(ifNode->elseBlock);
                popScope();
            }
            break;
        }

        case NodeType::WHILE:
        {
            auto whileNode = static_cast<WhileNode*>(node);
            TypeInfo condType = checkExpression(whileNode->condition.get());
            if (condType.kind != TypeKind::BOOLEAN && condType.kind != TypeKind::ANY)
            {
                if (errorHandler)
                    errorHandler->warning(ErrorCode::TYPE_MISMATCH,
                        "While condition should be boolean, got '" +
                        condType.toString() + "'",
                        SourceLocation("", node->line, node->column));
            }

            loopContext.push(true);
            pushScope();
            checkBlock(whileNode->body);
            popScope();
            loopContext.pop();
            break;
        }

        case NodeType::FOR:
        {
            auto forNode = static_cast<ForNode*>(node);

            loopContext.push(true);
            pushScope();
            declareSymbol(forNode->variable,
                SymbolInfo(SymbolInfo::Kind::VARIABLE, TypeInfo(TypeKind::ANY)));
            checkBlock(forNode->body);
            popScope();
            loopContext.pop();
            break;
        }

        case NodeType::BREAK:
        {
            if (loopContext.empty())
            {
                if (errorHandler)
                    errorHandler->error(ErrorCode::BREAK_OUTSIDE_LOOP,
                        "'break' outside loop",
                        SourceLocation("", node->line, node->column));
            }
            break;
        }

        case NodeType::CONTINUE:
        {
            if (loopContext.empty())
            {
                if (errorHandler)
                    errorHandler->error(ErrorCode::CONTINUE_OUTSIDE_LOOP,
                        "'continue' outside loop",
                        SourceLocation("", node->line, node->column));
            }
            break;
        }

        case NodeType::SWITCH:
        {
            auto switchNode = static_cast<SwitchNode*>(node);
            checkExpression(switchNode->expression.get());

            for (auto& case_ : switchNode->cases)
            {
                checkExpression(case_.value.get());
                pushScope();
                checkBlock(case_.body);
                popScope();
            }

            if (!switchNode->defaultCase.empty())
            {
                pushScope();
                checkBlock(switchNode->defaultCase);
                popScope();
            }
            break;
        }

        case NodeType::FUNCTION_DECLARATION:
        {
            auto funcDecl = static_cast<FunctionDeclarationNode*>(node);
            checkFunctionDeclaration(funcDecl);
            break;
        }

        case NodeType::RETURN:
        {
            if (returnContext.empty())
            {
                if (errorHandler)
                    errorHandler->error(ErrorCode::RETURN_OUTSIDE_FUNCTION,
                        "'return' outside function",
                        SourceLocation("", node->line, node->column));
                break;
            }

            auto retNode = static_cast<ReturnNode*>(node);
            TypeInfo expectedReturn = returnContext.top();

            if (retNode->value)
            {
                TypeInfo returnType = checkExpression(retNode->value.get());
                if (expectedReturn.kind != TypeKind::VOID && !isAssignable(expectedReturn, returnType))
                {
                    if (errorHandler)
                        errorHandler->error(ErrorCode::TYPE_MISMATCH,
                            "Function expects return type '" +
                            expectedReturn.toString() + "' but got '" +
                            returnType.toString() + "'",
                            SourceLocation("", node->line, node->column));
                }
                hasReturnPath = true;
            }
            else if (expectedReturn.kind != TypeKind::VOID)
            {
                if (errorHandler)
                    errorHandler->error(ErrorCode::TYPE_MISMATCH,
                        "Function expects return type '" +
                        expectedReturn.toString() + "' but no value returned",
                        SourceLocation("", node->line, node->column));
            }
            break;
        }

        case NodeType::CLASS_DECLARATION:
        {
            auto classDecl = static_cast<ClassDeclarationNode*>(node);
            checkClassDeclaration(classDecl);
            break;
        }

        case NodeType::IMPORT_STATEMENT:
        case NodeType::FROM_IMPORT:
            break;

        case NodeType::MATCH:
        {
            auto matchNode = static_cast<MatchNode*>(node);
            checkExpression(matchNode->expression.get());
            for (auto& caseNode : matchNode->cases)
            {
                pushScope();
                checkBlock(caseNode.body);
                popScope();
            }
            if (!matchNode->elseBody.empty())
            {
                pushScope();
                checkBlock(matchNode->elseBody);
                popScope();
            }
            break;
        }

        case NodeType::TYPE_ALIAS:
        {
            auto alias = static_cast<TypeAliasNode*>(node);
            currentScope->typeAliases[alias->aliasName] = alias->underlyingType;
            break;
        }

        case NodeType::LAMBDA:
        {
            auto lambda = static_cast<LambdaNode*>(node);
            checkLambda(lambda);
            break;
        }

        default:
            checkExpression(node);
            break;
    }
}

void TypeChecker::checkBlock(const std::vector<std::unique_ptr<ASTNode>>& statements)
{
    for (const auto& stmt : statements)
    {
        if (stmt)
            checkStatement(stmt.get());
    }
}

void TypeChecker::checkClassDeclaration(ClassDeclarationNode* node)
{
    ClassInfo classInfo;
    classInfo.name = node->name;
    classInfo.baseClass = node->baseClass;

    for (const auto& field : node->fields)
    {
        classInfo.fields.push_back(field);
        classInfo.fieldTypes[field] = TypeInfo(TypeKind::ANY);
    }

    for (auto& method : node->methods)
    {
        FunctionSignature sig;
        sig.name = method.function.name;
        for (const auto& p : method.function.parameters)
        {
            sig.paramTypes.push_back(p.type);
            sig.paramNames.push_back(p.name);
        }
        sig.returnType = method.function.returnType;
        sig.isGeneric = !method.function.genericParams.empty();
        sig.typeParams = method.function.genericParams;
        classInfo.methods.push_back(sig);
    }

    declareClass(node->name, classInfo);

    // Check interfaces
    for (const auto& ifaceName : node->implements)
    {
        InterfaceInfo* iface = lookupInterface(ifaceName);
        if (!iface)
        {
            if (errorHandler)
                errorHandler->error(ErrorCode::UNDEFINED_VARIABLE,
                    "Interface '" + ifaceName + "' not found for class '" + node->name + "'",
                    SourceLocation("", node->line, node->column));
            continue;
        }
        for (const auto& ifaceMethod : iface->methods)
        {
            bool found = false;
            for (const auto& clsMethod : classInfo.methods)
            {
                if (clsMethod.name == ifaceMethod.name)
                {
                    found = true;
                    break;
                }
            }
            if (!found && errorHandler)
                errorHandler->error(ErrorCode::TYPE_MISMATCH,
                    "Class '" + node->name + "' does not implement interface method '" +
                    ifaceMethod.name + "' from interface '" + ifaceName + "'",
                    SourceLocation("", node->line, node->column));
        }
    }
}

void TypeChecker::checkLambda(LambdaNode* node)
{
    pushScope();

    for (const auto& p : node->parameters)
    {
        declareSymbol(p.name, SymbolInfo(SymbolInfo::Kind::VARIABLE, p.type));
    }

    TypeInfo retType = node->returnType;
    if (retType.kind == TypeKind::ANY)
        retType = TypeInfo(TypeKind::ANY);

    returnContext.push(retType);
    hasReturnPath = false;

    for (const auto& stmt : node->body)
    {
        if (stmt)
            checkStatement(stmt.get());
    }

    returnContext.pop();
    popScope();
}

void TypeChecker::instantiateGenericFunction(FunctionSignature* sig, const std::vector<TypeInfo>& argTypes, TypeInfo& returnType)
{
    currentTypeBindings.clear();
    for (size_t i = 0; i < sig->typeParams.size() && i < argTypes.size(); i++)
    {
        currentTypeBindings[sig->typeParams[i]] = argTypes[i];
    }
    returnType = resolveTypeParam(sig->returnType);
    if (returnType.kind == TypeKind::TYPE_PARAM)
        returnType = TypeInfo(TypeKind::ANY);
}

TypeInfo TypeChecker::getErrorLocation(ASTNode* node)
{
    SourceLocation loc("", node->line, node->column);
    return TypeInfo(TypeKind::ANY);
}

void TypeChecker::checkFunctionDeclaration(FunctionDeclarationNode* node)
{
    pushScope();

    bool isGeneric = !node->genericParams.empty();
    if (isGeneric)
    {
        for (const auto& tp : node->genericParams)
        {
            declareSymbol(tp, SymbolInfo(SymbolInfo::Kind::TYPE_PARAM, TypeInfo::createTypeParam(tp)));
        }
    }

    std::vector<TypeInfo> paramTypes;
    std::vector<std::string> paramNames;
    for (const auto& p : node->parameters)
    {
        paramTypes.push_back(p.type);
        paramNames.push_back(p.name);
        declareSymbol(p.name, SymbolInfo(SymbolInfo::Kind::VARIABLE, p.type));
    }

    TypeInfo retType = node->returnType.kind == TypeKind::ANY ?
        TypeInfo(TypeKind::VOID) : node->returnType;

    FunctionSignature sig;
    sig.name = node->name;
    sig.paramTypes = paramTypes;
    sig.returnType = retType;
    sig.paramNames = paramNames;
    sig.isGeneric = isGeneric;
    sig.typeParams = node->genericParams;

    if (!node->name.empty())
    {
        SymbolInfo funcInfo(SymbolInfo::Kind::FUNCTION, TypeInfo(TypeKind::FUNCTION));
        funcInfo.type = TypeInfo::createFunction(paramTypes,
            std::make_unique<TypeInfo>(retType));
        declareSymbol(node->name, funcInfo);
        declareFunction(node->name, sig);
    }

    returnContext.push(retType);
    hasReturnPath = false;

    for (const auto& stmt : node->body)
    {
        if (stmt)
            checkStatement(stmt.get());
    }

    if (retType.kind != TypeKind::VOID && !hasReturnPath && !node->body.empty())
    {
        if (errorHandler)
            errorHandler->warning(ErrorCode::UNREACHABLE_CODE,
                "Function '" + node->name + "' may not return a value on all paths",
                SourceLocation("", node->line, node->column));
    }

    returnContext.pop();
    popScope();
}

bool TypeChecker::check(const std::vector<std::unique_ptr<ASTNode>>& ast)
{
    if (errorHandler)
    {
        errorHandler->clear();
    }

    // First pass: collect declarations for forward references
    for (const auto& node : ast)
    {
        if (!node) continue;

        switch (node->type)
        {
            case NodeType::FUNCTION_DECLARATION:
            {
                auto funcDecl = static_cast<FunctionDeclarationNode*>(node.get());
                std::vector<TypeInfo> paramTypes;
                std::vector<std::string> paramNames;
                for (const auto& p : funcDecl->parameters)
                {
                    paramTypes.push_back(p.type);
                    paramNames.push_back(p.name);
                }

                FunctionSignature sig;
                sig.name = funcDecl->name;
                sig.paramTypes = paramTypes;
                sig.returnType = funcDecl->returnType.kind == TypeKind::ANY ?
                    TypeInfo(TypeKind::VOID) : funcDecl->returnType;
                sig.paramNames = paramNames;
                sig.isGeneric = !funcDecl->genericParams.empty();
                sig.typeParams = funcDecl->genericParams;

                if (!funcDecl->name.empty())
                {
                    SymbolInfo funcInfo(SymbolInfo::Kind::FUNCTION, TypeInfo(TypeKind::FUNCTION));
                    funcInfo.type = TypeInfo::createFunction(paramTypes,
                        std::make_unique<TypeInfo>(sig.returnType));
                    currentScope->symbols[funcDecl->name] = funcInfo;
                    currentScope->functions[funcDecl->name] = sig;
                }
                break;
            }

            case NodeType::TYPE_ALIAS:
            {
                auto alias = static_cast<TypeAliasNode*>(node.get());
                currentScope->typeAliases[alias->aliasName] = alias->underlyingType;
                break;
            }

            default:
                break;
        }
    }

    // Second pass: type check everything
    for (const auto& node : ast)
    {
        if (node)
        {
            checkStatement(node.get());
        }
    }

    return !errorHandler || !errorHandler->hasErrors();
}

std::vector<ErrorCode> TypeChecker::getErrors() const
{
    std::vector<ErrorCode> codes;
    if (errorHandler)
    {
        for (const auto& err : errorHandler->getErrors())
            codes.push_back(err.getCode());
    }
    return codes;
}

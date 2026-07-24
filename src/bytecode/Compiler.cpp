#include "Compiler.h"

Compiler::Compiler(ErrorHandler* handler)
    : errorHandler(handler)
    , currentFunction(nullptr)
    , state(nullptr)
{
}

uint32_t Compiler::addConstant(const std::string& val)
{
    auto it = constantMap.find(val);
    if (it != constantMap.end())
        return it->second;
    uint32_t index = static_cast<uint32_t>(constants.size());
    constants.push_back(val);
    constantMap[val] = index;
    return index;
}

uint32_t Compiler::addString(const std::string& str)
{
    auto it = stringPool.find(str);
    if (it != stringPool.end())
        return it->second;
    uint32_t index = static_cast<uint32_t>(constants.size());
    constants.push_back(str);
    stringPool[str] = index;
    return index;
}

uint32_t Compiler::resolveGlobal(const std::string& name)
{
    auto it = globalMap.find(name);
    if (it != globalMap.end())
        return it->second;
    uint32_t index = static_cast<uint32_t>(state->func->globalNames.size());
    state->func->globalNames.push_back(name);
    globalMap[name] = index;
    return index;
}

void Compiler::emitByte(OpCode op)
{
    state->func->code.push_back({op, 0u});
}

void Compiler::emitByte(OpCode op, uint32_t operand)
{
    state->func->code.push_back({op, operand});
}

void Compiler::emitJump(OpCode op, int& offset)
{
    emitByte(op, 0xFFFFFFFF);
    offset = static_cast<int>(state->func->code.size()) - 1;
}

void Compiler::patchJump(int offset)
{
    int jumpTarget = static_cast<int>(state->func->code.size());
    state->func->code[offset].operand = static_cast<uint32_t>(jumpTarget);
}

int Compiler::emitLoop(int loopStart)
{
    emitByte(OpCode::LOOP, 0);
    int offset = static_cast<int>(state->func->code.size()) - 1;
    state->func->code[offset].operand = static_cast<uint32_t>(loopStart);
    return offset;
}

int Compiler::resolveLocal(const std::string& name)
{
    for (int i = static_cast<int>(state->locals.size()) - 1; i >= 0; i--)
    {
        if (state->locals[i].name == name && state->locals[i].depth >= 0)
            return i;
    }
    return -1;
}

int Compiler::resolveUpvalue(const std::string& name)
{
    if (stateStack.size() <= 1) return -1;

    // Look in enclosing function
    auto& prevState = stateStack[stateStack.size() - 2];
    for (int i = static_cast<int>(prevState.locals.size()) - 1; i >= 0; i--)
    {
        if (prevState.locals[i].name == name && prevState.locals[i].depth >= 0)
        {
            prevState.locals[i].isUpvalue = true;
            return addUpvalue(name, true);
        }
    }

    // Look further up
    if (!prevState.upvalues.empty())
    {
        for (size_t i = 0; i < prevState.upvalues.size(); i++)
        {
            (void)i;
        }
    }

    return -1;
}

int Compiler::addUpvalue(const std::string& name, bool isLocal)
{
    for (size_t i = 0; i < state->upvalues.size(); i++)
    {
        auto& uv = state->upvalues[i];
        if (uv.index == static_cast<uint8_t>(isLocal ? resolveLocal(name) : 0) && uv.isLocal == isLocal)
            return static_cast<int>(i);
    }

    UpvalueInfo uv;
    uv.index = isLocal ? static_cast<uint8_t>(resolveLocal(name)) : 0;
    uv.isLocal = isLocal;
    state->upvalues.push_back(uv);
    state->func->upvalues.push_back(uv);
    return static_cast<int>(state->upvalues.size()) - 1;
}

int Compiler::addLocal(const std::string& name)
{
    Local local;
    local.name = name;
    local.depth = state->scopeDepth;
    local.isUpvalue = false;
    state->locals.push_back(local);
    return static_cast<int>(state->locals.size()) - 1;
}

void Compiler::markLocalAsUpvalue(int index)
{
    if (index >= 0 && index < static_cast<int>(state->locals.size()))
        state->locals[index].isUpvalue = true;
}

void Compiler::beginScope()
{
    state->scopeDepth++;
}

void Compiler::endScope()
{
    state->scopeDepth--;
    while (!state->locals.empty() && state->locals.back().depth > state->scopeDepth)
    {
        if (state->locals.back().isUpvalue)
            emitByte(OpCode::CLOSE_UPVALUE);
        else
            emitByte(OpCode::POP);
        state->locals.pop_back();
    }
}

int Compiler::currentDepth() const
{
    return state->scopeDepth;
}

FunctionProto* Compiler::createFunction(const std::string& name, uint8_t arity)
{
    auto func = std::make_unique<FunctionProto>(name, arity);
    FunctionProto* ptr = func.get();
    functions.push_back(std::move(func));
    return ptr;
}

void Compiler::pushFunction(FunctionProto* func)
{
    CompilerState newState;
    newState.func = func;
    newState.scopeDepth = 0;

    // Reserve slot for function itself
    if (!func->name.empty())
    {
        Local local;
        local.name = func->name;
        local.depth = 0;
        local.isUpvalue = false;
        newState.locals.push_back(local);
    }

    if (state)
        stateStack.push_back(std::move(*state));
    stateStack.push_back(std::move(newState));
    state = &stateStack.back();
}

void Compiler::popFunction()
{
    stateStack.pop_back();
    if (!stateStack.empty())
        state = &stateStack.back();
    else
        state = nullptr;
}

// ============================================================
// EXPRESSION COMPILATION
// ============================================================

void Compiler::compileExpression(ASTNode* node)
{
    if (!node) return;

    switch (node->type)
    {
        case NodeType::NUMBER_LITERAL:
        case NodeType::FLOAT_LITERAL:
        case NodeType::STRING_LITERAL:
        case NodeType::BOOLEAN_LITERAL:
        case NodeType::CHAR_LITERAL:
        case NodeType::NULL_LITERAL:
            compileLiteral(node);
            break;

        case NodeType::VARIABLE_ACCESS:
            compileVariableAccess(static_cast<VariableAccessNode*>(node));
            break;

        case NodeType::ASSIGNMENT:
            compileAssignment(static_cast<AssignmentNode*>(node));
            break;

        case NodeType::BINARY_EXPRESSION:
            compileBinary(static_cast<BinaryExpressionNode*>(node));
            break;

        case NodeType::UNARY_EXPRESSION:
            compileUnary(static_cast<UnaryExpressionNode*>(node));
            break;

        case NodeType::FUNCTION_CALL:
            compileFunctionCall(static_cast<FunctionCallNode*>(node));
            break;

        case NodeType::ARRAY_LITERAL:
            compileArrayLiteral(static_cast<ArrayLiteralNode*>(node));
            break;

        case NodeType::ARRAY_ACCESS:
            compileArrayAccess(static_cast<ArrayAccessNode*>(node));
            break;

        case NodeType::MEMBER_ACCESS:
            compileMemberAccess(static_cast<MemberAccessNode*>(node));
            break;

        case NodeType::NEW_EXPRESSION:
            compileNewExpression(static_cast<NewExpressionNode*>(node));
            break;

        case NodeType::OBJECT_LITERAL:
            compileObjectLiteral(static_cast<ObjectLiteralNode*>(node));
            break;

        case NodeType::FUNCTION_DECLARATION:
            compileFunction(static_cast<FunctionDeclarationNode*>(node));
            break;

        case NodeType::LAMBDA:
            emitByte(OpCode::LOAD_NULL);
            break;

        default:
            break;
    }
}

void Compiler::compileLiteral(ASTNode* node)
{
    switch (node->type)
    {
        case NodeType::NUMBER_LITERAL:
        {
            auto num = static_cast<NumberNode*>(node);
            uint32_t idx = addConstant(std::to_string(num->value));
            emitByte(OpCode::LOAD_CONST, idx);
            break;
        }
        case NodeType::FLOAT_LITERAL:
        {
            auto flt = static_cast<FloatNode*>(node);
            uint32_t idx = addConstant(std::to_string(flt->value));
            emitByte(OpCode::LOAD_CONST, idx);
            break;
        }
        case NodeType::STRING_LITERAL:
        {
            auto str = static_cast<StringNode*>(node);
            uint32_t idx = addString(str->value);
            emitByte(OpCode::LOAD_CONST, idx);
            break;
        }
        case NodeType::BOOLEAN_LITERAL:
        {
            auto bol = static_cast<BooleanNode*>(node);
            emitByte(bol->value ? OpCode::LOAD_TRUE : OpCode::LOAD_FALSE);
            break;
        }
        case NodeType::CHAR_LITERAL:
        {
            auto ch = static_cast<CharNode*>(node);
            uint32_t idx = addConstant(std::string(1, ch->value));
            emitByte(OpCode::LOAD_CONST, idx);
            break;
        }
        case NodeType::NULL_LITERAL:
            emitByte(OpCode::LOAD_NULL);
            break;
        default:
            break;
    }
}

void Compiler::compileVariableAccess(VariableAccessNode* node)
{
    int local = resolveLocal(node->name);
    if (local >= 0)
    {
        emitByte(OpCode::LOAD_VAR, static_cast<uint32_t>(local));
        return;
    }

    int upvalue = resolveUpvalue(node->name);
    if (upvalue >= 0)
    {
        emitByte(OpCode::LOAD_UPVALUE, static_cast<uint32_t>(upvalue));
        return;
    }

    uint32_t global = resolveGlobal(node->name);
    emitByte(OpCode::LOAD_GLOBAL, global);
}

void Compiler::compileAssignment(AssignmentNode* node)
{
    const std::string& op = node->op;
    bool isCompound = (op != "=");

    if (isCompound)
    {
        // Load current value of the variable
        int local = resolveLocal(node->name);
        if (local >= 0)
            emitByte(OpCode::LOAD_VAR, static_cast<uint32_t>(local));
        else
        {
            uint32_t global = resolveGlobal(node->name);
            emitByte(OpCode::LOAD_GLOBAL, global);
        }

        // Compile the right-hand side value
        compileExpression(node->value.get());

        // Apply the operation
        std::string arithOp = op.substr(0, op.size() - 1);
        if (arithOp == "+") emitByte(OpCode::ADD);
        else if (arithOp == "-") emitByte(OpCode::SUB);
        else if (arithOp == "*") emitByte(OpCode::MUL);
        else if (arithOp == "/") emitByte(OpCode::DIV);
        else if (arithOp == "%") emitByte(OpCode::MOD);
        else if (arithOp == "**") emitByte(OpCode::POW);
    }
    else
    {
        compileExpression(node->value.get());
    }

    int local = resolveLocal(node->name);
    if (local >= 0)
    {
        emitByte(OpCode::STORE_VAR, static_cast<uint32_t>(local));
        return;
    }

    uint32_t global = resolveGlobal(node->name);
    emitByte(OpCode::STORE_GLOBAL, global);
}

void Compiler::compileBinary(BinaryExpressionNode* node)
{
    const std::string& op = node->op;

    // Short-circuit operators
    if (op == "&&")
    {
        compileExpression(node->left.get());
        int endJump = 0;
        emitJump(OpCode::JUMP_IF_FALSE, endJump);
        emitByte(OpCode::POP);
        compileExpression(node->right.get());
        patchJump(endJump);
        return;
    }

    if (op == "||")
    {
        compileExpression(node->left.get());
        int endJump = 0;
        emitJump(OpCode::JUMP_IF_TRUE, endJump);
        emitByte(OpCode::POP);
        compileExpression(node->right.get());
        patchJump(endJump);
        return;
    }

    compileExpression(node->left.get());
    compileExpression(node->right.get());

    if (op == "+")
    {
        emitByte(OpCode::ADD);
    }
    else if (op == "-")     emitByte(OpCode::SUB);
    else if (op == "*")     emitByte(OpCode::MUL);
    else if (op == "/")     emitByte(OpCode::DIV);
    else if (op == "%")     emitByte(OpCode::MOD);
    else if (op == "**")    emitByte(OpCode::POW);
    else if (op == "==")    emitByte(OpCode::EQ);
    else if (op == "!=")    emitByte(OpCode::NE);
    else if (op == ">")     emitByte(OpCode::GT);
    else if (op == "<")     emitByte(OpCode::LT);
    else if (op == ">=")    emitByte(OpCode::GE);
    else if (op == "<=")    emitByte(OpCode::LE);
    else if (op == "<=>")   emitByte(OpCode::CMP);
    else if (op == "..")
    {
        emitByte(OpCode::SUB);
        emitByte(OpCode::ADD);
    }
}

void Compiler::compileUnary(UnaryExpressionNode* node)
{
    compileExpression(node->operand.get());

    if (node->op == "-")
        emitByte(OpCode::NEGATE);
    else if (node->op == "!")
        emitByte(OpCode::NOT);
}

void Compiler::compileFunctionCall(FunctionCallNode* node)
{
    bool isBuiltin = false;
    BuiltinIndex builtinIdx = BuiltinIndex::MAX_BUILTINS;

    // Check for built-in
    if (node->functionName == "print") { isBuiltin = true; builtinIdx = BuiltinIndex::PRINT; }
    else if (node->functionName == "println") { isBuiltin = true; builtinIdx = BuiltinIndex::PRINTLN; }
    else if (node->functionName == "input") { isBuiltin = true; builtinIdx = BuiltinIndex::INPUT; }
    else if (node->functionName == "type") { isBuiltin = true; builtinIdx = BuiltinIndex::TYPE; }
    else if (node->functionName == "len") { isBuiltin = true; builtinIdx = BuiltinIndex::LEN; }
    else if (node->functionName == "toInt") { isBuiltin = true; builtinIdx = BuiltinIndex::TO_INT; }
    else if (node->functionName == "toFloat") { isBuiltin = true; builtinIdx = BuiltinIndex::TO_FLOAT; }
    else if (node->functionName == "toString") { isBuiltin = true; builtinIdx = BuiltinIndex::TO_STRING; }
    else if (node->functionName == "range") { isBuiltin = true; builtinIdx = BuiltinIndex::RANGE; }

    // Compile arguments
    for (auto& arg : node->arguments)
        compileExpression(arg.get());

    if (isBuiltin)
    {
        uint32_t encoded = (static_cast<uint32_t>(builtinIdx) << 16) | static_cast<uint32_t>(node->arguments.size());
        emitByte(OpCode::BUILTIN_CALL, encoded);
    }
    else
    {
        // Load function
        int local = resolveLocal(node->functionName);
        if (local >= 0)
            emitByte(OpCode::LOAD_VAR, static_cast<uint32_t>(local));
        else
        {
            uint32_t global = resolveGlobal(node->functionName);
            emitByte(OpCode::LOAD_GLOBAL, global);
        }

        emitByte(OpCode::CALL, static_cast<uint32_t>(node->arguments.size()));
    }
}

void Compiler::compileArrayLiteral(ArrayLiteralNode* node)
{
    for (auto& elem : node->elements)
        compileExpression(elem.get());
    emitByte(OpCode::MAKE_ARRAY, static_cast<uint32_t>(node->elements.size()));
}

void Compiler::compileArrayAccess(ArrayAccessNode* node)
{
    compileExpression(node->array.get());
    compileExpression(node->index.get());
    emitByte(OpCode::LOAD_INDEX);
}

void Compiler::compileMemberAccess(MemberAccessNode* node)
{
    compileExpression(node->object.get());
    uint32_t fieldIdx = addString(node->member);
    emitByte(OpCode::LOAD_FIELD, fieldIdx);
}

void Compiler::compileNewExpression(NewExpressionNode* node)
{
    emitByte(OpCode::MAKE_OBJECT);
    uint32_t classIdx = addString(node->className);
    emitByte(OpCode::LOAD_CONST, classIdx);

    for (auto& arg : node->arguments)
        compileExpression(arg.get());

    emitByte(OpCode::CALL, static_cast<uint32_t>(node->arguments.size()));
}

void Compiler::compileObjectLiteral(ObjectLiteralNode* node)
{
    emitByte(OpCode::MAKE_OBJECT);
    for (auto& [key, valNode] : node->properties)
    {
        emitByte(OpCode::DUP);
        uint32_t fieldIdx = addString(key);
        emitByte(OpCode::LOAD_CONST, fieldIdx);
        compileExpression(valNode.get());
        emitByte(OpCode::STORE_FIELD);
    }
}

// ============================================================
// STATEMENT COMPILATION
// ============================================================

void Compiler::compileStatement(ASTNode* node)
{
    if (!node) return;

    switch (node->type)
    {
        case NodeType::VARIABLE_DECLARATION:
            compileVariableDeclaration(static_cast<VariableDeclarationNode*>(node));
            break;

        case NodeType::ASSIGNMENT:
            compileAssignment(static_cast<AssignmentNode*>(node));
            break;

        case NodeType::PRINT:
            compilePrint(static_cast<PrintNode*>(node));
            break;

        case NodeType::EXPRESSION_STATEMENT:
            compileExpression(static_cast<ExpressionStatementNode*>(node)->expression.get());
            emitByte(OpCode::POP);
            break;

        case NodeType::BLOCK:
            compileBlock(static_cast<BlockNode*>(node)->statements);
            break;

        case NodeType::IF:
            compileIf(static_cast<IfNode*>(node));
            break;

        case NodeType::WHILE:
            compileWhile(static_cast<WhileNode*>(node));
            break;

        case NodeType::FOR:
            compileFor(static_cast<ForNode*>(node));
            break;

        case NodeType::SWITCH:
            compileSwitch(static_cast<SwitchNode*>(node));
            break;

        case NodeType::BREAK:
        case NodeType::CONTINUE:
            break;

        case NodeType::RETURN:
            compileReturn(static_cast<ReturnNode*>(node));
            break;

        case NodeType::FUNCTION_DECLARATION:
            compileFunction(static_cast<FunctionDeclarationNode*>(node));
            break;

        case NodeType::CLASS_DECLARATION:
            compileClass(static_cast<ClassDeclarationNode*>(node));
            break;

        case NodeType::IMPORT_STATEMENT:
            compileImport(static_cast<ImportNode*>(node));
            break;

        case NodeType::FROM_IMPORT:
            break;

        case NodeType::MATCH:
        {
            auto matchNode = static_cast<MatchNode*>(node);
            compileExpression(matchNode->expression.get());
            for (auto& caseNode : matchNode->cases)
            {
                if (caseNode.pattern.literal)
                    compileExpression(caseNode.pattern.literal.get());
                compileBlock(caseNode.body);
            }
            if (!matchNode->elseBody.empty())
                compileBlock(matchNode->elseBody);
            break;
        }

        case NodeType::TYPE_ALIAS:
            break;

        case NodeType::LAMBDA:
        {
            emitByte(OpCode::LOAD_NULL);
            break;
        }

        default:
            compileExpression(node);
            emitByte(OpCode::POP);
            break;
    }
}

void Compiler::compileBlock(const std::vector<std::unique_ptr<ASTNode>>& statements)
{
    beginScope();
    for (const auto& stmt : statements)
    {
        if (stmt)
            compileStatement(stmt.get());
    }
    endScope();
}

void Compiler::compileVariableDeclaration(VariableDeclarationNode* node)
{
    if (node->initializer)
        compileExpression(node->initializer.get());
    else
        emitByte(OpCode::LOAD_NULL);

    int local = addLocal(node->name);
    emitByte(OpCode::STORE_VAR, static_cast<uint32_t>(local));
}

void Compiler::compilePrint(PrintNode* node)
{
    compileExpression(node->expression.get());
    emitByte(OpCode::PRINTLN);
}

void Compiler::compileIf(IfNode* node)
{
    compileExpression(node->condition.get());

    int elseJump = 0;
    emitJump(OpCode::JUMP_IF_FALSE, elseJump);
    emitByte(OpCode::POP);

    compileBlock(node->thenBlock);

    int endJump = 0;
    if (!node->elseBlock.empty())
    {
        emitJump(OpCode::JUMP, endJump);
    }

    patchJump(elseJump);
    emitByte(OpCode::POP);

    if (!node->elseBlock.empty())
    {
        compileBlock(node->elseBlock);
        patchJump(endJump);
    }
}

void Compiler::compileWhile(WhileNode* node)
{
    int loopStart = static_cast<int>(state->func->code.size());
    state->breakTargets.push_back(-1);
    state->continueTargets.push_back(loopStart);

    compileExpression(node->condition.get());

    int exitJump = 0;
    emitJump(OpCode::JUMP_IF_FALSE, exitJump);
    emitByte(OpCode::POP);

    compileBlock(node->body);

    emitLoop(loopStart);

    patchJump(exitJump);
    emitByte(OpCode::POP);

    if (state->breakTargets.back() >= 0)
        patchJump(state->breakTargets.back());

    state->breakTargets.pop_back();
    state->continueTargets.pop_back();
}

void Compiler::compileFor(ForNode* node)
{
    beginScope();

    // Reserve local for loop variable
    int varIdx = addLocal(node->variable);

    // Compile iterable array and keep on stack
    compileExpression(node->iterable.get());

    // Reserve local for index (current position)
    int idxLocal = addLocal("__idx__");
    emitByte(OpCode::LOAD_CONST, addConstant("0"));
    emitByte(OpCode::STORE_VAR, static_cast<uint32_t>(idxLocal));

    int loopStart = static_cast<int>(state->func->code.size());
    state->breakTargets.push_back(-1);
    state->continueTargets.push_back(loopStart);

    // Load array (duplicate it from stack)
    emitByte(OpCode::DUP);
    emitByte(OpCode::ARRAY_LEN);
    // Load current index
    emitByte(OpCode::LOAD_VAR, static_cast<uint32_t>(idxLocal));
    // Compare: index < length?
    emitByte(OpCode::GT);
    // If index >= length, exit loop
    int exitJump = 0;
    emitJump(OpCode::JUMP_IF_TRUE, exitJump);
    emitByte(OpCode::POP);

    // Load array[index]
    emitByte(OpCode::DUP);
    emitByte(OpCode::LOAD_VAR, static_cast<uint32_t>(idxLocal));
    emitByte(OpCode::LOAD_INDEX);

    // Store in loop variable
    emitByte(OpCode::STORE_VAR, static_cast<uint32_t>(varIdx));

    // Compile body
    compileBlock(node->body);

    // Increment index
    emitByte(OpCode::LOAD_VAR, static_cast<uint32_t>(idxLocal));
    emitByte(OpCode::LOAD_CONST, addConstant("1"));
    emitByte(OpCode::ADD);
    emitByte(OpCode::STORE_VAR, static_cast<uint32_t>(idxLocal));

    emitLoop(loopStart);

    patchJump(exitJump);
    emitByte(OpCode::POP); // pop array

    state->breakTargets.pop_back();
    state->continueTargets.pop_back();

    endScope();
}

void Compiler::compileSwitch(SwitchNode* node)
{
    compileExpression(node->expression.get());

    std::vector<int> caseJumps;

    for (auto& case_ : node->cases)
    {
        emitByte(OpCode::DUP);
        compileExpression(case_.value.get());
        emitByte(OpCode::EQ);

        int caseJump = 0;
        emitJump(OpCode::JUMP_IF_FALSE, caseJump);
        emitByte(OpCode::POP);
        emitByte(OpCode::POP);

        compileBlock(case_.body);

        int endJump = 0;
        emitJump(OpCode::JUMP, endJump);
        caseJumps.push_back(endJump);
        patchJump(caseJump);
        emitByte(OpCode::POP);
    }

    if (!node->defaultCase.empty())
    {
        emitByte(OpCode::POP);
        compileBlock(node->defaultCase);
    }
    else
    {
        emitByte(OpCode::POP);
    }

    for (int j : caseJumps)
        patchJump(j);
}

void Compiler::compileReturn(ReturnNode* node)
{
    if (node->value)
        compileExpression(node->value.get());
    else
        emitByte(OpCode::LOAD_NULL);
    emitByte(OpCode::RETURN);
}

void Compiler::compileFunction(FunctionDeclarationNode* node)
{
    auto func = createFunction(node->name, static_cast<uint8_t>(node->parameters.size()));

    // Copy parameters info
    for (const auto& p : node->parameters)
    {
        func->localNames.push_back(p.name);
    }

    pushFunction(func);

    // Add parameters as locals
    for (const auto& p : node->parameters)
        addLocal(p.name);

    // Compile body
    compileBlock(node->body);

    // Implicit return
    emitByte(OpCode::LOAD_NULL);
    emitByte(OpCode::RETURN);

    // Save upvalues before popping
    auto upvaluesCopy = state->upvalues;

    popFunction();

    // Emit make function
    uint32_t funcIdx = static_cast<uint32_t>(functions.size() - 1);

    if (!upvaluesCopy.empty())
    {
        emitByte(OpCode::MAKE_CLOSURE, funcIdx);
        for (auto& uv : upvaluesCopy)
        {
            (void)uv;
            emitByte(OpCode::CLOSE_UPVALUE);
        }
    }
    else
    {
        emitByte(OpCode::MAKE_FUNCTION, funcIdx);
    }

    if (!node->name.empty())
    {
        int local = addLocal(node->name);
        emitByte(OpCode::STORE_VAR, static_cast<uint32_t>(local));
    }
}

void Compiler::compileClass(ClassDeclarationNode* node)
{
    emitByte(OpCode::MAKE_CLASS);
    uint32_t nameIdx = addString(node->name);
    emitByte(OpCode::LOAD_CONST, nameIdx);

    // Store class
    int local = addLocal(node->name);
    emitByte(OpCode::STORE_VAR, static_cast<uint32_t>(local));
}

void Compiler::compileImport(ImportNode* node)
{
    uint32_t nameIdx = addString(node->moduleName);
    emitByte(OpCode::LOAD_CONST, nameIdx);
    emitByte(OpCode::LOAD_MODULE);
}

// ============================================================
// MAIN COMPILATION ENTRY
// ============================================================

std::unique_ptr<FunctionProto> Compiler::compile(const std::vector<std::unique_ptr<ASTNode>>& ast)
{
    auto mainFunc = createFunction("__main__", 0);
    pushFunction(mainFunc);

    for (const auto& node : ast)
    {
        if (node)
            compileStatement(node.get());
    }

    emitByte(OpCode::LOAD_NULL);
    emitByte(OpCode::RETURN);

    popFunction();

    return std::unique_ptr<FunctionProto>(functions[0].release());
}

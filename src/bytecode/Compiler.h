#pragma once

#include "Bytecode.h"
#include "../AST.h"
#include "../errors/Error.h"
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <stack>

class Compiler
{
private:
    ErrorHandler* errorHandler;

    // Current function being compiled
    FunctionProto* currentFunction;
    std::vector<std::unique_ptr<FunctionProto>> functions;

    // Scope tracking for locals
    struct Local
    {
        std::string name;
        int depth;
        bool isUpvalue;
    };

    struct CompilerState
    {
        FunctionProto* func;
        std::vector<Local> locals;
        int scopeDepth;
        std::vector<int> breakTargets;
        std::vector<int> continueTargets;
        std::vector<UpvalueInfo> upvalues;
    };

    std::vector<CompilerState> stateStack;
    CompilerState* state;

    // String table for constants
    std::unordered_map<std::string, uint32_t> constantMap;
    std::vector<std::string> constants;

    // Global names
    std::unordered_map<std::string, uint32_t> globalMap;

    // String constants pool
    std::unordered_map<std::string, uint32_t> stringPool;

    // Helper methods
    uint32_t addConstant(const std::string& val);
    uint32_t addString(const std::string& str);
    uint32_t resolveGlobal(const std::string& name);

    void emitByte(OpCode op);
    void emitByte(OpCode op, uint32_t operand);
    void emitJump(OpCode op, int& offset);
    void patchJump(int offset);
    int emitLoop(int loopStart);

    int resolveLocal(const std::string& name);
    int resolveUpvalue(const std::string& name);
    int addUpvalue(const std::string& name, bool isLocal);
    int addLocal(const std::string& name);
    void markLocalAsUpvalue(int index);

    void beginScope();
    void endScope();
    int currentDepth() const;

    // Expression compilation
    void compileExpression(ASTNode* node);
    void compileLiteral(ASTNode* node);
    void compileVariableAccess(VariableAccessNode* node);
    void compileAssignment(AssignmentNode* node);
    void compileBinary(BinaryExpressionNode* node);
    void compileUnary(UnaryExpressionNode* node);
    void compileFunctionCall(FunctionCallNode* node);
    void compileArrayLiteral(ArrayLiteralNode* node);
    void compileArrayAccess(ArrayAccessNode* node);
    void compileMemberAccess(MemberAccessNode* node);
    void compileNewExpression(NewExpressionNode* node);
    void compileObjectLiteral(ObjectLiteralNode* node);

    // Statement compilation
    void compileStatement(ASTNode* node);
    void compileBlock(const std::vector<std::unique_ptr<ASTNode>>& statements);
    void compileVariableDeclaration(VariableDeclarationNode* node);
    void compilePrint(PrintNode* node);
    void compileIf(IfNode* node);
    void compileWhile(WhileNode* node);
    void compileFor(ForNode* node);
    void compileSwitch(SwitchNode* node);
    void compileReturn(ReturnNode* node);
    void compileFunction(FunctionDeclarationNode* node);
    void compileClass(ClassDeclarationNode* node);
    void compileImport(ImportNode* node);

    // Function compilation
    FunctionProto* createFunction(const std::string& name, uint8_t arity);
    void pushFunction(FunctionProto* func);
    void popFunction();

public:
    Compiler(ErrorHandler* handler = nullptr);

    std::unique_ptr<FunctionProto> compile(const std::vector<std::unique_ptr<ASTNode>>& ast);

    void setErrorHandler(ErrorHandler* handler) { errorHandler = handler; }
};

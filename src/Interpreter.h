#pragma once

#include "AST.h"
#include "Lexer.h"
#include "runtime/Environment.h"
#include "errors/Error.h"

#include <memory>
#include <vector>
#include <unordered_map>
#include <stack>
#include <functional>

class Interpreter
{
private:
    std::shared_ptr<Environment> globals;
    std::shared_ptr<Environment> environment;
    ErrorHandler* errorHandler;

    // Loop control
    enum class LoopControl { NONE, BREAK, CONTINUE };
    struct ReturnSignal {
        std::unique_ptr<Value> value;
        bool hasValue;
    };
    std::stack<LoopControl> loopStack;

    // Pattern matching
    bool matchPattern(const MatchPattern& pattern, Value* value, std::shared_ptr<Environment> env);
    bool matchLiteral(const MatchPattern& pattern, Value* value);

    // Module cache
    std::unordered_map<std::string, std::shared_ptr<Environment>> moduleCache;

    // Helper methods
    std::unique_ptr<Value> evaluate(ASTNode* node);
    void executeStatement(ASTNode* node);
    void executeBlock(const std::vector<std::unique_ptr<ASTNode>>& statements,
                      std::shared_ptr<Environment> blockEnv);

    // Expression evaluation helpers
    std::unique_ptr<Value> evaluateBinary(BinaryExpressionNode* node);
    std::unique_ptr<Value> evaluateUnary(UnaryExpressionNode* node);
    std::unique_ptr<Value> evaluateFunctionCall(FunctionCallNode* node);

    // Built-in functions
    void registerBuiltins();
    static std::unique_ptr<Value> builtinPrint(const std::vector<std::unique_ptr<Value>>& args, Environment* env);
    static std::unique_ptr<Value> builtinPrintln(const std::vector<std::unique_ptr<Value>>& args, Environment* env);
    static std::unique_ptr<Value> builtinInput(const std::vector<std::unique_ptr<Value>>& args, Environment* env);
    static std::unique_ptr<Value> builtinType(const std::vector<std::unique_ptr<Value>>& args, Environment* env);
    static std::unique_ptr<Value> builtinLen(const std::vector<std::unique_ptr<Value>>& args, Environment* env);
    static std::unique_ptr<Value> builtinToInt(const std::vector<std::unique_ptr<Value>>& args, Environment* env);
    static std::unique_ptr<Value> builtinToFloat(const std::vector<std::unique_ptr<Value>>& args, Environment* env);
    static std::unique_ptr<Value> builtinToString(const std::vector<std::unique_ptr<Value>>& args, Environment* env);
    static std::unique_ptr<Value> builtinRange(const std::vector<std::unique_ptr<Value>>& args, Environment* env);

    // Type conversion helpers
    Value* getValue(const std::string& name);
    bool checkNumeric(Value* val);

    // Import system
    void loadModule(const std::string& moduleName);
    void executeFile(const std::string& filename, std::shared_ptr<Environment> targetEnv);

public:
    Interpreter(ErrorHandler* errorHandler = nullptr);
    ~Interpreter() = default;

    void execute(ASTNode* node);
    void executeProgram(const std::vector<std::unique_ptr<ASTNode>>& nodes);

    std::shared_ptr<Environment> getGlobals() const { return globals; }
};

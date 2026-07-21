#pragma once

#include "Bytecode.h"
#include "../runtime/Environment.h"
#include "../runtime/values/Value.h"
#include "../errors/Error.h"

#include <memory>
#include <vector>
#include <unordered_map>
#include <stack>

enum class InterpretResult
{
    OK,
    COMPILE_ERROR,
    RUNTIME_ERROR
};

struct CallFrame
{
    FunctionProto* function;
    const Instruction* ip;
    std::shared_ptr<Environment> closure;
    std::vector<Value*> stackBase;
    int returnSlot;
};

class VM
{
private:
    // VM stack
    std::vector<std::unique_ptr<Value>> stack;
    std::vector<UpvalueInfo> openUpvalues;

    // Call frames
    std::vector<CallFrame> frames;
    int frameCount;

    // Globals
    std::shared_ptr<Environment> globals;

    // Error handler
    ErrorHandler* errorHandler;

    // Function cache
    std::vector<std::unique_ptr<FunctionProto>> functions;

    // Module cache
    std::unordered_map<std::string, std::shared_ptr<Environment>> moduleCache;
    std::vector<std::unique_ptr<Value>> moduleValues;

    // Built-in function implementations
    using BuiltinFunc = std::unique_ptr<Value>(*)(const std::vector<std::unique_ptr<Value>>&, Environment*);
    std::vector<BuiltinFunc> builtins;

    // Stack operations
    void push(std::unique_ptr<Value> value);
    std::unique_ptr<Value> pop();
    Value* peek(int distance = 0);
    void resetStack();

    // Call operations
    bool callFunction(FunctionProto* func, int argCount,
                      std::shared_ptr<Environment> closure);
    bool callBuiltin(BuiltinIndex index, int argCount);
    bool callValue(Value* callee, int argCount);

    // Upvalue management
    void captureUpvalues();
    UpvalueInfo* getUpvalue(int index);

    // Runtime helpers
    std::unique_ptr<Value> stringConcat(const std::string& a, const std::string& b);

    // Error reporting
    void runtimeError(const std::string& message);
    void runtimeError(ErrorCode code, const std::string& message);

public:
    VM(ErrorHandler* handler = nullptr);
    ~VM() = default;

    void setErrorHandler(ErrorHandler* handler) { errorHandler = handler; }

    InterpretResult interpret(FunctionProto* mainFunc);

    // Access to globals
    std::shared_ptr<Environment> getGlobals() const { return globals; }

    // Module loading
    void loadModule(const std::string& name, std::shared_ptr<Environment> targetEnv);

    // Debug
    void printStack();
};

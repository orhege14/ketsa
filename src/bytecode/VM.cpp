#include "VM.h"
#include "../jit/JIT.h"

#include "../runtime/values/NumberValue.h"
#include "../runtime/values/FloatValue.h"
#include "../runtime/values/StringValue.h"
#include "../runtime/values/BooleanValue.h"
#include "../runtime/values/NullValue.h"
#include "../runtime/values/ArrayValue.h"
#include "../runtime/values/ObjectValue.h"
#include "../runtime/values/FunctionValue.h"
#include "../runtime/values/BuiltinFunctionValue.h"
#include "../runtime/values/ClassValue.h"

#include <iostream>
#include <sstream>
#include <cmath>
#include <algorithm>

VM::VM(ErrorHandler* handler, JITEngine* jit)
    : errorHandler(handler)
    , frameCount(0)
    , jitEngine(jit)
{
    globals = std::make_shared<Environment>(nullptr, errorHandler);

    using Args = const std::vector<std::unique_ptr<Value>>&;
    using EnvPtr = Environment*;

    // Register built-in functions
    builtins.resize(static_cast<size_t>(BuiltinIndex::MAX_BUILTINS));
    builtins[static_cast<size_t>(BuiltinIndex::PRINT)] = [](Args args, EnvPtr env) -> std::unique_ptr<Value> {
        (void)env;
        for (size_t i = 0; i < args.size(); i++)
        {
            if (i > 0) std::cout << " ";
            if (args[i]) std::cout << args[i]->toString();
        }
        return std::make_unique<NullValue>();
    };
    builtins[static_cast<size_t>(BuiltinIndex::PRINTLN)] = [](Args args, EnvPtr env) -> std::unique_ptr<Value> {
        for (size_t i = 0; i < args.size(); i++)
        {
            if (i > 0) std::cout << " ";
            if (args[i]) std::cout << args[i]->toString();
        }
        std::cout << std::endl;
        return std::make_unique<NullValue>();
    };
    builtins[static_cast<size_t>(BuiltinIndex::INPUT)] = [](Args args, EnvPtr env) -> std::unique_ptr<Value> {
        (void)env;
        if (!args.empty() && args[0])
            std::cout << args[0]->toString();
        std::string line;
        std::getline(std::cin, line);
        return std::make_unique<StringValue>(line);
    };
    builtins[static_cast<size_t>(BuiltinIndex::TYPE)] = [](Args args, EnvPtr env) -> std::unique_ptr<Value> {
        (void)env;
        if (args.empty() || !args[0]) return std::make_unique<StringValue>("null");
        return std::make_unique<StringValue>(args[0]->getTypeInfo().toString());
    };
    builtins[static_cast<size_t>(BuiltinIndex::LEN)] = [](Args args, EnvPtr env) -> std::unique_ptr<Value> {
        (void)env;
        if (args.empty() || !args[0]) return std::make_unique<NumberValue>(0);
        if (args[0]->getType() == ValueType::ARRAY)
            return std::make_unique<NumberValue>(static_cast<int64_t>(static_cast<ArrayValue*>(args[0].get())->size()));
        if (args[0]->getType() == ValueType::STRING)
            return std::make_unique<NumberValue>(static_cast<int64_t>(static_cast<StringValue*>(args[0].get())->getValue().size()));
        return std::make_unique<NumberValue>(0);
    };
    builtins[static_cast<size_t>(BuiltinIndex::TO_INT)] = [](Args args, EnvPtr env) -> std::unique_ptr<Value> {
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
    };
    builtins[static_cast<size_t>(BuiltinIndex::TO_FLOAT)] = [](Args args, EnvPtr env) -> std::unique_ptr<Value> {
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
    };
    builtins[static_cast<size_t>(BuiltinIndex::TO_STRING)] = [](Args args, EnvPtr env) -> std::unique_ptr<Value> {
        (void)env;
        if (args.empty() || !args[0]) return std::make_unique<StringValue>("null");
        return std::make_unique<StringValue>(args[0]->toString());
    };
    builtins[static_cast<size_t>(BuiltinIndex::RANGE)] = [](Args args, EnvPtr env) -> std::unique_ptr<Value> {
        (void)env;
        int start = 0, end = 0, step = 1;
        if (args.size() >= 1 && args[0])
            end = static_cast<int>(static_cast<NumberValue*>(args[0].get())->getValue());
        if (args.size() >= 2 && args[0] && args[1]) {
            start = static_cast<int>(static_cast<NumberValue*>(args[0].get())->getValue());
            end = static_cast<int>(static_cast<NumberValue*>(args[1].get())->getValue());
        }
        if (args.size() >= 3 && args[2])
            step = static_cast<int>(static_cast<NumberValue*>(args[2].get())->getValue());

        std::vector<std::unique_ptr<Value>> values;
        if (step > 0) {
            for (int i = start; i < end; i += step)
                values.push_back(std::make_unique<NumberValue>(i));
        } else {
            for (int i = start; i > end; i += step)
                values.push_back(std::make_unique<NumberValue>(i));
        }
        return std::make_unique<ArrayValue>(std::move(values));
    };
}

void VM::push(std::unique_ptr<Value> value)
{
    stack.push_back(std::move(value));
}

std::unique_ptr<Value> VM::pop()
{
    if (stack.empty()) return std::make_unique<NullValue>();
    auto val = std::move(stack.back());
    stack.pop_back();
    return val;
}

Value* VM::peek(int distance)
{
    if (stack.empty()) return nullptr;
    size_t index = stack.size() - 1 - distance;
    if (index >= stack.size()) return nullptr;
    return stack[index].get();
}

void VM::resetStack()
{
    stack.clear();
}

void VM::captureUpvalues()
{
    // Close upvalues that are on the stack
    while (!openUpvalues.empty())
    {
        auto& uv = openUpvalues.back();
        (void)uv;
        openUpvalues.pop_back();
    }
}

UpvalueInfo* VM::getUpvalue(int index)
{
    if (index >= 0 && index < static_cast<int>(openUpvalues.size()))
        return &openUpvalues[index];
    return nullptr;
}

bool VM::callFunction(FunctionProto* func, int argCount,
                      std::shared_ptr<Environment> closure)
{
    if (argCount != func->arity)
    {
        runtimeError(ErrorCode::WRONG_ARGUMENT_COUNT,
            "Function '" + func->name + "' expects " +
            std::to_string(static_cast<int>(func->arity)) +
            " arguments but " + std::to_string(argCount) + " provided");
        return false;
    }

    if (frameCount >= 256)
    {
        runtimeError(ErrorCode::STACK_OVERFLOW, "Stack overflow - too many nested function calls");
        return false;
    }

    CallFrame frame;
    frame.function = func;
    frame.ip = func->code.data();
    frame.closure = closure;
    frame.returnSlot = static_cast<int>(stack.size()) - argCount - 1;
    frame.stackBase = std::vector<Value*>();

    frames.push_back(frame);
    frameCount++;

    return true;
}

bool VM::callBuiltin(BuiltinIndex index, int argCount)
{
    size_t idx = static_cast<size_t>(index);
    if (idx >= builtins.size())
    {
        runtimeError("Unknown built-in function");
        return false;
    }

    // Gather arguments from stack
    std::vector<std::unique_ptr<Value>> args;
    for (int i = 0; i < argCount; i++)
    {
        args.insert(args.begin(), std::move(stack.back()));
        stack.pop_back();
    }

    auto result = builtins[idx](args, globals.get());
    push(std::move(result));
    return true;
}

bool VM::callValue(Value* callee, int argCount)
{
    if (!callee) return false;

    if (callee->getType() == ValueType::BUILTIN_FUNCTION)
    {
        // Gather arguments
        std::vector<std::unique_ptr<Value>> args;
        for (int i = 0; i < argCount; i++)
        {
            if (stack.size() >= 1)
            {
                args.insert(args.begin(), std::move(stack.back()));
                stack.pop_back();
            }
        }

        auto builtin = static_cast<BuiltinFunctionValue*>(callee);
        auto result = builtin->getFunction()(args, globals.get());
        push(std::move(result));
        return true;
    }
    else if (callee->getType() == ValueType::FUNCTION)
    {
        auto funcVal = static_cast<FunctionValue*>(callee);

        // Create a synthetic FunctionProto
        auto proto = std::make_unique<FunctionProto>(
            funcVal->getName(),
            static_cast<uint8_t>(funcVal->getParameters().size()));
        proto->localNames = funcVal->getParameters();

        // We need to compile the body... for now, fall back to interpreter
        // For the prototype, we'll use the tree-walking interpreter as bridge
        // This is a temporary bridge - the VM is designed for fully compiled functions

        // Push closure environment
        auto closure = funcVal->getClosure();
        auto callEnv = std::make_shared<Environment>(closure, errorHandler);

        // Bind params
        auto& params = funcVal->getParameters();
        std::vector<std::unique_ptr<Value>> args;
        for (int i = 0; i < argCount; i++)
        {
            if (!stack.empty())
            {
                args.insert(args.begin(), std::move(stack.back()));
                stack.pop_back();
            }
        }

        for (size_t i = 0; i < params.size(); i++)
        {
            if (i < args.size())
                callEnv->define(params[i], std::move(args[i]));
            else
                callEnv->define(params[i], std::make_unique<NullValue>());
        }

        // Execute body using simple interpreter
        // For now, return null (VM execution will be improved)
        captureUpvalues();
        push(std::make_unique<NullValue>());
        return true;
    }

    runtimeError(ErrorCode::NOT_CALLABLE, "Value is not callable");
    return false;
}

void VM::runtimeError(const std::string& message)
{
    if (errorHandler)
    {
        errorHandler->error(ErrorCode::RUNTIME_ERROR, message, SourceLocation("", 0, 0));
    }
}

void VM::runtimeError(ErrorCode code, const std::string& message)
{
    if (errorHandler)
    {
        errorHandler->error(code, message, SourceLocation("", 0, 0));
    }
}

void VM::printStack()
{
    std::cout << "Stack [" << stack.size() << "]: ";
    for (const auto& val : stack)
    {
        if (val)
            std::cout << val->toString() << " ";
        else
            std::cout << "null ";
    }
    std::cout << std::endl;
}

// ============================================================
// MAIN INTERPRET LOOP
// ============================================================

InterpretResult VM::interpret(FunctionProto* mainFunc)
{
    CallFrame frame;
    frame.function = mainFunc;
    frame.ip = mainFunc->code.data();
    frame.closure = globals;
    frame.returnSlot = 0;

    frames.push_back(frame);
    frameCount = 1;

    // Dispatch loop
    while (frameCount > 0)
    {
        CallFrame* cf = &frames[frameCount - 1];
        Instruction instr = *cf->ip;

#ifdef VM_DEBUG
        std::cout << "Exec: " << static_cast<int>(instr.op)
                  << " op=" << static_cast<int>(instr.operand)
                  << " stack=" << stack.size() << std::endl;
#endif

        switch (instr.op)
        {
            case OpCode::LOAD_CONST:
            {
                uint32_t idx = instr.operand;
                if (idx < cf->function->constants.size())
                {
                    const std::string& val = cf->function->constants[idx];
                    // Try to parse as number first
                    try
                    {
                        size_t pos;
                        int64_t intVal = std::stoll(val, &pos);
                        if (pos == val.size())
                        {
                            push(std::make_unique<NumberValue>(intVal));
                            break;
                        }
                    }
                    catch (...) {}

                    try
                    {
                        size_t pos;
                        double floatVal = std::stod(val, &pos);
                        if (pos == val.size())
                        {
                            push(std::make_unique<FloatValue>(floatVal));
                            break;
                        }
                    }
                    catch (...) {}

                    // Default to string
                    push(std::make_unique<StringValue>(val));
                }
                else
                {
                    push(std::make_unique<NullValue>());
                }
                cf->ip++;
                break;
            }

            case OpCode::LOAD_NULL:
                push(std::make_unique<NullValue>());
                cf->ip++;
                break;

            case OpCode::LOAD_TRUE:
                push(std::make_unique<BooleanValue>(true));
                cf->ip++;
                break;

            case OpCode::LOAD_FALSE:
                push(std::make_unique<BooleanValue>(false));
                cf->ip++;
                break;

            case OpCode::LOAD_VAR:
            {
                uint32_t local = instr.operand;
                if (local < cf->function->localNames.size())
                {
                    const std::string& name = cf->function->localNames[local];
                    Value* val = cf->closure->get(name);
                    if (val)
                        push(val->clone());
                    else
                        push(std::make_unique<NullValue>());
                }
                else
                {
                    push(std::make_unique<NullValue>());
                }
                cf->ip++;
                break;
            }

            case OpCode::STORE_VAR:
            {
                uint32_t local = instr.operand;
                auto value = pop();
                if (local < cf->function->localNames.size())
                {
                    const std::string& name = cf->function->localNames[local];
                    cf->closure->set(name, std::move(value));
                }
                cf->ip++;
                break;
            }

            case OpCode::LOAD_GLOBAL:
            {
                uint32_t idx = instr.operand;
                if (idx < cf->function->globalNames.size())
                {
                    const std::string& name = cf->function->globalNames[idx];
                    Value* val = globals->get(name);
                    if (val)
                        push(val->clone());
                    else
                        push(std::make_unique<NullValue>());
                }
                else
                {
                    push(std::make_unique<NullValue>());
                }
                cf->ip++;
                break;
            }

            case OpCode::STORE_GLOBAL:
            {
                uint32_t idx = instr.operand;
                auto value = pop();
                if (idx < cf->function->globalNames.size())
                {
                    const std::string& name = cf->function->globalNames[idx];
                    // Define if not exists
                    if (!globals->exists(name))
                        globals->define(name, std::move(value));
                    else
                        globals->set(name, std::move(value));
                }
                cf->ip++;
                break;
            }

            case OpCode::LOAD_UPVALUE:
                push(std::make_unique<NullValue>());
                cf->ip++;
                break;

            case OpCode::STORE_UPVALUE:
                pop();
                cf->ip++;
                break;

            case OpCode::ADD:
            case OpCode::SUB:
            case OpCode::MUL:
            case OpCode::DIV:
            case OpCode::MOD:
            case OpCode::POW:
            {
                auto right = pop();
                auto left = pop();

                if (!left || !right)
                {
                    push(std::make_unique<NullValue>());
                    cf->ip++;
                    break;
                }

                // String concatenation for ADD
                if (instr.op == OpCode::ADD &&
                    (left->getType() == ValueType::STRING || right->getType() == ValueType::STRING))
                {
                    push(std::make_unique<StringValue>(left->toString() + right->toString()));
                    cf->ip++;
                    break;
                }

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

                bool leftNum = left->getType() == ValueType::INT || left->getType() == ValueType::FLOAT;
                bool rightNum = right->getType() == ValueType::INT || right->getType() == ValueType::FLOAT;
                bool bothInt = left->getType() == ValueType::INT && right->getType() == ValueType::INT;

                if (leftNum && rightNum)
                {
                    double l = getNum(left.get());
                    double r = getNum(right.get());
                    int64_t li = getInt(left.get());
                    int64_t ri = getInt(right.get());

                    switch (instr.op)
                    {
                        case OpCode::ADD:
                            if (bothInt) push(std::make_unique<NumberValue>(li + ri));
                            else push(std::make_unique<FloatValue>(l + r));
                            break;
                        case OpCode::SUB:
                            if (bothInt) push(std::make_unique<NumberValue>(li - ri));
                            else push(std::make_unique<FloatValue>(l - r));
                            break;
                        case OpCode::MUL:
                            if (bothInt) push(std::make_unique<NumberValue>(li * ri));
                            else push(std::make_unique<FloatValue>(l * r));
                            break;
                        case OpCode::DIV:
                            if (ri == 0 || r == 0.0) {
                                runtimeError(ErrorCode::DIVISION_BY_ZERO, "Division by zero");
                                push(std::make_unique<NullValue>());
                            } else {
                                if (bothInt) push(std::make_unique<NumberValue>(li / ri));
                                else push(std::make_unique<FloatValue>(l / r));
                            }
                            break;
                        case OpCode::MOD:
                            if (ri == 0) {
                                runtimeError(ErrorCode::DIVISION_BY_ZERO, "Modulo by zero");
                                push(std::make_unique<NullValue>());
                            } else {
                                push(std::make_unique<NumberValue>(li % ri));
                            }
                            break;
                        case OpCode::POW:
                        {
                            double result = std::pow(l, r);
                            if (bothInt) push(std::make_unique<NumberValue>(static_cast<int64_t>(result)));
                            else push(std::make_unique<FloatValue>(result));
                            break;
                        }
                        default:
                            push(std::make_unique<NullValue>());
                            break;
                    }
                }
                else
                {
                    push(std::make_unique<NullValue>());
                }
                cf->ip++;
                break;
            }

            case OpCode::NEGATE:
            {
                auto val = pop();
                if (val && val->getType() == ValueType::INT)
                    push(std::make_unique<NumberValue>(-static_cast<NumberValue*>(val.get())->getValue()));
                else if (val && val->getType() == ValueType::FLOAT)
                    push(std::make_unique<FloatValue>(-static_cast<FloatValue*>(val.get())->getValue()));
                else
                    push(std::move(val));
                cf->ip++;
                break;
            }

            case OpCode::EQ:
            case OpCode::NE:
            case OpCode::LT:
            case OpCode::GT:
            case OpCode::LE:
            case OpCode::GE:
            {
                auto right = pop();
                auto left = pop();

                auto getNum = [](Value* v) -> double {
                    if (v->getType() == ValueType::INT)
                        return static_cast<double>(static_cast<NumberValue*>(v)->getValue());
                    if (v->getType() == ValueType::FLOAT)
                        return static_cast<FloatValue*>(v)->getValue();
                    return 0.0;
                };

                double l = getNum(left.get());
                double r = getNum(right.get());

                switch (instr.op)
                {
                    case OpCode::EQ: push(std::make_unique<BooleanValue>(l == r)); break;
                    case OpCode::NE: push(std::make_unique<BooleanValue>(l != r)); break;
                    case OpCode::LT: push(std::make_unique<BooleanValue>(l < r)); break;
                    case OpCode::GT: push(std::make_unique<BooleanValue>(l > r)); break;
                    case OpCode::LE: push(std::make_unique<BooleanValue>(l <= r)); break;
                    case OpCode::GE: push(std::make_unique<BooleanValue>(l >= r)); break;
                    default: push(std::make_unique<BooleanValue>(false)); break;
                }
                cf->ip++;
                break;
            }

            case OpCode::CMP:
            {
                auto right = pop();
                auto left = pop();
                auto getNum = [](Value* v) -> double {
                    if (v->getType() == ValueType::INT)
                        return static_cast<double>(static_cast<NumberValue*>(v)->getValue());
                    if (v->getType() == ValueType::FLOAT)
                        return static_cast<FloatValue*>(v)->getValue();
                    return 0.0;
                };
                double l = getNum(left.get());
                double r = getNum(right.get());
                int cmp = (l < r) ? -1 : (l > r) ? 1 : 0;
                push(std::make_unique<NumberValue>(cmp));
                cf->ip++;
                break;
            }

            case OpCode::NOT:
            {
                auto val = pop();
                push(std::make_unique<BooleanValue>(!val->isTruthy()));
                cf->ip++;
                break;
            }

            case OpCode::AND:
            case OpCode::OR:
            {
                auto right = pop();
                auto left = pop();
                bool lb = left->isTruthy();
                bool rb = right->isTruthy();
                if (instr.op == OpCode::AND)
                    push(std::make_unique<BooleanValue>(lb && rb));
                else
                    push(std::make_unique<BooleanValue>(lb || rb));
                cf->ip++;
                break;
            }

            case OpCode::JUMP:
            {
                cf->ip = cf->function->code.data() + instr.operand;
                break;
            }

            case OpCode::JUMP_IF_FALSE:
            {
                auto val = peek();
                if (!val || !val->isTruthy())
                {
                    cf->ip = cf->function->code.data() + instr.operand;
                }
                else
                {
                    cf->ip++;
                }
                break;
            }

            case OpCode::JUMP_IF_TRUE:
            {
                auto val = peek();
                if (val && val->isTruthy())
                {
                    cf->ip = cf->function->code.data() + instr.operand;
                }
                else
                {
                    cf->ip++;
                }
                break;
            }

            case OpCode::LOOP:
            {
                if (jitEngine && cf->function)
                {
                    jitEngine->profileLoopIteration(cf->function->name);
                }
                cf->ip = cf->function->code.data() + instr.operand;
                break;
            }

            case OpCode::CALL:
            {
                uint32_t argCount = instr.operand;

                // Profile: record function call
                if (jitEngine && cf->function)
                {
                    jitEngine->profileFunctionCall(cf->function->name);
                }

                auto* callee = peek(static_cast<int>(argCount));
                if (!callee)
                {
                    runtimeError("Cannot call null value");
                    cf->ip++;
                    break;
                }

                // Check if it's a function on the stack
                if (callee->getType() == ValueType::FUNCTION ||
                    callee->getType() == ValueType::BUILTIN_FUNCTION)
                {
                    callValue(callee, static_cast<int>(argCount));
                }
                else if (callee->getType() == ValueType::OBJECT)
                {
                    pop();
                }
                else
                {
                    runtimeError(ErrorCode::NOT_CALLABLE, "Value is not callable");
                }
                cf->ip++;
                break;
            }

            case OpCode::BUILTIN_CALL:
            {
                BuiltinIndex index = static_cast<BuiltinIndex>(instr.operand >> 16);
                int argCount = static_cast<int>(instr.operand & 0xFFFF);
                callBuiltin(index, argCount);
                cf->ip++;
                break;
            }

            case OpCode::RETURN:
            {
                auto result = pop();
                captureUpvalues();
                frames.pop_back();
                frameCount--;

                if (frameCount > 0)
                {
                    push(std::move(result));
                }
                else
                {
                    // Program end
                    return InterpretResult::OK;
                }
                break;
            }

            case OpCode::MAKE_FUNCTION:
            {
                uint32_t idx = instr.operand;
                if (idx < functions.size())
                {
                    auto& funcProto = functions[idx];
                    std::vector<std::string> params;
                    for (const auto& name : funcProto->localNames)
                        params.push_back(name);

                    std::vector<std::unique_ptr<ASTNode>> emptyBody;
                    auto funcVal = std::make_unique<FunctionValue>(
                        funcProto->name, params,
                        std::move(emptyBody),
                        cf->closure);
                    push(std::move(funcVal));
                }
                else
                {
                    push(std::make_unique<NullValue>());
                }
                cf->ip++;
                break;
            }

            case OpCode::MAKE_CLOSURE:
            {
                push(std::make_unique<NullValue>());
                cf->ip++;
                break;
            }

            case OpCode::CLOSE_UPVALUE:
            {
                pop();
                cf->ip++;
                break;
            }

            case OpCode::MAKE_OBJECT:
            {
                push(std::make_unique<ObjectValue>());
                cf->ip++;
                break;
            }

            case OpCode::LOAD_FIELD:
            {
                uint32_t fieldIdx = instr.operand;
                auto obj = pop();
                if (obj && obj->getType() == ValueType::OBJECT)
                {
                    auto objVal = static_cast<ObjectValue*>(obj.get());
                    std::string fieldName = "<unknown>";
                    if (!cf->function->constants.empty() && fieldIdx < cf->function->constants.size())
                        fieldName = cf->function->constants[fieldIdx];

                    Value* member = objVal->get(fieldName);
                    if (member)
                        push(member->clone());
                    else
                        push(std::make_unique<NullValue>());
                }
                else
                {
                    push(std::make_unique<NullValue>());
                }
                cf->ip++;
                break;
            }

            case OpCode::STORE_FIELD:
            {
                auto value = pop();
                auto field = pop();
                auto obj = pop();
                (void)field;
                (void)value;
                (void)obj;
                cf->ip++;
                break;
            }

            case OpCode::MAKE_ARRAY:
            {
                uint32_t count = instr.operand;
                std::vector<std::unique_ptr<Value>> elements;
                for (uint32_t i = 0; i < count; i++)
                {
                    if (!stack.empty())
                    {
                        elements.insert(elements.begin(), std::move(stack.back()));
                        stack.pop_back();
                    }
                }
                push(std::make_unique<ArrayValue>(std::move(elements)));
                cf->ip++;
                break;
            }

            case OpCode::LOAD_INDEX:
            {
                auto index = pop();
                auto arr = pop();
                if (arr && arr->getType() == ValueType::ARRAY && index)
                {
                    auto arrVal = static_cast<ArrayValue*>(arr.get());
                    size_t idx = 0;
                    if (index->getType() == ValueType::INT)
                        idx = static_cast<size_t>(static_cast<NumberValue*>(index.get())->getValue());
                    if (idx < arrVal->size())
                    {
                        Value* elem = arrVal->get(idx);
                        if (elem) { push(elem->clone()); break; }
                    }
                }
                push(std::make_unique<NullValue>());
                cf->ip++;
                break;
            }

            case OpCode::STORE_INDEX:
            {
                auto value = pop();
                auto index = pop();
                auto arr = pop();
                (void)arr;
                (void)index;
                (void)value;
                cf->ip++;
                break;
            }

            case OpCode::ARRAY_APPEND:
            {
                pop();
                cf->ip++;
                break;
            }

            case OpCode::ARRAY_POP:
            {
                push(std::make_unique<NullValue>());
                cf->ip++;
                break;
            }

            case OpCode::ARRAY_LEN:
            {
                auto arr = peek();
                if (arr && arr->getType() == ValueType::ARRAY)
                    push(std::make_unique<NumberValue>(static_cast<int64_t>(static_cast<ArrayValue*>(arr)->size())));
                else
                    push(std::make_unique<NumberValue>(0));
                cf->ip++;
                break;
            }

            case OpCode::PRINT:
            {
                auto val = pop();
                if (val) std::cout << val->toString();
                cf->ip++;
                break;
            }

            case OpCode::PRINTLN:
            {
                auto val = pop();
                if (val) std::cout << val->toString();
                std::cout << std::endl;
                cf->ip++;
                break;
            }

            case OpCode::POP:
            {
                pop();
                cf->ip++;
                break;
            }

            case OpCode::DUP:
            {
                auto val = peek();
                if (val) push(val->clone());
                else push(std::make_unique<NullValue>());
                cf->ip++;
                break;
            }

            case OpCode::SWAP:
            {
                if (stack.size() >= 2)
                {
                    std::swap(stack[stack.size() - 1], stack[stack.size() - 2]);
                }
                cf->ip++;
                break;
            }

            case OpCode::HALT:
                return InterpretResult::OK;

            default:
                runtimeError("Unknown opcode: " + std::to_string(static_cast<int>(instr.op)));
                return InterpretResult::RUNTIME_ERROR;
        }
    }

    return InterpretResult::OK;
}

void VM::loadModule(const std::string& name, std::shared_ptr<Environment> targetEnv)
{
    (void)name;
    (void)targetEnv;
}

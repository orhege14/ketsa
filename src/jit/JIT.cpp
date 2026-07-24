#define NOMINMAX
#include "JIT.h"
#include "x64/Assembler.h"
#include <algorithm>
#include <chrono>
#include <cstring>

JITEngine::JITEngine(ErrorHandler* handler)
    : errorHandler(handler)
    , enabled(false)
{
    assembler = std::make_unique<X64Assembler>();
}

JITEngine::~JITEngine()
{
    // Free all native code pages
    for (auto& [name, func] : jitFunctions)
    {
        if (func.nativeCode)
            freeCodePage(func.nativeCode, func.codeSize);
    }
}

bool JITEngine::isBackendAvailable()
{
    // Check if we can allocate executable memory
    // Always returns true on x64; actual allocation may fail at runtime
    return true;
}

NativeCodeHandle JITEngine::allocateCodePage(size_t size)
{
#if defined(_WIN32) || defined(_WIN64)
    return VirtualAlloc(nullptr, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
#else
    void* addr = mmap(nullptr, size, PROT_READ | PROT_WRITE | PROT_EXEC,
                       MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    return (addr == MAP_FAILED) ? nullptr : addr;
#endif
}

void JITEngine::freeCodePage(NativeCodeHandle handle, size_t size)
{
    if (!handle) return;
#if defined(_WIN32) || defined(_WIN64)
    VirtualFree(handle, 0, MEM_RELEASE);
#else
    munmap(handle, size);
#endif
}

void JITEngine::makeExecutable(NativeCodeHandle handle, size_t size)
{
#if defined(_WIN32) || defined(_WIN64)
    DWORD oldProtect;
    VirtualProtect(handle, size, PAGE_EXECUTE_READ, &oldProtect);
#else
    // Already mapped with PROT_EXEC on Linux
    (void)handle;
    (void)size;
#endif
}

// ============================================================
// PROFILING
// ============================================================

void JITEngine::profileFunctionCall(const std::string& funcName)
{
    if (!enabled) return;
    recordCall(funcName);

    if (shouldCompile(funcName))
    {
        // Find the function proto and compile it
        // This is triggered from the VM when a function is hot
    }
}

void JITEngine::profileLoopIteration(const std::string& funcName)
{
    if (!enabled) return;
    auto it = jitFunctions.find(funcName);
    if (it != jitFunctions.end())
    {
        it->second.callCount++;
    }
}

void JITEngine::recordCall(const std::string& funcName)
{
    auto it = jitFunctions.find(funcName);
    if (it == jitFunctions.end())
    {
        JITFunction jf;
        jf.name = funcName;
        jf.callCount = 1;
        jitFunctions[funcName] = jf;
    }
    else
    {
        it->second.callCount++;
    }
}

bool JITEngine::shouldCompile(const std::string& funcName) const
{
    auto it = jitFunctions.find(funcName);
    if (it == jitFunctions.end()) return false;
    return it->second.callCount >= HOT_FUNCTION_THRESHOLD && !it->second.isCompiled;
}

// ============================================================
// BYTECODE OPTIMIZATION
// ============================================================

bool JITEngine::optimize(FunctionProto* proto)
{
    if (!proto) return false;
    bool changed = false;

    // Pass 1: Constant folding for simple arithmetic
    for (size_t i = 0; i + 2 < proto->code.size(); i++)
    {
        auto& instr = proto->code[i];
        auto& next = proto->code[i + 1];

        // LOAD_CONST + LOAD_CONST + ADD -> LOAD_CONST(result)
        if (instr.op == OpCode::LOAD_CONST && next.op == OpCode::LOAD_CONST)
        {
            if (i + 2 < proto->code.size() && proto->code[i + 2].op == OpCode::ADD)
            {
                // Parse constants
                auto it1 = std::find(proto->constants.begin(), proto->constants.end(),
                                     proto->constants[instr.operand]);
                auto it2 = std::find(proto->constants.begin(), proto->constants.end(),
                                     proto->constants[next.operand]);
                if (it1 != proto->constants.end() && it2 != proto->constants.end())
                {
                    int64_t a = 0, b = 0;
                    try { a = std::stoll(proto->constants[instr.operand]); } catch (...) {}
                    try { b = std::stoll(proto->constants[next.operand]); } catch (...) {}

                    std::string result = std::to_string(a + b);
                    uint32_t constIdx = static_cast<uint32_t>(proto->constants.size());
                    proto->constants.push_back(result);

                    proto->code[i].op = OpCode::LOAD_CONST;
                    proto->code[i].operand = constIdx;
                    proto->code.erase(proto->code.begin() + i + 1);
                    proto->code.erase(proto->code.begin() + i + 1);
                    changed = true;
                }
            }
        }
    }

    // Pass 2: Dead code elimination (POP after STORE_VAR is redundant)
    for (size_t i = 0; i + 1 < proto->code.size(); i++)
    {
        if (proto->code[i].op == OpCode::STORE_VAR &&
            proto->code[i + 1].op == OpCode::POP)
        {
            proto->code.erase(proto->code.begin() + i + 1);
            changed = true;
        }
    }

    // Pass 3: Remove NOP-like sequences (JUMP to next instruction)
    for (size_t i = 0; i + 1 < proto->code.size(); i++)
    {
        if (proto->code[i].op == OpCode::JUMP &&
            proto->code[i].operand == i + 1)
        {
            proto->code.erase(proto->code.begin() + i);
            changed = true;
        }
    }

    return changed;
}

// ============================================================
// COMPILATION
// ============================================================

bool JITEngine::compileFunction(FunctionProto* proto)
{
    if (!proto || !assembler) return false;

    std::string name = proto->name.empty() ? "__anonymous" : proto->name;

    JITFunction& jf = jitFunctions[name];
    if (jf.isCompiled) return true;

    // Allocate code page
    size_t estimated = proto->code.size() * 64 + 1024;
    size_t codeSize = (estimated < MAX_NATIVE_CODE_SIZE) ? estimated : MAX_NATIVE_CODE_SIZE;
    NativeCodeHandle code = allocateCodePage(codeSize);
    if (!code) return false;

    jf.nativeCode = code;
    jf.codeSize = codeSize;
    jf.isCompiled = true;

    // Use the assembler to generate native code
    assembler->begin(codeSize);

    // Prolog
    int stackFrameSize = static_cast<int>((proto->localNames.size() + 16) * 8);
    assembler->emitProlog(stackFrameSize);

    // Translate bytecode to native code
    for (size_t i = 0; i < proto->code.size(); i++)
    {
        auto& instr = proto->code[i];
        switch (instr.op)
        {
            case OpCode::LOAD_CONST:
                assembler->emitLoadConst(static_cast<int64_t>(instr.operand), 0);
                assembler->emitStoreRegToStack(0, 0);
                break;

            case OpCode::LOAD_VAR:
                assembler->emitLoadStackToReg(static_cast<int>(instr.operand), 0);
                break;

            case OpCode::STORE_VAR:
                assembler->emitLoadStackToReg(0, 0);
                assembler->emitStoreRegToStack(0, static_cast<int>(instr.operand));
                break;

            case OpCode::ADD:
                assembler->emitLoadStackToReg(0, 0);
                assembler->emitLoadStackToReg(1, 1);
                assembler->emitAdd(0, 1);
                assembler->emitStoreRegToStack(0, 0);
                break;

            case OpCode::RETURN:
                assembler->emitLoadStackToReg(0, 0);
                assembler->emitEpilog();
                assembler->emitRet();
                break;

            default:
                // For unsupported ops, fall back to interpreter
                // by calling a special trampoline
                break;
        }
    }

    size_t finalSize = assembler->end();
    if (finalSize > 0) {
        std::memcpy(code, assembler->getCode(), finalSize);
    }
    makeExecutable(code, finalSize);
    jf.codeSize = finalSize;
    jf.entryPoint = reinterpret_cast<JittedFunction>(code);

    return true;
}

bool JITEngine::execute(FunctionProto* proto, std::vector<Value*>& stack,
                        std::shared_ptr<Environment> closure)
{
    if (!enabled || !proto) return false;

    std::string name = proto->name.empty() ? "__anonymous" : proto->name;

    // Check if already compiled
    auto it = jitFunctions.find(name);
    if (it != jitFunctions.end() && it->second.isCompiled && it->second.entryPoint)
    {
        it->second.entryPoint(stack.data(), closure.get());
        return true;
    }

    // Check if should compile
    profileFunctionCall(name);
    if (shouldCompile(name))
    {
        if (compileFunction(proto))
        {
            auto jitIt = jitFunctions.find(name);
            if (jitIt != jitFunctions.end() && jitIt->second.entryPoint)
            {
                jitIt->second.entryPoint(stack.data(), closure.get());
                return true;
            }
        }
    }

    return false;
}

bool JITEngine::tryExecuteJIT(FunctionProto* proto,
                              Value** stack, int argCount,
                              std::shared_ptr<Environment> closure)
{
    (void)argCount;
    if (!enabled || !proto) return false;

    std::string name = proto->name.empty() ? "__anonymous" : proto->name;

    auto it = jitFunctions.find(name);
    if (it != jitFunctions.end() && it->second.entryPoint)
    {
        it->second.entryPoint(stack, closure.get());
        return true;
    }

    return false;
}

// ============================================================
// STATS
// ============================================================

int JITEngine::getCompiledCount() const
{
    int count = 0;
    for (const auto& [name, func] : jitFunctions)
    {
        if (func.isCompiled) count++;
    }
    return count;
}

std::vector<ProfileSample> JITEngine::getProfileData() const
{
    std::vector<ProfileSample> samples;
    for (const auto& [name, func] : jitFunctions)
    {
        ProfileSample s;
        s.functionName = name;
        s.callCount = func.callCount;
        samples.push_back(s);
    }
    return samples;
}

void JITEngine::addPass(std::unique_ptr<OptimizationPass> pass)
{
    if (pass)
        passes.push_back(std::move(pass));
}

bool JITEngine::runOptimizationPipeline(FunctionProto* proto)
{
    if (!proto) return false;
    bool anyChanged = false;
    for (auto& pass : passes)
    {
        if (pass && pass->enabled)
        {
            bool changed = pass->run(proto);
            if (changed) anyChanged = true;
        }
    }
    return anyChanged;
}

void JITEngine::clearPasses()
{
    passes.clear();
}

void JITEngine::resetProfiling()
{
    jitFunctions.clear();
}

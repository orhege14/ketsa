#pragma once

#include "../bytecode/Bytecode.h"
#include "../runtime/Environment.h"
#include "../runtime/values/Value.h"
#include "../errors/Error.h"

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <functional>
#include <cstdint>

// ============================================================
// JIT Configuration
// ============================================================
constexpr int HOT_FUNCTION_THRESHOLD = 50;
constexpr int HOT_LOOP_THRESHOLD = 100;
constexpr int MAX_NATIVE_CODE_SIZE = 65536;

// ============================================================
// Native code handle (platform-specific)
// ============================================================
#if defined(_WIN32) || defined(_WIN64)
    #define NOMINMAX
    #include <windows.h>
    #undef NOMINMAX
    using NativeCodeHandle = void*;
#else
    #include <sys/mman.h>
    #include <unistd.h>
    using NativeCodeHandle = void*;
#endif

// ============================================================
// JIT Function
// ============================================================
using JittedFunction = void(*)(Value** stack, Environment* env);

struct JITFunction
{
    std::string name;
    NativeCodeHandle nativeCode;
    JittedFunction entryPoint;
    size_t codeSize;
    int callCount;
    bool isHot;
    bool isCompiled;

    JITFunction() : nativeCode(nullptr), entryPoint(nullptr),
                    codeSize(0), callCount(0), isHot(false), isCompiled(false) {}
};

// ============================================================
// Profiling Data
// ============================================================
struct ProfileSample
{
    std::string functionName;
    int callCount;
    int loopIterations;
    double totalTimeMs;
};

// ============================================================
// Bytecode Optimization Passes
// ============================================================
struct OptimizationPass
{
    std::string name;
    bool enabled;

    virtual bool run(FunctionProto* proto) = 0;
    virtual ~OptimizationPass() = default;
};

// ============================================================
// Native Code Assembler (Platform Backend)
// ============================================================
class NativeAssembler
{
public:
    virtual ~NativeAssembler() = default;

    virtual void begin(size_t maxSize) = 0;
    virtual size_t end() = 0;
    virtual NativeCodeHandle getCode() = 0;
    virtual void release() = 0;

    // Stack operations
    virtual void emitProlog(int stackFrameSize) = 0;
    virtual void emitEpilog() = 0;

    // Memory ops
    virtual void emitLoadStackToReg(int stackSlot, int reg) = 0;
    virtual void emitStoreRegToStack(int reg, int stackSlot) = 0;
    virtual void emitLoadConst(int64_t value, int reg) = 0;
    virtual void emitLoadConstDouble(double value, int reg) = 0;

    // Arithmetic
    virtual void emitAdd(int dstReg, int srcReg) = 0;
    virtual void emitSub(int dstReg, int srcReg) = 0;
    virtual void emitMul(int dstReg, int srcReg) = 0;
    virtual void emitSDiv(int dstReg, int srcReg) = 0;
    virtual void emitSMod(int dstReg, int srcReg) = 0;

    // Comparison
    virtual void emitCmp(int reg1, int reg2) = 0;
    virtual void emitSetCC(const std::string& condition, int reg) = 0;

    // Control flow
    virtual void emitJmp(const std::string& target) = 0;
    virtual void emitJmpCC(const std::string& condition, const std::string& target) = 0;
    virtual void emitLabel(const std::string& name) = 0;
    virtual void emitCall(const std::string& target) = 0;
    virtual void emitRet() = 0;

    // Function calls
    virtual void emitNativeCall(void* funcPtr) = 0;
};

// ============================================================
// Main JIT Engine
// ============================================================
class JITEngine
{
private:
    ErrorHandler* errorHandler;
    bool enabled;

    // Profiling data
    std::unordered_map<std::string, JITFunction> jitFunctions;

    // Native assembler backend
    std::unique_ptr<NativeAssembler> assembler;

    // Bytecode optimizer
    std::vector<std::unique_ptr<OptimizationPass>> passes;

    // Compilation
    bool compileFunction(FunctionProto* proto);

    // Profiling
    void recordCall(const std::string& funcName);
    bool shouldCompile(const std::string& funcName) const;

    // Memory management for native code
    NativeCodeHandle allocateCodePage(size_t size);
    void freeCodePage(NativeCodeHandle handle, size_t size);
    void makeExecutable(NativeCodeHandle handle, size_t size);

public:
    JITEngine(ErrorHandler* handler = nullptr);
    ~JITEngine();

    void setEnabled(bool e) { enabled = e; }
    bool isEnabled() const { return enabled; }

    // Profiling
    void profileFunctionCall(const std::string& funcName);
    void profileLoopIteration(const std::string& funcName);

    // JIT execution
    bool tryExecuteJIT(FunctionProto* proto,
                       Value** stack, int argCount,
                       std::shared_ptr<Environment> closure);

    // Try to run a function via JIT
    bool execute(FunctionProto* proto, std::vector<Value*>& stack,
                 std::shared_ptr<Environment> closure);

    // Optimization pipeline
    bool optimize(FunctionProto* proto);
    void addPass(std::unique_ptr<OptimizationPass> pass);
    bool runOptimizationPipeline(FunctionProto* proto);
    void clearPasses();

    // Stats
    int getCompiledCount() const;
    std::vector<ProfileSample> getProfileData() const;

    // Check if backend is available
    static bool isBackendAvailable();

    // Reset profiling data
    void resetProfiling();
};

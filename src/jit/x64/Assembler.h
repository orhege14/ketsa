#pragma once

#include "../JIT.h"
#include <vector>
#include <string>
#include <unordered_map>
#include <cstdint>

// ============================================================
// x64 Register Mapping
// ============================================================
enum X64Reg : uint8_t
{
    RAX = 0, RCX = 1, RDX = 2, RBX = 3,
    RSP = 4, RBP = 5, RSI = 6, RDI = 7,
    R8  = 8, R9  = 9, R10 = 10, R11 = 11,
    R12 = 12, R13 = 13, R14 = 14, R15 = 15
};

// ============================================================
// x64 Native Assembler
// ============================================================
// Generates x64 machine code for Windows and Linux calling conventions.
// Windows x64 calling convention: RCX, RDX, R8, R9 for params
// Linux x64 calling convention: RDI, RSI, RDX, RCX, R8, R9

class X64Assembler : public NativeAssembler
{
private:
    std::vector<uint8_t> code;
    size_t maxSize;
    std::unordered_map<std::string, size_t> labels;
    std::vector<std::pair<std::string, size_t>> pendingJumps;
    int stackFrameSize;

    // Emit raw bytes
    void emit8(uint8_t b);
    void emit16(uint16_t w);
    void emit32(uint32_t d);
    void emit64(uint64_t q);
    void emitBytes(const uint8_t* data, size_t len);

    // REX prefix
    void emitREX(bool w, bool r, bool x, bool b);

    // ModRM byte
    void emitModRM(uint8_t mod, uint8_t reg, uint8_t rm);

    // SIB byte
    void emitSIB(uint8_t scale, uint8_t index, uint8_t base);

    // Stack offset to register mapping
    int stackToRBPOffset(int stackSlot) const;

    // Register helpers
    uint8_t regToRM(X64Reg reg) const { return static_cast<uint8_t>(reg) & 7; }
    bool regNeedsREX(X64Reg reg) const { return static_cast<uint8_t>(reg) >= 8; }

    // Resolve label
    size_t resolveLabel(const std::string& name) const;

public:
    X64Assembler();
    ~X64Assembler() override;

    // Lifecycle
    void begin(size_t maxSize) override;
    size_t end() override;
    NativeCodeHandle getCode() override;
    void release() override;

    // Prolog/Epilog
    void emitProlog(int stackFrameSize) override;
    void emitEpilog() override;

    // Stack to/from register
    void emitLoadStackToReg(int stackSlot, int reg) override;
    void emitStoreRegToStack(int reg, int stackSlot) override;

    // Constants
    void emitLoadConst(int64_t value, int reg) override;
    void emitLoadConstDouble(double value, int reg) override;

    // Arithmetic (reg forms: dst = dst op src)
    void emitAdd(int dstReg, int srcReg) override;
    void emitSub(int dstReg, int srcReg) override;
    void emitMul(int dstReg, int srcReg) override;
    void emitSDiv(int dstReg, int srcReg) override;
    void emitSMod(int dstReg, int srcReg) override;

    // Comparison
    void emitCmp(int reg1, int reg2) override;
    void emitSetCC(const std::string& condition, int reg) override;

    // Control flow
    void emitJmp(const std::string& target) override;
    void emitJmpCC(const std::string& condition, const std::string& target) override;
    void emitLabel(const std::string& name) override;
    void emitCall(const std::string& target) override;
    void emitRet() override;

    // External function calls
    void emitNativeCall(void* funcPtr) override;

    // Direct x64 helpers
    void emitMovRegReg(int dst, int src);
    void emitMovRegImm(int reg, int64_t value);
    void emitMovRegStack(int reg, int stackSlot);
    void emitMovStackReg(int stackSlot, int reg);
    void emitPush(int reg);
    void emitPop(int reg);

    // Debug
    void dumpCode() const;
};

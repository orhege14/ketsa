#include "Assembler.h"
#include <cstring>
#include <algorithm>

X64Assembler::X64Assembler()
    : maxSize(0)
    , stackFrameSize(0)
{
}

X64Assembler::~X64Assembler()
{
    code.clear();
}

void X64Assembler::begin(size_t size)
{
    maxSize = size;
    code.clear();
    code.reserve(size);
    labels.clear();
    pendingJumps.clear();
    stackFrameSize = 0;
}

size_t X64Assembler::end()
{
    // Resolve pending jumps
    for (auto& [label, offset] : pendingJumps)
    {
        auto it = labels.find(label);
        if (it != labels.end())
        {
            int32_t disp = static_cast<int32_t>(it->second) -
                           static_cast<int32_t>(offset) - 4;
            // Patch the displacement
            code[offset + 1] = static_cast<uint8_t>(disp & 0xFF);
            code[offset + 2] = static_cast<uint8_t>((disp >> 8) & 0xFF);
            code[offset + 3] = static_cast<uint8_t>((disp >> 16) & 0xFF);
            code[offset + 4] = static_cast<uint8_t>((disp >> 24) & 0xFF);
        }
    }
    pendingJumps.clear();

    return code.size();
}

NativeCodeHandle X64Assembler::getCode()
{
    return static_cast<NativeCodeHandle>(code.data());
}

void X64Assembler::release()
{
    code.clear();
}

// ============================================================
// RAW BYTE EMISSION
// ============================================================

void X64Assembler::emit8(uint8_t b)
{
    code.push_back(b);
}

void X64Assembler::emit16(uint16_t w)
{
    emit8(static_cast<uint8_t>(w & 0xFF));
    emit8(static_cast<uint8_t>((w >> 8) & 0xFF));
}

void X64Assembler::emit32(uint32_t d)
{
    emit8(static_cast<uint8_t>(d & 0xFF));
    emit8(static_cast<uint8_t>((d >> 8) & 0xFF));
    emit8(static_cast<uint8_t>((d >> 16) & 0xFF));
    emit8(static_cast<uint8_t>((d >> 24) & 0xFF));
}

void X64Assembler::emit64(uint64_t q)
{
    emit32(static_cast<uint32_t>(q & 0xFFFFFFFF));
    emit32(static_cast<uint32_t>((q >> 32) & 0xFFFFFFFF));
}

void X64Assembler::emitBytes(const uint8_t* data, size_t len)
{
    for (size_t i = 0; i < len; i++)
        code.push_back(data[i]);
}

void X64Assembler::emitREX(bool w, bool r, bool x, bool b)
{
    uint8_t rex = 0x40;
    if (w) rex |= 0x08;
    if (r) rex |= 0x04;
    if (x) rex |= 0x02;
    if (b) rex |= 0x01;
    emit8(rex);
}

void X64Assembler::emitModRM(uint8_t mod, uint8_t reg, uint8_t rm)
{
    emit8(static_cast<uint8_t>((mod << 6) | ((reg & 7) << 3) | (rm & 7)));
}

void X64Assembler::emitSIB(uint8_t scale, uint8_t index, uint8_t base)
{
    emit8(static_cast<uint8_t>((scale << 6) | ((index & 7) << 3) | (base & 7)));
}

int X64Assembler::stackToRBPOffset(int stackSlot) const
{
    // Stack slots are at positive offsets from RBP
    // First slot at [rbp + 16] (after return address and saved RBP)
    // But we store locals BELOW RBP
    return -(stackSlot * 8 + 8);  // First local at [rbp - 8]
}

size_t X64Assembler::resolveLabel(const std::string& name) const
{
    auto it = labels.find(name);
    if (it != labels.end()) return it->second;
    return 0;
}

// ============================================================
// PROLOG / EPILOG
// ============================================================

void X64Assembler::emitProlog(int frameSize)
{
    stackFrameSize = frameSize;

    // push rbp
    emit8(0x55);

    // mov rbp, rsp
    emit8(0x48);
    emit8(0x89);
    emit8(0xE5);

    // sub rsp, frameSize (align to 16 bytes)
    int alignedSize = (frameSize + 15) & ~15;
    if (alignedSize > 0)
    {
        if (alignedSize < 128)
        {
            emit8(0x48);
            emit8(0x83);
            emit8(0xEC);
            emit8(static_cast<uint8_t>(alignedSize));
        }
        else
        {
            emit8(0x48);
            emit8(0x81);
            emit8(0xEC);
            emit32(static_cast<uint32_t>(alignedSize));
        }
    }

    // Save non-volatile registers (RBX, RSI, RDI, R12-R15)
    emitPush(RBX);
    emitPush(RSI);
    emitPush(RDI);
    emitPush(R12);
    emitPush(R13);
    emitPush(R14);
    emitPush(R15);
}

void X64Assembler::emitEpilog()
{
    // Restore non-volatile registers
    emitPop(R15);
    emitPop(R14);
    emitPop(R13);
    emitPop(R12);
    emitPop(RDI);
    emitPop(RSI);
    emitPop(RBX);

    // mov rsp, rbp
    emit8(0x48);
    emit8(0x89);
    emit8(0xEC);

    // pop rbp
    emit8(0x5D);
}

// ============================================================
// MEMORY OPS - STACK
// ============================================================

void X64Assembler::emitLoadStackToReg(int stackSlot, int reg)
{
    // mov reg, [rbp - offset]
    X64Reg r = static_cast<X64Reg>(reg);
    int offset = stackToRBPOffset(stackSlot);

    emitREX(true, false, false, regNeedsREX(r));
    emit8(0x8B);  // mov r/m64, r64
    emitModRM(1, regToRM(r), 5);  // [rbp + disp8]
    emit8(static_cast<uint8_t>(offset & 0xFF));
}

void X64Assembler::emitStoreRegToStack(int reg, int stackSlot)
{
    // mov [rbp - offset], reg
    X64Reg r = static_cast<X64Reg>(reg);
    int offset = stackToRBPOffset(stackSlot);

    emitREX(true, false, false, regNeedsREX(r));
    emit8(0x89);  // mov r64, r/m64
    emitModRM(1, regToRM(r), 5);  // [rbp + disp8]
    emit8(static_cast<uint8_t>(offset & 0xFF));
}

// ============================================================
// CONSTANTS
// ============================================================

void X64Assembler::emitLoadConst(int64_t value, int reg)
{
    emitMovRegImm(reg, value);
}

void X64Assembler::emitLoadConstDouble(double value, int reg)
{
    // For simplicity, load via memory
    // In production, use MOVSD XMM register
    int64_t val = *reinterpret_cast<int64_t*>(&value);
    emitMovRegImm(reg, val);
    // Note: proper double handling would use XMM registers
}

// ============================================================
// MOV INSTRUCTIONS
// ============================================================

void X64Assembler::emitMovRegImm(int reg, int64_t value)
{
    X64Reg r = static_cast<X64Reg>(reg);

    if (value >= -128 && value <= 127)
    {
        // mov reg, imm8 (sign-extended)
        emitREX(true, false, false, regNeedsREX(r));
        emit8(0xC7);
        emitModRM(3, 0, regToRM(r));
        emit8(static_cast<uint8_t>(value & 0xFF));
    }
    else
    {
        // mov reg, imm64
        emitREX(true, false, false, regNeedsREX(r));
        emit8(0xB8 | regToRM(r));
        emit64(static_cast<uint64_t>(value));
    }
}

void X64Assembler::emitMovRegReg(int dst, int src)
{
    X64Reg d = static_cast<X64Reg>(dst);
    X64Reg s = static_cast<X64Reg>(src);

    // mov dst, src
    bool needREX = regNeedsREX(d) || regNeedsREX(s);
    if (needREX) emitREX(true, regNeedsREX(d), false, regNeedsREX(s));
    emit8(0x89);
    emitModRM(3, regToRM(d), regToRM(s));
}

void X64Assembler::emitMovRegStack(int reg, int stackSlot)
{
    emitLoadStackToReg(stackSlot, reg);
}

void X64Assembler::emitMovStackReg(int stackSlot, int reg)
{
    emitStoreRegToStack(reg, stackSlot);
}

void X64Assembler::emitPush(int reg)
{
    X64Reg r = static_cast<X64Reg>(reg);
    if (regNeedsREX(r)) emitREX(false, false, false, true);
    emit8(0x50 | regToRM(r));
}

void X64Assembler::emitPop(int reg)
{
    X64Reg r = static_cast<X64Reg>(reg);
    if (regNeedsREX(r)) emitREX(false, false, false, true);
    emit8(0x58 | regToRM(r));
}

// ============================================================
// ARITHMETIC
// ============================================================

void X64Assembler::emitAdd(int dstReg, int srcReg)
{
    // add dst, src
    X64Reg d = static_cast<X64Reg>(dstReg);
    X64Reg s = static_cast<X64Reg>(srcReg);
    bool needREX = regNeedsREX(d) || regNeedsREX(s);
    if (needREX) emitREX(true, regNeedsREX(d), false, regNeedsREX(s));
    emit8(0x01);
    emitModRM(3, regToRM(d), regToRM(s));
}

void X64Assembler::emitSub(int dstReg, int srcReg)
{
    // sub dst, src
    X64Reg d = static_cast<X64Reg>(dstReg);
    X64Reg s = static_cast<X64Reg>(srcReg);
    bool needREX = regNeedsREX(d) || regNeedsREX(s);
    if (needREX) emitREX(true, regNeedsREX(d), false, regNeedsREX(s));
    emit8(0x29);
    emitModRM(3, regToRM(d), regToRM(s));
}

void X64Assembler::emitMul(int dstReg, int srcReg)
{
    // imul dst, src (dst *= src)
    X64Reg d = static_cast<X64Reg>(dstReg);
    X64Reg s = static_cast<X64Reg>(srcReg);
    bool needREX = regNeedsREX(d) || regNeedsREX(s);
    if (needREX) emitREX(true, regNeedsREX(d), false, regNeedsREX(s));
    emit8(0x0F);
    emit8(0xAF);
    emitModRM(3, regToRM(d), regToRM(s));
}

void X64Assembler::emitSDiv(int dstReg, int srcReg)
{
    // cdq (sign extend RAX to RDX:RAX)
    // idiv src
    // Result in RAX, remainder in RDX
    // mov dst, src first
    X64Reg d = static_cast<X64Reg>(dstReg);
    X64Reg s = static_cast<X64Reg>(srcReg);

    // mov RAX, dst (put dividend in RAX)
    emitMovRegReg(RAX, dstReg);

    // cdq (sign extend RAX -> RDX:RAX)
    emit8(0x48);
    emit8(0x99);

    // idiv src
    bool needREX = regNeedsREX(s);
    if (needREX) emitREX(true, false, false, regNeedsREX(s));
    emit8(0xF7);
    emitModRM(3, 7, regToRM(s));  // /7 opcode for IDIV

    // mov dst, RAX (result)
    emitMovRegReg(dstReg, RAX);
}

void X64Assembler::emitSMod(int dstReg, int srcReg)
{
    // For modulo, result is in RDX after IDIV
    X64Reg d = static_cast<X64Reg>(dstReg);
    X64Reg s = static_cast<X64Reg>(srcReg);

    emitMovRegReg(RAX, dstReg);
    emit8(0x48);
    emit8(0x99);  // cdq

    bool needREX = regNeedsREX(s);
    if (needREX) emitREX(true, false, false, regNeedsREX(s));
    emit8(0xF7);
    emitModRM(3, 7, regToRM(s));  // idiv

    emitMovRegReg(dstReg, RDX);  // remainder is in RDX
}

// ============================================================
// COMPARISON
// ============================================================

void X64Assembler::emitCmp(int reg1, int reg2)
{
    // cmp reg1, reg2
    X64Reg r1 = static_cast<X64Reg>(reg1);
    X64Reg r2 = static_cast<X64Reg>(reg2);
    bool needREX = regNeedsREX(r1) || regNeedsREX(r2);
    if (needREX) emitREX(true, regNeedsREX(r1), false, regNeedsREX(r2));
    emit8(0x39);
    emitModRM(3, regToRM(r1), regToRM(r2));
}

void X64Assembler::emitSetCC(const std::string& condition, int reg)
{
    uint8_t cc = 0;
    if (condition == "e" || condition == "eq") cc = 0x94;     // sete
    else if (condition == "ne" || condition == "neq") cc = 0x95;  // setne
    else if (condition == "l" || condition == "lt") cc = 0x9C;    // setl
    else if (condition == "le") cc = 0x9E;   // setle
    else if (condition == "g" || condition == "gt") cc = 0x9F;    // setg
    else if (condition == "ge") cc = 0x9D;   // setge
    else cc = 0x94;  // default: sete

    X64Reg r = static_cast<X64Reg>(reg);
    if (regNeedsREX(r)) emitREX(false, false, false, regNeedsREX(r));
    emit8(0x0F);
    emit8(cc);
    emitModRM(3, 0, regToRM(r));
}

// ============================================================
// CONTROL FLOW
// ============================================================

void X64Assembler::emitJmp(const std::string& target)
{
    // jmp rel32
    auto it = labels.find(target);
    if (it != labels.end())
    {
        int32_t disp = static_cast<int32_t>(it->second) -
                       static_cast<int32_t>(code.size()) - 5;
        emit8(0xE9);
        emit32(static_cast<uint32_t>(disp));
    }
    else
    {
        // Forward jump, patch later
        emit8(0xE9);
        pendingJumps.push_back({target, code.size()});
        emit32(0);
    }
}

void X64Assembler::emitJmpCC(const std::string& condition, const std::string& target)
{
    uint8_t opcode = 0;
    if (condition == "e" || condition == "eq") opcode = 0x84;     // je rel32
    else if (condition == "ne" || condition == "neq") opcode = 0x85;  // jne
    else if (condition == "l" || condition == "lt") opcode = 0x8C;    // jl
    else if (condition == "le") opcode = 0x8E;   // jle
    else if (condition == "g" || condition == "gt") opcode = 0x8F;    // jg
    else if (condition == "ge") opcode = 0x8D;   // jge
    else opcode = 0x84;  // default: je

    auto it = labels.find(target);
    if (it != labels.end())
    {
        int32_t disp = static_cast<int32_t>(it->second) -
                       static_cast<int32_t>(code.size()) - 6;
        emit8(0x0F);
        emit8(opcode);
        emit32(static_cast<uint32_t>(disp));
    }
    else
    {
        emit8(0x0F);
        emit8(opcode);
        pendingJumps.push_back({target, code.size()});
        emit32(0);
    }
}

void X64Assembler::emitLabel(const std::string& name)
{
    labels[name] = code.size();
}

void X64Assembler::emitCall(const std::string& target)
{
    // call rel32
    auto it = labels.find(target);
    if (it != labels.end())
    {
        int32_t disp = static_cast<int32_t>(it->second) -
                       static_cast<int32_t>(code.size()) - 5;
        emit8(0xE8);
        emit32(static_cast<uint32_t>(disp));
    }
    else
    {
        emit8(0xE8);
        pendingJumps.push_back({target, code.size()});
        emit32(0);
    }
}

void X64Assembler::emitRet()
{
    emit8(0xC3);
}

// ============================================================
// EXTERNAL FUNCTION CALLS
// ============================================================

void X64Assembler::emitNativeCall(void* funcPtr)
{
    // mov rax, funcPtr
    emit8(0x48);
    emit8(0xB8);
    emit64(reinterpret_cast<uintptr_t>(funcPtr));

    // call rax
    emit8(0xFF);
    emit8(0xD0);
}

// ============================================================
// DEBUG
// ============================================================

void X64Assembler::dumpCode() const
{
    printf("Native code (%zu bytes):\n", code.size());
    for (size_t i = 0; i < code.size(); i++)
    {
        printf("%02X ", code[i]);
        if ((i + 1) % 16 == 0) printf("\n");
    }
    printf("\n");
}

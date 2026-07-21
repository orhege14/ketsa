#include "Optimizer.h"
#include <cstdlib>

// ============================================================
// CONSTANT FOLDING
// ============================================================

bool ConstantFoldingPass::run(FunctionProto* proto)
{
    if (!proto) return false;
    bool changed = false;

    for (size_t i = 0; i + 2 < proto->code.size(); i++)
    {
        auto& op1 = proto->code[i].op;
        auto& op2 = proto->code[i + 1].op;
        auto& op3 = proto->code[i + 2].op;

        if (op1 == OpCode::LOAD_CONST && op2 == OpCode::LOAD_CONST)
        {
            uint32_t idx1 = proto->code[i].operand;
            uint32_t idx2 = proto->code[i + 1].operand;

            if (idx1 < proto->constants.size() && idx2 < proto->constants.size())
            {
                auto parseNum = [](const std::string& s) -> int64_t {
                    try { return std::stoll(s); } catch (...) { return 0; }
                };

                int64_t a = parseNum(proto->constants[idx1]);
                int64_t b = parseNum(proto->constants[idx2]);

                if (op3 == OpCode::ADD || op3 == OpCode::SUB ||
                    op3 == OpCode::MUL || op3 == OpCode::DIV)
                {
                    int64_t result = 0;
                    if (op3 == OpCode::ADD) result = a + b;
                    else if (op3 == OpCode::SUB) result = a - b;
                    else if (op3 == OpCode::MUL) result = a * b;
                    else if (op3 == OpCode::DIV) result = b != 0 ? a / b : 0;

                    std::string resultStr = std::to_string(result);
                    uint32_t constIdx = static_cast<uint32_t>(proto->constants.size());
                    proto->constants.push_back(resultStr);

                    proto->code[i].operand = constIdx;
                    proto->code.erase(proto->code.begin() + i + 1);
                    proto->code.erase(proto->code.begin() + i + 1);
                    changed = true;
                }
            }
        }
    }

    return changed;
}

// ============================================================
// DEAD CODE ELIMINATION
// ============================================================

bool DeadCodeEliminationPass::run(FunctionProto* proto)
{
    if (!proto) return false;
    bool changed = false;

    // Remove POP after STORE_VAR
    for (size_t i = 0; i + 1 < proto->code.size(); i++)
    {
        if ((proto->code[i].op == OpCode::STORE_VAR ||
             proto->code[i].op == OpCode::STORE_GLOBAL) &&
            proto->code[i + 1].op == OpCode::POP)
        {
            proto->code.erase(proto->code.begin() + i + 1);
            changed = true;
        }
    }

    // Remove POP after PRINT/PRINTLN (print leaves nothing useful on stack)
    for (size_t i = 0; i + 1 < proto->code.size(); i++)
    {
        if ((proto->code[i].op == OpCode::PRINT ||
             proto->code[i].op == OpCode::PRINTLN) &&
            proto->code[i + 1].op == OpCode::POP)
        {
            proto->code.erase(proto->code.begin() + i + 1);
            changed = true;
        }
    }

    // Remove duplicate POPs
    for (size_t i = 0; i + 1 < proto->code.size(); i++)
    {
        if (proto->code[i].op == OpCode::POP &&
            proto->code[i + 1].op == OpCode::POP)
        {
            proto->code.erase(proto->code.begin() + i);
            changed = true;
        }
    }

    return changed;
}

// ============================================================
// JUMP OPTIMIZATION
// ============================================================

bool JumpOptimizationPass::run(FunctionProto* proto)
{
    if (!proto) return false;
    bool changed = false;

    // Remove jumps to next instruction
    for (size_t i = 0; i < proto->code.size(); i++)
    {
        if (proto->code[i].op == OpCode::JUMP &&
            proto->code[i].operand == i + 1)
        {
            proto->code.erase(proto->code.begin() + i);
            changed = true;
        }
    }

    // Collapse jump chains: JMP to JMP -> JMP to final target
    for (size_t i = 0; i < proto->code.size(); i++)
    {
        if (proto->code[i].op == OpCode::JUMP)
        {
            uint32_t target = proto->code[i].operand;
            if (target < proto->code.size() &&
                proto->code[target].op == OpCode::JUMP)
            {
                proto->code[i].operand = proto->code[target].operand;
                changed = true;
            }
        }
    }

    // Remove JUMP_IF_FALSE followed by POP after condition
    // (condition result not used)
    for (size_t i = 0; i + 1 < proto->code.size(); i++)
    {
        if (proto->code[i].op == OpCode::JUMP_IF_FALSE &&
            proto->code[i + 1].op == OpCode::POP)
        {
            // Keep both: JUMP_IF_FALSE consumes the condition, POP cleans up
            // This is valid but could be optimized further
        }
    }

    return changed;
}

// ============================================================
// PEPPHOLE OPTIMIZATION
// ============================================================

bool PeepholeOptimizationPass::run(FunctionProto* proto)
{
    if (!proto) return false;
    bool changed = false;

    // Replace LOAD_CONST 0 + ADD -> no-op
    for (size_t i = 0; i + 1 < proto->code.size(); i++)
    {
        if (proto->code[i].op == OpCode::LOAD_CONST)
        {
            uint32_t idx = proto->code[i].operand;
            if (idx < proto->constants.size() && proto->constants[idx] == "0")
            {
                if (i + 1 < proto->code.size() && proto->code[i + 1].op == OpCode::ADD)
                {
                    // LOAD_CONST 0, ADD just pops the 0, leave the other value
                    proto->code.erase(proto->code.begin() + i);
                    proto->code.erase(proto->code.begin() + i);
                    changed = true;
                }
            }
        }
    }

    // Replace LOAD_CONST 1 + MUL -> no-op
    for (size_t i = 0; i + 1 < proto->code.size(); i++)
    {
        if (proto->code[i].op == OpCode::LOAD_CONST)
        {
            uint32_t idx = proto->code[i].operand;
            if (idx < proto->constants.size() && proto->constants[idx] == "1")
            {
                if (i + 1 < proto->code.size() && proto->code[i + 1].op == OpCode::MUL)
                {
                    proto->code.erase(proto->code.begin() + i);
                    proto->code.erase(proto->code.begin() + i);
                    changed = true;
                }
            }
        }
    }

    // Replace NOT + JUMP_IF_TRUE -> JUMP_IF_FALSE (inverse)
    for (size_t i = 0; i + 1 < proto->code.size(); i++)
    {
        if (proto->code[i].op == OpCode::NOT &&
            i + 1 < proto->code.size() &&
            proto->code[i + 1].op == OpCode::JUMP_IF_TRUE)
        {
            proto->code[i + 1].op = OpCode::JUMP_IF_FALSE;
            proto->code.erase(proto->code.begin() + i);
            changed = true;
        }
    }

    return changed;
}

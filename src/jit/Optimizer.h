#pragma once

#include "../bytecode/Bytecode.h"
#include "JIT.h"
#include <string>
#include <memory>

// ============================================================
// Constant Folding Optimization
// ============================================================
// Folds constant expressions at compile time.
// Example: LOAD_CONST 1, LOAD_CONST 2, ADD -> LOAD_CONST 3
class ConstantFoldingPass : public OptimizationPass
{
public:
    ConstantFoldingPass() { name = "Constant Folding"; enabled = true; }
    bool run(FunctionProto* proto) override;
};

// ============================================================
// Dead Code Elimination
// ============================================================
// Removes unused instructions.
// Example: STORE_VAR followed by POP -> remove POP
class DeadCodeEliminationPass : public OptimizationPass
{
public:
    DeadCodeEliminationPass() { name = "Dead Code Elimination"; enabled = true; }
    bool run(FunctionProto* proto) override;
};

// ============================================================
// Jump Optimization
// ============================================================
// Removes jumps to the next instruction and redundant branches.
class JumpOptimizationPass : public OptimizationPass
{
public:
    JumpOptimizationPass() { name = "Jump Optimization"; enabled = true; }
    bool run(FunctionProto* proto) override;
};

// ============================================================
// Peephole Optimization
// ============================================================
// Replaces common bytecode patterns with faster equivalents.
class PeepholeOptimizationPass : public OptimizationPass
{
public:
    PeepholeOptimizationPass() { name = "Peephole Optimization"; enabled = true; }
    bool run(FunctionProto* proto) override;
};

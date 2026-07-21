#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <memory>
#include <variant>

// ============================================================
// Bytecode Instruction Set for Ketsa VM
// ============================================================

enum class OpCode : uint8_t
{
    // Constants
    LOAD_CONST,        // Push constant from pool[index]
    LOAD_NULL,         // Push null
    LOAD_TRUE,         // Push true
    LOAD_FALSE,        // Push false

    // Variables
    LOAD_VAR,          // Push value of variable[name_index]
    STORE_VAR,         // Pop and store to variable[name_index]
    LOAD_GLOBAL,       // Push value of global[name_index]
    STORE_GLOBAL,      // Pop and store to global[name_index]
    LOAD_UPVALUE,      // Push upvalue[index] (closure)
    STORE_UPVALUE,     // Pop and store to upvalue[index]

    // Arithmetic
    ADD,               // a + b
    SUB,               // a - b
    MUL,               // a * b
    DIV,               // a / b
    MOD,               // a % b
    POW,               // a ** b
    NEGATE,            // -a

    // Comparison
    EQ,                // a == b
    NE,                // a != b
    LT,                // a < b
    GT,                // a > b
    LE,                // a <= b
    GE,                // a >= b
    CMP,               // a <=> b (three-way compare)

    // Logic
    NOT,               // !a
    AND,               // a && b
    OR,                // a || b

    // String operations
    CONCAT,            // string concat

    // Control flow
    JUMP,              // Unconditional jump to offset
    JUMP_IF_FALSE,     // Pop, jump if false
    JUMP_IF_TRUE,      // Pop, jump if true
    LOOP,              // Loop back to offset

    // Functions
    CALL,              // Call function (arg_count on stack)
    RETURN,            // Return from function
    MAKE_FUNCTION,     // Create function from bytecode at index
    MAKE_CLOSURE,      // Create closure from bytecode at index (with upvalues)
    CLOSE_UPVALUE,     // Close upvalue for local

    // Objects
    MAKE_OBJECT,       // Create empty object
    LOAD_FIELD,        // Push object.field (field_name_index)
    STORE_FIELD,       // Pop value, set object.field
    MAKE_CLASS,        // Create class blueprint

    // Arrays
    MAKE_ARRAY,        // Create array from top n items
    LOAD_INDEX,        // Push array[index]
    STORE_INDEX,       // Pop value, set array[index]
    ARRAY_APPEND,      // array.push(value)
    ARRAY_POP,         // Pop and return last element
    ARRAY_LEN,         // Push array length

    // Built-in calls (fast path)
    BUILTIN_CALL,      // Call builtin by index

    // Modules
    LOAD_MODULE,       // Load module by name
    IMPORT_NAME,       // Import specific names from module

    // Debug / Special
    PRINT,             // Pop and print
    PRINTLN,           // Pop and println
    POP,               // Discard top of stack
    DUP,               // Duplicate top of stack
    SWAP,              // Swap top two stack items
    HALT               // Stop execution
};

struct Instruction
{
    OpCode op;
    union
    {
        uint32_t operand;
        int32_t sOperand;
    };

    Instruction() : op(OpCode::HALT), operand(0) {}
    Instruction(OpCode op, uint32_t operand = 0) : op(op), operand(operand) {}
};

struct UpvalueInfo
{
    uint8_t index;
    bool isLocal;
};

struct FunctionProto
{
    std::string name;
    uint8_t arity;
    std::vector<Instruction> code;
    std::vector<uint32_t> lines;
    std::vector<std::string> constants;
    std::vector<std::string> localNames;
    std::vector<UpvalueInfo> upvalues;
    std::vector<std::unique_ptr<FunctionProto>> innerFunctions;
    std::vector<std::string> globalNames;
    std::vector<std::string> fieldNames;

    FunctionProto(std::string name = "", uint8_t arity = 0)
        : name(std::move(name)), arity(arity) {}
};

// Constant pool helpers
inline std::variant<int64_t, double, std::string> parseConstant(const std::string& val)
{
    return val; // default to string, conversion happens at VM runtime
}

enum class BuiltinIndex : uint8_t
{
    PRINT = 0,
    PRINTLN,
    INPUT,
    TYPE,
    LEN,
    TO_INT,
    TO_FLOAT,
    TO_STRING,
    RANGE,
    MAX_BUILTINS
};

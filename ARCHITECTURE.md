# Ketsa Programming Language - Architecture

## Overview

Ketsa is a multi-paradigm programming language with Python-like syntax, C++ performance characteristics, and Rust-inspired safety features.

## Compiler Pipeline

```
Source Code (.ketsa)
    |
    v
[1] LEXER (Lexer.cpp/.h)
    - Character stream -> Token stream
    - Keywords, identifiers, literals, operators
    - Token types: 500+ defined
    |
    v
[2] PARSER (Parser.cpp/.h)
    - Token stream -> AST (Abstract Syntax Tree)
    - Precedence climbing for expressions
    - Recursive descent for statements
    - Error recovery with synchronize()
    |
    v
[3] TYPE CHECKER (TypeChecker.cpp/.h)  [NEW]
    - AST -> Typed AST
    - Type inference & unification
    - Generic resolution
    - Interface/trait validation
    - Compile-time error detection
    |
    v
[4] BYTECODE COMPILER (bytecode/Compiler.cpp/.h)  [NEW]
    - Typed AST -> Bytecode
    - Stack-based instruction generation
    - Constant pool management
    - Closure compilation
    - Optimization passes
    |
    v
[5] VIRTUAL MACHINE (bytecode/VM.cpp/.h)  [NEW]
    - Bytecode -> Execution
    - Stack-based VM with registers
    - JIT compilation ready (future)
    - Garbage collection integration
    |
    v
[6] RUNTIME (Interpreter.cpp/.h)
    - Legacy tree-walking interpreter
    - Module loading
    - Built-in functions
    - Bridge to VM (hybrid mode)
```

## Phase 1: Type Checker

### Purpose
The type checker validates semantic correctness before execution, catching:
- Type mismatches at compile time
- Undefined variables/functions
- Invalid operations
- Wrong argument counts/types

### Design
- Single pass with symbol table
- Type inference for untyped declarations
- Structural typing for objects
- Generic type parameter resolution

### File Changes
- NEW: `src/TypeChecker.h` - Type checker interface
- NEW: `src/TypeChecker.cpp` - Type checker implementation  
- MODIFIED: `src/main.cpp` - Add type checking phase
- MODIFIED: `CMakeLists.txt` - Add new source files

## Phase 2: Bytecode VM

### Purpose
Replace the tree-walking interpreter with a stack-based VM for 10-50x performance improvement.

### Instruction Set (64 instructions)
- Constants: LOAD_CONST
- Variables: LOAD_VAR, STORE_VAR, LOAD_GLOBAL, STORE_GLOBAL
- Arithmetic: ADD, SUB, MUL, DIV, MOD, POW
- Comparison: EQ, NE, LT, GT, LE, GE
- Logic: AND, OR, NOT
- Control: JUMP, JUMP_IF_FALSE, JUMP_IF_TRUE
- Functions: CALL, RETURN, MAKE_FUNCTION, MAKE_CLOSURE
- Objects: MAKE_OBJECT, LOAD_FIELD, STORE_FIELD
- Arrays: MAKE_ARRAY, LOAD_INDEX, STORE_INDEX
- Modules: LOAD_MODULE, IMPORT_NAME

### Performance Impact
- Tree-walking: O(AST depth) per operation, virtual dispatch overhead
- Bytecode VM: O(1) dispatch via jump table, linear instruction stream
- Expected: 5-10x faster for computation, 10-50x for tight loops

### File Changes
- NEW: `src/bytecode/Bytecode.h` - Instruction definitions
- NEW: `src/bytecode/Compiler.h` - Bytecode compiler interface
- NEW: `src/bytecode/Compiler.cpp` - Bytecode compiler implementation
- NEW: `src/bytecode/VM.h` - Virtual machine interface
- NEW: `src/bytecode/VM.cpp` - Virtual machine implementation
- MODIFIED: `CMakeLists.txt` - Add new source files

## Phase 3: Advanced Features

### Pattern Matching
- `match` expression with pattern types:
  - Literal patterns
  - Variable patterns (binding)
  - Wildcard patterns (_)
  - Destructuring (arrays, objects)
  - Guard conditions

### Generics
- `<T>` syntax for generic functions/classes
- Type parameter bounds with interfaces
- Monomorphization at compile time

### Interface & Trait System
- `interface` defines method signatures
- `trait` defines method implementations
- `implements` keyword for implementation
- Multiple inheritance alternative

### Async/Await
- Built-in async runtime
- Green threads / coroutines
- Channel-based communication
- Actor model support

## Directory Structure

```
src/
├── main.cpp                 # Entry point
├── Lexer.h/.cpp            # Lexer
├── Parser.h/.cpp           # Parser
├── AST.h                   # AST node definitions
├── TypeChecker.h/.cpp      # Type checker [NEW]
├── Interpreter.h/.cpp      # Legacy interpreter
├── errors/
│   ├── Error.h/.cpp        # Error system
├── runtime/
│   ├── Environment.h/.cpp  # Variable scope
│   ├── values/             # Runtime value types
│   │   ├── Value.h/.cpp
│   │   ├── NumberValue.h/.cpp
│   │   ├── FloatValue.h/.cpp
│   │   ├── StringValue.h/.cpp
│   │   ├── BooleanValue.h/.cpp
│   │   ├── CharValue.h/.cpp
│   │   ├── NullValue.h/.cpp
│   │   ├── ArrayValue.h/.cpp
│   │   ├── ObjectValue.h/.cpp
│   │   ├── FunctionValue.h/.cpp
│   │   ├── BuiltinFunctionValue.h/.cpp
│   │   ├── ClassValue.h/.cpp
│   │   └── ModuleValue.h/.cpp
├── bytecode/               # Bytecode VM [NEW]
│   ├── Bytecode.h
│   ├── Compiler.h/.cpp
│   └── VM.h/.cpp
```

## Design Decisions

1. **C++20**: Leverage modern C++ features (smart pointers, variant, optional)
2. **No external dependencies**: Self-contained compiler
3. **Incremental compilation**: Module-level caching
4. **Error recovery**: Continue parsing after errors for better diagnostics
5. **Hybrid execution**: Tree-walking for top-level, bytecode for functions

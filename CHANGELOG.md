# Changelog

## [0.5.0] - 2026-07-21

### Added

#### JIT Compiler Engine

* Added Just-In-Time (JIT) compilation system
* Added runtime profiling infrastructure
* Added hot function detection for frequently executed code
* Added JIT compilation pipeline:

  * Bytecode analysis
  * Optimization passes
  * Native code generation
  * Runtime execution integration
* Added `--jit` execution mode

#### Native Code Generation

* Added x64 native code generator
* Added x64 assembler backend
* Added Windows x64 calling convention support
* Added register management system
* Added native instruction generation:

  * Move operations
  * Arithmetic operations
  * Comparison operations
  * Branching
  * Function calls
  * Returns

#### Bytecode Optimizer

* Added bytecode optimization pipeline
* Added constant folding optimization
* Added dead code elimination
* Added jump optimization
* Added peephole optimization

#### Runtime Improvements

* Added JIT + VM integration
* Added optimized execution path for hot functions
* Added native compiler detection

#### Developer Experience

* Updated `ketsa doctor` with JIT diagnostics:

  * JIT Engine check
  * JIT Backend check
  * Native Compiler check
* Improved compiler architecture documentation
* Added JIT benchmark examples

---

## [0.3.0] - 2026-07-21

### Added

#### Compiler Features

* Added Type Checker for compile-time error detection
* Added Bytecode Compiler (AST → Bytecode, 64+ instructions)
* Added stack-based Virtual Machine
* Added faster bytecode execution pipeline

#### Language Features

* Added Pattern Matching:

  * Literal patterns
  * Variable patterns
  * Wildcard patterns
  * Destructuring patterns

#### Developer Tools

* Added professional CLI system:

  * `run`
  * `build`
  * `install`
  * `doctor`
  * `package`
  * `fmt`
  * `test`
  * `new`

* Added System Doctor:

  * Compiler diagnostics
  * Build checks
  * Environment checks

* Added project scaffolding:

  * `ketsa new <project>`

* Added package configuration:

  * `ketsa.toml`

* Added cross-platform build system:

  * `build.bat`
  * `build.sh`

* Added comprehensive test suite:

  * `examples/comprehensive.ketsa`

* Added compiler architecture documentation:

  * `ARCHITECTURE.md`

### Fixed

* Fixed compound assignment operators:

  * `+=`
  * `-=`
  * `*=`
  * `/=`

* Fixed variable names conflicting with keywords:

  * `pi`
  * `version`

* Fixed expression statement execution

* Fixed match parser infinite loop with `else`

* Fixed MSVC C++20 compatibility issues

* Fixed TypeInfo copy/move semantics

* Fixed class method ownership model

---

## [0.2.0] - 2026-06-15

### Added

* Added function declarations with:

  * Parameters
  * Return types

* Added class system:

  * Classes
  * Methods
  * Fields

* Added arrays and index access

* Added object literals

* Added member access system

### Language Features

* Added control flow:

  * if/else
  * while
  * for
  * switch

* Added built-in functions:

  * `print`
  * `println`
  * `input`
  * `type`
  * `len`
  * `toString`
  * `range`

* Added module/import system

* Added string interpolation support

* Added structured error system with ErrorCode enums

### Fixed

* Fixed binary expression evaluation
* Fixed closure scope management
* Fixed string concatenation in print

---

## [0.1.0] - 2026-05-01

### Added

* Initial compiler architecture:

  * Lexer
  * Parser
  * AST
  * Interpreter

* Added variables:

  * `let`
  * `const`

* Added primitive data types:

  * int
  * float
  * string
  * bool

* Added arithmetic operators:

  * `+`
  * `-`
  * `*`
  * `/`
  * `%`
  * `**`

* Added comparison operators

* Added print statement

* Added C++20 CMake build system

* Added Windows MSVC support

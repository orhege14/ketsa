# Changelog

## [0.3.0] - 2026-07-21

### Added
- Type Checker (semantic analysis) for compile-time error detection
- Bytecode Compiler (AST → bytecode, 64+ instructions)
- Stack-based Virtual Machine for faster execution
- Pattern Matching: `match` expression with literal, variable, wildcard, and destructuring patterns
- Professional CLI with subcommands: `run`, `build`, `install`, `doctor`, `package`, `fmt`, `test`, `new`
- System Doctor: `ketsa doctor` for diagnostics
- Project scaffolding: `ketsa new <project>`
- Package format: `ketsa.toml` for project metadata
- Build system: `build.bat` (Windows), `build.sh` (Unix)
- Comprehensive test suite in `examples/comprehensive.ketsa`
- Architecture documentation (`ARCHITECTURE.md`)

### Fixed
- Compound assignment operators (`+=`, `-=`, `*=`, `/=`)
- Variable names that clash with keywords (`pi`, `version`)
- Expression statement execution for function calls and assignments
- Match parser infinite loop with `else` keyword
- MSVC C++20 build compatibility
- TypeInfo copy/move semantics
- Class method ownership model

## [0.2.0] - 2026-06-15

### Added
- Function declarations with parameters and return types
- Class declarations with methods and fields
- Array literals and index access
- Object literals and member access
- Control flow: if/else, while, for, switch
- Built-in functions: print, println, input, type, len, toString, range
- Module/import system
- String interpolation support in lexer
- Error system with ErrorCode enums

### Fixed
- Binary expression evaluation for all numeric types
- Scope management for closures
- String concatenation in print

## [0.1.0] - 2026-05-01

### Added
- Initial compiler structure (Lexer, Parser, AST, Interpreter)
- Variables: `let`, `const`
- Data types: int, float, string, bool
- Arithmetic operators: `+`, `-`, `*`, `/`, `%`, `**`
- Comparison operators
- `print` statement
- C++20 CMake build system
- Windows MSVC support

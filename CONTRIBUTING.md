# Contributing to Ketsa

## Development Setup

```bash
git clone https://github.com/ketsa-lang/ketsa
cd ketsa
build.bat            # Windows
./build.sh           # Linux/macOS
```

## Project Structure

```
src/
├── main.cpp              # Entry point
├── CLI.h/.cpp            # Command-line interface
├── Lexer.h/.cpp          # Lexer (tokenizer)
├── Parser.h/.cpp         # Parser (AST builder)
├── AST.h                 # Abstract Syntax Tree nodes
├── TypeChecker.h/.cpp    # Semantic analysis
├── Interpreter.h/.cpp    # Tree-walking interpreter
├── errors/
│   ├── Error.h/.cpp      # Error reporting system
├── runtime/
│   ├── Environment.h/.cpp # Variable scoping
│   └── values/           # Runtime value types
├── bytecode/
│   ├── Bytecode.h        # Instruction set
│   ├── Compiler.h/.cpp   # Bytecode compiler
│   └── VM.h/.cpp         # Virtual machine
examples/                 # Example programs
```

## Building

```bash
# Debug build (default)
./build.sh

# Release build
./build.sh --release

# Clean build
./build.sh --clean

# Build and test
./build.sh --test
```

## Testing

```bash
ketsa test                          # Run all tests
ketsa run examples/comprehensive.ketsa
ketsa run --check examples/comprehensive.ketsa
```

## Code Style

- Modern C++20
- 4-space indentation
- PascalCase for types, camelCase for functions/variables
- No external dependencies
- RAII for resource management
- Strong error handling with ErrorCode enums

## Pull Request Process

1. Fork the repository
2. Create a feature branch
3. Write tests for new features
4. Ensure all tests pass
5. Submit a pull request

## Adding a Feature

Each feature requires:

1. Lexer tokens (if new keywords)
2. Parser rules
3. AST nodes if needed
4. Type checker validation
5. Interpreter/VM implementation
6. Tests in examples/

## Reporting Issues

- Bug reports: include the Ketsa code that fails
- Feature requests: describe the use case
- Performance issues: include benchmark code

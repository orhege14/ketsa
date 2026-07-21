# Ketsa Roadmap

## v0.3 — Current (Type Checker + Bytecode VM)
- ✅ Lexer and Parser
- ✅ Type Checker (semantic analysis)
- ✅ Bytecode Compiler and Virtual Machine
- ✅ Pattern Matching
- ✅ Professional CLI and build system
- ✅ System Doctor diagnostics
- ✅ Project scaffolding (`ketsa new`)

## v0.4 — Generics & Interfaces
- Generic functions and types (`<T>` syntax)
- Interface and Trait system
- Compile-time type parameter resolution
- Monomorphization

## v0.5 — Async Runtime
- Green threads / coroutines
- Async/await syntax
- Channel-based communication
- Actor model support
- Built-in async I/O

## v0.6 — Standard Library
- Complete std library with:
  - File system operations
  - JSON parser/generator
  - HTTP client/server
  - Date/time handling
  - Regex engine
  - Cryptography primitives
  - Compression (gzip, zlib)
  - Database connectivity

## v0.7 — Package Manager (kpm)
- Package registry
- Dependency resolution
- Lock files
- Version management
- Package publishing

## v0.8 — Tooling
- Code formatter (`ketsa fmt`)
- Linter (`ketsa lint`)
- Debugger (`ketsa debug`)
- Profiler (`ketsa profile`)
- Language Server Protocol (`ketsa-language-server`)
- VS Code extension

## v0.9 — JIT Compilation
- LLVM backend integration
- JIT compiler for hot paths
- Native code generation
- AOT compilation for standalone binaries
- Cross-compilation support

## v1.0 — Stable Release
- Complete language specification
- Stable standard library API
- Package manager v1.0
- Full IDE support
- Performance benchmarks vs Python, Go, Rust
- Cross-platform binary releases

## Long-term Goals

### IDE Integration
- VS Code extension with syntax highlighting, autocomplete, debugging
- JetBrains plugin
- Vim/Neovim plugin

### Platform Support
- Windows (x64, ARM64)
- Linux (x64, ARM64)
- macOS (x64, ARM64/Apple Silicon)
- WebAssembly target

### Ecosystem
- Package registry (ketsa.dev)
- Documentation generator
- Web framework
- Game development framework
- AI/ML framework

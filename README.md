# Ketsa Programming Language

> Python'un kolaylığı, C++'ın hızı, Rust'ın güvenliği.

Ketsa is a modern general-purpose programming language that combines Python's elegant syntax with C++-level performance and Rust-inspired safety features.

```ketsa
func fibonacci(n: int): int {
    if (n <= 1) return n
    return fibonacci(n - 1) + fibonacci(n - 2)
}

print fibonacci(10)  // 55
```

## Features

- **Python-compatible syntax** with modern improvements
- **Type-safe**: optional static typing with compile-time type checking
- **High performance**: bytecode VM + tree-walking interpreter
- **Pattern matching**: `match` with destructuring and guards
- **Memory safe**: automatic memory management with ownership semantics
- **Concurrent**: async/await, channels, actors (coming in v0.5)
- **Cross-platform**: Windows, Linux, macOS

## Quick Start

### Install

```bash
git clone https://github.com/ketsa-lang/ketsa
cd ketsa
build.bat           # Windows
# or
chmod +x build.sh && ./build.sh  # Linux/macOS
```

### Run

```bash
ketsa run examples/comprehensive.ketsa
ketsa --version
ketsa doctor         # System diagnostics
ketsa new myapp      # Create a new project
```

## Language Overview

### Variables
```ketsa
let name = "Ketsa"       # Type-inferred string
let count: int = 42      # Explicit type annotation
const PI = 3.14159       # Immutable constant
```

### Functions
```ketsa
func greet(name: string): string {
    return "Hello, " + name + "!"
}
```

### Control Flow
```ketsa
if (score >= 90) {
    print "A"
} else if (score >= 80) {
    print "B"
} else {
    print "C"
}

while (count < 10) {
    print count
    count += 1
}

for (i in [1, 2, 3]) {
    print i
}
```

### Pattern Matching
```ketsa
match (value) {
    case 1:
        print "one"
    case 2:
        print "two"
    else:
        print "other"
}
```

## Architecture

```
Source → Lexer → Parser → Type Checker → Bytecode Compiler → VM / Interpreter → Output
```

| Component | Description |
|-----------|-------------|
| Lexer | Tokenizes source into 500+ token types |
| Parser | Builds AST with precedence climbing |
| Type Checker | Static semantic analysis |
| Bytecode Compiler | AST → 64+ instruction bytecode |
| VM | Stack-based virtual machine |
| Interpreter | Tree-walking fallback |

## Project Status

| Phase | Feature | Status |
|-------|---------|--------|
| 0.3 | Type Checker + Bytecode VM | ✅ |
| 0.4 | Generics & Interfaces | 🔄 |
| 0.5 | Async Runtime | 📋 |
| 0.6 | Standard Library | 📋 |
| 0.7 | Package Manager | 📋 |
| 0.8 | Tooling & IDE Support | 📋 |
| 0.9 | JIT Compilation | 📋 |
| 1.0 | Stable Release | 📋 |

## License

Open source. MIT license.

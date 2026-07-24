# Ketsa Programming Language

> Python's simplicity, C++'s performance, Rust-inspired safety.

Ketsa is a modern general-purpose programming language designed to combine elegant syntax, high performance, strong typing, and modern tooling.

📚 **Wiki:** [`docs/wiki/`](docs/wiki/Home.md) — Full documentation, language reference, architecture guide, and examples.

```ketsa
func fibonacci(n: int): int {
    if (n <= 1) return n
    return fibonacci(n - 1) + fibonacci(n - 2)
}

print fibonacci(10)  // 55
```

---

# Features

- **Modern syntax** inspired by Python and C-like languages
- **Static type checking** with compile-time error detection
- **High performance runtime**
  - Bytecode Virtual Machine
  - Tree-walking Interpreter
  - JIT compilation infrastructure
- **Pattern matching** with literal, variable, wildcard, and destructuring patterns
- **Memory safety concepts** with managed runtime features
- **Cross-platform architecture**
  - Windows
  - Linux
  - macOS
- **Modern developer tooling**
  - CLI tools
  - Project creation
  - Diagnostics
  - VS Code integration

---

# Quick Start

## Build

```bash
git clone https://github.com/orhege14/ketsa
cd ketsa

# Windows
build.bat

# Linux/macOS
chmod +x build.sh
./build.sh
```

---

# Running Ketsa

Run a file:

```bash
ketsa run examples/comprehensive.ketsa
```

Check version:

```bash
ketsa --version
```

System diagnostics:

```bash
ketsa doctor
```

Create a project:

```bash
ketsa new myapp
```

---

# VS Code Extension

Ketsa provides official Visual Studio Code support.

Location:

```
editors/vscode
```

## Features

- `.ketsa` file support
- Syntax highlighting
- Ketsa file icons
- Run Ketsa button
- Integrated Ketsa Output panel
- Automatic compiler detection

## Usage

1. Install the Ketsa VS Code extension.
2. Open a `.ketsa` file.
3. Press **Run Ketsa**.
4. View output in the **Ketsa Output** panel.

Example:

```ketsa
print("Hello Ketsa")
```

---

# Language Overview

## Variables

```ketsa
let name = "Ketsa"

let count: int = 42

const PI = 3.14159
```

---

## Functions

```ketsa
func greet(name: string): string {
    return "Hello, " + name + "!"
}
```

---

## Control Flow

```ketsa
if (score >= 90) {
    print "A"
} else if (score >= 80) {
    print "B"
} else {
    print "C"
}
```

### While Loop

```ketsa
while (count < 10) {
    print count
    count += 1
}
```

### For Loop

```ketsa
for (i in [1, 2, 3]) {
    print i
}
```

---

# Pattern Matching

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

---

# Architecture

```
Source Code

    ↓

Lexer

    ↓

Parser

    ↓

Type Checker

    ↓

Bytecode Compiler

    ↓

Bytecode VM

    ↓

JIT Engine

    ↓

Native x64 Code
```

---

# Runtime Components

| Component | Description |
|---|---|
| Lexer | Converts source code into tokens |
| Parser | Builds Abstract Syntax Tree |
| Type Checker | Performs compile-time semantic analysis |
| Bytecode Compiler | Converts AST into bytecode instructions |
| Virtual Machine | Stack-based bytecode execution engine |
| Interpreter | Tree-walking execution fallback |
| JIT Engine | Generates optimized native x64 code |

---

# JIT Compilation

Ketsa includes a JIT compilation infrastructure.

Current capabilities:

- x64 native code generation
- JIT backend
- Runtime compilation support
- VM integration

Future improvements:

- Hot code detection
- Function inlining
- Register allocation
- Runtime profiling
- Advanced optimizations

---

# Project Structure

```
ketsa/

├── src/
│   ├── compiler
│   ├── runtime
│   ├── vm
│   └── jit
│
├── examples/
│
├── editors/
│   └── vscode/
│       ├── syntax highlighting
│       ├── file icons
│       └── run integration
│
├── CMakeLists.txt
└── README.md
```

---

# System Doctor

Ketsa includes a built-in diagnostic system:

```bash
ketsa doctor
```

Checks:

- Compiler availability
- Build system
- C++ environment
- Architecture
- JIT availability
- Native compiler backend

---

# Project Status

| Version | Feature | Status |
|---|---|---|
| 0.1 | Lexer, Parser, AST, Interpreter | ✅ |
| 0.2 | Functions, Classes, Modules, Built-ins | ✅ |
| 0.3 | Type Checker + Bytecode VM | ✅ |
| 0.4 | Generics & Interfaces | 🔄 |
| 0.5 | Async Runtime | 📋 |
| 0.6 | Standard Library | 📋 |
| 0.7 | Package Manager | 📋 |
| 0.8 | Advanced Tooling | 📋 |
| 0.9 | Advanced JIT Optimization | 📋 |
| 1.0 | Stable Release | 📋 |

---

# License

Open source under the MIT License.
# Ketsa Programming Language

Ketsa is a custom programming language and interpreter written in **C++**.

The goal of Ketsa is to create a simple, powerful and extensible programming language with its own syntax, lexer, parser, AST system and interpreter.

## Features

Current features:

- Lexer system
- Parser system
- AST (Abstract Syntax Tree)
- Interpreter runtime
- Variables
- Numbers
- Strings
- Booleans
- Print statement
- Binary expressions
- Compound assignment operators

## Example

Ketsa code:

```ketsa
let age = 20

age *= 10

print age
```

Output:

```
200
```

---

## How Ketsa Works

Ketsa follows a compiler/interpreter pipeline:

```
Source Code
     |
     v
Lexer
     |
     v
Tokens
     |
     v
Parser
     |
     v
AST
     |
     v
Interpreter
     |
     v
Output
```

### Lexer

The lexer reads the source code and converts it into tokens.

Example:

```ketsa
let age = 10
```

becomes:

```
LET
IDENTIFIER(age)
EQUAL
NUMBER(10)
```

---

### Parser

The parser converts tokens into an Abstract Syntax Tree (AST).

Example:

```ketsa
age *= 10
```

becomes:

```
BinaryExpressionNode

Operator:
    *=

Left:
    VariableAccessNode
        age

Right:
    NumberNode
        10
```

---

### Interpreter

The interpreter executes the AST.

Example:

```
age = 20

age *= 10

age = 200
```

---

# Project Structure

```
Ketsa Compiler/

├── main.cpp
│
├── Lexer.h
├── Lexer.cpp
│
├── Parser.h
├── Parser.cpp
│
├── AST.h
│
├── Value.h
│
├── Interpreter.h
├── Interpreter.cpp
│
└── test.ketsa
```

---

# Building

## Windows (MSVC)

Compile:

```bash
cl main.cpp Lexer.cpp Parser.cpp Interpreter.cpp
```

Run:

```bash
main.exe test.ketsa
```

---

# Syntax

## Variables

```ketsa
let name = "Ege"
let age = 12
```

---

## Print

```ketsa
print "Hello"
```

or:

```ketsa
let age = 20
print age
```

---

## Operators

Supported:

```
+
-
*
/
%
```

Assignment:

```
=
+=
-=
*=
/=
```

Special operators:

```
?+=
?-= 
?*= 
?**=
```

---

# Roadmap

Future plans:

- [ ] Better expression parser
- [ ] Functions
- [ ] If / Else statements
- [ ] Loops
- [ ] Arrays
- [ ] Objects
- [ ] Modules
- [ ] Error reporting
- [ ] Bytecode compiler
- [ ] Virtual Machine

---

# Language Goals

Ketsa aims to be:

- Simple like Python
- Fast like C++
- Structured like modern languages
- Easy to extend

---

# License

Open source project.

Created with C++.

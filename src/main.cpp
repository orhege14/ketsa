#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include "Lexer.h"
#include "Parser.h"
#include "TypeChecker.h"
#include "Interpreter.h"
#include "bytecode/Compiler.h"
#include "bytecode/VM.h"
#include "errors/Error.h"

static void printUsage()
{
    std::cout << "Ketsa Programming Language v0.3\n";
    std::cout << "Usage: ketsa <file.ketsa> [options]\n";
    std::cout << "Options:\n";
    std::cout << "  --help        Show this help\n";
    std::cout << "  --version     Show version\n";
    std::cout << "  --check       Only check syntax & types (no execution)\n";
    std::cout << "  --ast         Print AST before execution\n";
    std::cout << "  --tokens      Print tokens before execution\n";
    std::cout << "  --bytecode    Print bytecode before execution\n";
    std::cout << "  --vm          Use bytecode VM instead of interpreter\n";
    std::cout << "  --no-typecheck  Skip type checking\n";
}

static void printVersion()
{
    std::cout << "Ketsa v0.3.0\n";
    std::cout << "Type Checker + Bytecode VM\n";
}

static void printBytecode(const std::vector<std::unique_ptr<FunctionProto>>& funcs, int depth = 0)
{
    for (const auto& func : funcs)
    {
        std::string indent(depth * 2, ' ');
        std::cout << indent << "=== Function: " << func->name
                  << " (arity=" << static_cast<int>(func->arity)
                  << ", code=" << func->code.size() << " instrs) ===\n";

        for (size_t i = 0; i < func->code.size(); i++)
        {
            auto& instr = func->code[i];
            std::cout << indent << "  " << i << ": ";

            switch (instr.op)
            {
                case OpCode::LOAD_CONST: std::cout << "LOAD_CONST " << instr.operand; break;
                case OpCode::LOAD_NULL: std::cout << "LOAD_NULL"; break;
                case OpCode::LOAD_TRUE: std::cout << "LOAD_TRUE"; break;
                case OpCode::LOAD_FALSE: std::cout << "LOAD_FALSE"; break;
                case OpCode::LOAD_VAR: std::cout << "LOAD_VAR " << instr.operand; break;
                case OpCode::STORE_VAR: std::cout << "STORE_VAR " << instr.operand; break;
                case OpCode::LOAD_GLOBAL: std::cout << "LOAD_GLOBAL " << instr.operand; break;
                case OpCode::STORE_GLOBAL: std::cout << "STORE_GLOBAL " << instr.operand; break;
                case OpCode::ADD: std::cout << "ADD"; break;
                case OpCode::SUB: std::cout << "SUB"; break;
                case OpCode::MUL: std::cout << "MUL"; break;
                case OpCode::DIV: std::cout << "DIV"; break;
                case OpCode::MOD: std::cout << "MOD"; break;
                case OpCode::EQ: std::cout << "EQ"; break;
                case OpCode::NE: std::cout << "NE"; break;
                case OpCode::LT: std::cout << "LT"; break;
                case OpCode::GT: std::cout << "GT"; break;
                case OpCode::JUMP: std::cout << "JUMP -> " << instr.operand; break;
                case OpCode::JUMP_IF_FALSE: std::cout << "JUMP_IF_FALSE -> " << instr.operand; break;
                case OpCode::JUMP_IF_TRUE: std::cout << "JUMP_IF_TRUE -> " << instr.operand; break;
                case OpCode::LOOP: std::cout << "LOOP -> " << instr.operand; break;
                case OpCode::CALL: std::cout << "CALL " << instr.operand; break;
                case OpCode::RETURN: std::cout << "RETURN"; break;
                case OpCode::MAKE_FUNCTION: std::cout << "MAKE_FUNCTION " << instr.operand; break;
                case OpCode::MAKE_OBJECT: std::cout << "MAKE_OBJECT"; break;
                case OpCode::LOAD_FIELD: std::cout << "LOAD_FIELD " << instr.operand; break;
                case OpCode::MAKE_ARRAY: std::cout << "MAKE_ARRAY " << instr.operand; break;
                case OpCode::LOAD_INDEX: std::cout << "LOAD_INDEX"; break;
                case OpCode::PRINT: std::cout << "PRINT"; break;
                case OpCode::PRINTLN: std::cout << "PRINTLN"; break;
                case OpCode::POP: std::cout << "POP"; break;
                case OpCode::DUP: std::cout << "DUP"; break;
                case OpCode::LOAD_UPVALUE: std::cout << "LOAD_UPVALUE " << instr.operand; break;
                case OpCode::CLOSE_UPVALUE: std::cout << "CLOSE_UPVALUE"; break;
                case OpCode::BUILTIN_CALL: std::cout << "BUILTIN_CALL " << instr.operand; break;
                case OpCode::HALT: std::cout << "HALT"; break;
                default: std::cout << "OP_" << static_cast<int>(instr.op); break;
            }
            std::cout << "\n";
        }

        printBytecode(func->innerFunctions, depth + 1);
    }
}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        printUsage();
        return 1;
    }

    std::string filename;
    bool checkOnly = false;
    bool printAst = false;
    bool printTokens = false;
    bool printBytecodeFlag = false;
    bool useVm = false;
    bool noTypeCheck = false;

    // Parse arguments
    for (int i = 1; i < argc; i++)
    {
        std::string arg = argv[i];
        if (arg == "--help") { printUsage(); return 0; }
        else if (arg == "--version") { printVersion(); return 0; }
        else if (arg == "--check") { checkOnly = true; }
        else if (arg == "--ast") { printAst = true; }
        else if (arg == "--tokens") { printTokens = true; }
        else if (arg == "--bytecode") { printBytecodeFlag = true; }
        else if (arg == "--vm") { useVm = true; }
        else if (arg == "--no-typecheck") { noTypeCheck = true; }
        else if (arg[0] != '-') { filename = arg; }
    }

    if (filename.empty())
    {
        std::cerr << "Error: No input file specified.\n";
        printUsage();
        return 1;
    }

    std::ifstream file(filename);
    if (!file)
    {
        std::cerr << "Error: Cannot open file '" << filename << "'\n";
        return 1;
    }

    std::string source(
        (std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>()
    );

    ErrorHandler errorHandler;
    errorHandler.setSource(source);
    errorHandler.setFilename(filename);

    // === LEXER ===
    Lexer lexer(source, &errorHandler);
    auto tokens = lexer.tokenize();

    if (printTokens)
    {
        std::cout << "=== Tokens ===\n";
        for (const auto& token : tokens)
        {
            std::cout << "  " << static_cast<int>(token.type)
                      << " [" << token.value << "]"
                      << " at " << token.line << ":" << token.column << "\n";
        }
        std::cout << "\n";
    }

    if (errorHandler.hasErrors())
    {
        errorHandler.printAll();
        errorHandler.printSummary();
        return 1;
    }

    // === PARSER ===
    Parser parser(tokens, &errorHandler);
    auto ast = parser.parse();

    if (errorHandler.hasErrors())
    {
        errorHandler.printAll();
        errorHandler.printSummary();
        return 1;
    }

    if (printAst)
    {
        std::cout << "=== AST ===\n";
        std::cout << "Parsed " << ast.size() << " top-level statements\n\n";
    }

    // === TYPE CHECKER (Static Analysis) ===
    if (!noTypeCheck)
    {
        TypeChecker typeChecker(&errorHandler);
        typeChecker.check(ast);

        if (errorHandler.hasErrors())
        {
            errorHandler.printAll();
            errorHandler.printSummary();
            return 1;
        }
    }

    if (checkOnly)
    {
        if (errorHandler.hasWarnings())
        {
            errorHandler.printAll();
        }
        std::cout << "Syntax and type checking OK\n";
        return 0;
    }

    // === BYTECODE COMPILER (Optional) ===
    std::unique_ptr<FunctionProto> mainFunc;
    if (useVm || printBytecodeFlag)
    {
        Compiler compiler(&errorHandler);
        mainFunc = compiler.compile(ast);

        if (printBytecodeFlag && mainFunc)
        {
            std::vector<std::unique_ptr<FunctionProto>> funcs;
            funcs.push_back(std::move(mainFunc));
            std::cout << "=== Bytecode ===\n";
            printBytecode(funcs);
            mainFunc = std::move(funcs[0]);
        }
    }

    // === VM EXECUTION (if --vm) ===
    if (useVm && mainFunc)
    {
        VM vm(&errorHandler);
        auto result = vm.interpret(mainFunc.get());

        if (result == InterpretResult::RUNTIME_ERROR)
        {
            errorHandler.printAll();
            errorHandler.printSummary();
            return 1;
        }
        return 0;
    }

    // === INTERPRETER (Tree-walking, default) ===
    Interpreter interpreter(&errorHandler);

    try
    {
        interpreter.executeProgram(ast);
    }
    catch (const std::exception& e)
    {
        std::cerr << "Runtime error: " << e.what() << "\n";
        return 1;
    }
    catch (...)
    {
        std::cerr << "Unknown runtime error\n";
        return 1;
    }

    if (errorHandler.hasErrors())
    {
        errorHandler.printAll();
        errorHandler.printSummary();
        return 1;
    }

    if (errorHandler.hasWarnings())
    {
        errorHandler.printAll();
    }

    return 0;
}

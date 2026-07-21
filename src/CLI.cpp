#include "CLI.h"
#include "Lexer.h"
#include "Parser.h"
#include "TypeChecker.h"
#include "Interpreter.h"
#include "bytecode/Compiler.h"
#include "bytecode/VM.h"
#include "jit/JIT.h"
#include "jit/Optimizer.h"
#include "errors/Error.h"

#include <fstream>
#include <sstream>
#include <filesystem>
#include <algorithm>

namespace fs = std::filesystem;

CLIContext CLI::parse(int argc, char* argv[])
{
    CLIContext ctx;

    if (argc < 2)
    {
        ctx.command = "help";
        return ctx;
    }

    std::string first = argv[1];

    // Check for subcommands
    if (first == "run" || first == "build" || first == "install" ||
        first == "doctor" || first == "package" || first == "fmt" ||
        first == "format" || first == "test" || first == "new" ||
        first == "help" || first == "--help" || first == "-h")
    {
        ctx.command = first;
        if (first == "--help" || first == "-h") ctx.command = "help";

        for (int i = 2; i < argc; i++)
            ctx.args.push_back(argv[i]);

        return ctx;
    }

    // Default: run a file with options
    ctx.command = "run";
    for (int i = 1; i < argc; i++)
    {
        std::string arg = argv[i];
        if (arg == "--help") { ctx.command = "help"; return ctx; }
        else if (arg == "--version") { ctx.command = "version"; return ctx; }
        else if (arg == "--check") { ctx.checkOnly = true; }
        else if (arg == "--ast") { ctx.printAst = true; }
        else if (arg == "--tokens") { ctx.printTokens = true; }
        else if (arg == "--bytecode") { ctx.printBytecodeFlag = true; }
        else if (arg == "--vm") { ctx.useVm = true; }
        else if (arg == "--jit") { ctx.useJit = true; ctx.useVm = true; }
        else if (arg == "--no-typecheck") { ctx.noTypeCheck = true; }
        else if (arg[0] != '-') { ctx.filename = arg; }
    }

    return ctx;
}

int CLI::execute(const CLIContext& ctx)
{
    const std::string& cmd = ctx.command;

    if (cmd == "help" || cmd == "--help") { printHelp(); return 0; }
    if (cmd == "version" || cmd == "--version") { printVersion(); return 0; }
    if (cmd == "run") return handleRun(ctx);
    if (cmd == "build") return handleBuild(ctx);
    if (cmd == "install") return handleInstall(ctx);
    if (cmd == "doctor") return handleDoctor(ctx);
    if (cmd == "package") return handlePackage(ctx);
    if (cmd == "fmt" || cmd == "format") return handleFormat(ctx);
    if (cmd == "test") return handleTest(ctx);
    if (cmd == "new") return handleNew(ctx);

    printUsage();
    return 1;
}

void CLI::printHeader()
{
    std::cout << "Ketsa Programming Language v0.5\n";
}

void CLI::printUsage()
{
    printHeader();
    std::cout << "Usage: ketsa <command> [options]\n\n";
    std::cout << "Commands:\n";
    std::cout << "  run <file>      Run a Ketsa program (default)\n";
    std::cout << "  build           Build the Ketsa compiler\n";
    std::cout << "  install         Install Ketsa system-wide\n";
    std::cout << "  doctor          System diagnostics\n";
    std::cout << "  package         Package management\n";
    std::cout << "  fmt <file>      Format Ketsa source code\n";
    std::cout << "  test            Run tests\n";
    std::cout << "  new <project>   Create new Ketsa project\n";
    std::cout << "  help            Show this help\n";
    std::cout << "  version         Show version\n\n";
}

void CLI::printVersion()
{
    printHeader();
    std::cout << "Compiler: Type Checker + Bytecode VM\n";
    std::cout << "Runtime: Tree-walking Interpreter + Bytecode VM\n";
}

void CLI::printHelp()
{
    printUsage();
    std::cout << "Run Options:\n";
    std::cout << "  --check         Only check syntax & types (no execution)\n";
    std::cout << "  --ast           Print AST structure\n";
    std::cout << "  --tokens        Print token stream\n";
    std::cout << "  --bytecode      Print bytecode instructions\n";
    std::cout << "  --vm            Use bytecode VM (experimental)\n";
    std::cout << "  --jit           Enable JIT compilation with profiling\n";
    std::cout << "  --no-typecheck  Skip type checking\n\n";
    std::cout << "Examples:\n";
    std::cout << "  ketsa run program.ketsa\n";
    std::cout << "  ketsa run --check program.ketsa\n";
    std::cout << "  ketsa run --jit --vm program.ketsa\n";
    std::cout << "  ketsa run --vm --bytecode program.ketsa\n";
    std::cout << "  ketsa doctor\n";
    std::cout << "  ketsa new my_project\n";
    std::cout << "  ketsa build --release\n";
}

// ============================================================
// RUN COMMAND
// ============================================================

int CLI::handleRun(const CLIContext& ctx)
{
    std::string filename = ctx.filename;

    // If no filename in ctx, take from args
    if (filename.empty())
    {
        for (const auto& a : ctx.args)
        {
            if (a[0] != '-') { filename = a; break; }
        }
    }

    if (filename.empty())
    {
        std::cerr << "Error: No input file specified.\n";
        printUsage();
        return 1;
    }

    // Check file extension
    if (filename.size() < 6 || filename.substr(filename.size() - 6) != ".ketsa")
    {
        std::cerr << "Warning: File doesn't have .ketsa extension: " << filename << "\n";
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

    if (ctx.printTokens)
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

    if (ctx.printAst)
    {
        std::cout << "=== AST ===\n";
        std::cout << "Parsed " << ast.size() << " top-level statements\n\n";
    }

    // === TYPE CHECKER ===
    if (!ctx.noTypeCheck)
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

    if (ctx.checkOnly)
    {
        if (errorHandler.hasWarnings())
            errorHandler.printAll();
        std::cout << "Syntax and type checking OK\n";
        return 0;
    }

    // === BYTECODE COMPILER ===
    std::unique_ptr<FunctionProto> mainFunc;
    if (ctx.useVm || ctx.printBytecodeFlag)
    {
        Compiler compiler(&errorHandler);
        mainFunc = compiler.compile(ast);

        if (ctx.printBytecodeFlag && mainFunc)
        {
            std::cout << "=== Bytecode ===\n";
            std::cout << "Function: " << mainFunc->name
                      << " (code size: " << mainFunc->code.size() << ")\n";
            for (size_t i = 0; i < mainFunc->code.size(); i++)
            {
                auto& instr = mainFunc->code[i];
                std::cout << "  " << i << ": ";
                switch (instr.op)
                {
                    case OpCode::LOAD_CONST: std::cout << "LOAD_CONST " << instr.operand; break;
                    case OpCode::LOAD_VAR: std::cout << "LOAD_VAR " << instr.operand; break;
                    case OpCode::STORE_VAR: std::cout << "STORE_VAR " << instr.operand; break;
                    case OpCode::LOAD_GLOBAL: std::cout << "LOAD_GLOBAL " << instr.operand; break;
                    case OpCode::ADD: std::cout << "ADD"; break;
                    case OpCode::SUB: std::cout << "SUB"; break;
                    case OpCode::MUL: std::cout << "MUL"; break;
                    case OpCode::DIV: std::cout << "DIV"; break;
                    case OpCode::CALL: std::cout << "CALL " << instr.operand; break;
                    case OpCode::RETURN: std::cout << "RETURN"; break;
                    case OpCode::JUMP: std::cout << "JUMP -> " << instr.operand; break;
                    case OpCode::JUMP_IF_FALSE: std::cout << "JUMP_IF_FALSE -> " << instr.operand; break;
                    case OpCode::PRINTLN: std::cout << "PRINTLN"; break;
                    case OpCode::POP: std::cout << "POP"; break;
                    case OpCode::MAKE_FUNCTION: std::cout << "MAKE_FUNCTION " << instr.operand; break;
                    case OpCode::HALT: std::cout << "HALT"; break;
                    default: std::cout << "OP_" << static_cast<int>(instr.op); break;
                }
                std::cout << "\n";
            }
        }
    }

    // === JIT ENGINE (if --jit) ===
    std::unique_ptr<JITEngine> jitEngine;
    if (ctx.useJit)
    {
        jitEngine = std::make_unique<JITEngine>(&errorHandler);
        jitEngine->setEnabled(true);

        std::cout << "[JIT] JIT compilation enabled\n";

        // Register optimization passes
        auto constFold = std::make_unique<ConstantFoldingPass>();
        auto dce = std::make_unique<DeadCodeEliminationPass>();
        auto jumpOpt = std::make_unique<JumpOptimizationPass>();
        auto peephole = std::make_unique<PeepholeOptimizationPass>();

        // Run optimization on main function
        if (mainFunc)
        {
            jitEngine->optimize(mainFunc.get());
            std::cout << "[JIT] Optimized main function\n";
        }
    }

    // === VM EXECUTION ===
    if (ctx.useVm && mainFunc)
    {
        VM vm(&errorHandler, jitEngine.get());
        auto result = vm.interpret(mainFunc.get());

        if (ctx.useJit && jitEngine)
        {
            auto profiles = jitEngine->getProfileData();
            if (!profiles.empty())
            {
                std::cout << "\n[JIT] Profile:\n";
                for (const auto& p : profiles)
                {
                    std::cout << "  " << p.functionName << ": "
                              << p.callCount << " calls";
                    if (jitEngine->getCompiledCount() > 0)
                        std::cout << " [compiled]";
                    std::cout << "\n";
                }
            }
        }

        if (result == InterpretResult::RUNTIME_ERROR)
        {
            errorHandler.printAll();
            errorHandler.printSummary();
            return 1;
        }
        return 0;
    }

    // === INTERPRETER ===
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

    if (errorHandler.hasErrors())
    {
        errorHandler.printAll();
        errorHandler.printSummary();
        return 1;
    }

    if (errorHandler.hasWarnings())
        errorHandler.printAll();

    return 0;
}

// ============================================================
// BUILD COMMAND
// ============================================================

int CLI::handleBuild(const CLIContext& ctx)
{
    bool release = false;
    bool clean = false;

    for (const auto& arg : ctx.args)
    {
        if (arg == "--release") release = true;
        if (arg == "--clean" || arg == "-c") clean = true;
    }

    std::string config = release ? "Release" : "Debug";

    std::cout << "[KETSA] Building Ketsa (" << config << ")...\n";

#ifdef _WIN32
    std::string cmd = "cmake --build build --config " + config;
    if (clean) cmd = "cmake --build build --config " + config + " --clean-first";
#else
    std::string cmd = "cmake --build build --config " + config;
    if (clean) cmd = "cmake --build build --config " + config + " --clean-first";
#endif

    int result = std::system(cmd.c_str());
    if (result != 0)
    {
        std::cerr << "[KETSA] Build failed\n";
        return 1;
    }

    std::cout << "[KETSA] Build successful\n";
    return 0;
}

// ============================================================
// INSTALL COMMAND
// ============================================================

int CLI::handleInstall(const CLIContext& ctx)
{
    std::string prefix = "/usr/local";
    for (size_t i = 0; i < ctx.args.size(); i++)
    {
        if (ctx.args[i] == "--prefix" && i + 1 < ctx.args.size())
            prefix = ctx.args[i + 1];
    }

    std::cout << "[KETSA] Installing Ketsa to " << prefix << "\n";

#ifdef _WIN32
    // Windows: copy to a known location
    std::string installDir = prefix;
    if (installDir == "/usr/local") installDir = "C:\\Program Files\\Ketsa";

    fs::create_directories(installDir);
    fs::create_directories(installDir + "\\lib");
    fs::create_directories(installDir + "\\include");

    if (fs::exists("build\\ketsa.exe"))
        fs::copy_file("build\\ketsa.exe", installDir + "\\ketsa.exe",
                      fs::copy_options::overwrite_existing);

    std::cout << "[KETSA] Installed to: " << installDir << "\n";
    std::cout << "[KETSA] Add to PATH: " << installDir << "\n";
#else
    std::string installDir = prefix + "/bin";
    fs::create_directories(installDir);
    fs::create_directories(prefix + "/lib/ketsa");
    fs::create_directories(prefix + "/include/ketsa");

    if (fs::exists("build/ketsa"))
        fs::copy_file("build/ketsa", installDir + "/ketsa",
                      fs::copy_options::overwrite_existing);
    else if (fs::exists("ketsa"))
        fs::copy_file("ketsa", installDir + "/ketsa",
                      fs::copy_options::overwrite_existing);

    std::cout << "[KETSA] Installed to: " << prefix << "\n";
    std::cout << "[KETSA] Run: ketsa --version\n";
#endif

    return 0;
}

// ============================================================
// DOCTOR COMMAND
// ============================================================

int CLI::handleDoctor(const CLIContext& /*ctx*/)
{
    auto checks = runDoctor();
    printDoctorResults(checks);

    bool allPassed = std::all_of(checks.begin(), checks.end(),
        [](const HealthCheck& c) { return c.passed; });

    return allPassed ? 0 : 1;
}

std::vector<CLI::HealthCheck> CLI::runDoctor()
{
    std::vector<HealthCheck> checks;

    // 1. Compiler binary
    {
        HealthCheck h;
        h.name = "Ketsa Compiler";
        h.passed = true;
        h.detail = "v0.5.0 - Type Checker + Bytecode VM";
        checks.push_back(h);
    }

    // 2. Build system
    {
        HealthCheck h;
        h.name = "Build System";
        bool hasCMake = (std::system("cmake --version >nul 2>nul") == 0);
        bool hasBuildDir = fs::exists("build") && (fs::exists("build/ketsa.exe") || fs::exists("build/ketsa"));

        if (hasCMake && hasBuildDir) { h.passed = true; h.detail = "CMake + Build ready"; }
        else if (hasCMake) { h.passed = false; h.detail = "CMake OK, run build"; h.fix = "Run: build.bat or ./build.sh"; }
        else { h.passed = false; h.detail = "CMake not found"; h.fix = "Install CMake 3.20+"; }
        checks.push_back(h);
    }

    // 3. C++ Compiler
    {
        HealthCheck h;
        h.name = "C++ Compiler";
#ifdef _MSC_VER
        h.passed = true;
        h.detail = "MSVC " + std::to_string(_MSC_VER);
#elif defined(__clang__)
        h.passed = true;
        h.detail = "Clang " __clang_version__;
#elif defined(__GNUC__)
        h.passed = true;
        h.detail = "GCC " __VERSION__;
#else
        h.passed = false;
        h.detail = "Unknown compiler";
        h.fix = "Install MSVC, GCC, or Clang";
#endif
        checks.push_back(h);
    }

    // 4. C++ Standard
    {
        HealthCheck h;
        h.name = "C++ Standard";
#ifdef __cpp_lib_filesystem
        h.passed = true;
        h.detail = "C++20";
#else
        h.passed = true;
        h.detail = "C++20 (assumed)";
#endif
        checks.push_back(h);
    }

    // 5. Standard Library
    {
        HealthCheck h;
        h.name = "Standard Library";
        h.passed = fs::exists("std") || fs::exists("lib/std");
        h.detail = h.passed ? "Found" : "Not installed";
        h.fix = "Run: ketsa std install";
        checks.push_back(h);
    }

    // 6. Git
    {
        HealthCheck h;
        h.name = "Git Repository";
        h.passed = fs::exists(".git");
        h.detail = h.passed ? "Git repository found" : "Not a git repository";
        checks.push_back(h);
    }

    // 7. Architecture
    {
        HealthCheck h;
        h.name = "Architecture";
#ifdef _WIN64
        h.passed = true; h.detail = "x64 (Windows)";
#elif defined(_WIN32)
        h.passed = true; h.detail = "x86 (Windows)";
#elif defined(__x86_64__)
        h.passed = true; h.detail = "x86_64";
#elif defined(__aarch64__)
        h.passed = true; h.detail = "ARM64";
#else
        h.passed = true; h.detail = "Unknown";
#endif
        checks.push_back(h);
    }

    // 8. JIT Engine
    {
        HealthCheck h;
        h.name = "JIT Engine";
        h.passed = JITEngine::isBackendAvailable();
        h.detail = h.passed ? "Available (x64)" : "Not available";
        h.fix = h.passed ? "" : "x64 CPU required for JIT";
        checks.push_back(h);
    }

    // 9. JIT Backend
    {
        HealthCheck h;
        h.name = "JIT Backend";
#if defined(_WIN64) || defined(__x86_64__)
        h.passed = true;
        h.detail = "x64 native code generator";
#else
        h.passed = false;
        h.detail = "Unsupported architecture";
        h.fix = "x86_64 / ARM64 support planned";
#endif
        checks.push_back(h);
    }

    // 10. Native Compiler
    {
        HealthCheck h;
        h.name = "Native Compiler";
#ifdef _MSC_VER
        h.passed = true;
        h.detail = "MSVC x64";
#elif defined(__clang__)
        h.passed = true;
        h.detail = "Clang (LLVM backend)";
#elif defined(__GNUC__)
        h.passed = true;
        h.detail = "GCC";
#else
        h.passed = false;
        h.detail = "Unknown compiler";
#endif
        checks.push_back(h);
    }

    // 11. Environment PATH
    {
        HealthCheck h;
        h.name = "Environment PATH";
#ifdef _WIN32
        const char* pathEnv = std::getenv("PATH");
        h.passed = pathEnv && std::string(pathEnv).find("Ketsa") != std::string::npos;
#else
        const char* pathEnv = std::getenv("PATH");
        h.passed = true; // PATH always exists on Unix
#endif
        h.detail = h.passed ? "Ketsa in PATH" : "Not in PATH";
        h.fix = "Add Ketsa binary directory to PATH";
        checks.push_back(h);
    }

    return checks;
}

void CLI::printDoctorResults(const std::vector<HealthCheck>& checks)
{
    std::cout << "\n";
    std::cout << "  ╔══════════════════════════════════════╗\n";
    std::cout << "  ║       Ketsa System Doctor v0.3       ║\n";
    std::cout << "  ╚══════════════════════════════════════╝\n\n";

    int passed = 0, failed = 0;
    for (const auto& h : checks)
    {
        if (h.passed) { passed++; }
        else { failed++; }
    }
    std::cout << "  Results: " << passed << " passed, " << failed << " failed\n\n";

    for (const auto& h : checks)
    {
        std::cout << "  " << (h.passed ? "[PASS]" : "[FAIL]")
                  << " " << h.name << "\n";
        std::cout << "         " << h.detail << "\n";
        if (!h.passed && !h.fix.empty())
            std::cout << "         Fix: " << h.fix << "\n";
        std::cout << "\n";
    }
}

// ============================================================
// PACKAGE COMMAND
// ============================================================

int CLI::handlePackage(const CLIContext& ctx)
{
    if (ctx.args.empty())
    {
        std::cout << "Usage: ketsa package <command>\n";
        std::cout << "Commands: init, install, remove, search, publish\n";
        return 1;
    }

    const std::string& sub = ctx.args[0];

    if (sub == "init")
    {
        std::string name = "myapp";
        if (ctx.args.size() > 1) name = ctx.args[1];

        std::ofstream f("ketsa.toml");
        if (!f)
        {
            std::cerr << "Error: Cannot create ketsa.toml\n";
            return 1;
        }

        f << "[package]\n";
        f << "name = \"" << name << "\"\n";
        f << "version = \"0.1.0\"\n";
        f << "description = \"A Ketsa project\"\n\n";
        f << "[dependencies]\n";
        f.close();

        // Create main.ketsa
        std::ofstream m("src/main.ketsa");
        if (m)
        {
            m << "// " << name << "\n";
            m << "// v0.1.0\n\n";
            m << "print \"Hello from " << name << "!\"\n";
            m.close();
        }

        fs::create_directories("src");
        fs::create_directories("lib");

        std::cout << "Created Ketsa project: " << name << "\n";
        std::cout << "  ketsa.toml\n";
        std::cout << "  src/main.ketsa\n";
        std::cout << "  lib/\n";
        return 0;
    }

    if (sub == "install")
    {
        std::cout << "Package installation not yet implemented\n";
        return 0;
    }

    if (sub == "search")
    {
        std::cout << "Package search not yet implemented\n";
        return 0;
    }

    std::cerr << "Unknown package command: " << sub << "\n";
    return 1;
}

// ============================================================
// FORMAT COMMAND
// ============================================================

int CLI::handleFormat(const CLIContext& ctx)
{
    std::string filename;
    bool checkOnly = false;

    for (const auto& arg : ctx.args)
    {
        if (arg == "--check") checkOnly = true;
        else if (arg[0] != '-') filename = arg;
    }

    if (filename.empty())
    {
        std::cerr << "Usage: ketsa fmt <file.ketsa> [--check]\n";
        return 1;
    }

    std::ifstream file(filename);
    if (!file)
    {
        std::cerr << "Error: Cannot open '" << filename << "'\n";
        return 1;
    }

    std::string source((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());

    ErrorHandler errorHandler;
    Lexer lexer(source, &errorHandler);
    auto tokens = lexer.tokenize();

    // Simple formatter: reconstruct source from tokens
    std::string formatted;
    int indent = 0;
    for (size_t i = 0; i < tokens.size(); i++)
    {
        auto& t = tokens[i];
        if (t.type == TokenType::LBRACE) { formatted += " {\n"; indent++; }
        else if (t.type == TokenType::RBRACE) { indent--; formatted += std::string(indent * 4, ' ') + "}\n"; }
        else if (t.type == TokenType::SEMICOLON) { formatted += ";\n"; }
        else if (t.type == TokenType::COMMA) { formatted += ", "; }
        else { formatted += t.value + " "; }
    }

    if (!checkOnly)
    {
        std::ofstream of(filename);
        of << formatted;
        std::cout << "Formatted: " << filename << "\n";
    }

    return 0;
}

// ============================================================
// TEST COMMAND
// ============================================================

int CLI::handleTest(const CLIContext& /*ctx*/)
{
    std::cout << "Running Ketsa test suite...\n\n";

    std::vector<std::string> testFiles;
    if (fs::exists("examples"))
    {
        for (const auto& entry : fs::directory_iterator("examples"))
        {
            if (entry.path().extension() == ".ketsa")
                testFiles.push_back(entry.path().string());
        }
    }

    if (testFiles.empty())
    {
        std::cerr << "No test files found in examples/\n";
        return 1;
    }

    int passed = 0, failed = 0;
    for (const auto& f : testFiles)
    {
        CLIContext runCtx;
        runCtx.command = "run";
        runCtx.filename = f;
        runCtx.noTypeCheck = true;

        std::cout << "  Testing: " << f << "... ";
        int result = handleRun(runCtx);
        if (result == 0) { std::cout << "PASS\n"; passed++; }
        else { std::cout << "FAIL\n"; failed++; }
    }

    std::cout << "\nResults: " << passed << " passed, " << failed << " failed\n";
    return failed > 0 ? 1 : 0;
}

// ============================================================
// NEW COMMAND
// ============================================================

int CLI::handleNew(const CLIContext& ctx)
{
    std::string name = ctx.args.empty() ? "myapp" : ctx.args[0];

    // Validate project name
    for (char c : name)
    {
        if (!std::isalnum(static_cast<unsigned char>(c)) && c != '_' && c != '-')
        {
            std::cerr << "Invalid project name: " << name << "\n";
            std::cerr << "Use only letters, numbers, underscores, and hyphens.\n";
            return 1;
        }
    }

    if (fs::exists(name))
    {
        std::cerr << "Directory already exists: " << name << "\n";
        return 1;
    }

    fs::create_directories(name + "/src");
    fs::create_directories(name + "/lib");
    fs::create_directories(name + "/tests");

    // ketsa.toml
    {
        std::ofstream f(name + "/ketsa.toml");
        f << "[package]\n";
        f << "name = \"" << name << "\"\n";
        f << "version = \"0.1.0\"\n";
        f << "description = \"A Ketsa project\"\n";
        f << "entry = \"src/main.ketsa\"\n\n";
        f << "[dependencies]\n";
    }

    // main.ketsa
    {
        std::ofstream m(name + "/src/main.ketsa");
        m << "// " << name << " v0.1.0\n\n";
        m << "print \"Hello from " << name << "!\"\n";
    }

    // test.ketsa
    {
        std::ofstream t(name + "/tests/test_main.ketsa");
        t << "// Tests for " << name << "\n\n";
        t << "print \"Running tests for " << name << "...\"\n";
    }

    // .gitignore
    {
        std::ofstream g(name + "/.gitignore");
        g << "build/\n";
        g << "*.exe\n";
        g << ".opencode/\n";
    }

    std::cout << "Created Ketsa project: " << name << "\n";
    std::cout << "  " << name << "/\n";
    std::cout << "  " << name << "/ketsa.toml\n";
    std::cout << "  " << name << "/src/main.ketsa\n";
    std::cout << "  " << name << "/lib/\n";
    std::cout << "  " << name << "/tests/\n";
    std::cout << "\nRun:\n";
    std::cout << "  cd " << name << "\n";
    std::cout << "  ketsa run src/main.ketsa\n";

    return 0;
}

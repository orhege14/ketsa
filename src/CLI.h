#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <cstdlib>

struct CLIContext
{
    std::string command;
    std::vector<std::string> args;
    std::string filename;
    bool checkOnly = false;
    bool printAst = false;
    bool printTokens = false;
    bool printBytecodeFlag = false;
    bool useVm = false;
    bool useJit = false;
    bool noTypeCheck = false;
};

class CLI
{
public:
    static CLIContext parse(int argc, char* argv[]);
    static int execute(const CLIContext& ctx);

    static void printHeader();
    static void printUsage();
    static void printVersion();
    static void printHelp();

    // Subcommand handlers
    static int handleRun(const CLIContext& ctx);
    static int handleBuild(const CLIContext& ctx);
    static int handleInstall(const CLIContext& ctx);
    static int handleDoctor(const CLIContext& ctx);
    static int handlePackage(const CLIContext& ctx);
    static int handleFormat(const CLIContext& ctx);
    static int handleTest(const CLIContext& ctx);
    static int handleNew(const CLIContext& ctx);

    // Doctor checks
    struct HealthCheck
    {
        std::string name;
        bool passed;
        std::string detail;
        std::string fix;
    };

    static std::vector<HealthCheck> runDoctor();
    static void printDoctorResults(const std::vector<HealthCheck>& checks);
};

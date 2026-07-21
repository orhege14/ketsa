#include "CLI.h"

int main(int argc, char* argv[])
{
    auto ctx = CLI::parse(argc, argv);
    return CLI::execute(ctx);
}

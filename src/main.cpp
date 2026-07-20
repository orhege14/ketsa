#include <iostream>
#include <fstream>
#include <string>

#include "Lexer.h"
#include "Parser.h"
#include "Interpreter.h"


int main(int argc, char* argv[])
{
    if(argc < 2)
    {
        std::cout << "Usage: ketsa <file.ketsa>\n";
        return 1;
    }


    std::ifstream file(argv[1]);

    if(!file)
    {
        std::cout << "Cannot open file\n";
        return 1;
    }


    std::string source(
        (std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>()
    );


    // Lexer
    Lexer lexer(source);

    auto tokens = lexer.tokenize();


    // Parser
    Parser parser(tokens);

    auto nodes = parser.parse();


    // Interpreter
    Interpreter interpreter;


    for(auto& node : nodes)
    {
        interpreter.execute(node.get());
    }


    return 0;
}
#include <iostream>
#include <fstream>
#include <sstream>

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


    if(!file.is_open())
    {
        std::cout << "File not found!\n";
        return 1;
    }


    std::stringstream buffer;

    buffer << file.rdbuf();


    std::string code = buffer.str();



    // Lexer

    Lexer lexer(code);

    auto tokens = lexer.tokenize();



    // Parser

    Parser parser(tokens);

    auto ast = parser.parse();



    // Interpreter

    Interpreter interpreter;


    for(auto& node : ast)
    {
        interpreter.execute(node.get());
    }



    return 0;
}
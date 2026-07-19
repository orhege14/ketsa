#pragma once

#include "AST.h"
#include "Value.h"

#include <unordered_map>
#include <string>
#include <memory>


class Interpreter
{

private:

    // Değişken hafızası
    std::unordered_map<std::string, Value> variables;


    // Node'dan değer üretir
    Value evaluate(ASTNode* node);


public:

    Interpreter();


    // Programı çalıştırır
    void execute(ASTNode* node);

};
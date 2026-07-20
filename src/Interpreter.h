#pragma once

#include "AST.h"
#include "runtime/Environment.h"

#include <memory>


class Interpreter
{

private:

    Environment environment;


    std::unique_ptr<Value> evaluate(ASTNode* node);


public:

    Interpreter();


    void execute(ASTNode* node);

};
#pragma once

#include "Value.h"
#include "../../AST.h"
#include <memory>
#include <vector>
#include <string>
#include <shared_mutex>

class Environment;

class FunctionValue : public Value
{
    std::string name;
    std::vector<std::string> parameters;
    std::vector<std::unique_ptr<ASTNode>> body;
    std::shared_ptr<Environment> closure;
    bool isAnonymous;
public:
    FunctionValue(std::string name, std::vector<std::string> params,
                  std::vector<std::unique_ptr<ASTNode>> body,
                  std::shared_ptr<Environment> closure,
                  bool isAnonymous = false);
    ValueType getType() const override;
    std::string toString() const override;
    std::unique_ptr<Value> clone() const override;
    TypeInfo getTypeInfo() const override;

    const std::string& getName() const;
    const std::vector<std::string>& getParameters() const;
    std::vector<std::unique_ptr<ASTNode>>& getBody();
    std::shared_ptr<Environment> getClosure() const;
    bool isAnonymousFunc() const;
};

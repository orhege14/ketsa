#pragma once

#include "Value.h"
#include <memory>
#include <string>

class Environment;

class ModuleValue : public Value
{
    std::string name;
    std::unique_ptr<Environment> moduleEnv;
public:
    ModuleValue(std::string name, std::unique_ptr<Environment> moduleEnv);
    ValueType getType() const override;
    std::string toString() const override;
    std::unique_ptr<Value> clone() const override;
    TypeInfo getTypeInfo() const override;

    const std::string& getName() const;
    Environment* getModuleEnv() const;
};

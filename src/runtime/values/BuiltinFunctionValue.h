#pragma once

#include "Value.h"

class Environment;

using BuiltinFunctionType = std::unique_ptr<Value>(*)(const std::vector<std::unique_ptr<Value>>&, Environment*);

class BuiltinFunctionValue : public Value
{
    std::string name;
    BuiltinFunctionType function;
public:
    BuiltinFunctionValue(std::string name, BuiltinFunctionType function);
    ValueType getType() const override;
    std::string toString() const override;
    std::unique_ptr<Value> clone() const override;
    TypeInfo getTypeInfo() const override;

    const std::string& getName() const;
    BuiltinFunctionType getFunction() const;
};

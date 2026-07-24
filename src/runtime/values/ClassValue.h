#pragma once

#include "Value.h"
#include "../../AST.h"
#include <memory>
#include <string>
#include <vector>

class Environment;

class ClassValue : public Value
{
    std::string name;
    std::vector<std::string> fields;
    std::shared_ptr<std::vector<MethodDeclaration>> methods;
    std::shared_ptr<Environment> classEnv;
public:
    ClassValue(std::string name, std::vector<std::string> fields,
               std::vector<MethodDeclaration> methods,
               std::shared_ptr<Environment> classEnv);
    ValueType getType() const override;
    std::string toString() const override;
    std::unique_ptr<Value> clone() const override;
    TypeInfo getTypeInfo() const override;

    const std::string& getName() const;
    const std::vector<std::string>& getFields() const;
    std::vector<MethodDeclaration>& getMethods();
    std::shared_ptr<Environment> getClassEnv() const;
};

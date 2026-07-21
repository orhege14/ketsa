#include "ClassValue.h"
#include "../Environment.h"

ClassValue::ClassValue(std::string name, std::vector<std::string> fields,
                       std::vector<MethodDeclaration> methodsList,
                       std::shared_ptr<Environment> classEnv)
    : name(std::move(name))
    , fields(std::move(fields))
    , methods(std::make_shared<std::vector<MethodDeclaration>>(std::move(methodsList)))
    , classEnv(std::move(classEnv)) {}

ValueType ClassValue::getType() const { return ValueType::CLASS; }

std::string ClassValue::toString() const
{
    return "<class " + name + ">";
}

std::unique_ptr<Value> ClassValue::clone() const
{
    auto cloned = std::make_unique<ClassValue>(name, fields, std::vector<MethodDeclaration>(), classEnv);
    cloned->methods = methods;
    return cloned;
}

TypeInfo ClassValue::getTypeInfo() const
{
    TypeInfo t(TypeKind::CLASS);
    t.name = name;
    return t;
}

const std::string& ClassValue::getName() const { return name; }
const std::vector<std::string>& ClassValue::getFields() const { return fields; }
std::vector<MethodDeclaration>& ClassValue::getMethods() { return *methods; }
std::shared_ptr<Environment> ClassValue::getClassEnv() const { return classEnv; }

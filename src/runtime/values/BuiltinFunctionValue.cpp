#include "BuiltinFunctionValue.h"

BuiltinFunctionValue::BuiltinFunctionValue(std::string name, BuiltinFunctionType func)
    : name(std::move(name)), function(func) {}

ValueType BuiltinFunctionValue::getType() const { return ValueType::BUILTIN_FUNCTION; }

std::string BuiltinFunctionValue::toString() const
{
    return "<builtin function " + name + ">";
}

std::unique_ptr<Value> BuiltinFunctionValue::clone() const
{
    return std::make_unique<BuiltinFunctionValue>(name, function);
}

TypeInfo BuiltinFunctionValue::getTypeInfo() const { return TypeInfo(TypeKind::FUNCTION); }

const std::string& BuiltinFunctionValue::getName() const { return name; }
BuiltinFunctionType BuiltinFunctionValue::getFunction() const { return function; }

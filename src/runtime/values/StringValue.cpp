#include "StringValue.h"

StringValue::StringValue(const std::string& value) : value(value) {}

ValueType StringValue::getType() const { return ValueType::STRING; }

std::string StringValue::toString() const { return value; }

std::unique_ptr<Value> StringValue::clone() const
{
    return std::make_unique<StringValue>(value);
}

const std::string& StringValue::getValue() const { return value; }

TypeInfo StringValue::getTypeInfo() const { return TypeInfo(TypeKind::STRING); }

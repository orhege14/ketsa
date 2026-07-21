#include "NumberValue.h"

NumberValue::NumberValue(int64_t value) : value(value) {}

ValueType NumberValue::getType() const { return ValueType::INT; }

std::string NumberValue::toString() const { return std::to_string(value); }

std::unique_ptr<Value> NumberValue::clone() const
{
    return std::make_unique<NumberValue>(value);
}

int64_t NumberValue::getValue() const { return value; }

TypeInfo NumberValue::getTypeInfo() const { return TypeInfo(TypeKind::INT); }

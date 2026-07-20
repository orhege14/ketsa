#include "NumberValue.h"

NumberValue::NumberValue(int value)
{
    this->value = value;
}

ValueType NumberValue::getType() const
{
    return ValueType::NUMBER;
}

std::string NumberValue::toString() const
{
    return std::to_string(value);
}
std::unique_ptr<Value> NumberValue::clone() const
{
    return std::make_unique<NumberValue>(value);
}

int NumberValue::getValue() const
{
    return value;
}
#include "BooleanValue.h"

BooleanValue::BooleanValue(bool value) : value(value) {}

ValueType BooleanValue::getType() const { return ValueType::BOOLEAN; }

std::string BooleanValue::toString() const { return value ? "true" : "false"; }

std::unique_ptr<Value> BooleanValue::clone() const
{
    return std::make_unique<BooleanValue>(value);
}

bool BooleanValue::getValue() const { return value; }

TypeInfo BooleanValue::getTypeInfo() const { return TypeInfo(TypeKind::BOOLEAN); }

bool BooleanValue::isTruthy() const { return value; }

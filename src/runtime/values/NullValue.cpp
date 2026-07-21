#include "NullValue.h"

NullValue::NullValue() {}

ValueType NullValue::getType() const { return ValueType::NULL_VALUE; }

std::string NullValue::toString() const { return "null"; }

std::unique_ptr<Value> NullValue::clone() const
{
    return std::make_unique<NullValue>();
}

TypeInfo NullValue::getTypeInfo() const { return TypeInfo(TypeKind::NULL_TYPE); }

bool NullValue::isTruthy() const { return false; }

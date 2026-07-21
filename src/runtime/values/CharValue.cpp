#include "CharValue.h"

CharValue::CharValue(char val) : value(val) {}

ValueType CharValue::getType() const { return ValueType::CHAR; }

std::string CharValue::toString() const { return std::string(1, value); }

std::unique_ptr<Value> CharValue::clone() const
{
    return std::make_unique<CharValue>(value);
}

char CharValue::getValue() const { return value; }

TypeInfo CharValue::getTypeInfo() const { return TypeInfo(TypeKind::CHAR); }

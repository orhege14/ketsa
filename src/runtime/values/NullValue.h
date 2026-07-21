#pragma once

#include "Value.h"

class NullValue : public Value
{
public:
    NullValue();
    ValueType getType() const override;
    std::string toString() const override;
    std::unique_ptr<Value> clone() const override;
    TypeInfo getTypeInfo() const override;
    bool isTruthy() const override;
};

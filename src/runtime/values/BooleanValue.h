#pragma once

#include "Value.h"

class BooleanValue : public Value
{
    bool value;
public:
    explicit BooleanValue(bool value);
    ValueType getType() const override;
    std::string toString() const override;
    std::unique_ptr<Value> clone() const override;
    bool getValue() const;
    TypeInfo getTypeInfo() const override;
    bool isTruthy() const override;
};

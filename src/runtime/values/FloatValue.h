#pragma once

#include "Value.h"

class FloatValue : public Value
{
    double value;
public:
    explicit FloatValue(double value);
    ValueType getType() const override;
    std::string toString() const override;
    std::unique_ptr<Value> clone() const override;
    double getValue() const;
    TypeInfo getTypeInfo() const override;
};

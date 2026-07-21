#pragma once

#include "Value.h"

class NumberValue : public Value
{
    int64_t value;
public:
    explicit NumberValue(int64_t value);
    ValueType getType() const override;
    std::string toString() const override;
    std::unique_ptr<Value> clone() const override;
    int64_t getValue() const;
    TypeInfo getTypeInfo() const override;
};

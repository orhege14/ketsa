#pragma once

#include "Value.h"

class CharValue : public Value
{
    char value;
public:
    explicit CharValue(char value);
    ValueType getType() const override;
    std::string toString() const override;
    std::unique_ptr<Value> clone() const override;
    char getValue() const;
    TypeInfo getTypeInfo() const override;
};

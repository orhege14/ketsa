#pragma once

#include "Value.h"

class StringValue : public Value
{
    std::string value;
public:
    explicit StringValue(const std::string& value);
    ValueType getType() const override;
    std::string toString() const override;
    std::unique_ptr<Value> clone() const override;
    const std::string& getValue() const;
    TypeInfo getTypeInfo() const override;
};

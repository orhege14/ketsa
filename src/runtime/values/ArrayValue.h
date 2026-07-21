#pragma once

#include "Value.h"
#include <vector>
#include <memory>
#include <string>

class ArrayValue : public Value
{
    std::vector<std::unique_ptr<Value>> elements;
public:
    ArrayValue();
    explicit ArrayValue(std::vector<std::unique_ptr<Value>> elements);
    ValueType getType() const override;
    std::string toString() const override;
    std::unique_ptr<Value> clone() const override;
    TypeInfo getTypeInfo() const override;

    size_t size() const;
    Value* get(size_t index) const;
    void set(size_t index, std::unique_ptr<Value> value);
    void push(std::unique_ptr<Value> value);
    std::unique_ptr<Value> pop();
    const std::vector<std::unique_ptr<Value>>& getElements() const;
};

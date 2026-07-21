#pragma once

#include "Value.h"
#include <unordered_map>

class ObjectValue : public Value
{
    std::unordered_map<std::string, std::unique_ptr<Value>> properties;
    std::string className;
public:
    explicit ObjectValue(const std::string& className = "");
    ValueType getType() const override;
    std::string toString() const override;
    std::unique_ptr<Value> clone() const override;
    TypeInfo getTypeInfo() const override;

    void set(const std::string& name, std::unique_ptr<Value> value);
    Value* get(const std::string& name) const;
    bool has(const std::string& name) const;
    const std::string& getClassName() const;
};

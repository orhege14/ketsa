#pragma once

#include <string>
#include <memory>


enum class ValueType
{
    NUMBER,
    STRING,
    BOOLEAN,
    NULL_VALUE,
    ARRAY
};


class Value
{

public:

    virtual ~Value() = default;


    virtual ValueType getType() const = 0;


    virtual std::string toString() const = 0;


    virtual std::unique_ptr<Value> clone() const = 0;

};
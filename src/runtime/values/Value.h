#pragma once

#include <string>
#include <memory>
#include <vector>
#include <cstdint>

#include "../../AST.h"

enum class ValueType : uint8_t
{
    INT,
    FLOAT,
    DOUBLE,
    CHAR,
    STRING,
    BOOLEAN,
    NULL_VALUE,
    ARRAY,
    OBJECT,
    FUNCTION,
    CLASS,
    MODULE,
    BUILTIN_FUNCTION
};

class Value
{
public:
    virtual ~Value() = default;
    virtual ValueType getType() const = 0;
    virtual std::string toString() const = 0;
    virtual std::unique_ptr<Value> clone() const = 0;

    virtual bool isTruthy() const;
    virtual TypeInfo getTypeInfo() const;
};

// Forward declarations
class NumberValue;
class FloatValue;
class CharValue;
class StringValue;
class BooleanValue;
class NullValue;
class ArrayValue;
class ObjectValue;
class FunctionValue;
class BuiltinFunctionValue;
class ClassValue;
class ModuleValue;
class Environment;

using BuiltinFunctionType = std::unique_ptr<Value>(*)(const std::vector<std::unique_ptr<Value>>&, Environment*);

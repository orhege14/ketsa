#include "FloatValue.h"
#include <sstream>

FloatValue::FloatValue(double val) : value(val) {}

ValueType FloatValue::getType() const { return ValueType::FLOAT; }

std::string FloatValue::toString() const
{
    std::string s = std::to_string(value);
    size_t dot = s.find('.');
    if (dot != std::string::npos)
    {
        size_t last = s.find_last_not_of('0');
        if (last > dot) s = s.substr(0, last + 1);
        else s = s.substr(0, dot + 2);
    }
    return s;
}

std::unique_ptr<Value> FloatValue::clone() const
{
    return std::make_unique<FloatValue>(value);
}

double FloatValue::getValue() const { return value; }

TypeInfo FloatValue::getTypeInfo() const { return TypeInfo(TypeKind::FLOAT); }

#include "ObjectValue.h"
#include <sstream>

ObjectValue::ObjectValue(const std::string& className)
    : className(className) {}

ValueType ObjectValue::getType() const { return ValueType::OBJECT; }

std::string ObjectValue::toString() const
{
    std::stringstream ss;
    ss << className << " { ";
    bool first = true;
    for (const auto& [key, val] : properties)
    {
        if (!first) ss << ", ";
        ss << key << ": " << val->toString();
        first = false;
    }
    ss << " }";
    return ss.str();
}

std::unique_ptr<Value> ObjectValue::clone() const
{
    auto obj = std::make_unique<ObjectValue>(className);
    for (const auto& [key, val] : properties)
        obj->properties[key] = val->clone();
    return obj;
}

TypeInfo ObjectValue::getTypeInfo() const
{
    return TypeInfo::createObject(className);
}

void ObjectValue::set(const std::string& name, std::unique_ptr<Value> value)
{
    properties[name] = std::move(value);
}

Value* ObjectValue::get(const std::string& name) const
{
    auto it = properties.find(name);
    return it != properties.end() ? it->second.get() : nullptr;
}

bool ObjectValue::has(const std::string& name) const
{
    return properties.find(name) != properties.end();
}

const std::string& ObjectValue::getClassName() const { return className; }

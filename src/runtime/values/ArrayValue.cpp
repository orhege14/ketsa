#include "ArrayValue.h"
#include <sstream>

ArrayValue::ArrayValue() = default;

ArrayValue::ArrayValue(std::vector<std::unique_ptr<Value>> elements)
    : elements(std::move(elements)) {}

ValueType ArrayValue::getType() const { return ValueType::ARRAY; }

std::string ArrayValue::toString() const
{
    std::string result = "[";
    for (size_t i = 0; i < elements.size(); i++)
    {
        if (i > 0) result += ", ";
        result += elements[i]->toString();
    }
    result += "]";
    return result;
}

std::unique_ptr<Value> ArrayValue::clone() const
{
    std::vector<std::unique_ptr<Value>> copy;
    for (const auto& elem : elements)
        copy.push_back(elem->clone());
    return std::make_unique<ArrayValue>(std::move(copy));
}

TypeInfo ArrayValue::getTypeInfo() const
{
    return TypeInfo::createArray(std::make_unique<TypeInfo>(TypeKind::ANY));
}

size_t ArrayValue::size() const { return elements.size(); }

Value* ArrayValue::get(size_t index) const
{
    if (index < elements.size())
        return elements[index].get();
    return nullptr;
}

void ArrayValue::set(size_t index, std::unique_ptr<Value> value)
{
    if (index < elements.size())
        elements[index] = std::move(value);
}

void ArrayValue::push(std::unique_ptr<Value> value)
{
    elements.push_back(std::move(value));
}

std::unique_ptr<Value> ArrayValue::pop()
{
    if (elements.empty()) return nullptr;
    auto val = std::move(elements.back());
    elements.pop_back();
    return val;
}

const std::vector<std::unique_ptr<Value>>& ArrayValue::getElements() const { return elements; }

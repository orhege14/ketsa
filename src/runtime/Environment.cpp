#include "Environment.h"

void Environment::define(
    const std::string &name,
    std::unique_ptr<Value> value)
{
    variables[name] = std::move(value);
}
void Environment::set(
    const std::string &name,
    std::unique_ptr<Value> value)
{
    variables[name] = std::move(value);
}   

Value *Environment::get(
    const std::string &name)
{
    auto iterator = variables.find(name);

    if (iterator == variables.end())
    {
        return nullptr;
    }

    return iterator->second.get();
}

bool Environment::exists(
    const std::string &name)
{
    return variables.find(name) != variables.end();
}
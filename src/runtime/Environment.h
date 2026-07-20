#pragma once

#include <unordered_map>
#include <string>
#include <memory>

#include "values/Value.h"

class Environment
{
private:
    std::unordered_map<
        std::string,
        std::unique_ptr<Value>>
        variables;

public:
    void define(
        const std::string &name,
        std::unique_ptr<Value> value);
    void set(
        const std::string &name,
        std::unique_ptr<Value> value);

    Value *get(
        const std::string &name);

    bool exists(
        const std::string &name);
};
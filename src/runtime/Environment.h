#pragma once

#include <unordered_map>
#include <string>
#include <memory>
#include <vector>

#include "values/Value.h"
#include "../errors/Error.h"

struct VariableInfo
{
    std::unique_ptr<Value> value;
    bool isConst;
    TypeInfo type;

    VariableInfo() = default;
    VariableInfo(std::unique_ptr<Value> val, bool isConst = false, TypeInfo typeInfo = TypeInfo(TypeKind::ANY))
        : value(std::move(val))
        , isConst(isConst)
        , type(std::move(typeInfo))
    {
    }
};

class Environment : public std::enable_shared_from_this<Environment>
{
private:
    std::unordered_map<std::string, VariableInfo> variables;
    std::shared_ptr<Environment> parent;
    ErrorHandler* errorHandler;

public:
    Environment()
        : parent(nullptr)
        , errorHandler(nullptr)
    {
    }

    explicit Environment(std::shared_ptr<Environment> parent, ErrorHandler* errorHandler = nullptr)
        : parent(std::move(parent))
        , errorHandler(errorHandler)
    {
    }

    void setErrorHandler(ErrorHandler* handler) { errorHandler = handler; }

    Environment* getParent() const { return parent.get(); }

    std::shared_ptr<Environment> shared() { return shared_from_this(); }

    void define(const std::string& name, std::unique_ptr<Value> value,
                bool isConst = false, const TypeInfo& type = TypeInfo(TypeKind::ANY))
    {
        if (variables.find(name) != variables.end())
        {
            if (errorHandler)
                errorHandler->error(ErrorCode::REDECLARED_VARIABLE,
                    "Variable '" + name + "' is already declared in this scope",
                    SourceLocation("", 0, 0));
            return;
        }
        variables[name] = VariableInfo(std::move(value), isConst, type);
    }

    void set(const std::string& name, std::unique_ptr<Value> value)
    {
        auto it = variables.find(name);
        if (it != variables.end())
        {
            if (it->second.isConst)
            {
                if (errorHandler)
                    errorHandler->error(ErrorCode::ASSIGN_TO_CONST,
                        "Cannot assign to constant variable '" + name + "'",
                        SourceLocation("", 0, 0));
                return;
            }
            it->second.value = std::move(value);
            return;
        }
        if (parent)
        {
            parent->set(name, std::move(value));
            return;
        }
        if (errorHandler)
            errorHandler->error(ErrorCode::UNDEFINED_VARIABLE,
                "Undefined variable '" + name + "'",
                SourceLocation("", 0, 0));
    }

    Value* get(const std::string& name)
    {
        auto it = variables.find(name);
        if (it != variables.end())
            return it->second.value.get();
        if (parent)
            return parent->get(name);
        return nullptr;
    }

    bool exists(const std::string& name) const
    {
        if (variables.find(name) != variables.end())
            return true;
        if (parent)
            return parent->exists(name);
        return false;
    }

    bool isConst(const std::string& name) const
    {
        auto it = variables.find(name);
        if (it != variables.end())
            return it->second.isConst;
        if (parent)
            return parent->isConst(name);
        return false;
    }

    TypeInfo getType(const std::string& name) const
    {
        auto it = variables.find(name);
        if (it != variables.end())
            return it->second.type;
        if (parent)
            return parent->getType(name);
        return TypeInfo(TypeKind::ANY);
    }

    std::shared_ptr<Environment> createChild()
    {
        return std::make_shared<Environment>(shared_from_this(), errorHandler);
    }

    // For debugging/inspection
    void dump() const
    {
        for (const auto& [name, info] : variables)
        {
            (void)info;
        }
    }

    std::shared_ptr<Environment> getTopLevel()
    {
        if (!parent) return shared_from_this();
        return parent->getTopLevel();
    }

    std::shared_ptr<Environment> cloneForClosure()
    {
        auto env = std::make_shared<Environment>(parent, errorHandler);
        for (const auto& [name, info] : variables)
        {
            env->variables[name] = VariableInfo(
                info.value ? info.value->clone() : nullptr,
                info.isConst, info.type);
        }
        return env;
    }
};

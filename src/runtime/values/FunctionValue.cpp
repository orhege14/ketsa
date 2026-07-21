#include "FunctionValue.h"
#include "../Environment.h"

FunctionValue::FunctionValue(std::string name, std::vector<std::string> params,
                             std::vector<std::unique_ptr<ASTNode>> body,
                             std::shared_ptr<Environment> closure,
                             bool isAnonymous)
    : name(std::move(name))
    , parameters(std::move(params))
    , body(std::move(body))
    , closure(std::move(closure))
    , isAnonymous(isAnonymous) {}

ValueType FunctionValue::getType() const { return ValueType::FUNCTION; }

std::string FunctionValue::toString() const
{
    if (isAnonymous) return "<anonymous function>";
    return "<function " + name + "(" + std::to_string(parameters.size()) + " params)>";
}

std::unique_ptr<Value> FunctionValue::clone() const
{
    std::vector<std::unique_ptr<ASTNode>> bodyCopy;
    return std::make_unique<FunctionValue>(name, parameters, std::move(bodyCopy), closure, isAnonymous);
}

TypeInfo FunctionValue::getTypeInfo() const
{
    return TypeInfo(TypeKind::FUNCTION);
}

const std::string& FunctionValue::getName() const { return name; }
const std::vector<std::string>& FunctionValue::getParameters() const { return parameters; }
std::vector<std::unique_ptr<ASTNode>>& FunctionValue::getBody() { return body; }
std::shared_ptr<Environment> FunctionValue::getClosure() const { return closure; }
bool FunctionValue::isAnonymousFunc() const { return isAnonymous; }

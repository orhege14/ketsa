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
    for (const auto& stmt : body)
        bodyCopy.push_back(stmt ? stmt->clone() : nullptr);
    auto cloned = std::make_unique<FunctionValue>(name, parameters, std::move(bodyCopy), closure, isAnonymous);
    std::vector<std::unique_ptr<DefaultParamInfo>> defaultsCopy;
    for (const auto& d : defaultValues)
    {
        auto di = std::make_unique<DefaultParamInfo>();
        if (d && d->defaultValue)
            di->defaultValue = d->defaultValue->clone();
        defaultsCopy.push_back(std::move(di));
    }
    cloned->setDefaultParams(std::move(defaultsCopy));
    return cloned;
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
void FunctionValue::setDefaultParams(std::vector<std::unique_ptr<DefaultParamInfo>> defaults) { defaultValues = std::move(defaults); }
const std::vector<std::unique_ptr<DefaultParamInfo>>& FunctionValue::getDefaultParams() const { return defaultValues; }

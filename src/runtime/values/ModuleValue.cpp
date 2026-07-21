#include "ModuleValue.h"
#include "../Environment.h"

ModuleValue::ModuleValue(std::string name, std::unique_ptr<Environment> moduleEnv)
    : name(std::move(name))
    , moduleEnv(std::move(moduleEnv)) {}

ValueType ModuleValue::getType() const { return ValueType::MODULE; }

std::string ModuleValue::toString() const { return "<module " + name + ">"; }

std::unique_ptr<Value> ModuleValue::clone() const
{
    return std::make_unique<ModuleValue>(name, std::make_unique<Environment>());
}

TypeInfo ModuleValue::getTypeInfo() const
{
    TypeInfo t(TypeKind::MODULE);
    t.name = name;
    return t;
}

const std::string& ModuleValue::getName() const { return name; }
Environment* ModuleValue::getModuleEnv() const { return moduleEnv.get(); }

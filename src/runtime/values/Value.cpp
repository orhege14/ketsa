#include "Value.h"

bool Value::isTruthy() const { return true; }

TypeInfo Value::getTypeInfo() const
{
    switch (getType())
    {
        case ValueType::INT:     return TypeInfo(TypeKind::INT);
        case ValueType::FLOAT:   return TypeInfo(TypeKind::FLOAT);
        case ValueType::DOUBLE:  return TypeInfo(TypeKind::DOUBLE);
        case ValueType::CHAR:    return TypeInfo(TypeKind::CHAR);
        case ValueType::STRING:  return TypeInfo(TypeKind::STRING);
        case ValueType::BOOLEAN: return TypeInfo(TypeKind::BOOLEAN);
        case ValueType::NULL_VALUE: return TypeInfo(TypeKind::NULL_TYPE);
        case ValueType::ARRAY:   return TypeInfo(TypeKind::ARRAY);
        case ValueType::OBJECT:  return TypeInfo(TypeKind::OBJECT);
        case ValueType::FUNCTION: return TypeInfo(TypeKind::FUNCTION);
        case ValueType::CLASS:   return TypeInfo(TypeKind::CLASS);
        case ValueType::MODULE:  return TypeInfo(TypeKind::MODULE);
        case ValueType::BUILTIN_FUNCTION: return TypeInfo(TypeKind::FUNCTION);
    }
    return TypeInfo(TypeKind::ANY);
}

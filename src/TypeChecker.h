#pragma once

#include "AST.h"
#include "errors/Error.h"
#include <unordered_map>
#include <string>
#include <memory>
#include <vector>
#include <optional>
#include <stack>
#include <set>

struct TypeBindings
{
    std::unordered_map<std::string, TypeInfo> typeParams;
};

struct FunctionSignature
{
    std::string name;
    std::vector<TypeInfo> paramTypes;
    TypeInfo returnType;
    std::vector<std::string> paramNames;
    bool isGeneric;
    std::vector<std::string> typeParams;

    bool matches(const std::vector<TypeInfo>& argTypes) const
    {
        if (argTypes.size() != paramTypes.size()) return false;
        for (size_t i = 0; i < paramTypes.size(); i++)
        {
            if (paramTypes[i].kind != TypeKind::ANY && paramTypes[i] != argTypes[i])
            {
                if (argTypes[i].kind != TypeKind::ANY)
                    return false;
            }
        }
        return true;
    }
};

struct ClassInfo
{
    std::string name;
    std::vector<std::string> fields;
    std::unordered_map<std::string, TypeInfo> fieldTypes;
    std::vector<FunctionSignature> methods;
    std::optional<std::string> baseClass;
};

struct InterfaceInfo
{
    std::string name;
    std::vector<FunctionSignature> methods;
};

struct SymbolInfo
{
    enum class Kind { VARIABLE, FUNCTION, CLASS, INTERFACE, TRAIT, MODULE, TYPE_PARAM };

    Kind kind;
    TypeInfo type;
    bool isConst;
    bool isInitialized;

    SymbolInfo() : kind(Kind::VARIABLE), isConst(false), isInitialized(false) {}
    SymbolInfo(Kind k, TypeInfo t, bool const_flag = false)
        : kind(k), type(std::move(t)), isConst(const_flag), isInitialized(true) {}
};

class TypeChecker
{
private:
    ErrorHandler* errorHandler;

    // Scope management
    struct Scope
    {
        std::unordered_map<std::string, SymbolInfo> symbols;
        std::unordered_map<std::string, FunctionSignature> functions;
        std::unordered_map<std::string, ClassInfo> classes;
        std::unordered_map<std::string, InterfaceInfo> interfaces;
        std::unordered_map<std::string, TypeInfo> typeAliases;
        Scope* parent;

        explicit Scope(Scope* p = nullptr) : parent(p) {}
    };

    Scope* currentScope;
    std::vector<std::unique_ptr<Scope>> scopeStorage;

    // Loop/function tracking
    std::stack<bool> loopContext;
    std::stack<TypeInfo> returnContext;
    bool hasReturnPath;

    // Generic type parameters for current function
    std::vector<std::string> currentTypeParams;
    std::unordered_map<std::string, TypeInfo> currentTypeBindings;

    // Defined classes/interfaces
    std::unordered_map<std::string, ClassInfo> globalClasses;
    std::unordered_map<std::string, InterfaceInfo> globalInterfaces;

    Scope* pushScope();
    void popScope();

    SymbolInfo* lookupSymbol(const std::string& name);
    FunctionSignature* lookupFunction(const std::string& name);
    ClassInfo* lookupClass(const std::string& name);
    InterfaceInfo* lookupInterface(const std::string& name);

    void declareSymbol(const std::string& name, const SymbolInfo& info);
    void declareFunction(const std::string& name, const FunctionSignature& sig);
    void declareClass(const std::string& name, const ClassInfo& info);

    TypeInfo resolveTypeAnnotation(const std::optional<TypeInfo>& declared, ASTNode* initExpr);
    TypeInfo inferType(ASTNode* node);
    TypeInfo checkExpression(ASTNode* node);
    void checkStatement(ASTNode* node);
    void checkBlock(const std::vector<std::unique_ptr<ASTNode>>& statements);

    bool isAssignable(TypeInfo target, TypeInfo source);
    TypeInfo resolveTypeParam(const TypeInfo& type);

    void checkFunctionDeclaration(FunctionDeclarationNode* node);
    void checkClassDeclaration(ClassDeclarationNode* node);

public:
    explicit TypeChecker(ErrorHandler* handler = nullptr);

    void setErrorHandler(ErrorHandler* handler) { errorHandler = handler; }

    bool check(const std::vector<std::unique_ptr<ASTNode>>& ast);

    std::vector<ErrorCode> getErrors() const;
};

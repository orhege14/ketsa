#pragma once

#include <string>
#include <vector>
#include <memory>
#include <cstdint>
#include <optional>

// ============================================================
// TYPE SYSTEM
// ============================================================

enum class TypeKind : uint8_t
{
    INT,
    FLOAT,
    DOUBLE,
    CHAR,
    STRING,
    BOOLEAN,
    VOID,
    ANY,
    NULL_TYPE,
    ARRAY,
    OBJECT,
    FUNCTION,
    CLASS,
    MODULE
};

struct TypeInfo
{
    TypeKind kind;
    std::optional<std::string> name;
    std::unique_ptr<TypeInfo> elementType;
    std::vector<TypeInfo> paramTypes;
    std::unique_ptr<TypeInfo> returnType;

    explicit TypeInfo(TypeKind kind = TypeKind::ANY)
        : kind(kind) {}

    TypeInfo(const TypeInfo& other)
        : kind(other.kind)
        , name(other.name)
        , elementType(other.elementType ? std::make_unique<TypeInfo>(*other.elementType) : nullptr)
        , paramTypes(other.paramTypes)
        , returnType(other.returnType ? std::make_unique<TypeInfo>(*other.returnType) : nullptr)
    {
    }

    TypeInfo& operator=(const TypeInfo& other)
    {
        if (this != &other)
        {
            kind = other.kind;
            name = other.name;
            elementType = other.elementType ? std::make_unique<TypeInfo>(*other.elementType) : nullptr;
            paramTypes = other.paramTypes;
            returnType = other.returnType ? std::make_unique<TypeInfo>(*other.returnType) : nullptr;
        }
        return *this;
    }

    TypeInfo(TypeInfo&&) noexcept = default;
    TypeInfo& operator=(TypeInfo&&) noexcept = default;

    std::string toString() const
    {
        switch (kind)
        {
            case TypeKind::INT:     return "int";
            case TypeKind::FLOAT:   return "float";
            case TypeKind::DOUBLE:  return "double";
            case TypeKind::CHAR:    return "char";
            case TypeKind::STRING:  return "string";
            case TypeKind::BOOLEAN: return "boolean";
            case TypeKind::VOID:    return "void";
            case TypeKind::ANY:     return "any";
            case TypeKind::NULL_TYPE: return "null";
            case TypeKind::ARRAY:
                return "[" + (elementType ? elementType->toString() : "any") + "]";
            case TypeKind::OBJECT:
                return name.value_or("object");
            case TypeKind::FUNCTION:
            {
                std::string s = "(";
                for (size_t i = 0; i < paramTypes.size(); i++)
                {
                    if (i > 0) s += ", ";
                    s += paramTypes[i].toString();
                }
                s += ") -> " + (returnType ? returnType->toString() : "void");
                return s;
            }
            case TypeKind::CLASS:   return "class";
            case TypeKind::MODULE:  return "module";
        }
        return "any";
    }

    static TypeInfo createArray(std::unique_ptr<TypeInfo> elem)
    {
        TypeInfo t(TypeKind::ARRAY);
        t.elementType = std::move(elem);
        return t;
    }

    static TypeInfo createFunction(std::vector<TypeInfo> params, std::unique_ptr<TypeInfo> ret)
    {
        TypeInfo t(TypeKind::FUNCTION);
        t.paramTypes = std::move(params);
        t.returnType = std::move(ret);
        return t;
    }

    static TypeInfo createObject(std::string className)
    {
        TypeInfo t(TypeKind::OBJECT);
        t.name = std::move(className);
        return t;
    }

    bool operator==(const TypeInfo& other) const
    {
        if (kind != other.kind) return false;
        if (kind == TypeKind::OBJECT || kind == TypeKind::CLASS)
            return name == other.name;
        if (kind == TypeKind::ARRAY)
            return (elementType == nullptr && other.elementType == nullptr) ||
                   (elementType && other.elementType && *elementType == *other.elementType);
        if (kind == TypeKind::FUNCTION)
            return paramTypes == other.paramTypes &&
                   ((returnType == nullptr && other.returnType == nullptr) ||
                    (returnType && other.returnType && *returnType == *other.returnType));
        return true;
    }

    bool operator!=(const TypeInfo& other) const { return !(*this == other); }
};

// ============================================================
// AST NODE TYPES
// ============================================================

enum class NodeType : uint8_t
{
    // Literals
    NUMBER_LITERAL,
    FLOAT_LITERAL,
    STRING_LITERAL,
    BOOLEAN_LITERAL,
    NULL_LITERAL,
    CHAR_LITERAL,

    // Variables
    VARIABLE_DECLARATION,
    VARIABLE_ACCESS,
    ASSIGNMENT,

    // Operators
    BINARY_EXPRESSION,
    UNARY_EXPRESSION,

    // Statements
    EXPRESSION_STATEMENT,
    BLOCK,
    PRINT,

    // Control Flow
    IF,
    ELSE,
    WHILE,
    FOR,
    BREAK,
    CONTINUE,
    SWITCH,
    CASE,

    // Functions
    FUNCTION_DECLARATION,
    FUNCTION_CALL,
    RETURN,

    // Classes / Objects
    CLASS_DECLARATION,
    CONSTRUCTOR,
    METHOD_DECLARATION,
    OBJECT_LITERAL,
    MEMBER_ACCESS,
    MEMBER_ASSIGNMENT,
    THIS_EXPRESSION,
    NEW_EXPRESSION,

    // Collections
    ARRAY_LITERAL,
    ARRAY_ACCESS,
    INDEX_ASSIGNMENT,

    // Modules
    IMPORT_STATEMENT,
    FROM_IMPORT,
    MODULE_DECLARATION,

    // Pattern Matching
    MATCH,
    MATCH_CASE,

    // Program
    PROGRAM
};

struct ASTNode
{
    NodeType type;
    int line;
    int column;

    explicit ASTNode(NodeType type, int line = 0, int column = 0)
        : type(type), line(line), column(column) {}

    virtual ~ASTNode() = default;
};

// ============================================================
// LITERALS
// ============================================================

struct NumberNode : ASTNode
{
    int64_t value;

    explicit NumberNode(int64_t value, int line = 0, int column = 0)
        : ASTNode(NodeType::NUMBER_LITERAL, line, column), value(value) {}
};

struct FloatNode : ASTNode
{
    double value;

    explicit FloatNode(double value, int line = 0, int column = 0)
        : ASTNode(NodeType::FLOAT_LITERAL, line, column), value(value) {}
};

struct StringNode : ASTNode
{
    std::string value;

    explicit StringNode(std::string value, int line = 0, int column = 0)
        : ASTNode(NodeType::STRING_LITERAL, line, column), value(std::move(value)) {}
};

struct BooleanNode : ASTNode
{
    bool value;

    explicit BooleanNode(bool value, int line = 0, int column = 0)
        : ASTNode(NodeType::BOOLEAN_LITERAL, line, column), value(value) {}
};

struct CharNode : ASTNode
{
    char value;

    explicit CharNode(char value, int line = 0, int column = 0)
        : ASTNode(NodeType::CHAR_LITERAL, line, column), value(value) {}
};

struct NullNode : ASTNode
{
    explicit NullNode(int line = 0, int column = 0)
        : ASTNode(NodeType::NULL_LITERAL, line, column) {}
};

// ============================================================
// VARIABLES
// ============================================================

struct VariableDeclarationNode : ASTNode
{
    std::string name;
    std::unique_ptr<ASTNode> initializer;
    std::optional<TypeInfo> declaredType;
    bool isConst;

    VariableDeclarationNode(
        std::string name,
        std::unique_ptr<ASTNode> initializer,
        std::optional<TypeInfo> declaredType = std::nullopt,
        bool isConst = false,
        int line = 0,
        int column = 0)
        : ASTNode(NodeType::VARIABLE_DECLARATION, line, column)
        , name(std::move(name))
        , initializer(std::move(initializer))
        , declaredType(std::move(declaredType))
        , isConst(isConst) {}
};

struct VariableAccessNode : ASTNode
{
    std::string name;

    explicit VariableAccessNode(std::string name, int line = 0, int column = 0)
        : ASTNode(NodeType::VARIABLE_ACCESS, line, column), name(std::move(name)) {}
};

struct AssignmentNode : ASTNode
{
    std::string name;
    std::unique_ptr<ASTNode> value;
    std::string op;

    AssignmentNode(std::string name, std::unique_ptr<ASTNode> value, std::string op = "=",
                   int line = 0, int column = 0)
        : ASTNode(NodeType::ASSIGNMENT, line, column)
        , name(std::move(name))
        , value(std::move(value))
        , op(std::move(op)) {}
};

// ============================================================
// OPERATORS
// ============================================================

struct UnaryExpressionNode : ASTNode
{
    std::string op;
    std::unique_ptr<ASTNode> operand;
    bool prefix;

    UnaryExpressionNode(std::string op, std::unique_ptr<ASTNode> operand, bool prefix = true,
                        int line = 0, int column = 0)
        : ASTNode(NodeType::UNARY_EXPRESSION, line, column)
        , op(std::move(op))
        , operand(std::move(operand))
        , prefix(prefix) {}
};

struct BinaryExpressionNode : ASTNode
{
    std::string op;
    std::unique_ptr<ASTNode> left;
    std::unique_ptr<ASTNode> right;

    BinaryExpressionNode(std::string op, std::unique_ptr<ASTNode> left,
                         std::unique_ptr<ASTNode> right, int line = 0, int column = 0)
        : ASTNode(NodeType::BINARY_EXPRESSION, line, column)
        , op(std::move(op))
        , left(std::move(left))
        , right(std::move(right)) {}
};

// ============================================================
// STATEMENTS
// ============================================================

struct ExpressionStatementNode : ASTNode
{
    std::unique_ptr<ASTNode> expression;

    explicit ExpressionStatementNode(std::unique_ptr<ASTNode> expression, int line = 0, int column = 0)
        : ASTNode(NodeType::EXPRESSION_STATEMENT, line, column)
        , expression(std::move(expression)) {}
};

struct BlockNode : ASTNode
{
    std::vector<std::unique_ptr<ASTNode>> statements;

    BlockNode(int line = 0, int column = 0)
        : ASTNode(NodeType::BLOCK, line, column) {}
};

struct PrintNode : ASTNode
{
    std::unique_ptr<ASTNode> expression;

    explicit PrintNode(std::unique_ptr<ASTNode> expression, int line = 0, int column = 0)
        : ASTNode(NodeType::PRINT, line, column)
        , expression(std::move(expression)) {}
};

// ============================================================
// CONTROL FLOW
// ============================================================

struct IfNode : ASTNode
{
    std::unique_ptr<ASTNode> condition;
    std::vector<std::unique_ptr<ASTNode>> thenBlock;
    std::vector<std::unique_ptr<ASTNode>> elseBlock;

    IfNode(std::unique_ptr<ASTNode> condition, int line = 0, int column = 0)
        : ASTNode(NodeType::IF, line, column)
        , condition(std::move(condition)) {}
};

struct WhileNode : ASTNode
{
    std::unique_ptr<ASTNode> condition;
    std::vector<std::unique_ptr<ASTNode>> body;

    WhileNode(std::unique_ptr<ASTNode> condition, int line = 0, int column = 0)
        : ASTNode(NodeType::WHILE, line, column)
        , condition(std::move(condition)) {}
};

struct ForNode : ASTNode
{
    std::string variable;
    std::unique_ptr<ASTNode> iterable;
    std::vector<std::unique_ptr<ASTNode>> body;

    ForNode(std::string variable, std::unique_ptr<ASTNode> iterable, int line = 0, int column = 0)
        : ASTNode(NodeType::FOR, line, column)
        , variable(std::move(variable))
        , iterable(std::move(iterable)) {}
};

struct BreakNode : ASTNode
{
    explicit BreakNode(int line = 0, int column = 0)
        : ASTNode(NodeType::BREAK, line, column) {}
};

struct ContinueNode : ASTNode
{
    explicit ContinueNode(int line = 0, int column = 0)
        : ASTNode(NodeType::CONTINUE, line, column) {}
};

struct SwitchCase
{
    std::unique_ptr<ASTNode> value;
    std::vector<std::unique_ptr<ASTNode>> body;
};

struct SwitchNode : ASTNode
{
    std::unique_ptr<ASTNode> expression;
    std::vector<SwitchCase> cases;
    std::vector<std::unique_ptr<ASTNode>> defaultCase;

    SwitchNode(std::unique_ptr<ASTNode> expression, int line = 0, int column = 0)
        : ASTNode(NodeType::SWITCH, line, column)
        , expression(std::move(expression)) {}
};

// ============================================================
// PATTERN MATCHING
// ============================================================

enum class PatternKind : uint8_t
{
    LITERAL,
    VARIABLE,
    WILDCARD,
    DESTRUCTURE_ARRAY,
    DESTRUCTURE_OBJECT,
    GUARD
};

struct MatchPattern
{
    PatternKind kind;
    std::unique_ptr<ASTNode> literal;
    std::string variableName;
    std::vector<MatchPattern> subPatterns;
    std::vector<std::pair<std::string, MatchPattern>> objectPatterns;
    std::unique_ptr<ASTNode> guard;

    MatchPattern() : kind(PatternKind::WILDCARD) {}
};

struct MatchCaseNode : ASTNode
{
    MatchPattern pattern;
    std::vector<std::unique_ptr<ASTNode>> body;

    MatchCaseNode(int line = 0, int column = 0)
        : ASTNode(NodeType::MATCH_CASE, line, column) {}
};

struct MatchNode : ASTNode
{
    std::unique_ptr<ASTNode> expression;
    std::vector<MatchCaseNode> cases;
    std::vector<std::unique_ptr<ASTNode>> elseBody;

    MatchNode(std::unique_ptr<ASTNode> expression, int line = 0, int column = 0)
        : ASTNode(NodeType::MATCH, line, column)
        , expression(std::move(expression)) {}
};

// ============================================================
// FUNCTIONS
// ============================================================

struct ParameterNode
{
    std::string name;
    TypeInfo type;

    ParameterNode(std::string name, TypeInfo type = TypeInfo(TypeKind::ANY))
        : name(std::move(name)), type(std::move(type)) {}
};

struct FunctionDeclarationNode : ASTNode
{
    std::string name;
    std::vector<ParameterNode> parameters;
    TypeInfo returnType;
    std::vector<std::unique_ptr<ASTNode>> body;

    FunctionDeclarationNode()
        : ASTNode(NodeType::FUNCTION_DECLARATION, 0, 0)
        , returnType(TypeKind::VOID) {}

    FunctionDeclarationNode(std::string name, int line = 0, int column = 0)
        : ASTNode(NodeType::FUNCTION_DECLARATION, line, column)
        , name(std::move(name))
        , returnType(TypeKind::VOID) {}

    FunctionDeclarationNode(FunctionDeclarationNode&&) = default;
    FunctionDeclarationNode& operator=(FunctionDeclarationNode&&) = default;
};

struct FunctionCallNode : ASTNode
{
    std::string functionName;
    std::vector<std::unique_ptr<ASTNode>> arguments;

    FunctionCallNode(std::string functionName, int line = 0, int column = 0)
        : ASTNode(NodeType::FUNCTION_CALL, line, column)
        , functionName(std::move(functionName)) {}
};

struct ReturnNode : ASTNode
{
    std::unique_ptr<ASTNode> value;

    explicit ReturnNode(std::unique_ptr<ASTNode> value, int line = 0, int column = 0)
        : ASTNode(NodeType::RETURN, line, column)
        , value(std::move(value)) {}
};

// ============================================================
// CLASSES / OOP
// ============================================================

struct MethodDeclaration
{
    FunctionDeclarationNode function;

    MethodDeclaration() = default;
    MethodDeclaration(MethodDeclaration&&) = default;
    MethodDeclaration& operator=(MethodDeclaration&&) = default;
};

struct ClassDeclarationNode : ASTNode
{
    std::string name;
    std::vector<ParameterNode> constructorParams;
    std::vector<std::unique_ptr<ASTNode>> constructorBody;
    std::vector<std::string> fields;
    std::vector<MethodDeclaration> methods;
    std::optional<std::string> baseClass;

    ClassDeclarationNode(ClassDeclarationNode&&) = default;

    explicit ClassDeclarationNode(std::string name, int line = 0, int column = 0)
        : ASTNode(NodeType::CLASS_DECLARATION, line, column)
        , name(std::move(name)) {}
};

struct ObjectLiteralNode : ASTNode
{
    std::vector<std::pair<std::string, std::unique_ptr<ASTNode>>> properties;

    ObjectLiteralNode(int line = 0, int column = 0)
        : ASTNode(NodeType::OBJECT_LITERAL, line, column) {}
};

struct MemberAccessNode : ASTNode
{
    std::unique_ptr<ASTNode> object;
    std::string member;

    MemberAccessNode(std::unique_ptr<ASTNode> object, std::string member,
                     int line = 0, int column = 0)
        : ASTNode(NodeType::MEMBER_ACCESS, line, column)
        , object(std::move(object))
        , member(std::move(member)) {}
};

struct MemberAssignmentNode : ASTNode
{
    std::unique_ptr<ASTNode> object;
    std::string member;
    std::unique_ptr<ASTNode> value;

    MemberAssignmentNode(std::unique_ptr<ASTNode> object, std::string member,
                         std::unique_ptr<ASTNode> value, int line = 0, int column = 0)
        : ASTNode(NodeType::MEMBER_ASSIGNMENT, line, column)
        , object(std::move(object))
        , member(std::move(member))
        , value(std::move(value)) {}
};

struct NewExpressionNode : ASTNode
{
    std::string className;
    std::vector<std::unique_ptr<ASTNode>> arguments;

    NewExpressionNode(std::string className, int line = 0, int column = 0)
        : ASTNode(NodeType::NEW_EXPRESSION, line, column)
        , className(std::move(className)) {}
};

// ============================================================
// COLLECTIONS
// ============================================================

struct ArrayLiteralNode : ASTNode
{
    std::vector<std::unique_ptr<ASTNode>> elements;

    ArrayLiteralNode(int line = 0, int column = 0)
        : ASTNode(NodeType::ARRAY_LITERAL, line, column) {}
};

struct ArrayAccessNode : ASTNode
{
    std::unique_ptr<ASTNode> array;
    std::unique_ptr<ASTNode> index;

    ArrayAccessNode(std::unique_ptr<ASTNode> array, std::unique_ptr<ASTNode> index,
                    int line = 0, int column = 0)
        : ASTNode(NodeType::ARRAY_ACCESS, line, column)
        , array(std::move(array))
        , index(std::move(index)) {}
};

struct IndexAssignmentNode : ASTNode
{
    std::unique_ptr<ASTNode> array;
    std::unique_ptr<ASTNode> index;
    std::unique_ptr<ASTNode> value;

    IndexAssignmentNode(std::unique_ptr<ASTNode> array, std::unique_ptr<ASTNode> index,
                        std::unique_ptr<ASTNode> value, int line = 0, int column = 0)
        : ASTNode(NodeType::INDEX_ASSIGNMENT, line, column)
        , array(std::move(array))
        , index(std::move(index))
        , value(std::move(value)) {}
};

// ============================================================
// MODULES
// ============================================================

struct ImportNode : ASTNode
{
    std::string moduleName;

    explicit ImportNode(std::string moduleName, int line = 0, int column = 0)
        : ASTNode(NodeType::IMPORT_STATEMENT, line, column)
        , moduleName(std::move(moduleName)) {}
};

struct FromImportNode : ASTNode
{
    std::string moduleName;
    std::vector<std::string> names;

    FromImportNode(std::string moduleName, int line = 0, int column = 0)
        : ASTNode(NodeType::FROM_IMPORT, line, column)
        , moduleName(std::move(moduleName)) {}
};

// ============================================================
// PROGRAM
// ============================================================

struct ProgramNode : ASTNode
{
    std::vector<std::unique_ptr<ASTNode>> statements;

    ProgramNode(int line = 0, int column = 0)
        : ASTNode(NodeType::PROGRAM, line, column) {}
};

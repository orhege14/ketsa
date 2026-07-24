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
    MODULE,
    UNION,
    OPTIONAL,
    TYPE_PARAM
};

struct TypeConstraint
{
    std::string typeParamName;
    std::string boundName;
};

struct TypeInfo
{
    TypeKind kind;
    std::optional<std::string> name;
    std::unique_ptr<TypeInfo> elementType;
    std::vector<TypeInfo> paramTypes;
    std::unique_ptr<TypeInfo> returnType;
    std::unique_ptr<TypeInfo> optionalType;
    std::vector<TypeInfo> unionTypes;
    std::optional<TypeConstraint> constraint;
    std::vector<std::string> genericParams;

    explicit TypeInfo(TypeKind kind = TypeKind::ANY)
        : kind(kind) {}

    TypeInfo(const TypeInfo& other)
        : kind(other.kind)
        , name(other.name)
        , elementType(other.elementType ? std::make_unique<TypeInfo>(*other.elementType) : nullptr)
        , paramTypes(other.paramTypes)
        , returnType(other.returnType ? std::make_unique<TypeInfo>(*other.returnType) : nullptr)
        , optionalType(other.optionalType ? std::make_unique<TypeInfo>(*other.optionalType) : nullptr)
        , unionTypes(other.unionTypes)
        , constraint(other.constraint)
        , genericParams(other.genericParams)
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
            optionalType = other.optionalType ? std::make_unique<TypeInfo>(*other.optionalType) : nullptr;
            unionTypes = other.unionTypes;
            constraint = other.constraint;
            genericParams = other.genericParams;
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
            case TypeKind::TYPE_PARAM: return name.value_or("T");
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
            case TypeKind::CLASS:   return name.value_or("class");
            case TypeKind::MODULE:  return "module";
            case TypeKind::OPTIONAL:
                return (optionalType ? optionalType->toString() : "any") + "?";
            case TypeKind::UNION:
            {
                std::string s;
                for (size_t i = 0; i < unionTypes.size(); i++)
                {
                    if (i > 0) s += " | ";
                    s += unionTypes[i].toString();
                }
                return s;
            }
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

    static TypeInfo createOptional(std::unique_ptr<TypeInfo> inner)
    {
        TypeInfo t(TypeKind::OPTIONAL);
        t.optionalType = std::move(inner);
        return t;
    }

    static TypeInfo createUnion(std::vector<TypeInfo> types)
    {
        TypeInfo t(TypeKind::UNION);
        t.unionTypes = std::move(types);
        return t;
    }

    static TypeInfo createTypeParam(std::string paramName)
    {
        TypeInfo t(TypeKind::TYPE_PARAM);
        t.name = std::move(paramName);
        return t;
    }

    bool operator==(const TypeInfo& other) const
    {
        if (kind != other.kind) return false;
        switch (kind)
        {
            case TypeKind::OBJECT:
            case TypeKind::CLASS:
            case TypeKind::TYPE_PARAM:
                return name == other.name;
            case TypeKind::ARRAY:
                return (elementType == nullptr && other.elementType == nullptr) ||
                       (elementType && other.elementType && *elementType == *other.elementType);
            case TypeKind::FUNCTION:
                return paramTypes == other.paramTypes &&
                       ((returnType == nullptr && other.returnType == nullptr) ||
                        (returnType && other.returnType && *returnType == *other.returnType));
            case TypeKind::OPTIONAL:
                return (optionalType == nullptr && other.optionalType == nullptr) ||
                       (optionalType && other.optionalType && *optionalType == *other.optionalType);
            case TypeKind::UNION:
                return unionTypes == other.unionTypes;
            default:
                return true;
        }
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

    // Pattern Matching
    MATCH,
    MATCH_CASE,

    // Type System
    TYPE_ALIAS,
    LAMBDA,
    NAMED_ARGUMENT,

    // Error Handling
    TRY,
    CATCH,
    THROW,

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
    virtual std::unique_ptr<ASTNode> clone() const = 0;
};

// ============================================================
// LITERALS
// ============================================================

struct NumberNode : ASTNode
{
    int64_t value;

    explicit NumberNode(int64_t value, int line = 0, int column = 0)
        : ASTNode(NodeType::NUMBER_LITERAL, line, column), value(value) {}

    std::unique_ptr<ASTNode> clone() const override
    {
        return std::make_unique<NumberNode>(value, line, column);
    }
};

struct FloatNode : ASTNode
{
    double value;

    explicit FloatNode(double value, int line = 0, int column = 0)
        : ASTNode(NodeType::FLOAT_LITERAL, line, column), value(value) {}

    std::unique_ptr<ASTNode> clone() const override
    {
        return std::make_unique<FloatNode>(value, line, column);
    }
};

struct StringNode : ASTNode
{
    std::string value;

    explicit StringNode(std::string value, int line = 0, int column = 0)
        : ASTNode(NodeType::STRING_LITERAL, line, column), value(std::move(value)) {}

    std::unique_ptr<ASTNode> clone() const override
    {
        return std::make_unique<StringNode>(value, line, column);
    }
};

struct BooleanNode : ASTNode
{
    bool value;

    explicit BooleanNode(bool value, int line = 0, int column = 0)
        : ASTNode(NodeType::BOOLEAN_LITERAL, line, column), value(value) {}

    std::unique_ptr<ASTNode> clone() const override
    {
        return std::make_unique<BooleanNode>(value, line, column);
    }
};

struct CharNode : ASTNode
{
    char value;

    explicit CharNode(char value, int line = 0, int column = 0)
        : ASTNode(NodeType::CHAR_LITERAL, line, column), value(value) {}

    std::unique_ptr<ASTNode> clone() const override
    {
        return std::make_unique<CharNode>(value, line, column);
    }
};

struct NullNode : ASTNode
{
    explicit NullNode(int line = 0, int column = 0)
        : ASTNode(NodeType::NULL_LITERAL, line, column) {}

    std::unique_ptr<ASTNode> clone() const override
    {
        return std::make_unique<NullNode>(line, column);
    }
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
    bool isMutable;

    VariableDeclarationNode(
        std::string name,
        std::unique_ptr<ASTNode> initializer,
        std::optional<TypeInfo> declaredType = std::nullopt,
        bool isConst = false,
        bool isMutable = false,
        int line = 0,
        int column = 0)
        : ASTNode(NodeType::VARIABLE_DECLARATION, line, column)
        , name(std::move(name))
        , initializer(std::move(initializer))
        , declaredType(std::move(declaredType))
        , isConst(isConst)
        , isMutable(isMutable) {}

    std::unique_ptr<ASTNode> clone() const override
    {
        return std::make_unique<VariableDeclarationNode>(
            name,
            initializer ? initializer->clone() : nullptr,
            declaredType, isConst, isMutable, line, column);
    }
};

struct VariableAccessNode : ASTNode
{
    std::string name;

    explicit VariableAccessNode(std::string name, int line = 0, int column = 0)
        : ASTNode(NodeType::VARIABLE_ACCESS, line, column), name(std::move(name)) {}

    std::unique_ptr<ASTNode> clone() const override
    {
        return std::make_unique<VariableAccessNode>(name, line, column);
    }
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

    std::unique_ptr<ASTNode> clone() const override
    {
        return std::make_unique<AssignmentNode>(
            name, value ? value->clone() : nullptr, op, line, column);
    }
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

    std::unique_ptr<ASTNode> clone() const override
    {
        return std::make_unique<UnaryExpressionNode>(
            op, operand ? operand->clone() : nullptr, prefix, line, column);
    }
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

    std::unique_ptr<ASTNode> clone() const override
    {
        return std::make_unique<BinaryExpressionNode>(
            op,
            left ? left->clone() : nullptr,
            right ? right->clone() : nullptr,
            line, column);
    }
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

    std::unique_ptr<ASTNode> clone() const override
    {
        return std::make_unique<ExpressionStatementNode>(
            expression ? expression->clone() : nullptr, line, column);
    }
};

struct BlockNode : ASTNode
{
    std::vector<std::unique_ptr<ASTNode>> statements;

    BlockNode(int line = 0, int column = 0)
        : ASTNode(NodeType::BLOCK, line, column) {}

    std::unique_ptr<ASTNode> clone() const override
    {
        auto block = std::make_unique<BlockNode>(line, column);
        for (const auto& stmt : statements)
            block->statements.push_back(stmt ? stmt->clone() : nullptr);
        return block;
    }
};

struct PrintNode : ASTNode
{
    std::unique_ptr<ASTNode> expression;

    explicit PrintNode(std::unique_ptr<ASTNode> expression, int line = 0, int column = 0)
        : ASTNode(NodeType::PRINT, line, column)
        , expression(std::move(expression)) {}

    std::unique_ptr<ASTNode> clone() const override
    {
        return std::make_unique<PrintNode>(
            expression ? expression->clone() : nullptr, line, column);
    }
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

    std::unique_ptr<ASTNode> clone() const override
    {
        auto ifNode = std::make_unique<IfNode>(
            condition ? condition->clone() : nullptr, line, column);
        for (const auto& stmt : thenBlock)
            ifNode->thenBlock.push_back(stmt ? stmt->clone() : nullptr);
        for (const auto& stmt : elseBlock)
            ifNode->elseBlock.push_back(stmt ? stmt->clone() : nullptr);
        return ifNode;
    }
};

struct WhileNode : ASTNode
{
    std::unique_ptr<ASTNode> condition;
    std::vector<std::unique_ptr<ASTNode>> body;

    WhileNode(std::unique_ptr<ASTNode> condition, int line = 0, int column = 0)
        : ASTNode(NodeType::WHILE, line, column)
        , condition(std::move(condition)) {}

    std::unique_ptr<ASTNode> clone() const override
    {
        auto whileNode = std::make_unique<WhileNode>(
            condition ? condition->clone() : nullptr, line, column);
        for (const auto& stmt : body)
            whileNode->body.push_back(stmt ? stmt->clone() : nullptr);
        return whileNode;
    }
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

    std::unique_ptr<ASTNode> clone() const override
    {
        auto forNode = std::make_unique<ForNode>(
            variable, iterable ? iterable->clone() : nullptr, line, column);
        for (const auto& stmt : body)
            forNode->body.push_back(stmt ? stmt->clone() : nullptr);
        return forNode;
    }
};

struct BreakNode : ASTNode
{
    explicit BreakNode(int line = 0, int column = 0)
        : ASTNode(NodeType::BREAK, line, column) {}

    std::unique_ptr<ASTNode> clone() const override
    {
        return std::make_unique<BreakNode>(line, column);
    }
};

struct ContinueNode : ASTNode
{
    explicit ContinueNode(int line = 0, int column = 0)
        : ASTNode(NodeType::CONTINUE, line, column) {}

    std::unique_ptr<ASTNode> clone() const override
    {
        return std::make_unique<ContinueNode>(line, column);
    }
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

    std::unique_ptr<ASTNode> clone() const override
    {
        auto switchNode = std::make_unique<SwitchNode>(
            expression ? expression->clone() : nullptr, line, column);
        for (const auto& case_ : cases)
        {
            SwitchCase sc;
            sc.value = case_.value ? case_.value->clone() : nullptr;
            for (const auto& stmt : case_.body)
                sc.body.push_back(stmt ? stmt->clone() : nullptr);
            switchNode->cases.push_back(std::move(sc));
        }
        for (const auto& stmt : defaultCase)
            switchNode->defaultCase.push_back(stmt ? stmt->clone() : nullptr);
        return switchNode;
    }
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

    std::unique_ptr<ASTNode> clone() const override
    {
        auto mcn = std::make_unique<MatchCaseNode>(line, column);
        mcn->pattern.kind = pattern.kind;
        mcn->pattern.variableName = pattern.variableName;
        mcn->pattern.literal = pattern.literal ? pattern.literal->clone() : nullptr;
        mcn->pattern.guard = pattern.guard ? pattern.guard->clone() : nullptr;
        for (const auto& sp : pattern.subPatterns)
        {
            MatchPattern cp;
            cp.kind = sp.kind;
            cp.variableName = sp.variableName;
            cp.literal = sp.literal ? sp.literal->clone() : nullptr;
            cp.guard = sp.guard ? sp.guard->clone() : nullptr;
            mcn->pattern.subPatterns.push_back(std::move(cp));
        }
        for (const auto& [key, op] : pattern.objectPatterns)
        {
            MatchPattern cp;
            cp.kind = op.kind;
            cp.variableName = op.variableName;
            cp.literal = op.literal ? op.literal->clone() : nullptr;
            cp.guard = op.guard ? op.guard->clone() : nullptr;
            mcn->pattern.objectPatterns.emplace_back(key, std::move(cp));
        }
        for (const auto& stmt : body)
            mcn->body.push_back(stmt ? stmt->clone() : nullptr);
        return mcn;
    }
};

struct MatchNode : ASTNode
{
    std::unique_ptr<ASTNode> expression;
    std::vector<MatchCaseNode> cases;
    std::vector<std::unique_ptr<ASTNode>> elseBody;

    MatchNode(std::unique_ptr<ASTNode> expression, int line = 0, int column = 0)
        : ASTNode(NodeType::MATCH, line, column)
        , expression(std::move(expression)) {}

    std::unique_ptr<ASTNode> clone() const override
    {
        auto mn = std::make_unique<MatchNode>(
            expression ? expression->clone() : nullptr, line, column);
        for (const auto& c : cases)
        {
            auto cloned = c.clone();
            std::unique_ptr<MatchCaseNode> mcn(static_cast<MatchCaseNode*>(cloned.release()));
            mn->cases.push_back(std::move(*mcn));
        }
        for (const auto& stmt : elseBody)
            mn->elseBody.push_back(stmt ? stmt->clone() : nullptr);
        return mn;
    }
};

// ============================================================
// FUNCTIONS
// ============================================================

struct ParameterNode
{
    std::string name;
    TypeInfo type;
    std::unique_ptr<ASTNode> defaultValue;

    ParameterNode() : type(TypeKind::ANY) {}

    ParameterNode(std::string name, TypeInfo type = TypeInfo(TypeKind::ANY))
        : name(std::move(name)), type(std::move(type)) {}

    ParameterNode(const ParameterNode& other)
        : name(other.name), type(other.type)
        , defaultValue(other.defaultValue ? other.defaultValue->clone() : nullptr) {}

    ParameterNode& operator=(const ParameterNode& other)
    {
        if (this != &other)
        {
            name = other.name;
            type = other.type;
            defaultValue = other.defaultValue ? other.defaultValue->clone() : nullptr;
        }
        return *this;
    }

    ParameterNode(ParameterNode&&) = default;
    ParameterNode& operator=(ParameterNode&&) = default;
};

struct FunctionDeclarationNode : ASTNode
{
    std::string name;
    std::vector<ParameterNode> parameters;
    TypeInfo returnType;
    std::vector<std::unique_ptr<ASTNode>> body;
    std::vector<std::string> genericParams;
    std::vector<TypeConstraint> genericConstraints;

    FunctionDeclarationNode()
        : ASTNode(NodeType::FUNCTION_DECLARATION, 0, 0)
        , returnType(TypeKind::VOID) {}

    FunctionDeclarationNode(std::string name, int line = 0, int column = 0)
        : ASTNode(NodeType::FUNCTION_DECLARATION, line, column)
        , name(std::move(name))
        , returnType(TypeKind::VOID) {}

    FunctionDeclarationNode(FunctionDeclarationNode&&) = default;
    FunctionDeclarationNode& operator=(FunctionDeclarationNode&&) = default;

    std::unique_ptr<ASTNode> clone() const override
    {
        auto func = std::make_unique<FunctionDeclarationNode>(name, line, column);
        func->parameters = parameters;
        func->returnType = returnType;
        func->genericParams = genericParams;
        func->genericConstraints = genericConstraints;
        for (const auto& stmt : body)
            func->body.push_back(stmt ? stmt->clone() : nullptr);
        return func;
    }
};

struct NamedArgumentNode : ASTNode
{
    std::string name;
    std::unique_ptr<ASTNode> value;

    NamedArgumentNode(std::string name, std::unique_ptr<ASTNode> value, int line = 0, int column = 0)
        : ASTNode(NodeType::NAMED_ARGUMENT, line, column)
        , name(std::move(name))
        , value(std::move(value)) {}

    std::unique_ptr<ASTNode> clone() const override
    {
        return std::make_unique<NamedArgumentNode>(name, value ? value->clone() : nullptr, line, column);
    }
};

struct FunctionCallNode : ASTNode
{
    std::string functionName;
    std::vector<std::unique_ptr<ASTNode>> arguments;
    std::vector<NamedArgumentNode> namedArguments;

    FunctionCallNode(std::string functionName, int line = 0, int column = 0)
        : ASTNode(NodeType::FUNCTION_CALL, line, column)
        , functionName(std::move(functionName)) {}

    std::unique_ptr<ASTNode> clone() const override
    {
        auto fc = std::make_unique<FunctionCallNode>(functionName, line, column);
        for (const auto& arg : arguments)
            fc->arguments.push_back(arg ? arg->clone() : nullptr);
        for (const auto& na : namedArguments)
            fc->namedArguments.push_back(NamedArgumentNode(na.name, na.value ? na.value->clone() : nullptr, line, column));
        return fc;
    }
};

struct ReturnNode : ASTNode
{
    std::unique_ptr<ASTNode> value;

    explicit ReturnNode(std::unique_ptr<ASTNode> value, int line = 0, int column = 0)
        : ASTNode(NodeType::RETURN, line, column)
        , value(std::move(value)) {}

    std::unique_ptr<ASTNode> clone() const override
    {
        return std::make_unique<ReturnNode>(
            value ? value->clone() : nullptr, line, column);
    }
};

struct LambdaNode : ASTNode
{
    std::vector<ParameterNode> parameters;
    TypeInfo returnType;
    std::vector<std::unique_ptr<ASTNode>> body;
    bool isExpressionBody;

    LambdaNode(int line = 0, int column = 0)
        : ASTNode(NodeType::LAMBDA, line, column)
        , returnType(TypeKind::ANY)
        , isExpressionBody(false) {}

    LambdaNode(LambdaNode&&) = default;

    std::unique_ptr<ASTNode> clone() const override
    {
        auto ln = std::make_unique<LambdaNode>(line, column);
        ln->parameters = parameters;
        ln->returnType = returnType;
        ln->isExpressionBody = isExpressionBody;
        for (const auto& stmt : body)
            ln->body.push_back(stmt ? stmt->clone() : nullptr);
        return ln;
    }
};

struct CatchClause
{
    std::string variableName;
    TypeInfo errorType;
    std::vector<std::unique_ptr<ASTNode>> body;
};

struct TryNode : ASTNode
{
    std::vector<std::unique_ptr<ASTNode>> tryBody;
    std::vector<CatchClause> catchClauses;
    std::vector<std::unique_ptr<ASTNode>> finallyBody;

    TryNode(int line = 0, int column = 0)
        : ASTNode(NodeType::TRY, line, column) {}

    std::unique_ptr<ASTNode> clone() const override
    {
        auto tn = std::make_unique<TryNode>(line, column);
        for (const auto& stmt : tryBody)
            tn->tryBody.push_back(stmt ? stmt->clone() : nullptr);
        for (const auto& cc : catchClauses)
        {
            CatchClause c;
            c.variableName = cc.variableName;
            c.errorType = cc.errorType;
            for (const auto& stmt : cc.body)
                c.body.push_back(stmt ? stmt->clone() : nullptr);
            tn->catchClauses.push_back(std::move(c));
        }
        for (const auto& stmt : finallyBody)
            tn->finallyBody.push_back(stmt ? stmt->clone() : nullptr);
        return tn;
    }
};

struct ThrowNode : ASTNode
{
    std::unique_ptr<ASTNode> expression;

    ThrowNode(std::unique_ptr<ASTNode> expression, int line = 0, int column = 0)
        : ASTNode(NodeType::THROW, line, column)
        , expression(std::move(expression)) {}

    std::unique_ptr<ASTNode> clone() const override
    {
        return std::make_unique<ThrowNode>(
            expression ? expression->clone() : nullptr, line, column);
    }
};

struct TypeAliasNode : ASTNode
{
    std::string aliasName;
    TypeInfo underlyingType;

    TypeAliasNode(std::string aliasName, TypeInfo underlyingType, int line = 0, int column = 0)
        : ASTNode(NodeType::TYPE_ALIAS, line, column)
        , aliasName(std::move(aliasName))
        , underlyingType(std::move(underlyingType)) {}

    std::unique_ptr<ASTNode> clone() const override
    {
        return std::make_unique<TypeAliasNode>(aliasName, underlyingType, line, column);
    }
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
    std::vector<std::string> genericParams;
    std::vector<TypeConstraint> genericConstraints;
    std::vector<std::string> implements;

    ClassDeclarationNode(ClassDeclarationNode&&) = default;

    explicit ClassDeclarationNode(std::string name, int line = 0, int column = 0)
        : ASTNode(NodeType::CLASS_DECLARATION, line, column)
        , name(std::move(name)) {}

    std::unique_ptr<ASTNode> clone() const override
    {
        auto cd = std::make_unique<ClassDeclarationNode>(name, line, column);
        cd->constructorParams = constructorParams;
        for (const auto& stmt : constructorBody)
            cd->constructorBody.push_back(stmt ? stmt->clone() : nullptr);
        cd->fields = fields;
        cd->genericParams = genericParams;
        cd->genericConstraints = genericConstraints;
        cd->implements = implements;
        for (const auto& m : methods)
        {
            MethodDeclaration md;
            auto cloned = m.function.clone();
            if (cloned)
                md.function = std::move(*static_cast<FunctionDeclarationNode*>(cloned.get()));
            cd->methods.push_back(std::move(md));
        }
        cd->baseClass = baseClass;
        return cd;
    }
};

struct ObjectLiteralNode : ASTNode
{
    std::vector<std::pair<std::string, std::unique_ptr<ASTNode>>> properties;

    ObjectLiteralNode(int line = 0, int column = 0)
        : ASTNode(NodeType::OBJECT_LITERAL, line, column) {}

    std::unique_ptr<ASTNode> clone() const override
    {
        auto ol = std::make_unique<ObjectLiteralNode>(line, column);
        for (const auto& [key, val] : properties)
            ol->properties.emplace_back(key, val ? val->clone() : nullptr);
        return ol;
    }
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

    std::unique_ptr<ASTNode> clone() const override
    {
        return std::make_unique<MemberAccessNode>(
            object ? object->clone() : nullptr, member, line, column);
    }
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

    std::unique_ptr<ASTNode> clone() const override
    {
        return std::make_unique<MemberAssignmentNode>(
            object ? object->clone() : nullptr, member,
            value ? value->clone() : nullptr, line, column);
    }
};

struct NewExpressionNode : ASTNode
{
    std::string className;
    std::vector<std::unique_ptr<ASTNode>> arguments;

    NewExpressionNode(std::string className, int line = 0, int column = 0)
        : ASTNode(NodeType::NEW_EXPRESSION, line, column)
        , className(std::move(className)) {}

    std::unique_ptr<ASTNode> clone() const override
    {
        auto ne = std::make_unique<NewExpressionNode>(className, line, column);
        for (const auto& arg : arguments)
            ne->arguments.push_back(arg ? arg->clone() : nullptr);
        return ne;
    }
};

// ============================================================
// COLLECTIONS
// ============================================================

struct ArrayLiteralNode : ASTNode
{
    std::vector<std::unique_ptr<ASTNode>> elements;

    ArrayLiteralNode(int line = 0, int column = 0)
        : ASTNode(NodeType::ARRAY_LITERAL, line, column) {}

    std::unique_ptr<ASTNode> clone() const override
    {
        auto al = std::make_unique<ArrayLiteralNode>(line, column);
        for (const auto& elem : elements)
            al->elements.push_back(elem ? elem->clone() : nullptr);
        return al;
    }
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

    std::unique_ptr<ASTNode> clone() const override
    {
        return std::make_unique<ArrayAccessNode>(
            array ? array->clone() : nullptr,
            index ? index->clone() : nullptr, line, column);
    }
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

    std::unique_ptr<ASTNode> clone() const override
    {
        return std::make_unique<IndexAssignmentNode>(
            array ? array->clone() : nullptr,
            index ? index->clone() : nullptr,
            value ? value->clone() : nullptr, line, column);
    }
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

    std::unique_ptr<ASTNode> clone() const override
    {
        return std::make_unique<ImportNode>(moduleName, line, column);
    }
};

struct FromImportNode : ASTNode
{
    std::string moduleName;
    std::vector<std::string> names;

    FromImportNode(std::string moduleName, int line = 0, int column = 0)
        : ASTNode(NodeType::FROM_IMPORT, line, column)
        , moduleName(std::move(moduleName)) {}

    std::unique_ptr<ASTNode> clone() const override
    {
        auto fi = std::make_unique<FromImportNode>(moduleName, line, column);
        fi->names = names;
        return fi;
    }
};

// ============================================================
// PROGRAM
// ============================================================

struct ProgramNode : ASTNode
{
    std::vector<std::unique_ptr<ASTNode>> statements;

    ProgramNode(int line = 0, int column = 0)
        : ASTNode(NodeType::PROGRAM, line, column) {}

    std::unique_ptr<ASTNode> clone() const override
    {
        auto pn = std::make_unique<ProgramNode>(line, column);
        for (const auto& stmt : statements)
            pn->statements.push_back(stmt ? stmt->clone() : nullptr);
        return pn;
    }
};

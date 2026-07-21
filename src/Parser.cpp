#include "Parser.h"
#include <stdexcept>

Parser::Parser(std::vector<Token> tokens, ErrorHandler* errorHandler)
    : tokens(std::move(tokens))
    , position(0)
    , errorHandler(errorHandler)
{
}

Token Parser::peek() const
{
    return tokens[position];
}

Token Parser::previous() const
{
    if (position > 0) return tokens[position - 1];
    return tokens[0];
}

Token Parser::advance()
{
    if (!isAtEnd()) position++;
    return previous();
}

bool Parser::check(TokenType type) const
{
    if (isAtEnd()) return false;
    return peek().type == type;
}

bool Parser::match(TokenType type)
{
    if (check(type))
    {
        advance();
        return true;
    }
    return false;
}

bool Parser::matchAny(std::initializer_list<TokenType> types)
{
    for (auto type : types)
    {
        if (match(type)) return true;
    }
    return false;
}

Token Parser::consume(TokenType type, const std::string& message)
{
    if (check(type)) return advance();

    if (errorHandler)
    {
        SourceLocation loc("", peek().line, peek().column);
        errorHandler->error(ErrorCode::UNEXPECTED_TOKEN, message, loc);
    }
    return peek();
}

Token Parser::consumeAssignmentOp()
{
    if (match(TokenType::EQUAL)) return previous();
    if (match(TokenType::PLUS_EQUAL)) return previous();
    if (match(TokenType::MINUS_EQUAL)) return previous();
    if (match(TokenType::MULTIPLY_EQUAL)) return previous();
    if (match(TokenType::DIVIDE_EQUAL)) return previous();
    if (match(TokenType::MODULO_EQUAL)) return previous();
    if (match(TokenType::POWER_EQUAL)) return previous();
    if (match(TokenType::AND_EQUAL)) return previous();
    if (match(TokenType::OR_EQUAL)) return previous();
    if (match(TokenType::XOR_EQUAL)) return previous();

    Token t = peek();
    return t;
}

bool Parser::isAtEnd() const
{
    return peek().type == TokenType::END;
}

void Parser::synchronize()
{
    advance();
    while (!isAtEnd())
    {
        if (previous().type == TokenType::SEMICOLON) return;
        switch (peek().type)
        {
            case TokenType::LET:
            case TokenType::CONST:
            case TokenType::FUNC:
            case TokenType::CLASS:
            case TokenType::IF:
            case TokenType::WHILE:
            case TokenType::FOR:
            case TokenType::SWITCH:
            case TokenType::RETURN:
            case TokenType::BREAK:
            case TokenType::CONTINUE:
            case TokenType::PRINT:
            case TokenType::IMPORT:
            case TokenType::FROM:
                return;
            default:
                advance();
        }
    }
}

// ============================================================
// TYPE PARSING
// ============================================================

TypeInfo Parser::parseTypeAnnotation()
{
    auto opt = tryParseTypeAnnotation();
    if (opt.has_value()) return std::move(opt.value());
    return TypeInfo(TypeKind::ANY);
}

std::optional<TypeInfo> Parser::tryParseTypeAnnotation()
{
    if (!match(TokenType::COLON)) return std::nullopt;

    TypeInfo type(TypeKind::ANY);

    if (match(TokenType::INT_TYPE)) type.kind = TypeKind::INT;
    else if (match(TokenType::FLOAT_TYPE)) type.kind = TypeKind::FLOAT;
    else if (match(TokenType::STRING_TYPE)) type.kind = TypeKind::STRING;
    else if (match(TokenType::BOOL_TYPE)) type.kind = TypeKind::BOOLEAN;
    else if (match(TokenType::VOID_TYPE)) type.kind = TypeKind::VOID;
    else if (match(TokenType::ANY_TYPE)) type.kind = TypeKind::ANY;
    else if (match(TokenType::IDENTIFIER))
    {
        type.kind = TypeKind::OBJECT;
        type.name = previous().value;
    }
    else
    {
        type.kind = TypeKind::ANY;
    }

    return type;
}

// ============================================================
// STATEMENT PARSING
// ============================================================

std::vector<std::unique_ptr<ASTNode>> Parser::parse()
{
    std::vector<std::unique_ptr<ASTNode>> statements;
    while (!isAtEnd())
    {
        auto stmt = parseStatement();
        if (stmt)
            statements.push_back(std::move(stmt));
        else
            advance();
    }
    return statements;
}

std::unique_ptr<ASTNode> Parser::parseStatement()
{
    if (match(TokenType::LET)) return parseVariableDeclaration(false);
    if (match(TokenType::CONST)) return parseVariableDeclaration(true);
    if (match(TokenType::FUNC)) return parseFunctionDeclaration();
    if (match(TokenType::CLASS)) return parseClassDeclaration();
    if (match(TokenType::IF)) return parseIf();
    if (match(TokenType::WHILE)) return parseWhile();
    if (match(TokenType::FOR)) return parseFor();
    if (match(TokenType::SWITCH)) return parseSwitch();
    if (match(TokenType::RETURN)) return parseReturn();
    if (match(TokenType::BREAK)) return parseBreak();
    if (match(TokenType::CONTINUE)) return parseContinue();
    if (match(TokenType::PRINT)) return parsePrint();
    if (match(TokenType::IMPORT)) return parseImport();
    if (match(TokenType::FROM)) return parseFromImport();
    if (match(TokenType::MATCH)) return parseMatch();
    if (match(TokenType::LBRACE)) return parseBlock();

    return parseExpressionStatement();
}

std::unique_ptr<ASTNode> Parser::parseExpressionStatement()
{
    auto expr = parseExpression();
    match(TokenType::SEMICOLON);
    return std::make_unique<ExpressionStatementNode>(std::move(expr));
}

std::unique_ptr<ASTNode> Parser::parseVariableDeclaration(bool isConst)
{
    // Accept IDENTIFIER or common keywords as variable names
    Token nameToken(TokenType::IDENTIFIER, "");
    if (check(TokenType::IDENTIFIER)) {
        nameToken = advance();
    } else if (check(TokenType::PI)) {
        nameToken = advance();
        nameToken.type = TokenType::IDENTIFIER;
    } else if (check(TokenType::VERSION)) {
        nameToken = advance();
        nameToken.type = TokenType::IDENTIFIER;
    } else {
        nameToken = consume(TokenType::IDENTIFIER, "Expected variable name");
    }
    std::string name = nameToken.value;
    int line = nameToken.line;
    int col = nameToken.column;

    auto typeAnnotation = tryParseTypeAnnotation();

    std::unique_ptr<ASTNode> initializer;
    if (match(TokenType::EQUAL))
    {
        initializer = parseExpression();
    }

    match(TokenType::SEMICOLON);

    return std::make_unique<VariableDeclarationNode>(
        name, std::move(initializer),
        std::move(typeAnnotation), isConst, line, col);
}

std::unique_ptr<ASTNode> Parser::parseBlock()
{
    int line = previous().line;
    int col = previous().column;
    auto block = std::make_unique<BlockNode>(line, col);

    while (!isAtEnd() && !check(TokenType::RBRACE))
    {
        auto stmt = parseStatement();
        if (stmt)
            block->statements.push_back(std::move(stmt));
        else
            advance();
    }

    consume(TokenType::RBRACE, "Expected '}' after block");
    return block;
}

std::unique_ptr<ASTNode> Parser::parseIf()
{
    int line = previous().line;
    int col = previous().column;

    consume(TokenType::LPAREN, "Expected '(' after 'if'");
    auto condition = parseExpression();
    consume(TokenType::RPAREN, "Expected ')' after condition");

    auto ifNode = std::make_unique<IfNode>(std::move(condition), line, col);

    consume(TokenType::LBRACE, "Expected '{' for if body");
    while (!isAtEnd() && !check(TokenType::RBRACE))
    {
        auto stmt = parseStatement();
        if (stmt) ifNode->thenBlock.push_back(std::move(stmt));
    }
    consume(TokenType::RBRACE, "Expected '}' after if body");

    if (match(TokenType::ELSE))
    {
        if (match(TokenType::IF))
        {
            auto elseIf = parseIf();
            ifNode->elseBlock.push_back(std::move(elseIf));
        }
        else
        {
            consume(TokenType::LBRACE, "Expected '{' for else body");
            while (!isAtEnd() && !check(TokenType::RBRACE))
            {
                auto stmt = parseStatement();
                if (stmt) ifNode->elseBlock.push_back(std::move(stmt));
            }
            consume(TokenType::RBRACE, "Expected '}' after else body");
        }
    }

    return ifNode;
}

std::unique_ptr<ASTNode> Parser::parseWhile()
{
    int line = previous().line;
    int col = previous().column;

    consume(TokenType::LPAREN, "Expected '(' after 'while'");
    auto condition = parseExpression();
    consume(TokenType::RPAREN, "Expected ')' after condition");

    auto whileNode = std::make_unique<WhileNode>(std::move(condition), line, col);

    consume(TokenType::LBRACE, "Expected '{' for while body");
    while (!isAtEnd() && !check(TokenType::RBRACE))
    {
        auto stmt = parseStatement();
        if (stmt) whileNode->body.push_back(std::move(stmt));
    }
    consume(TokenType::RBRACE, "Expected '}' after while body");

    return whileNode;
}

std::unique_ptr<ASTNode> Parser::parseFor()
{
    int line = previous().line;
    int col = previous().column;

    consume(TokenType::LPAREN, "Expected '(' after 'for'");
    Token varToken = consume(TokenType::IDENTIFIER, "Expected variable name in 'for'");
    consume(TokenType::IN, "Expected 'in' in 'for' loop");
    auto iterable = parseExpression();
    consume(TokenType::RPAREN, "Expected ')' after for iterable");

    auto forNode = std::make_unique<ForNode>(varToken.value, std::move(iterable), line, col);

    consume(TokenType::LBRACE, "Expected '{' for for body");
    while (!isAtEnd() && !check(TokenType::RBRACE))
    {
        auto stmt = parseStatement();
        if (stmt) forNode->body.push_back(std::move(stmt));
    }
    consume(TokenType::RBRACE, "Expected '}' after for body");

    return forNode;
}

std::unique_ptr<ASTNode> Parser::parseSwitch()
{
    int line = previous().line;
    int col = previous().column;

    consume(TokenType::LPAREN, "Expected '(' after 'switch'");
    auto expr = parseExpression();
    consume(TokenType::RPAREN, "Expected ')' after switch expression");

    auto switchNode = std::make_unique<SwitchNode>(std::move(expr), line, col);

    consume(TokenType::LBRACE, "Expected '{' for switch");

    while (!isAtEnd() && !check(TokenType::RBRACE))
    {
        if (match(TokenType::CASE))
        {
            SwitchCase case_;
            case_.value = parseExpression();
            consume(TokenType::COLON, "Expected ':' after case value");
            while (!isAtEnd() && !check(TokenType::RBRACE) &&
                   !check(TokenType::CASE) && !check(TokenType::DEFAULT))
            {
                auto stmt = parseStatement();
                if (stmt) case_.body.push_back(std::move(stmt));
            }
            switchNode->cases.push_back(std::move(case_));
        }
        else if (match(TokenType::DEFAULT))
        {
            consume(TokenType::COLON, "Expected ':' after default");
            while (!isAtEnd() && !check(TokenType::RBRACE) &&
                   !check(TokenType::CASE) && !check(TokenType::DEFAULT))
            {
                auto stmt = parseStatement();
                if (stmt) switchNode->defaultCase.push_back(std::move(stmt));
            }
        }
        else break;
    }

    consume(TokenType::RBRACE, "Expected '}' after switch");
    return switchNode;
}

std::unique_ptr<ASTNode> Parser::parseReturn()
{
    int line = previous().line;
    int col = previous().column;

    if (check(TokenType::SEMICOLON) || check(TokenType::RBRACE) || check(TokenType::END))
    {
        match(TokenType::SEMICOLON);
        return std::make_unique<ReturnNode>(nullptr, line, col);
    }

    auto value = parseExpression();
    match(TokenType::SEMICOLON);
    return std::make_unique<ReturnNode>(std::move(value), line, col);
}

std::unique_ptr<ASTNode> Parser::parseBreak()
{
    int line = previous().line;
    int col = previous().column;
    match(TokenType::SEMICOLON);
    return std::make_unique<BreakNode>(line, col);
}

std::unique_ptr<ASTNode> Parser::parseContinue()
{
    int line = previous().line;
    int col = previous().column;
    match(TokenType::SEMICOLON);
    return std::make_unique<ContinueNode>(line, col);
}

std::unique_ptr<ASTNode> Parser::parsePrint()
{
    int line = previous().line;
    int col = previous().column;
    auto expr = parseExpression();
    match(TokenType::SEMICOLON);
    return std::make_unique<PrintNode>(std::move(expr), line, col);
}

// ============================================================
// FUNCTION PARSING
// ============================================================

std::unique_ptr<ASTNode> Parser::parseFunctionDeclaration()
{
    int line = previous().line;
    int col = previous().column;

    auto func = std::make_unique<FunctionDeclarationNode>("", line, col);

    // Anonymous function
    if (check(TokenType::LPAREN))
    {
        func->parameters = parseParameters();
        func->body = parseFunctionBody();
        return func;
    }

    // Named function
    Token nameToken = consume(TokenType::IDENTIFIER, "Expected function name");
    func->name = nameToken.value;

    consume(TokenType::LPAREN, "Expected '(' after function name");
    if (!check(TokenType::RPAREN))
    {
        do {
            Token paramName = consume(TokenType::IDENTIFIER, "Expected parameter name");
            TypeInfo paramType(TypeKind::ANY);
            if (match(TokenType::COLON))
            {
                if (match(TokenType::INT_TYPE)) paramType = TypeInfo(TypeKind::INT);
                else if (match(TokenType::FLOAT_TYPE)) paramType = TypeInfo(TypeKind::FLOAT);
                else if (match(TokenType::STRING_TYPE)) paramType = TypeInfo(TypeKind::STRING);
                else if (match(TokenType::BOOL_TYPE)) paramType = TypeInfo(TypeKind::BOOLEAN);
                else if (match(TokenType::ANY_TYPE)) paramType = TypeInfo(TypeKind::ANY);
                else paramType = TypeInfo(TypeKind::ANY);
            }
            func->parameters.emplace_back(paramName.value, paramType);
        } while (match(TokenType::COMMA));
    }
    consume(TokenType::RPAREN, "Expected ')' after parameters");

    if (match(TokenType::COLON))
    {
        if (match(TokenType::INT_TYPE)) func->returnType = TypeInfo(TypeKind::INT);
        else if (match(TokenType::FLOAT_TYPE)) func->returnType = TypeInfo(TypeKind::FLOAT);
        else if (match(TokenType::STRING_TYPE)) func->returnType = TypeInfo(TypeKind::STRING);
        else if (match(TokenType::BOOL_TYPE)) func->returnType = TypeInfo(TypeKind::BOOLEAN);
        else if (match(TokenType::VOID_TYPE)) func->returnType = TypeInfo(TypeKind::VOID);
        else func->returnType = TypeInfo(TypeKind::ANY);
    }

    func->body = parseFunctionBody();
    return func;
}

std::vector<ParameterNode> Parser::parseParameters()
{
    std::vector<ParameterNode> params;
    consume(TokenType::LPAREN, "Expected '('");
    if (!check(TokenType::RPAREN))
    {
        do {
            Token name = consume(TokenType::IDENTIFIER, "Expected parameter name");
            TypeInfo type(TypeKind::ANY);
            if (match(TokenType::COLON))
            {
                if (match(TokenType::INT_TYPE)) type = TypeInfo(TypeKind::INT);
                else if (match(TokenType::FLOAT_TYPE)) type = TypeInfo(TypeKind::FLOAT);
                else if (match(TokenType::STRING_TYPE)) type = TypeInfo(TypeKind::STRING);
                else if (match(TokenType::BOOL_TYPE)) type = TypeInfo(TypeKind::BOOLEAN);
                else type = TypeInfo(TypeKind::ANY);
            }
            params.emplace_back(name.value, type);
        } while (match(TokenType::COMMA));
    }
    consume(TokenType::RPAREN, "Expected ')' after parameters");
    return params;
}

std::vector<std::unique_ptr<ASTNode>> Parser::parseFunctionBody()
{
    std::vector<std::unique_ptr<ASTNode>> body;
    consume(TokenType::LBRACE, "Expected '{' for function body");
    while (!isAtEnd() && !check(TokenType::RBRACE))
    {
        auto stmt = parseStatement();
        if (stmt) body.push_back(std::move(stmt));
    }
    consume(TokenType::RBRACE, "Expected '}' after function body");
    return body;
}

// ============================================================
// CLASS PARSING
// ============================================================

std::unique_ptr<ASTNode> Parser::parseClassDeclaration()
{
    int line = previous().line;
    int col = previous().column;

    Token nameToken = consume(TokenType::IDENTIFIER, "Expected class name");
    auto classNode = std::make_unique<ClassDeclarationNode>(nameToken.value, line, col);

    if (match(TokenType::EXTENDS))
    {
        Token baseToken = consume(TokenType::IDENTIFIER, "Expected base class name");
        classNode->baseClass = baseToken.value;
    }

    consume(TokenType::LBRACE, "Expected '{' for class body");

    while (!isAtEnd() && !check(TokenType::RBRACE))
    {
        if (check(TokenType::FUNC))
        {
            advance();
            // Method
            Token methodName = consume(TokenType::IDENTIFIER, "Expected method name");
            auto methodFunc = std::make_unique<FunctionDeclarationNode>(methodName.value);

            consume(TokenType::LPAREN, "Expected '(' after method name");
            if (!check(TokenType::RPAREN))
            {
                do {
                    Token paramName = consume(TokenType::IDENTIFIER, "Expected parameter name");
                    methodFunc->parameters.emplace_back(paramName.value);
                } while (match(TokenType::COMMA));
            }
            consume(TokenType::RPAREN, "Expected ')' after parameters");

            methodFunc->body = parseFunctionBody();
            MethodDeclaration method;
            method.function = std::move(*methodFunc);
            classNode->methods.push_back(std::move(method));
        }
        else
        {
            // Field
            Token fieldToken = consume(TokenType::IDENTIFIER, "Expected field name");
            classNode->fields.push_back(fieldToken.value);
            match(TokenType::SEMICOLON);
        }
    }

    consume(TokenType::RBRACE, "Expected '}' after class body");
    return classNode;
}

// ============================================================
// PATTERN MATCHING
// ============================================================

std::unique_ptr<ASTNode> Parser::parseMatch()
{
    int line = previous().line;
    int col = previous().column;

    consume(TokenType::LPAREN, "Expected '(' after 'match'");
    auto expr = parseExpression();
    consume(TokenType::RPAREN, "Expected ')' after match expression");

    auto matchNode = std::make_unique<MatchNode>(std::move(expr), line, col);

    consume(TokenType::LBRACE, "Expected '{' for match block");

    while (!isAtEnd() && !check(TokenType::RBRACE))
    {
        if (match(TokenType::CASE))
        {
            MatchCaseNode caseNode(previous().line, previous().column);
            caseNode.pattern = parsePattern();

            if (match(TokenType::IF))
            {
                caseNode.pattern.guard = parseExpression();
            }

            if (match(TokenType::FAT_ARROW))
            {
                caseNode.body.push_back(parseExpression());
                match(TokenType::SEMICOLON);
            }
            else
            {
                consume(TokenType::COLON, "Expected ':' after case pattern");
                while (!isAtEnd() && !check(TokenType::RBRACE) &&
                       !check(TokenType::CASE) && !check(TokenType::DEFAULT) &&
                       !check(TokenType::ELSE))
                {
                    auto stmt = parseStatement();
                    if (stmt) caseNode.body.push_back(std::move(stmt));
                }
            }

            matchNode->cases.push_back(std::move(caseNode));
        }
        else if (match(TokenType::ELSE))
        {
            if (match(TokenType::FAT_ARROW))
            {
                matchNode->elseBody.push_back(parseExpression());
                match(TokenType::SEMICOLON);
            }
            else
            {
                consume(TokenType::COLON, "Expected ':' after else");
                while (!isAtEnd() && !check(TokenType::RBRACE) &&
                       !check(TokenType::CASE) && !check(TokenType::ELSE))
                {
                    auto stmt = parseStatement();
                    if (stmt) matchNode->elseBody.push_back(std::move(stmt));
                }
            }
        }
        else break;
    }

    consume(TokenType::RBRACE, "Expected '}' after match block");
    return matchNode;
}

MatchPattern Parser::parsePattern()
{
    MatchPattern pattern;

    if (check(TokenType::IDENTIFIER) && peek().value == "_")
    {
        advance();
        pattern.kind = PatternKind::WILDCARD;
    }
    else if (match(TokenType::NUMBER))
    {
        pattern.kind = PatternKind::LITERAL;
        pattern.literal = std::make_unique<NumberNode>(std::stoll(previous().value), previous().line, previous().column);
    }
    else if (match(TokenType::FLOAT))
    {
        pattern.kind = PatternKind::LITERAL;
        pattern.literal = std::make_unique<FloatNode>(std::stod(previous().value), previous().line, previous().column);
    }
    else if (match(TokenType::STRING))
    {
        pattern.kind = PatternKind::LITERAL;
        pattern.literal = std::make_unique<StringNode>(previous().value, previous().line, previous().column);
    }
    else if (match(TokenType::CHARACTER))
    {
        pattern.kind = PatternKind::LITERAL;
        char ch = previous().value.empty() ? '\0' : previous().value[0];
        pattern.literal = std::make_unique<CharNode>(ch, previous().line, previous().column);
    }
    else if (match(TokenType::TRUE))
    {
        pattern.kind = PatternKind::LITERAL;
        pattern.literal = std::make_unique<BooleanNode>(true, previous().line, previous().column);
    }
    else if (match(TokenType::FALSE))
    {
        pattern.kind = PatternKind::LITERAL;
        pattern.literal = std::make_unique<BooleanNode>(false, previous().line, previous().column);
    }
    else if (match(TokenType::NULL_VALUE))
    {
        pattern.kind = PatternKind::LITERAL;
        pattern.literal = std::make_unique<NullNode>(previous().line, previous().column);
    }
    else if (match(TokenType::LBRACKET))
    {
        pattern.kind = PatternKind::DESTRUCTURE_ARRAY;
        if (!check(TokenType::RBRACKET))
        {
            do {
                pattern.subPatterns.push_back(parsePattern());
            } while (match(TokenType::COMMA));
        }
        consume(TokenType::RBRACKET, "Expected ']' in array pattern");
    }
    else if (check(TokenType::IDENTIFIER))
    {
        // Could be a variable binding or a named pattern
        Token token = peek();
        if (tokens.size() > position + 1 && tokens[position + 1].type == TokenType::LBRACE)
        {
            // Object destructure
            advance();
            pattern.kind = PatternKind::DESTRUCTURE_OBJECT;
            advance(); // skip {
            while (!check(TokenType::RBRACE) && !isAtEnd())
            {
                Token fieldToken = consume(TokenType::IDENTIFIER, "Expected field name");
                MatchPattern subPat;
                subPat.kind = PatternKind::VARIABLE;
                subPat.variableName = fieldToken.value;
                if (match(TokenType::COLON))
                {
                    subPat = parsePattern();
                }
                pattern.objectPatterns.emplace_back(fieldToken.value, std::move(subPat));
                match(TokenType::COMMA);
            }
            consume(TokenType::RBRACE, "Expected '}' after object pattern");
            if (!token.value.empty() && token.value != "_")
                pattern.variableName = token.value;
        }
        else
        {
            advance(); // consume identifier
            pattern.kind = PatternKind::VARIABLE;
            pattern.variableName = token.value;
        }
    }
    else
    {
        // Default to wildcard
        pattern.kind = PatternKind::WILDCARD;
    }

    return pattern;
}

// ============================================================
// IMPORT PARSING
// ============================================================

std::unique_ptr<ASTNode> Parser::parseImport()
{
    int line = previous().line;
    int col = previous().column;
    Token moduleToken = consume(TokenType::IDENTIFIER, "Expected module name");
    match(TokenType::SEMICOLON);
    return std::make_unique<ImportNode>(moduleToken.value, line, col);
}

std::unique_ptr<ASTNode> Parser::parseFromImport()
{
    int line = previous().line;
    int col = previous().column;

    Token moduleToken = consume(TokenType::IDENTIFIER, "Expected module name after 'from'");
    consume(TokenType::IMPORT, "Expected 'import' after module name");

    auto fromImport = std::make_unique<FromImportNode>(moduleToken.value, line, col);

    do {
        Token nameToken = consume(TokenType::IDENTIFIER, "Expected name to import");
        fromImport->names.push_back(nameToken.value);
    } while (match(TokenType::COMMA));

    match(TokenType::SEMICOLON);
    return fromImport;
}

// ============================================================
// EXPRESSION PARSING (Precedence Climbing)
// ============================================================

std::unique_ptr<ASTNode> Parser::parseExpression()
{
    return parseAssignment();
}

std::unique_ptr<ASTNode> Parser::parseAssignment()
{
    auto expr = parseLogicalOr();

    if (matchAny({TokenType::EQUAL, TokenType::PLUS_EQUAL, TokenType::MINUS_EQUAL,
                  TokenType::MULTIPLY_EQUAL, TokenType::DIVIDE_EQUAL, TokenType::MODULO_EQUAL,
                  TokenType::POWER_EQUAL, TokenType::AND_EQUAL, TokenType::OR_EQUAL,
                  TokenType::XOR_EQUAL}))
    {
        Token op = previous();
        auto value = parseAssignment();

        if (auto varAccess = dynamic_cast<VariableAccessNode*>(expr.get()))
        {
            return std::make_unique<AssignmentNode>(
                varAccess->name, std::move(value), op.value, op.line, op.column);
        }
        else if (auto memberAccess = dynamic_cast<MemberAccessNode*>(expr.get()))
        {
            return std::make_unique<MemberAssignmentNode>(
                std::move(memberAccess->object),
                memberAccess->member,
                std::move(value), op.line, op.column);
        }
        else if (auto arrAccess = dynamic_cast<ArrayAccessNode*>(expr.get()))
        {
            return std::make_unique<IndexAssignmentNode>(
                std::move(arrAccess->array),
                std::move(arrAccess->index),
                std::move(value), op.line, op.column);
        }

        if (errorHandler)
            errorHandler->error(ErrorCode::INVALID_ASSIGNMENT_TARGET,
                "Invalid assignment target", SourceLocation("", op.line, op.column));
        return nullptr;
    }

    return expr;
}

std::unique_ptr<ASTNode> Parser::parseLogicalOr()
{
    auto expr = parseLogicalAnd();

    while (match(TokenType::OR_OR))
    {
        Token op = previous();
        auto right = parseLogicalAnd();
        expr = std::make_unique<BinaryExpressionNode>(
            op.value, std::move(expr), std::move(right), op.line, op.column);
    }

    return expr;
}

std::unique_ptr<ASTNode> Parser::parseLogicalAnd()
{
    auto expr = parseEquality();

    while (match(TokenType::AND_AND))
    {
        Token op = previous();
        auto right = parseEquality();
        expr = std::make_unique<BinaryExpressionNode>(
            op.value, std::move(expr), std::move(right), op.line, op.column);
    }

    return expr;
}

std::unique_ptr<ASTNode> Parser::parseEquality()
{
    auto expr = parseComparison();

    while (matchAny({TokenType::EQUAL_EQUAL, TokenType::NOT_EQUAL}))
    {
        Token op = previous();
        auto right = parseComparison();
        expr = std::make_unique<BinaryExpressionNode>(
            op.value, std::move(expr), std::move(right), op.line, op.column);
    }

    return expr;
}

std::unique_ptr<ASTNode> Parser::parseComparison()
{
    auto expr = parseTerm();

    while (matchAny({TokenType::BIGGER, TokenType::SMALLER, TokenType::BIGGER_EQUAL,
                     TokenType::SMALLER_EQUAL, TokenType::THREE_WAY_COMPARE,
                     TokenType::RANGE}))
    {
        Token op = previous();
        auto right = parseTerm();
        expr = std::make_unique<BinaryExpressionNode>(
            op.value, std::move(expr), std::move(right), op.line, op.column);
    }

    return expr;
}

std::unique_ptr<ASTNode> Parser::parseTerm()
{
    auto expr = parseFactor();

    while (matchAny({TokenType::PLUS, TokenType::MINUS}))
    {
        Token op = previous();
        auto right = parseFactor();
        expr = std::make_unique<BinaryExpressionNode>(
            op.value, std::move(expr), std::move(right), op.line, op.column);
    }

    return expr;
}

std::unique_ptr<ASTNode> Parser::parseFactor()
{
    auto expr = parseUnary();

    while (matchAny({TokenType::MULTIPLY, TokenType::DIVIDE, TokenType::MODULO, TokenType::POWER}))
    {
        Token op = previous();
        auto right = parseUnary();
        expr = std::make_unique<BinaryExpressionNode>(
            op.value, std::move(expr), std::move(right), op.line, op.column);
    }

    return expr;
}

std::unique_ptr<ASTNode> Parser::parseUnary()
{
    if (matchAny({TokenType::NOT, TokenType::MINUS, TokenType::PLUS,
                  TokenType::INCREMENT, TokenType::DECREMENT}))
    {
        Token op = previous();
        auto operand = parseUnary();

        if (op.type == TokenType::INCREMENT || op.type == TokenType::DECREMENT)
        {
            if (operand && operand->type == NodeType::VARIABLE_ACCESS)
            {
                auto varName = static_cast<VariableAccessNode*>(operand.get())->name;
                std::string opStr = op.type == TokenType::INCREMENT ? "++" : "--";
                auto one = std::make_unique<NumberNode>(1);
                auto binOp = std::make_unique<BinaryExpressionNode>(
                    "+", std::move(operand), std::move(one));
                return std::make_unique<AssignmentNode>(
                    varName, std::move(binOp), "+=", op.line, op.column);
            }
        }

        return std::make_unique<UnaryExpressionNode>(
            op.value, std::move(operand), true, op.line, op.column);
    }

    return parseCall();
}

std::unique_ptr<ASTNode> Parser::parseCall()
{
    auto expr = parsePrimary();

    while (true)
    {
        if (match(TokenType::LPAREN))
        {
            expr = finishCall(std::move(expr));
        }
        else if (match(TokenType::DOT))
        {
            expr = finishMemberAccess(std::move(expr));
        }
        else if (match(TokenType::LBRACKET))
        {
            expr = finishIndexAccess(std::move(expr));
        }
        else
        {
            break;
        }
    }

    return expr;
}

std::unique_ptr<ASTNode> Parser::finishCall(std::unique_ptr<ASTNode> callee)
{
    int line = previous().line;
    int col = previous().column;

    std::string funcName;
    if (auto varAccess = dynamic_cast<VariableAccessNode*>(callee.get()))
        funcName = varAccess->name;

    auto call = std::make_unique<FunctionCallNode>(funcName, line, col);

    if (!check(TokenType::RPAREN))
    {
        do {
            call->arguments.push_back(parseExpression());
        } while (match(TokenType::COMMA));
    }

    consume(TokenType::RPAREN, "Expected ')' after arguments");
    return call;
}

std::unique_ptr<ASTNode> Parser::finishMemberAccess(std::unique_ptr<ASTNode> object)
{
    Token member = consume(TokenType::IDENTIFIER, "Expected member name after '.'");
    return std::make_unique<MemberAccessNode>(
        std::move(object), member.value, member.line, member.column);
}

std::unique_ptr<ASTNode> Parser::finishIndexAccess(std::unique_ptr<ASTNode> array)
{
    int line = previous().line;
    int col = previous().column;
    auto index = parseExpression();
    consume(TokenType::RBRACKET, "Expected ']' after index");

    if (check(TokenType::EQUAL))
    {
        advance();
        auto value = parseExpression();
        return std::make_unique<IndexAssignmentNode>(
            std::move(array), std::move(index), std::move(value), line, col);
    }

    return std::make_unique<ArrayAccessNode>(
        std::move(array), std::move(index), line, col);
}

// ============================================================
// PRIMARY EXPRESSIONS
// ============================================================

std::unique_ptr<ASTNode> Parser::parsePrimary()
{
    if (match(TokenType::NUMBER))
    {
        int line = previous().line;
        int col = previous().column;
        return std::make_unique<NumberNode>(
            std::stoll(previous().value), line, col);
    }

    if (match(TokenType::FLOAT))
    {
        int line = previous().line;
        int col = previous().column;
        return std::make_unique<FloatNode>(
            std::stod(previous().value), line, col);
    }

    if (match(TokenType::STRING))
    {
        int line = previous().line;
        int col = previous().column;
        return std::make_unique<StringNode>(previous().value, line, col);
    }

    if (match(TokenType::CHARACTER))
    {
        int line = previous().line;
        int col = previous().column;
        char ch = previous().value.empty() ? '\0' : previous().value[0];
        return std::make_unique<CharNode>(ch, line, col);
    }

    if (match(TokenType::TRUE))
    {
        int line = previous().line;
        int col = previous().column;
        return std::make_unique<BooleanNode>(true, line, col);
    }

    if (match(TokenType::FALSE))
    {
        int line = previous().line;
        int col = previous().column;
        return std::make_unique<BooleanNode>(false, line, col);
    }

    if (match(TokenType::NULL_VALUE))
    {
        int line = previous().line;
        int col = previous().column;
        return std::make_unique<NullNode>(line, col);
    }

    if (match(TokenType::IDENTIFIER))
    {
        int line = previous().line;
        int col = previous().column;
        std::string name = previous().value;

        if (check(TokenType::LPAREN))
        {
            advance();
            auto call = std::make_unique<FunctionCallNode>(name, line, col);
            if (!check(TokenType::RPAREN))
            {
                do {
                    call->arguments.push_back(parseExpression());
                } while (match(TokenType::COMMA));
            }
            consume(TokenType::RPAREN, "Expected ')' after arguments");
            return call;
        }

        return std::make_unique<VariableAccessNode>(name, line, col);
    }

    if (match(TokenType::THIS))
    {
        int line = previous().line;
        int col = previous().column;
        return std::make_unique<VariableAccessNode>("this", line, col);
    }

    if (match(TokenType::NEW))
    {
        int line = previous().line;
        int col = previous().column;
        Token className = consume(TokenType::IDENTIFIER, "Expected class name after 'new'");
        auto newExpr = std::make_unique<NewExpressionNode>(className.value, line, col);
        if (match(TokenType::LPAREN))
        {
            if (!check(TokenType::RPAREN))
            {
                do {
                    newExpr->arguments.push_back(parseExpression());
                } while (match(TokenType::COMMA));
            }
            consume(TokenType::RPAREN, "Expected ')' after arguments");
        }
        return newExpr;
    }

    if (match(TokenType::FUNC))
    {
        auto func = std::make_unique<FunctionDeclarationNode>("", previous().line, previous().column);
        func->parameters = parseParameters();
        func->body = parseFunctionBody();
        return func;
    }

    if (match(TokenType::LBRACKET))
    {
        return parseArrayLiteral();
    }

    if (match(TokenType::LBRACE))
    {
        return parseObjectLiteral();
    }

    if (match(TokenType::LPAREN))
    {
        auto expr = parseExpression();
        consume(TokenType::RPAREN, "Expected ')' after expression");
        return expr;
    }

    if (errorHandler)
    {
        errorHandler->error(ErrorCode::UNEXPECTED_TOKEN,
            "Unexpected token: " + peek().value,
            SourceLocation("", peek().line, peek().column));
    }

    return nullptr;
}

std::unique_ptr<ASTNode> Parser::parseArrayLiteral()
{
    int line = previous().line;
    int col = previous().column;
    auto arr = std::make_unique<ArrayLiteralNode>(line, col);

    if (!check(TokenType::RBRACKET))
    {
        do {
            arr->elements.push_back(parseExpression());
        } while (match(TokenType::COMMA));
    }

    consume(TokenType::RBRACKET, "Expected ']' after array literal");
    return arr;
}

std::unique_ptr<ASTNode> Parser::parseObjectLiteral()
{
    int line = previous().line;
    int col = previous().column;

    // Check if this is a block statement (starts with identifier: or keyword)
    if (!isAtEnd() && check(TokenType::RBRACE))
    {
        return std::make_unique<BlockNode>(line, col);
    }

    // Try to parse as object literal
    size_t savedPos = position;
    auto obj = std::make_unique<ObjectLiteralNode>(line, col);

    if (!check(TokenType::RBRACE))
    {
        do {
            if (check(TokenType::IDENTIFIER) && tokens.size() > position + 1 &&
                tokens[position + 1].type == TokenType::COLON)
            {
                Token key = advance();
                advance(); // consume ':'
                auto value = parseExpression();
                obj->properties.emplace_back(key.value, std::move(value));
            }
            else
            {
                // Not an object literal, parse as block
                position = savedPos;
                return parseBlock();
            }
        } while (match(TokenType::COMMA));
    }

    consume(TokenType::RBRACE, "Expected '}' after object literal");
    return obj;
}

#include "Error.h"
#include <sstream>
#include <iostream>
#include <algorithm>

CompilerError::CompilerError(
    ErrorCode code,
    const std::string& message,
    const SourceLocation& location,
    ErrorLevel level,
    const std::string& hint,
    const std::string& sourceLine)
    : code(code)
    , message(message)
    , location(location)
    , level(level)
    , sourceLine(sourceLine)
    , hint(hint)
{
}

ErrorCode CompilerError::getCode() const { return code; }
ErrorLevel CompilerError::getLevel() const { return level; }
const std::string& CompilerError::getMessage() const { return message; }
const SourceLocation& CompilerError::getLocation() const { return location; }
const std::string& CompilerError::getHint() const { return hint; }

bool CompilerError::isWarning() const { return level == ErrorLevel::WARNING; }
bool CompilerError::isError() const { return level == ErrorLevel::ERROR; }
bool CompilerError::isFatal() const { return level == ErrorLevel::FATAL; }

std::string CompilerError::getErrorCodeString() const
{
    std::string prefix;
    switch (level)
    {
        case ErrorLevel::WARNING: prefix = "KETSA-W"; break;
        case ErrorLevel::ERROR:   prefix = "KETSA-E"; break;
        case ErrorLevel::FATAL:   prefix = "KETSA-F"; break;
    }
    return prefix + std::to_string(static_cast<int>(code));
}

std::string CompilerError::toString() const
{
    std::stringstream ss;
    std::string levelStr;
    switch (level)
    {
        case ErrorLevel::WARNING: levelStr = "warning"; break;
        case ErrorLevel::ERROR:   levelStr = "error";   break;
        case ErrorLevel::FATAL:   levelStr = "fatal";   break;
    }

    ss << levelStr << "[" << getErrorCodeString() << "]: " << message << "\n";

    if (!location.file.empty())
        ss << "  --> " << location.file << ":" << location.line << ":" << location.column << "\n";
    else
        ss << "  --> " << location.line << ":" << location.column << "\n";

    if (!sourceLine.empty())
    {
        ss << "   |\n";
        ss << " " << location.line << " | " << sourceLine << "\n";
        ss << "   | ";
        for (int i = 0; i < location.column - 1; i++)
            ss << " ";
        ss << "^";
        for (int i = 1; i < location.length; i++)
            ss << "~";
        ss << "\n";
    }

    if (!hint.empty())
        ss << "   = note: " << hint << "\n";

    return ss.str();
}

std::string CompilerError::toShortString() const
{
    std::string levelStr;
    switch (level)
    {
        case ErrorLevel::WARNING: levelStr = "warning"; break;
        case ErrorLevel::ERROR:   levelStr = "error";   break;
        case ErrorLevel::FATAL:   levelStr = "fatal";   break;
    }

    std::stringstream ss;
    ss << levelStr << "[" << getErrorCodeString() << "]: " << message
       << " (" << location.line << ":" << location.column << ")";
    return ss.str();
}

std::string CompilerError::formatSourcePreview(const std::string& source, int line, int column, int length)
{
    (void)source;
    (void)line;
    (void)column;
    (void)length;
    return "";
}

ErrorHandler::ErrorHandler()
    : errorCount(0)
    , warningCount(0)
{
}

ErrorHandler::ErrorHandler(std::string source)
    : source(std::move(source))
    , errorCount(0)
    , warningCount(0)
{
}

void ErrorHandler::setSource(const std::string& src) { source = src; }
void ErrorHandler::setFilename(const std::string& fname) { filename = fname; }

void ErrorHandler::report(const CompilerError& error)
{
    if (error.isWarning())
    {
        warnings.push_back(error);
        warningCount++;
    }
    else
    {
        errors.push_back(error);
        errorCount++;
    }
}

void ErrorHandler::report(ErrorCode code, const std::string& message, const SourceLocation& location,
                          ErrorLevel level, const std::string& hint)
{
    std::string sourceLine = getSourceLine(location.line);
    CompilerError error(code, message, location, level, hint, sourceLine);
    report(error);
}

void ErrorHandler::warning(ErrorCode code, const std::string& message, const SourceLocation& location,
                           const std::string& hint)
{
    report(code, message, location, ErrorLevel::WARNING, hint);
}

void ErrorHandler::error(ErrorCode code, const std::string& message, const SourceLocation& location,
                         const std::string& hint)
{
    report(code, message, location, ErrorLevel::ERROR, hint);
}

bool ErrorHandler::hasErrors() const { return errorCount > 0; }
bool ErrorHandler::hasWarnings() const { return warningCount > 0; }
int ErrorHandler::getErrorCount() const { return errorCount; }
int ErrorHandler::getWarningCount() const { return warningCount; }

const std::vector<CompilerError>& ErrorHandler::getErrors() const { return errors; }
const std::vector<CompilerError>& ErrorHandler::getWarnings() const { return warnings; }

void ErrorHandler::printAll() const
{
    for (const auto& err : errors)
        std::cerr << err.toString() << "\n";
    for (const auto& warn : warnings)
        std::cerr << warn.toString() << "\n";
}

void ErrorHandler::printSummary() const
{
    if (errorCount > 0 || warningCount > 0)
    {
        std::stringstream ss;
        if (errorCount > 0)
            ss << errorCount << " error(s)";
        if (errorCount > 0 && warningCount > 0)
            ss << ", ";
        if (warningCount > 0)
            ss << warningCount << " warning(s)";
        ss << " generated.\n";
        std::cerr << ss.str();
    }
}

void ErrorHandler::clear()
{
    errors.clear();
    warnings.clear();
    errorCount = 0;
    warningCount = 0;
}

std::string ErrorHandler::getSourceLine(int line) const
{
    if (source.empty() || line < 1)
        return "";

    int currentLine = 1;
    size_t start = 0;
    for (size_t i = 0; i < source.size(); i++)
    {
        if (currentLine == line)
        {
            size_t end = source.find('\n', i);
            if (end == std::string::npos)
                return source.substr(i);
            return source.substr(i, end - i);
        }
        if (source[i] == '\n')
        {
            currentLine++;
            start = i + 1;
        }
    }
    return "";
}

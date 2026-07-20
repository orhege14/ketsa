#include "Error.h"

#include <sstream>

KetsaError::KetsaError(
    const std::string& message,
    int line
)
{
    this->message = message;
    this->line = line;
}


std::string KetsaError::getMessage() const
{
    return message;
}


int KetsaError::getLine() const
{
    return line;
}


std::string KetsaError::toString() const
{
    std::stringstream ss;

    ss << "Ketsa Error:\n";
    ss << "Line " << line << ": ";
    ss << message;

    return ss.str();
}
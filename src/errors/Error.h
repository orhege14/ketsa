#pragma once

#include <string>

class KetsaError
{
private:
    std::string message;
    int line;

public:
    KetsaError(
        const std::string& message,
        int line
    );

    std::string getMessage() const;
    int getLine() const;

    std::string toString() const;
};
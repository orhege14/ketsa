#pragma once

#include "Value.h"


class StringValue : public Value
{

private:

    std::string value;


public:

    StringValue(const std::string& value);


    ValueType getType() const override;


    std::string toString() const override;


    std::unique_ptr<Value> clone() const override;


    const std::string& getValue() const;

};
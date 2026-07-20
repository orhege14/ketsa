#pragma once

#include "Value.h"


class NumberValue : public Value
{

private:

    int value;


public:

    NumberValue(int value);


    ValueType getType() const override;


    std::string toString() const override;


    std::unique_ptr<Value> clone() const override;


    int getValue() const;

};
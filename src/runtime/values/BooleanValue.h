#pragma once

#include "Value.h"


class BooleanValue : public Value
{

private:

    bool value;


public:

    BooleanValue(bool value);


    ValueType getType() const override;


    std::string toString() const override;


    std::unique_ptr<Value> clone() const override;


    bool getValue() const;

};
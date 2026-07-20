#pragma once

#include "Value.h"
#include <vector>
#include <memory>
#include <string>


class ArrayValue : public Value
{

private:

    std::vector<std::unique_ptr<Value>> values;


public:

    ArrayValue(
        std::vector<std::unique_ptr<Value>> values
    )
        : values(std::move(values))
    {
    }



    ValueType getType() const override
    {
        return ValueType::ARRAY;
    }



    std::string toString() const override
    {
        std::string result = "[";


        for(size_t i = 0; i < values.size(); i++)
        {
            result += values[i]->toString();


            if(i != values.size() - 1)
            {
                result += ", ";
            }
        }


        result += "]";


        return result;
    }



    std::unique_ptr<Value> clone() const override
    {
        std::vector<std::unique_ptr<Value>> copy;


        for(const auto& value : values)
        {
            copy.push_back(
                value->clone()
            );
        }


        return std::make_unique<ArrayValue>(
            std::move(copy)
        );
    }

};
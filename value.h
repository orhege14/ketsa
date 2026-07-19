#pragma once

#include <string>


enum class ValueType
{
    NUMBER,
    STRING,
    BOOLEAN,
    NULL_VALUE
};


struct Value
{
    ValueType type;


    int number;

    std::string string;

    bool boolean;



    Value()
    {
        type = ValueType::NULL_VALUE;
        number = 0;
        boolean = false;
    }



    static Value Number(int value)
    {
        Value v;

        v.type = ValueType::NUMBER;
        v.number = value;

        return v;
    }



    static Value String(std::string value)
    {
        Value v;

        v.type = ValueType::STRING;
        v.string = value;

        return v;
    }



    static Value Boolean(bool value)
    {
        Value v;

        v.type = ValueType::BOOLEAN;
        v.boolean = value;

        return v;
    }

};
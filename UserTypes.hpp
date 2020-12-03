#pragma once

#include <string>
#include <tuple>

struct FullName
{
    std::string first_name;
    std::string last_name;

    FullName(const std::string& first_name, const std::string& last_name) : first_name(first_name), last_name(last_name) {}
};

struct SomeStructure
{
    int a;
    int b;
    std::string c;

    SomeStructure(int a, int b, const std::string& c) : a(a), b(b), c(c) {}

    bool operator==(const SomeStructure& o) const
    {
        return std::tie(a, b, c) == std::tie(o.a, o.b, o.c);
    }
};

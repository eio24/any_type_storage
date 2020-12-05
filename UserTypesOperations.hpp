#pragma once

#include "UserTypes.hpp"

template<typename T> struct FalseValue : std::false_type {};

struct OperationToString
{
    template <typename T>
    std::string operator()(const T& value) { static_assert(FalseValue<T>::value, "not defined"); }
};
template<> std::string OperationToString::operator()(const int& value)    { return std::to_string(value); }
template<> std::string OperationToString::operator()(const double& value) { return std::to_string(value); }


struct OperationAddDotToString
{
    template <typename T>
    std::string operator()(const T& value) { static_assert(FalseValue<T>::value, "not defined"); }
};
template<> std::string OperationAddDotToString::operator()(const std::string& value) { return value + "."; }


struct OperationGetOnlyLastName
{
    template <typename T>
    std::string operator()(const T& value) { static_assert(FalseValue<T>::value, "not defined"); }
};
template<> std::string OperationGetOnlyLastName::operator()(const FullName& value) { return value.last_name; }

#pragma once

#include <memory>

struct BadCastException
{
};

// In real life I would use std::any, which acts about the same, but better
// Unfortunately std::any is C++17 only and uses RTTI for some things (but it can work without RTTI)
class Any
{
public:
    template <typename T>
    Any(T&& t) : mValue(std::make_shared<AnyValue<T>>(std::forward<T>(t)))
    {
        static_assert(std::is_copy_constructible<typename std::decay<T>::type>::value, "T shall satisfy the CopyConstructible requirements");
        static_assert(std::is_destructible<typename std::decay<T>::type>::value, "T shall satisfy the Destructible requirements");
    }

    template <typename T>
    const T& cast() const
    {
        // this part can be omitted if we trust the user
        if (UniqueForType<T>::pseudoTypeId() != mValue->pseudoTypeId)
            throw BadCastException();

        return std::static_pointer_cast<const AnyValue<T>>(mValue)->originalValue;
    }

private:
    template<typename T>
    struct UniqueForType
    {
        // yes, it works only in scope of one "module",
        // enough for our purposes
        static void id() {}
        static size_t pseudoTypeId() { return reinterpret_cast<size_t>(&id); }
    };

    struct AnyValueBase
    {
        size_t pseudoTypeId;

        AnyValueBase(const int id) : pseudoTypeId(id) {}
        virtual ~AnyValueBase() {}
    };

    template <typename T>
    struct AnyValue : AnyValueBase
    {
        T originalValue;

        AnyValue(T&& v) :
            AnyValueBase(UniqueForType<T>::pseudoTypeId()),
            originalValue(std::forward<T>(v))
        {
        }
    };

    std::shared_ptr<AnyValueBase> mValue;
};

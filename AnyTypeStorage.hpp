#pragma once

#include "Any.hpp"
#include "Observer.hpp"

struct UnknownKeyException
{
};

template <typename T_key>
class AnyTypeStorage : public NotificationCenter<T_key>
{
public:
    AnyTypeStorage() {};

    template <typename T_value>
    void insertOrAssign(const T_key& key, T_value&& value)
    {
        std::lock_guard<std::recursive_mutex> lock(this->mMutex);

        auto it = mStorage.find(key);
        if (it != mStorage.end())
        {
            it->second = Any(std::forward<T_value>(value));

            // would be good to check if new value is not the same as old value,
            // but by the terms our type may not have any comparison operators
            this->notifyValueChangedForKey(key);
        }
        else
        {
            mStorage.emplace(key, Any(std::forward<T_value>(value)));
            this->notifyValueChangedForKey(key);
        }
    }

    template <typename T_value, typename T_operation>
    void insertOrAssign(const T_key& key, T_value&& value, T_operation operation)
    {
        insertOrAssign(key, operation(std::forward<T_value>(value)));
    }

    template <typename T_value>
    T_value getAs(const T_key& key)
    {
        std::lock_guard<std::recursive_mutex> lock(this->mMutex);

        auto it = mStorage.find(key);
        if (it == mStorage.end())
            throw UnknownKeyException();

        const Any& value = it->second;
        return value.cast<T_value>();
    }

    template <typename T_value_get, typename T_value_out, typename T_operation>
    T_value_out getAndApply(const T_key& key, T_operation operation)
    {
        return operation(getAs<T_value_get>(key));
    }

private:
    std::map<T_key, Any> mStorage;
};


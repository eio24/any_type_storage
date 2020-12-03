#include "AnyTypeStorage.hpp"
#include "UserTypes.hpp"
#include "UserTypesOperations.hpp"

#include <iostream>
#include <cassert>

void test1()
{
    using KeyType = int;
    AnyTypeStorage<KeyType> storage;

    // simple type put and get
    storage.insertOrAssign(1, std::string("foo"));
    assert(storage.getAs<std::string>(1) == "foo");

    // get as wrong type
    bool exception = false;
    try
    {
        storage.getAs<int>(1);
    }
    catch (const BadCastException&)
    {
        exception = true;
    }
    assert(exception);

    // re-assing
    storage.insertOrAssign(1, std::string("bar"));
    assert(storage.getAs<std::string>(1) == "bar");

    // complex type put and get
    storage.insertOrAssign(2, FullName("John", "Silver"));
    auto& full_name = storage.getAs<FullName>(2);
    assert(full_name.first_name == "John");
    assert(full_name.last_name == "Silver");

    storage.insertOrAssign(22, SomeStructure(1, 2, "3"));
    auto& ss = storage.getAs<SomeStructure>(22);
    assert(ss == SomeStructure(1, 2, "3"));

    // put with some operation
    storage.insertOrAssign(3, 123, OperationToString());
    storage.insertOrAssign(4, std::string("abc"), OperationAddDotToString());
    assert(storage.getAs<std::string>(3) == "123");
    assert(storage.getAs<std::string>(4) == "abc.");

    // get with some operation
    auto res = storage.getAndApply<std::string, std::string>(3, OperationAddDotToString());
    assert(res == "123.");
    res = storage.getAndApply<std::string, std::string>(4, OperationAddDotToString());
    assert( res == "abc..");
    // values in storage has not been modified
    assert(storage.getAs<std::string>(3) == "123");
    assert(storage.getAs<std::string>(4) == "abc.");

    // get ref to the part of value
    auto& last_name = storage.getAndApply<FullName, const std::string&>(2, OperationGetOnlyLastName());
    assert(last_name == "Silver");
}

template <typename T>
class StorageObserverSync : public IStorageObserver<T>
{
public:
    void handleValueChangedForKey(const T& key) final
    {
        std::cout << "sync notification for key: " << key << std::endl;
    }
};

template <typename T>
class StorageObserverAsync : public IStorageObserver<T>
{
public:
    void handleValueChangedForKey(const T& key) final
    {
        std::cout << "start async notification for key: " << key << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(3));
        std::cout << "stop async notification for key: " << key << std::endl;
    }
};

void test2()
{
    using KeyType = std::string;

    AnyTypeStorage<KeyType> storage;

    StorageObserverSync<KeyType> observer_sync;
    StorageObserverAsync<KeyType> observer_async;

    storage.attachToSync(observer_sync);
    storage.attachToAsync(observer_async);

    storage.insertOrAssign("a", 1);
    storage.insertOrAssign("a", 2);

    storage.insertOrAssign("b", 3);
    storage.insertOrAssign("c", 4);

    storage.detach(observer_sync);
    storage.detach(observer_async);
}

void test3()
{
    // all of this should not compile due to static asserts

    using KeyType = int;
    AnyTypeStorage<KeyType> storage;

    struct NoMatterWhat {};

    // OperationToString can handle only int and double
    //storage.insertOrAssign(1, NoMatterWhat(), OperationToString());

    // OperationAddDotToString can handle only string
    //storage.insertOrAssign(1, NoMatterWhat(), OperationAddDotToString());

    // OperationGetOnlyLastName can handle only FullName struct
    //storage.insertOrAssign(1, NoMatterWhat(), OperationGetOnlyLastName());

    // storage value should be CopyConstructible
    struct NonCopyConstructable
    {
        NonCopyConstructable() {}
        NonCopyConstructable(const NonCopyConstructable&) = delete;
    };
    //storage.insertOrAssign(1, NonCopyConstructable());
}

int main()
{
    test1();
    test2();
    test3();

    return 0;
}


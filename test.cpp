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
    auto full_name = storage.getAs<FullName>(2);
    assert(full_name.first_name == "John");
    assert(full_name.last_name == "Silver");

    storage.insertOrAssign(22, SomeStructure(1, 2, "3"));
    auto ss = storage.getAs<SomeStructure>(22);
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

    auto last_name = storage.getAndApply<FullName, std::string>(2, OperationGetOnlyLastName());
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

void test4()
{
    using KeyType = int;
    AnyTypeStorage<KeyType> shared_storage;

    class ObserverSync : public IStorageObserver<KeyType>
    {
    public:
        ObserverSync(int thread_ix, AnyTypeStorage<KeyType>& shared_storage) : thread_ix(thread_ix), shared_storage(shared_storage) {}

        void handleValueChangedForKey(const KeyType& key) final
        {
            std::cout << "thread#" << thread_ix << ": sync notification for key: " << key << ", value=" << shared_storage.getAs<int>(key) << std::endl;
        }

        AnyTypeStorage<KeyType>& shared_storage;
        int thread_ix;
    };

    class ObserverAsync : public IStorageObserver<KeyType>
    {
    public:
        ObserverAsync(int thread_ix, AnyTypeStorage<KeyType>& shared_storage) : thread_ix(thread_ix), shared_storage(shared_storage) {}

        void handleValueChangedForKey(const KeyType& key) final
        {
            std::cout << "thread#" << thread_ix << ": start async notification for key: " << key << ", value=" << shared_storage.getAs<int>(key) << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));
            std::cout << "thread#" << thread_ix << ": stop async notification for key: " << key << std::endl;
        }

        AnyTypeStorage<KeyType>& shared_storage;
        int thread_ix;
    };

    std::list<std::future<void>> futures;
    for (int thread_ix = 0; thread_ix < 5; ++thread_ix)
    {
        futures.emplace_back(std::async(std::launch::async, [thread_ix, &shared_storage]()
        {
            std::cout << "start thread #" << thread_ix << std::endl;

            ObserverSync observer_sync(thread_ix, shared_storage);
            ObserverAsync observer_async(thread_ix, shared_storage);

            shared_storage.attachToSync(observer_sync);
            shared_storage.attachToAsync(observer_async);

            shared_storage.insertOrAssign(1, thread_ix);
            std::cout << "thread#" << thread_ix << ": value=" << shared_storage.getAs<int>(1) << std::endl;

            shared_storage.detach(observer_sync);
            shared_storage.detach(observer_async);

            std::cout << "stop thread #" << thread_ix << std::endl;
        }));
    }
}

int main()
{
    test1();
    test2();
    test3();
    test4();

    return 0;
}


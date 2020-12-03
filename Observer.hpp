#pragma once

#include <map>
#include <list>
#include <future>
#include <thread>
#include <chrono>

template <typename T>
class IStorageObserver
{
public:
    virtual void handleValueChangedForKey(const T& key) = 0;
    virtual ~IStorageObserver() = default;
};

// usually i just use boost::signals2
template <typename T>
class NotificationCenter
{
public:
    void attachToSync(IStorageObserver<T>& o)
    {
        mSyncObservers.push_back(&o);
    }

    // observer must detach before destruction
    void attachToAsync(IStorageObserver<T>& o)
    {
        mAsyncObservers.emplace(&o, std::list<std::future<void>>{});
    }

    void detach(IStorageObserver<T>& o)
    {
        mSyncObservers.remove(&o);

        auto it = mAsyncObservers.find(&o);
        if (it != mAsyncObservers.end())
        {
            auto& futures = it->second;

            // wait for all working threads for this observer
            for (auto& f : futures)
                f.wait();

            mAsyncObservers.erase(it);
        }
    }

protected:
    void notifyValueChangedForKey(const T& key)
    {
        for (auto& pair : mAsyncObservers)
        {
            auto* o = pair.first;
            auto& futures = pair.second;

            // remove completed futures from waiting list
            for (auto it = futures.begin(); it != futures.end(); )
            {
                auto& future = *it;
                if (future.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
                    it = futures.erase(it);
                else
                    ++it;
            }

            // in real life probably it would be better to have only one separate thread for all asynchronous notifications
            std::future<void> f = std::async(std::launch::async, [o, key]()
            {
                o->handleValueChangedForKey(key);
            });

            // put future to waiting list
            futures.emplace_back(std::move(f));
        }

        for (auto o : mSyncObservers)
            o->handleValueChangedForKey(key);
    }

private:
    std::list<IStorageObserver<T>*> mSyncObservers;
    std::map<IStorageObserver<T>*, std::list<std::future<void>>> mAsyncObservers;
};

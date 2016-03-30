#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <deque>
#include <map>
#include <memory>

#include <thread>
#include <list>

// Begin Dispatcher Class Section

using EventType = std::string;

class Subscriber;

class Dispatcher {

  private:
    Dispatcher();
    void Initialize();

    static std::atomic<bool> initialized;
    static std::atomic<bool> running;

    static Dispatcher* instance;

    std::deque<std::pair<EventType, std::shared_ptr<void>>>* dispatch_events;
    std::map<EventType, std::list<Subscriber*>*>* mapped_events;

    static std::deque<std::pair<Subscriber*, std::shared_ptr<void>>>* thread_queue;
    static std::deque<std::pair<Subscriber*, std::shared_ptr<void>>>* nonserial_queue;

    std::deque<std::thread*>* processing_threads; // using std::deque for constant time size() and O(1) random access

    static std::recursive_mutex nonserial_queue_mutex;
    static std::recursive_mutex dispatch_queue_mutex;
    static std::recursive_mutex thread_queue_mutex;
    static std::recursive_mutex mapped_event_mutex;
    static std::condition_variable_any thread_signal;

    Dispatcher(const Dispatcher&);            // disallow copying
    Dispatcher& operator=(const Dispatcher&); // disallow copying

    inline void CheckKey(EventType eventID) {
        std::lock_guard<std::recursive_mutex> mapped_event_lock(mapped_event_mutex);
        if (mapped_events->count(eventID) == 0) {
            mapped_events->emplace(eventID, new std::list<Subscriber*>());
            return;
        }
    }

  public:
    static Dispatcher* GetInstance();

    ~Dispatcher();

    void Terminate();

    void AddEventSubscriber(Subscriber* requestor, const EventType);
    std::list<Subscriber*> GetAllSubscribers(const void* owner);

    void DispatchEvent(const EventType eventID, const std::shared_ptr<void> eventData);
    void DispatchImmediate(const EventType eventID, const std::shared_ptr<void> eventData);

    void Pump();
    void NonSerialProcess();

    unsigned int GetApproximateSize() { return thread_queue->size(); }

  private:
    static void ThreadProcess(int thread_id);
};

#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <vector>
#include <map>
#include <memory>

#include <thread>
#include <list>

// Begin Dispatcher Class Section
class Subscriber;

using EventType = std::string;
using WorkTarget = Subscriber*;
using WorkArguments = std::shared_ptr<void>;
using WorkPair = std::pair<WorkTarget, WorkArguments>;

class Subscriber;

class Dispatcher {

  private:
    Dispatcher();
    void Initialize();

    static std::atomic<bool> initialized;
    static std::atomic<bool> running;

    static Dispatcher* instance;

    std::vector<std::pair<EventType, WorkArguments>>* dispatch_events;
    std::map<EventType, std::list<WorkTarget>*>* mapped_events;

    static std::list<WorkPair>* thread_queue;
    static std::list<WorkPair>* nonserial_queue;

    std::vector<std::thread*>* processing_threads; // using std::vector for constant time size() and O(1) random access

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
            mapped_events->emplace(eventID, new std::list<WorkTarget>());
            return;
        }
    }

  public:
    static Dispatcher* GetInstance();

    ~Dispatcher();

    void Terminate();

    void AddEventSubscriber(WorkTarget requestor, const EventType);
    std::list<Subscriber*> GetAllSubscribers(const void* owner);

    void DispatchEvent(const EventType eventID, const WorkArguments eventData);
    void DispatchImmediate(const EventType eventID, const WorkArguments eventData);

    void Pump();
    void NonSerialProcess();

    unsigned int GetApproximateSize() { return thread_queue->size(); }

  private:
    static void ThreadProcess(int thread_id);
};

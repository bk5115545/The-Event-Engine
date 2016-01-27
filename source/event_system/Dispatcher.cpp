// Copyright Casey Megginsons and Blaise Koch 2015

#include <condition_variable>
#include <mutex>
#include <deque>
#include <map>
#include <string>
#include <list>
#include <utility>
#include <iostream>

#include "event_system/Dispatcher.h"
#include "event_system/Subscriber.h"

#include "util/CrossPlatform.h"

// Dispatcher Static Variables
bool Dispatcher::initialized = false;
bool Dispatcher::running = false;

std::mutex Dispatcher::dispatch_queue_mutex;
std::mutex Dispatcher::thread_queue_mutex;
std::mutex Dispatcher::mapped_event_mutex;
std::condition_variable Dispatcher::thread_signal;

Dispatcher* Dispatcher::instance = nullptr;
std::deque<std::pair<Subscriber*, std::shared_ptr<void>>>* Dispatcher::thread_queue;
std::deque<std::pair<Subscriber*, std::shared_ptr<void>>>* Dispatcher::nonserial_queue;

std::atomic_int Dispatcher::processing_count;
std::atomic_int Dispatcher::in_queue_count;
std::atomic_int Dispatcher::nonserial_queue_count;

// Begin Class Methods
Dispatcher::Dispatcher() {}

Dispatcher::~Dispatcher() { Terminate(); }

Dispatcher* Dispatcher::GetInstance() {
    if (instance == nullptr) {
        instance = new Dispatcher();
        instance->Initialize();
    }

    return instance;
}

void Dispatcher::Initialize() {
    if (!initialized) {
        initialized = true;

        dispatch_events = new std::deque<std::pair<EventType, std::shared_ptr<void>>>();
        mapped_events = new std::map<EventType, std::list<Subscriber*>*>();
        thread_queue = new std::deque<std::pair<Subscriber*, std::shared_ptr<void>>>();
        nonserial_queue = new std::deque<std::pair<Subscriber*, std::shared_ptr<void>>>();

        processing_threads = new std::deque<std::thread*>();

        running = true;

        // Adjust the thread count
        for (int i = 0; i < 3; i++) {
            std::thread* processing_thread = new std::thread(ThreadProcess);
            processing_threads->push_back(processing_thread);
        }
    } else {
        std::cerr << "Attempting to reinitialize a dispatcher...  Ignoring." << std::endl;
    }
}

void Dispatcher::Pump() {
    if (!Dispatcher::running)
        return;

    std::lock_guard<std::mutex> dispatchLock(dispatch_queue_mutex);
    for (auto i : *dispatch_events) {
        // try is needed to handle linux map implementations
        try {
            // handle microsoft map implementation
            if (mapped_events->count(i.first) == 0) {
                mapped_events->emplace(i.first, new std::list<Subscriber*>());
            }
            if (mapped_events->count(i.first) == 0) {
                // std::cerr << "Event \"" + i.first + "\" does not apply to any Subscribers." << std::endl;
                continue;
            }
            DispatchImmediate(i.first, i.second);
        } catch (std::string msg) {
            // std::cerr << "Either event \"" + i.first + "\" does not apply to any Subscribers." << std::endl;
            // std::cerr << "Or " << msg << std::endl;
        }
    }
    dispatch_events->clear(); // we queued them all for processing so clear the cache
}

void Dispatcher::NonSerialProcess() {
    while (nonserial_queue->size() > 0) {
        auto work = nonserial_queue->front();
        nonserial_queue->pop_front();

        try {
            if (work.first->method == NULL)
                continue;
            work.first->method(work.second);
        } catch (std::string msg) {
            std::cerr << "Exception thrown by function called in nonserial processing." << std::endl;
            std::cerr << msg << std::endl;
        }
        nonserial_queue_count--;
    }
}

void Dispatcher::ThreadProcess() {
    while (running) {
        std::pair<Subscriber*, std::shared_ptr<void>> work;
        try {
            std::unique_lock<std::mutex> lock(thread_queue_mutex);
            while (running && in_queue_count == 0)
                thread_signal.wait(lock);
            if (!running)
                continue;
            in_queue_count--;
            work = thread_queue->front();
            thread_queue->pop_front();
        } catch (std::string e) {
            std::cerr << "Exception thrown while waiting/getting work for Thread." << std::endl;
            std::cerr << e << std::endl;
            continue;
        }

        try {
            if (work.first->method == NULL)
                continue;
            processing_count++;
            work.first->method(work.second);
        } catch (std::string e) {
            std::cerr << "Exception thrown by function called by Event Threads." << std::endl;
            std::cerr << e << std::endl;
        }
        processing_count--;
    }
}

void Dispatcher::DispatchEvent(const EventType eventID, const std::shared_ptr<void> eventData) {
    std::lock_guard<std::mutex> dispatchLock(dispatch_queue_mutex);
    dispatch_events->push_back(std::pair<EventType, std::shared_ptr<void>>(eventID, eventData));
}

void Dispatcher::DispatchImmediate(EventType eventID, const std::shared_ptr<void> eventData) {
    // std::cout << "Dispatcher --->  Received event " << eventID << "." << std::endl;
    std::lock_guard<std::mutex> dispatchLock(mapped_event_mutex);

    if (mapped_events->count(eventID) == 0) {
        mapped_events->emplace(eventID, new std::list<Subscriber*>());
        // std::cerr << "Event \"" + eventID + "\" does not apply to any Subscribers." << std::endl;
        return;
    }

    if (mapped_events->at(eventID)->size() == 0) {
        // std::cerr << "Event \"" + eventID + "\" does not apply to any Subscribers." << std::endl;
        return;
    }

    std::lock_guard<std::mutex> lock(thread_queue_mutex);
    for (auto it = mapped_events->at(eventID)->begin(); it != mapped_events->at(eventID)->end(); it++) {
        if (*it == nullptr) {
            it = mapped_events->at(eventID)->erase(it);
            continue;
        }
        if ((*it)->serialized) {
            thread_queue->push_back(std::pair<Subscriber*, std::shared_ptr<void>>(*it, eventData));
            in_queue_count++;
            thread_signal.notify_one();
        } else {
            nonserial_queue->push_back(std::pair<Subscriber*, std::shared_ptr<void>>(*it, eventData));
            nonserial_queue_count++;
        }
    }
}

void Dispatcher::AddEventSubscriber(Subscriber* requestor, const EventType event_id) {
    if (mapped_events->count(event_id) < 1) {
        std::cerr << "Dispatcher --->  Dynamically allocating list for EventID " << event_id << "." << std::endl;
        mapped_events->emplace(event_id, new std::list<Subscriber*>());
    }
    mapped_events->at(event_id)->push_back(requestor);
}

// TODO(bk5115545) reimplement using map traversal
std::list<Subscriber*> Dispatcher::GetAllSubscribers(const void* owner) {
    std::list<Subscriber*> tmp;
    UNUSED(owner);
    return tmp;
}

void Dispatcher::Terminate() {
    Dispatcher::running = false;

    // Notify threads to resume processing so they
    // terminate before the condition variable is uninitialized (avoid crash from microsoft)
    thread_signal.notify_all();
    for (std::thread* t : *processing_threads) {
        t->join(); // should stop eventually...
        delete t;  // i'm pretty sure we need to shutdown the threads before we delete them
    }

    delete processing_threads;

    // There is a race condition with the condition variable and the threadpool on Microsoft implementations
    // so we need to avoid it as much as reasonably possible
    sleep(500);

    dispatch_events->clear();
    delete dispatch_events;

    for (auto itr = mapped_events->begin(); itr != mapped_events->end(); itr++) {
        if (itr->second != nullptr)
            delete itr->second;
    }
    delete mapped_events;

    thread_queue->clear();
    delete thread_queue;

    nonserial_queue->clear();
    delete nonserial_queue;

    instance = nullptr;
    Dispatcher::initialized = false;
}

int Dispatcher::ThreadQueueSize() { return in_queue_count; }
int Dispatcher::NonSerialQueueSize() { return nonserial_queue_count; }
bool Dispatcher::Active() { return processing_count > 0; }

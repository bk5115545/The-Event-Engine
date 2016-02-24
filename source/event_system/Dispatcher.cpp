// Copyright Casey Megginsons and Blaise Koch 2015

#include <algorithm>
#include <condition_variable>
#include <mutex>
#include <deque>
#include <queue>
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
std::mutex Dispatcher::nonserial_queue_mutex;
std::mutex Dispatcher::dispatch_queue_mutex;
std::mutex Dispatcher::thread_queue_mutex;
std::mutex Dispatcher::mapped_event_mutex;
std::condition_variable Dispatcher::thread_signal;

Dispatcher* Dispatcher::instance = nullptr;
std::deque<std::pair<Subscriber*, std::shared_ptr<void>>>* Dispatcher::thread_queue;
std::deque<std::pair<Subscriber*, std::shared_ptr<void>>>* Dispatcher::nonserial_queue;

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
        for (int i = 0; i < 2; i++) {
            std::thread* processing_thread = new std::thread(ThreadProcess, i);
            processing_threads->push_back(processing_thread);
        }
    } else {
        std::cerr << "Attempting to reinitialize a dispatcher...  Ignoring." << std::endl;
    }
}

void Dispatcher::Pump() {
    if (!Dispatcher::running)
        return;

    std::unique_lock<std::mutex> dispatchLock(dispatch_queue_mutex);
    try {
        for (auto i : *dispatch_events) {
            // try is needed to handle linux map implementations
            try {
                // handle microsoft map implementation
                if (mapped_events->count(i.first) == 0) {
                    mapped_events->emplace(i.first, new std::list<Subscriber*>());
                    continue;
                }
                dispatchLock.unlock();
                DispatchImmediate(i.first, i.second);
                dispatchLock.lock();
            } catch (std::string msg) {
            }
        }
    } catch (std::string message) {
        std::cerr << "Error in Dispatcher::Pump()" << std::endl << message << std::endl;
    }

    thread_signal.notify_all(); // it's possible we're in a pseudo-deadlock because the entire thread_queue
                                // wasn't consumed in 1 pass of the thread pool
                                // so wake up all the threads even if there are no new events to process

    dispatchLock.unlock();
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
    }
}

void Dispatcher::ThreadProcess(int thread_id_passed) {
    const thread_local int cache_size = 1;
    const thread_local int thread_id = thread_id_passed;
    // There is a lot of issues with threads blocking on the thread_queue_mutex lock
    // if each thread had a list of jobs to do then this wouldn't be nearly as big of an issue
    thread_local std::deque<std::pair<Subscriber*, std::shared_ptr<void>>> thread_cache;
    thread_local std::pair<Subscriber*, std::shared_ptr<void>> work;

    while (running) {
        // std::cerr << "Thread acquire lock." << std::endl;
        std::unique_lock<std::mutex> lock(thread_queue_mutex);

        while (running) {
            // std::cerr << "Thread release lock." << std::endl;
            thread_signal.wait(lock);
            // std::cerr << "Thread awake." << std::endl;
            if (thread_queue->size() > 0)
                break;
        }

        if (!running)
            continue;

        unsigned int available = thread_queue->size() > cache_size - 1 ? cache_size : thread_queue->size() - 1;
        // std::cout << "Thread " << thread_id << " is consuming " << available << " work items from global queue."
        //   << std::endl;

        try {
            /* // This is slow and not threadsafe
            for (unsigned int i = 0; i < available; i++) {
                thread_cache.push_back(thread_queue->front());
                thread_queue->pop_front();
                // std::cout << "Got event" << std::endl;
            }
            */

            // new method to move work to the cache
            thread_cache.insert(thread_cache.begin(), std::make_move_iterator(thread_queue->begin()),
                                std::make_move_iterator(min(thread_queue->begin() + available, thread_queue->end())));

            thread_queue->erase(thread_queue->begin(), min(thread_queue->end() + available, thread_queue->end() - 1));

        } catch (std::string e) {
            std::cerr << "Exception thrown while waiting/getting work for Thread." << std::endl;
            std::cerr << e << std::endl;
            continue;
        }

        // std::cout << "Pre thread explicit unlock." << std::endl;
        thread_queue_mutex.unlock();
        // std::cerr << "Thread post unlock." << std::endl;

        for (unsigned int i = 0; i < thread_cache.size() && i < available; i++) {
            try {
                // std::cerr << "Thread try_call." << std::endl;
                work = thread_cache.at(i);
                if (!work.first->method) {
                    std::cerr << "Subscriber with NULL method was almost called." << std::endl;
                    continue;
                }
                work.first->method(work.second);
            } catch (std::string e) {
                std::cerr << "Exception thrown by function called by Dispatcher Threads." << std::endl;
                std::cerr << e << std::endl;
            }
        }
        thread_cache.clear();
    }
}

void Dispatcher::DispatchEvent(const EventType eventID, const std::shared_ptr<void> eventData) {
    std::lock_guard<std::mutex> dispatchLock(dispatch_queue_mutex);
    dispatch_events->push_back(std::pair<EventType, std::shared_ptr<void>>(eventID, eventData));
    thread_signal.notify_one();
}

void Dispatcher::DispatchImmediate(EventType eventID, const std::shared_ptr<void> eventData) {
    if (mapped_events->count(eventID) == 0) {
        mapped_events->emplace(eventID, new std::list<Subscriber*>());
        return;
    }

    if (mapped_events->at(eventID)->size() == 0) {
        return;
    }

    for (auto it = mapped_events->at(eventID)->begin(); it != mapped_events->at(eventID)->end(); it++) {
        if (*it == nullptr) {
            it = mapped_events->at(eventID)->erase(it);
            continue;
        }

        if ((*it)->serialized) {
            std::lock_guard<std::mutex> lock(thread_queue_mutex);
            thread_queue->push_back(std::pair<Subscriber*, std::shared_ptr<void>>(*it, eventData));
        } else {
            std::lock_guard<std::mutex> lock2(nonserial_queue_mutex);
            nonserial_queue->push_back(std::pair<Subscriber*, std::shared_ptr<void>>(*it, eventData));
        }
    }

    thread_signal.notify_all();
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

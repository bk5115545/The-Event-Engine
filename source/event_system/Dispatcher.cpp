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
std::recursive_mutex Dispatcher::nonserial_queue_mutex;
std::recursive_mutex Dispatcher::dispatch_queue_mutex;
std::mutex Dispatcher::thread_queue_mutex;
std::recursive_mutex Dispatcher::mapped_event_mutex;
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
        for (int i = 0; i < 4; i++) {
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

    std::unique_lock<std::recursive_mutex> dispatchLock(dispatch_queue_mutex);
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
                                // but there's probably events because you're using a high performance event library...
    dispatch_events->clear();   // we queued them all for processing so clear the cache

    dispatchLock.unlock();
}

void Dispatcher::NonSerialProcess() {
    std::lock_guard<std::recursive_mutex> lock(nonserial_queue_mutex);
    while (nonserial_queue->size() > 0) {
        // NonSericalQueue::value_type work
        std::pair<Subscriber*, std::shared_ptr<void>> work = nonserial_queue->front();
        nonserial_queue->pop_front();

        try {
            std::cout << work.first << "\t" << work.second << std::endl;
            work.first->call(work.second);
        } catch (std::string msg) {
            std::cerr << "Exception thrown by function called in nonserial processing." << std::endl;
            std::cerr << msg << std::endl;
        }
    }
}

void Dispatcher::ThreadProcess(int thread_id_passed) {
    const int cache_size = 5;
    const int thread_id = thread_id_passed;
    UNUSED(thread_id);

    // There is a lot of issues with threads blocking on the thread_queue_mutex lock
    // if each thread had a list of jobs to do then this wouldn't be nearly as big of an issue
    std::deque<std::pair<Subscriber*, std::shared_ptr<void>>> thread_cache; // a list of jobs to do

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

        try {
            // move work to the cache
            thread_cache.insert(thread_cache.begin(), std::make_move_iterator(thread_queue->begin()),
                                std::make_move_iterator(min(thread_queue->begin() + available, thread_queue->end())));

            thread_queue->erase(thread_queue->begin(), min(thread_queue->end() + available, thread_queue->end() - 1));
            thread_queue->shrink_to_fit(); // TODO(bk5115545) Find way to remove for performance reasons

        } catch (std::string e) {
            std::cerr << "Exception thrown while waiting/getting work for Thread.  " << available
                      << " jobs possibly lost." << std::endl;
            std::cerr << e << std::endl;

            thread_queue_mutex.unlock();
            continue;
        }

        // std::cout << "Pre thread explicit unlock." << std::endl;
        thread_queue_mutex.unlock();
        // std::cerr << "Thread post unlock." << std::endl;

        for (unsigned int i = 0; i < thread_cache.size() && i < available; i++) {
            try {
                // std::cerr << "Thread try_call try." << std::endl;
                std::pair<Subscriber*, std::shared_ptr<void>>& work = thread_cache.at(i);
                work.first->call(work.second);
                // std::cerr << "Thread try_call success." << std::endl;
            } catch (std::string e) {
                std::cerr << "Exception thrown by function called by Dispatcher Threads." << std::endl;
                std::cerr << e << std::endl;
            }
        }
        thread_cache.clear();
        thread_cache.shrink_to_fit(); // TODO(bk5115545) find way to remove for performance reasons
    }
}

void Dispatcher::DispatchEvent(const EventType eventID, const std::shared_ptr<void> eventData) {
    std::lock_guard<std::recursive_mutex> dispatchLock(dispatch_queue_mutex);
    dispatch_events->push_back(std::pair<EventType, std::shared_ptr<void>>(eventID, eventData));
    thread_signal.notify_one();
}

void Dispatcher::DispatchImmediate(EventType eventID, const std::shared_ptr<void> eventData) {
    std::cout << "DispatchImmediate " << eventID << "\t" << eventData << std::endl;
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

        if ((*it)->_serialized) {
            std::lock_guard<std::mutex> lock(thread_queue_mutex);
            thread_queue->push_back(std::pair<Subscriber*, std::shared_ptr<void>>(*it, eventData));
        } else {
            std::lock_guard<std::recursive_mutex> lock2(nonserial_queue_mutex);
            nonserial_queue->push_back(std::pair<Subscriber*, std::shared_ptr<void>>(*it, eventData));
        }
    }

    thread_signal.notify_all();
}

void Dispatcher::AddEventSubscriber(Subscriber* requestor, const EventType event_id) {
    if (mapped_events->count(event_id) == 0) {
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

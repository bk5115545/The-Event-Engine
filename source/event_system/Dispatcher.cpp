// Copyright Casey Megginsons and Blaise Koch 2015

#include <algorithm>
#include <atomic>
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
std::atomic<bool> Dispatcher::initialized(false);
std::atomic<bool> Dispatcher::running(false);
std::recursive_mutex Dispatcher::nonserial_queue_mutex;
std::recursive_mutex Dispatcher::dispatch_queue_mutex;
std::recursive_mutex Dispatcher::thread_queue_mutex;
std::recursive_mutex Dispatcher::mapped_event_mutex;
std::condition_variable_any Dispatcher::thread_signal;

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
        initialized.store(true);

        dispatch_events = new std::deque<std::pair<EventType, std::shared_ptr<void>>>();
        mapped_events = new std::map<EventType, std::list<Subscriber*>*>();
        thread_queue = new std::deque<std::pair<Subscriber*, std::shared_ptr<void>>>();
        nonserial_queue = new std::deque<std::pair<Subscriber*, std::shared_ptr<void>>>();

        processing_threads = new std::deque<std::thread*>();

        running.store(true);

        // Adjust the thread count
        for (int i = 0; i < 7; i++) {
            std::thread* processing_thread = new std::thread(ThreadProcess, i);
            processing_threads->push_back(processing_thread);
        }
    } else {
        std::cerr << "Attempting to reinitialize a dispatcher...  Ignoring." << std::endl;
    }
}

void Dispatcher::Pump() {
    if (!running)
        return;

    std::deque<std::pair<Subscriber*, std::shared_ptr<void>>> serialized;
    std::deque<std::pair<Subscriber*, std::shared_ptr<void>>> unserialized;

    std::lock_guard<std::recursive_mutex> dispatchLock(dispatch_queue_mutex);
    try {
        for (auto i : *dispatch_events) {
            // try is needed to handle linux map implementations
            try {
                CheckKey(i.first);

                // DispatchImmediate(i.first, i.second);

                // If there's nothing to deliver this event to just drop it
                if (mapped_events->at(i.first)->size() == 0) {
                    return;
                }

                std::lock_guard<std::recursive_mutex> mapped_event_lock(mapped_event_mutex);
                for (auto it = mapped_events->at(i.first)->begin(); it != mapped_events->at(i.first)->end(); it++) {

                    // Remove nullptr Subscriber* from the processing
                    if (*it == nullptr) {
                        it = mapped_events->at(i.first)->erase(it);
                        continue;
                    }

                    if ((*it)->_serialized) {
                        // thread_queue->push_back(std::pair<Subscriber*, std::shared_ptr<void>>(*it, eventData));
                        serialized.push_back(std::pair<Subscriber*, std::shared_ptr<void>>(*it, i.second));
                    } else {
                        // nonserial_queue->push_back(std::pair<Subscriber*, std::shared_ptr<void>>(*it, eventData));
                        unserialized.push_back(std::pair<Subscriber*, std::shared_ptr<void>>(*it, i.second));
                    }
                }
            } catch (std::string msg) {
                std::cerr << "Internal Logic error in Dispatcher::Pump()" << std::endl << msg << std::endl;
            }
        }
    } catch (std::string message) {
        std::cerr << "Error in Dispatcher::Pump()" << std::endl << message << std::endl;
    }

    std::lock_guard<std::recursive_mutex> thread_queue_lock(thread_queue_mutex);
    thread_queue->insert(thread_queue->end(), serialized.begin(), serialized.end());

    std::lock_guard<std::recursive_mutex> nonserial_queue_lock(nonserial_queue_mutex);
    nonserial_queue->insert(nonserial_queue->end(), unserialized.begin(), unserialized.end());

    thread_signal.notify_all(); // it's possible we're in a pseudo-deadlock because the entire thread_queue
                                // wasn't consumed in 1 pass of the thread pool
                                // so wake up all the threads even if there are no new events to process
                                // but there's probably events because you're using a high performance event library...

    dispatch_events->clear(); // we queued them all for processing so clear the cache
}

void Dispatcher::NonSerialProcess() {
    std::lock_guard<std::recursive_mutex> lock(nonserial_queue_mutex);
    while (nonserial_queue->size() > 0) {
        // NonSericalQueue::value_type work
        std::pair<Subscriber*, std::shared_ptr<void>> work = nonserial_queue->front();
        nonserial_queue->pop_front(); // wait to invalidate reference until work is finished

        try {
            // std::cout << work.first << "\t" << work.second << std::endl;
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

    // std::cerr << "Thread acquire lock." << std::endl;
    thread_queue_mutex.lock();

    while (running) {

        while (running) {
            // std::cerr << "Thread release lock." << std::endl;
            thread_signal.wait(thread_queue_mutex);
            // std::cerr << "Thread awake." << std::endl;
            if (thread_queue->size() > 0)
                break;
        }

        if (!running) {
            thread_queue_mutex.unlock();
            continue;
        }

        unsigned int available = thread_queue->size() > cache_size ? cache_size : thread_queue->size();
        unsigned int allocated = 0;
        bool erased = false;

        try {
            // move work to the cache
            thread_cache.insert(thread_cache.begin(), thread_queue->begin(), thread_queue->begin() + available);
            allocated = available;
            thread_queue->erase(thread_queue->begin(), thread_queue->begin() + available);
            erased = true;
        } catch (std::string e) {
            std::cerr << "Exception thrown while waiting/getting work for Thread.  " << available
                      << " jobs possibly lost." << std::endl;
            std::cerr << e << std::endl;

            thread_queue_mutex.unlock();
            continue; // no jobs so lets just wait
        }

        /*
            it's highly probable that these jobs don't exist in this thread's L2-i cache portion
            it's also probable that there are too many jobs to cache into the shared L3
            lets try to invoke the built-in prefetch since we're sure of the execution path

            Note that even when L2 is unified between code and data (and it often is nowadays), the TLB is rarely
            shared. As such the prefetch instruction when present will most likely use the data TLB (the intel
            documentation isn't explicit but it uses the word 'data' everywhere). So when the code gets to run the
            prefetched bytes, it might generate a TLB miss but the entries required to resolve it will already be primed
            in the L2 (page walking entries are stored in L2 like normal data) which SHOULD still be faster than
           defering to main memory.
        */

        if (allocated == available && !erased) {
            std::cerr << "Could not erase " << available << " jobs from work queue.  They will be double processed."
                      << std::endl;
        }

        // std::cout << "Pre thread explicit unlock." << std::endl;
        thread_queue_mutex.unlock();
        // std::cerr << "Thread post unlock." << std::endl;

        for (unsigned int i = 0; i < thread_cache.size(); i++) {
            try {
                // std::cerr << "Thread try_call try." << std::endl;
                std::pair<Subscriber*, std::shared_ptr<void>>& work = thread_cache.at(i);

                // put first cache-line (between 32 and 64 bytes) of next function into L2-d cache (which is HOPEFULLY
                // faster than referencing main memory)
                // but it would be a lot nicer if x86 had instructions to prefetch into the instruction cache
                // for rare but time-sensitive code paths
                if (i + 1 < thread_cache.size() - 1)
                    __builtin_prefetch((thread_cache.at(i + 1).first->target_for_prefetch()), 0, 1);

                work.first->call(work.second);
                // std::cerr << "Thread try_call success." << std::endl;
            } catch (std::string e) {
                std::cerr << "Exception thrown by function called by Dispatcher Threads." << std::endl;
                std::cerr << e << std::endl;
            }
        }
        thread_cache.clear();
    }
}

void Dispatcher::DispatchEvent(const EventType eventID, const std::shared_ptr<void> eventData) {
    std::lock_guard<std::recursive_mutex> dispatchLock(dispatch_queue_mutex);
    dispatch_events->push_back(std::pair<EventType, std::shared_ptr<void>>(eventID, eventData));
}

void Dispatcher::DispatchImmediate(EventType eventID, const std::shared_ptr<void> eventData) {
    // std::cout << "DispatchImmediate " << eventID << "\t" << eventData << std::endl;
    CheckKey(eventID);

    // If there's nothing to deliver this event to just drop it
    if (mapped_events->at(eventID)->size() == 0) {
        return;
    }

    for (auto it = mapped_events->at(eventID)->begin(); it != mapped_events->at(eventID)->end(); it++) {

        // Remove nullptr Subscriber* from the processing
        if (*it == nullptr) {
            it = mapped_events->at(eventID)->erase(it);
            continue;
        }

        if ((*it)->_serialized) {
            std::lock_guard<std::recursive_mutex> lock(thread_queue_mutex);
            thread_queue->push_back(std::pair<Subscriber*, std::shared_ptr<void>>(*it, eventData));
        } else {
            std::lock_guard<std::recursive_mutex> lock2(nonserial_queue_mutex);
            nonserial_queue->push_back(std::pair<Subscriber*, std::shared_ptr<void>>(*it, eventData));
        }
    }

    thread_signal.notify_all();
}

void Dispatcher::AddEventSubscriber(Subscriber* requestor, const EventType event_id) {
    std::lock_guard<std::recursive_mutex> mapped_event_lock(mapped_event_mutex);
    CheckKey(event_id);
    mapped_events->at(event_id)->push_back(requestor);
}

// TODO(bk5115545) reimplement using map traversal
std::list<Subscriber*> Dispatcher::GetAllSubscribers(const void* owner) {
    std::list<Subscriber*> tmp;
    UNUSED(owner);
    return tmp;
}

void Dispatcher::Terminate() {
    // Notify threads to resume processing so they
    // terminate before the condition variable is uninitialized (avoid crash from microsoft)
    running.store(false);
    for (std::thread* t : *processing_threads) {
        thread_queue_mutex.unlock(); // we definitly don't own the mutex but screw it we're going to shutdown NOW
        thread_signal.notify_all();
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
    Dispatcher::initialized.store(false);
}

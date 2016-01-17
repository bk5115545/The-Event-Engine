// Copyright Casey Megginsons and Blaise Koch 2015

#include <condition_variable>
#include <mutex>
#include <deque>
#include <map>
#include <string>
#include <list>
#include <utility>

#include "event_system/Dispatcher.h"
#include "util/definitions.h"

// Dispatcher Static Variables
bool Dispatcher::initialized_ = false;
bool Dispatcher::running_ = false;

std::mutex Dispatcher::dispatch_queue_mutex_;
std::mutex Dispatcher::thread_queue_mutex_;
std::mutex Dispatcher::mapped_event_mutex_;
std::condition_variable Dispatcher::thread_signal_;

Dispatcher* Dispatcher::instance_ = nullptr;
std::deque<std::pair<Subscriber*, std::shared_ptr<void>>>*  Dispatcher::thread_queue_;
std::deque<std::pair<Subscriber*, std::shared_ptr<void>>>* Dispatcher::nonserial_queue_;

// Begin Class Methods
Dispatcher::Dispatcher() { }

Dispatcher::~Dispatcher() {
    Terminate();
}

Dispatcher* Dispatcher::GetInstance() {
    if (instance_ == nullptr) {
        instance_ = new Dispatcher();
        instance_->Initialize();
    }

    return instance_;
}

void Dispatcher::Initialize() {
    if (!initialized_) {
        initialized_ = true;

        dispatch_events_  = new std::deque<std::pair<EventType, std::shared_ptr<void>>>();
        mapped_events_    = new std::map<EventType, std::list<Subscriber*>*>();
        thread_queue_     = new std::deque<std::pair<Subscriber*, std::shared_ptr<void>>>();
        nonserial_queue_ = new std::deque<std::pair<Subscriber*, std::shared_ptr<void>>>();

        processing_threads_ = new std::deque<std::thread*>();

        running_ = true;

        // Adjust the thread count
        for (int i=0; i < 3; i++) {
            std::thread* processing_thread = new std::thread(ThreadProcess);
            // it probably won't terminate before the end of this program so we want to ignore errors
            // processing_thread->detach();
            processing_threads_->push_back(processing_thread);
        }
    } else {
        std::cerr << "Attempting to reinitialize a dispatcher...  Ignoring." << std::endl;
    }
}

void Dispatcher::Pump() {
    if (!Dispatcher::running_) return;

    std::lock_guard<std::mutex> dispatchLock(dispatch_queue_mutex_);
    for (auto i : *dispatch_events_) {
        // try is needed to handle linux map implementations
        try {
            // handle microsoft map implementation
            if(mapped_events_->count(i.first) == 0) {
                mapped_events_->emplace(i.first, new std::list<Subscriber*>());
            }
            if (mapped_events_->count(i.first) == 0) {
                // std::cerr << "Event \"" + i.first + "\" does not apply to any Subscribers." << std::endl;
                continue;
            }
            DispatchImmediate(i.first, i.second);
        } catch (std::string msg) {
            // std::cerr << "Either event \"" + i.first + "\" does not apply to any Subscribers." << std::endl;
            // std::cerr << "Or " << msg << std::endl;
        }
    }
    dispatch_events_->clear();  // we queued them all for processing so clear the cache
}

void Dispatcher::NonSerialProcess() {
    while(nonserial_queue_->size() > 0) {
        auto work = nonserial_queue_->front();
        nonserial_queue_->pop_front();
        try {
            if (work.first->method == NULL) continue;
            work.first->method(work.second);
        } catch (std::string msg) {
            std::cerr << "Exception thrown by function called in nonserial processing." << std::endl;
            std::cerr << msg << std::endl;
        }
    }
}

void Dispatcher::ThreadProcess() {
    while (running_) {
        std::pair<Subscriber*, std::shared_ptr<void>> work;  // compiler might whine here

        try {
            // enter new scope so std::unique_lock will unlock on exceptions
            std::unique_lock<std::mutex> lock(thread_queue_mutex_);
            while (Dispatcher::running_ && thread_queue_->size() == 0) thread_signal_.wait(lock);
            if (!Dispatcher::running_) continue;
            work = thread_queue_->front();
            thread_queue_->pop_front();
        }
        catch (std::string e) {
            std::cerr << "Exception thrown while waiting/getting work for Thread." << std::endl;
            std::cerr << e << std::endl;
            continue;
        }

        try {
            if (work.first->method == NULL) continue;
            work.first->method(work.second);
        } catch (std::string e) {
            std::cerr << "Exception thrown by function called by Event Threads." << std::endl;
            std::cerr << e << std::endl;
        }
    }
}

void Dispatcher::DispatchEvent(const EventType eventID, const std::shared_ptr<void> eventData) {
    std::lock_guard<std::mutex> dispatchLock(dispatch_queue_mutex_);
    dispatch_events_->push_back(std::pair<EventType, std::shared_ptr<void>>(eventID, eventData));
}

void Dispatcher::DispatchImmediate(EventType eventID, const std::shared_ptr<void> eventData) {
    // std::cout << "Dispatcher --->  Received event " << eventID << "." << std::endl;
    std::lock_guard<std::mutex> dispatchLock(mapped_event_mutex_);

    if (mapped_events_->count(eventID) == 0) {
        mapped_events_->emplace(eventID, new std::list<Subscriber*>());
        // std::cerr << "Event \"" + eventID + "\" does not apply to any Subscribers." << std::endl;
        return;
    }

    if (mapped_events_->at(eventID)->size() == 0) {
        // std::cerr << "Event \"" + eventID + "\" does not apply to any Subscribers." << std::endl;
        return;
    }

    std::lock_guard<std::mutex> lock(thread_queue_mutex_);
    for (auto it=mapped_events_->at(eventID)->begin(); it != mapped_events_->at(eventID)->end(); it++) {
        if(*it == nullptr) {
            it = mapped_events_->at(eventID)->erase(it);
            continue;
        }
        if ((*it)->serialized) {
            thread_queue_->push_back(std::pair<Subscriber*, std::shared_ptr<void>>(*it, eventData));
            thread_signal_.notify_one();
        } else {
            nonserial_queue_->push_back(std::pair<Subscriber*, std::shared_ptr<void>>(*it, eventData));
        }
    }
}

void Dispatcher::AddEventSubscriber(Subscriber *requestor, const EventType event_id) {
    if (mapped_events_->count(event_id) < 1) {
        std::cerr << "Dispatcher --->  Dynamically allocating list for EventID "
            << event_id << "."
            << std::endl << "Dispatcher --->  This should be avoided for performance reasons."
            << std::endl;
        mapped_events_->emplace(event_id, new std::list<Subscriber*>());
    }
    mapped_events_->at(event_id)->push_back(requestor);
}

// TODO(bk5115545) reimplement using map traversal
std::list<Subscriber*> Dispatcher::GetAllSubscribers(const void* owner) {
    std::list<Subscriber*> tmp;
    UNUSED(owner);
    return tmp;
}

void Dispatcher::Terminate() {
    Dispatcher::running_ = false;

    // Notify threads to resume processing so they
    // terminate before the condition variable is uninitialized (avoid crash from microsoft)
    thread_signal_.notify_all();
    for (std::thread* t : *processing_threads_) {
        t->join();  // should stop eventually...
        delete t;   // i'm pretty sure we need to shutdown the threads before we delete them
    }

    // There is a race condition with the condition variable and the threadpool on Microsoft implementations
    // so we need to avoid it as much a possible
    sleep(500);

    dispatch_events_->clear();
    delete dispatch_events_;
    delete mapped_events_;

    thread_queue_->clear();
    delete thread_queue_;

    nonserial_queue_->clear();
    delete nonserial_queue_;

    instance_ = nullptr;
}

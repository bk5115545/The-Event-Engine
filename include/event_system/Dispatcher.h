#pragma once

#include "util/definitions.h"

#include <condition_variable>
#include <mutex>
#include <deque>
#include <map>
#include <memory>

//Begin Dispatcher Class Section

using EventType = std::string;

class Subscriber;

class Dispatcher {

    private:
        Dispatcher();
        void Initialize();

        static bool initialized_;
        static bool running_;

        static Dispatcher* instance_;

        std::deque<std::pair<EventType,std::shared_ptr<void>>>*  dispatch_events_;
        std::map<EventType,std::list<Subscriber*>*>* mapped_events_;

        static std::deque<std::pair<Subscriber*, std::shared_ptr<void>>>*  thread_queue_;
        static std::deque<std::pair<Subscriber*, std::shared_ptr<void>>>*  nonserial_queue_;

        std::deque<std::thread*>* processing_threads_; //using std::deque for constant time size() and O(1) random access

        static std::mutex dispatch_queue_mutex_;
        static std::mutex thread_queue_mutex_;
        static std::mutex mapped_event_mutex_;
        static std::condition_variable thread_signal_;


    public:
        static Dispatcher* GetInstance();

        ~Dispatcher();

        Dispatcher(const Dispatcher&); //disallow copying
        Dispatcher& operator= (const Dispatcher&); //disallow copying

        void Terminate();

        void AddEventSubscriber(Subscriber *requestor, const EventType);
        std::list<Subscriber*> GetAllSubscribers(const void* owner);

        void DispatchEvent(const EventType eventID, const std::shared_ptr<void> eventData);
        void DispatchImmediate(const EventType eventID, const std::shared_ptr<void> eventData);

        void Pump();
        void NonSerialProcess();

        int QueueSize() { return static_cast<int>(thread_queue_->size()); }

    private:
        static void ThreadProcess();
};

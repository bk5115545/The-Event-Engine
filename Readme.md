###The Event Engine###

##CAUTION##
This is a work in progress project.  All listed features are fully functional but may be subject to some edge-case bugs

##Uses:##
The Event Engine provides a basis for event-driven programming tightly coupled with a semi-automatic parallelization layer.

##Features:##
Event Driven  
Code Packs  
Semi-Automatic Parallelization  

#Event Driven#
Registering callbacks with The Event Engine is straightforward.
```cpp
Subscriber* sub = new Subscriber(this, Function_Cast(&Class::method)); // creates a multi-threaded Subscriber object
Dispatcher::GetInstance()->AddEventSubscriber(sub, "EVENT_STRING_HERE");
```

Deleting callbacks from the event registry is also straightforward.
Simply `delete sub;` and the Dispatcher will recognize that the Subscriber has been deleted.

To submit an event to The Event Engine
```cpp
Dispatcher::GetInstance()->DispatchEvent("EVENT_STRING_HERE", std::shared_ptr<void>(object)); // make shared_ptr to non-copyable object
// Dispatcher::GetInstance()->DispatchEvent("EVENT_STRING_HERE", std::make_shared<ObjectType>(object)); // all other objects
// Note that DispatchImmediate can be substituded for DispatchEvent if the event should be dispatched before the next call to Dispatcher::GetInstance()->Pump();
// But also that DispatchImmediate has significantly higher overhead
```

All methods that behave as event callbacks must take the form
```cpp
void method(std::shared_ptr<void>);
// or
void Class::method(std::shared_ptr<void>);
```

More documentation on how The Event Engine handles background tasks is in the works.

#Code Packs#
With it's drag-and-drop Code Packages, you can customize The Event Engine toward your specific goal.  
Code Packs register events with The Event Engine via Java-style static initialization.  This style has some caveats so it's highly recommended to closely follow the pattern in the base packs.  If you have doubts about this style, checkout some of the base packages.  

More documentation on Code Packs is available in the packs/ folder (most of it has yet to be written).
In order to keep Code Packs modular and portable, Code Packs must use the cmake build system.  The default CMakeLists in packs/ shows how to include a specific code pack in your build environment (WIP).

#Semi-Automatic Parallelization#
From the event documentation above
```cpp
Subscriber* sub = new Subscriber(this, Function_Cast(&Class::method), false); // creates an explicitly single-threaded Subscriber object
```
and
```cpp
Subscriber* sub = new Subscriber(this, Function_Cast(&Class::method)); // creates a multi-threaded Subscriber object
```
If you opt to use the multi-threaded Subscriber object, take care when designing callbacks as they may be called multiple times concurrently with different arguments.  Shorter callbacks will make this practice more scalable.

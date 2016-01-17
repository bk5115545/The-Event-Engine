To make a code pack for this engine each of the following requirements must be met:

1. Cmake must be used to build your pack
2. Your pack must have a CMakeLists.txt in its root folder that describes how to build and link your pack
    -- Include all your source files in the list ${PACK_SOURCE} by appending them by using
        LIST(APPEND PACK_SOURCE "${YourSourceFileList}")
    -- Include all necessary libs in the list ${APP_PACK_LIBS} by appending them using
        LIST(APPEND APP_PACK_LIBS "${YourRequiredLibs}")
    -- Include all header locations (include directories) in the list ${APP_PACK_INCLUDES} by appending them using
        LIST(APPEND APP_PACK_INCLUDES "${YourIncludeDirectories}")
3. Your pack must contain a static init block that registers Subscribers with Dispatcher
    -- Your code will not be automatically called in any other way than through Subscribers that are registered with Dispatcher
    -- Subscribers are called concurrently through Dispatcher.  Make sure to control non-threadsafe resources
4. Your pack probably wants to emit some kind of Events through Dispatcher
    -- Make sure to properly use std::shared_ptr<void> with a long enough object lifetime that the target exists when it event is processed


Useful EventID's:
EVENT_INPUT
EVENT_COMPONENT_UPDATE
EVENT_ACTOR_UPDATE

More useful base events can be seen in the BaseLoader, BaseApp, and BaseRender default packs.

You are free to dispatch your own events and respond to all EventID's.

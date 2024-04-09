#pragma once

/// The entry point of users C++ firmware. This comes after CubeHAL and FreeRTOS initialization.
/// All needed classes and objects have the root here.
class Application
{
public:
    Application();
    [[noreturn]] void run();

    static Application &getApplicationInstance();

private:
    static inline Application *instance{nullptr};
};

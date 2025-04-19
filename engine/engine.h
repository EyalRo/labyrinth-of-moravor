#pragma once
// Engine entry point and core loop
template<typename T> void unused(T const&) {}
namespace engine {
    void initialize();
    void shutdown();
}

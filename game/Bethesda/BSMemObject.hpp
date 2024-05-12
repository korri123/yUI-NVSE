#pragma once

#include "MemoryManager.hpp"

class BSMemObject {
public:
    static void* operator new(size_t stSize);
    static void* operator new[](size_t stSize);
    static void  operator delete(void* pvMem, size_t stElementSize);
    static void  operator delete[](void* pvMem, size_t stElementSize);
    static void* operator new(size_t stSize, void* p) { return p; }
    static void* operator new[](size_t stSize, void* p) { return p; }
    static void  operator delete(void*, void*) {}
    static void  operator delete[](void*, void*) {}
};
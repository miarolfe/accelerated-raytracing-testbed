// Copyright Mia Rolfe. All rights reserved.
#pragma once

#include <cstdint>
#include <new>
#include <utility>

namespace ART
{

// Super simple bump allocator
class ArenaAllocator
{
public:
    ArenaAllocator(std::size_t capacity_in_bytes);
    ~ArenaAllocator();

    void* Alloc(std::size_t size_in_bytes, std::size_t alignment_in_bytes = 16);
    void Clear();

    template<typename T, typename... Args>
    T* Create(Args&& ... args);

protected:
    uint8_t* m_buffer;
    std::size_t m_capacity;
    std::size_t m_offset;
};

// To be generic for all types, this needs to be in header :(
template<typename T, typename... Args>
T* ArenaAllocator::Create(Args&& ... args)
{
    void* allocation = Alloc(sizeof(T), alignof(T));
    if (!allocation)
    {
        return nullptr;
    }
    return new (allocation) T(std::forward<Args>(args)...);
}

} // namespace ART

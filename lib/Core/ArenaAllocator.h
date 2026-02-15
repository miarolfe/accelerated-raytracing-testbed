// Copyright Mia Rolfe. All rights reserved.
#pragma once

#include <cstdint>
#include <new>
#include <utility>

#include <Core/Logger.h>

namespace ART
{

// Super simple bump allocator
class ArenaAllocator
{
public:
    // Capacity rounds up to nearest multiple of 64 bytes for aligned_alloc
    ArenaAllocator(std::size_t capacity_in_bytes);
    ~ArenaAllocator();

    // Can't be copied
    ArenaAllocator(const ArenaAllocator&) = delete;
    ArenaAllocator& operator=(const ArenaAllocator&) = delete;

    // Can be moved
    ArenaAllocator(ArenaAllocator&& other) noexcept;
    ArenaAllocator& operator=(ArenaAllocator&& other) noexcept;

    // Alignment must be a power of 2
    void* Alloc(std::size_t size_in_bytes, std::size_t alignment_in_bytes = 16);
    void Clear();

    std::size_t MemoryUsedBytes() const;

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
        Logger::Get().LogFatal("Failed to allocate " + std::to_string(sizeof(T)) + " bytes");
        return nullptr;
    }
    return new (allocation) T(std::forward<Args>(args)...);
}

} // namespace ART

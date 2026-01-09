// Copyright Mia Rolfe. All rights reserved.
#include <ArenaAllocator.h>

#include <cstdlib>

namespace ART
{

ArenaAllocator::ArenaAllocator(std::size_t capacity_in_bytes)
    : m_buffer(nullptr), m_capacity(capacity_in_bytes), m_offset(0)
{
    m_buffer = static_cast<uint8_t*>(malloc(capacity_in_bytes));
}

ArenaAllocator::~ArenaAllocator()
{
    free(m_buffer);
}

void* ArenaAllocator::Alloc(std::size_t size_in_bytes, std::size_t alignment_in_bytes)
{
    // Round offset to next multiple of alignment
    std::size_t aligned_offset = (m_offset + alignment_in_bytes - 1) & ~(alignment_in_bytes - 1);

    // Can't allocate if the allocation would use more than remaining capacity
    if (aligned_offset + size_in_bytes > m_capacity)
    {
        return nullptr;
    }

    void* ptr = m_buffer + aligned_offset;
    m_offset = aligned_offset + size_in_bytes;
    return ptr;
}

void ArenaAllocator::Clear()
{
    m_offset = 0;
}

} // namespace ART

// Copyright Mia Rolfe. All rights reserved.
#include <ArenaAllocator.h>

#include <Common.h>

namespace ART
{

ArenaAllocator::ArenaAllocator(std::size_t capacity_in_bytes)
    : m_buffer(nullptr), m_capacity(capacity_in_bytes), m_offset(0)
{
    std::size_t aligned_capacity = (capacity_in_bytes + 63) & ~std::size_t(63);
#if defined (_MSC_VER)
    m_buffer = static_cast<uint8_t*>(_aligned_malloc(aligned_capacity, 64));
#else
    m_buffer = static_cast<uint8_t*>(aligned_alloc(64, aligned_capacity));
#endif // defined (_MSC_VER)
}

ArenaAllocator::~ArenaAllocator()
{
#if defined (_MSC_VER)
    _aligned_free(m_buffer);
#else
    free(m_buffer);
#endif // defined (_MSC_VER)
}

void* ArenaAllocator::Alloc(std::size_t size_in_bytes, std::size_t alignment_in_bytes)
{
    // Round offset to next multiple of alignment
    std::size_t aligned_offset = (m_offset + alignment_in_bytes - 1) & ~(alignment_in_bytes - 1);

    // Can't allocate if the allocation would use more than remaining capacity
    if (aligned_offset + size_in_bytes > m_capacity)
    {
        Logger::Get().LogFatal("Failed to allocate " + std::to_string(size_in_bytes) + " bytes");
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
